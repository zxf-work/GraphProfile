"""The systems compared in the headline experiment.

  full      : interpretation -> IR plan -> validate/optimise -> execute -> verify -> answer   (the proposal)
  no_verify : full without the verifier / re-plan / variants
  no_interp : full without the interpretation stage (raw file facts, strict node matching)
  flat      : conventional tool-calling agent over the same operators, no IR, no verification
  codegen   : LLM writes a NetworkX script against the file, executed in a subprocess

All systems receive the same user message: file name, optional dataset description, question.
All systems return the same Result record.
"""
from __future__ import annotations

import json
import math
import os
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass, field, asdict
from typing import Any, Optional

from .graphs import LoadedGraph, load_graph, profile_card, raw_card, read_raw
from .ir import Plan, validate, auto_tier, execute, extract_answer, summarise
from .llm import LLM, parse_json
from .ops import CATALOGUE, Ctx, catalogue_text
from .verify import verify


@dataclass
class Result:
    system: str
    qid: str
    answer: Any = None
    assumptions: dict[str, Any] = field(default_factory=dict)
    clarification: Optional[str] = None
    variants: list[dict[str, Any]] = field(default_factory=list)
    ops_used: list[str] = field(default_factory=list)
    plan: Any = None
    trace: Any = None
    findings: Any = None
    explanation: str = ""
    error: Optional[str] = None
    llm_calls: int = 0
    tokens_in: int = 0
    tokens_out: int = 0
    cost_usd: float = 0.0
    seconds: float = 0.0


ANSWER_SCHEMA = """
Final answers always use this JSON object:
{"answer": <number | string | boolean | list | object>,
 "assumptions": {"directed": bool, "weighted": bool, "use_weights": bool, "largest_component_only": bool, "measure": "<algorithm or metric used>", "<other>": ...},
 "clarification": null | "<a question to the user when the request is genuinely ambiguous and the answer would change materially>",
 "variants": [ {"label": "<e.g. by betweenness>", "answer": ...}, ... ]   // optional alternative interpretations you also computed
}
Rules: never estimate or guess a number; every number must come from a computation. If a node name cannot be resolved, say so in "clarification".
For rankings, "answer" is an ordered list of node ids. For a path, "answer" is {"length": .., "path": [...]}.
"""


# --------------------------------------------------------------------------- shared helpers
def user_message(file_name: str, description: Optional[str], question: str) -> str:
    d = f"Dataset description (from the user): {description}\n" if description else "Dataset description: none given.\n"
    return f"Graph file: {file_name}\n{d}Question: {question}"


def _finish(res: Result, llm: LLM, t0: float, start_usage: tuple[int, int, int, float]) -> Result:
    c0, i0, o0, u0 = start_usage
    res.llm_calls = llm.usage.calls - c0
    res.tokens_in = llm.usage.input_tokens - i0
    res.tokens_out = llm.usage.output_tokens - o0
    res.cost_usd = llm.usage.cost_usd - u0
    res.seconds = time.time() - t0
    return res


def _usage_mark(llm: LLM):
    u = llm.usage
    return (u.calls, u.input_tokens, u.output_tokens, u.cost_usd)


def _jsonable(x: Any) -> Any:
    try:
        json.dumps(x)
        return x
    except TypeError:
        return summarise(x, limit=100)


# =========================================================================== FULL (proposal)
PLANNER_SYSTEM = """You are the planner of a text-to-result graph analysis system. You never see the graph itself, only a profile card.
You translate the user's question into a typed plan over the operator catalogue below. A deterministic engine executes the plan.

OPERATOR CATALOGUE (types: Graph, NodeSet, NodeMap, Partition, Path, Scalar, Table, Bool; NodeRef = a node id or label):
{catalogue}

PLAN FORMAT (JSON only, no prose outside the JSON):
{{
 "interpretation": {{"directed": bool, "weighted": bool, "use_weights": bool, "largest_component_only": bool, "measure": "<metric chosen for vague words like 'important', 'central', 'community'>", "node_resolution": {{"<as written by user>": "<node id/label used>"}}}},
 "clarification": null | "<question to the user>",
 "variants": ["<label of an alternative measure also computed>", ...],
 "steps": [ {{"id": "s1", "op": "<op name>", "args": {{...}}, "note": "<why>"}}, ... ],
 "answer": {{"step": "<id of the step holding the answer>", "field": null | "<field of a Table/Partition>", "kind": "scalar|list|table|bool|ranking|path"}}
            OR for a multi-part answer: {{"steps": {{"<label>": "<step id or step.field>", ...}}, "kind": "table"}},
 "rationale": "<one or two sentences: which algorithm and why>"
}}

RULES
1. The graph is bound to the name "G". Steps may reference earlier step ids as arguments of type Graph/NodeMap/Partition/NodeSet.
2. Respect the profile card's semantics. If the card says needs_clarification for something the question depends on, and the description does not settle it, either ask (set "clarification") or state the assumption in "interpretation" and proceed if one reading is clearly more plausible.
3. Vague analytical words map to several measures ("most important/central/influential" -> degree, pagerank, betweenness, closeness, eigenvector; "community/group/cluster" -> communities). Pick the measure best suited to the edge semantics, state it in interpretation.measure, and when the question does not pin the measure down, add steps for one alternative measure and list it in "variants" so the user sees whether the choice matters. Ask a clarification only when no reasonable default exists.
4. Use the cost column: on large graphs prefer tier "approx" for O(nm) operators. The optimiser may also switch tiers.
5. Diameter and path questions on a disconnected graph: use largest_component_only=true and say so, unless the user asks otherwise.
6. Weighted graphs: use weights for shortest paths only when the weights are distances/costs. Weights that are counts or strengths must NOT be used as path lengths.
7. Directed graphs: distances and reachability follow edge direction unless the user says otherwise; in-degree and out-degree are different questions.
8. Node references: use the ids/labels exactly as in sample_node_ids style; the engine resolves case and close spellings and will reject unknown nodes.
9. The answer must be the output of a step. Never compute in your head. Yes/no questions need a Bool step (is_connected, is_reachable, is_finite, compare); a distance alone is not a yes/no answer.
10. Scalar arguments may reference a field of an earlier Table/Partition/Path step with "step.field" (e.g. "s1.modularity", "s2.value[9]" for the 10th row's value). A Table from top_k can be passed where a NodeSet is expected. Literal node sets are JSON lists.
11. When the question asks for several quantities (e.g. in-degree and out-degree), use the multi-part answer form so every value is reported.
"""

INTERPRETER_SYSTEM = """You write the final answer of a graph analysis system for the user. You receive the question, the executed plan with every step's result summary, the verification findings, and the structured answer extracted from the plan.
Write 2 to 5 sentences: the answer, the algorithm and interpretation used (directed/undirected, weights, largest component), any approximation and its error note, and any verification warning (e.g. two centrality measures disagree). Do not invent numbers; quote only values present in the results. Plain text, no JSON."""


def run_full(qid: str, lg_path: str, name: str, description: Optional[str], question: str, llm: LLM,
             interp: bool = True, verifier: bool = True, budget_ops: float = 2e9, cache: Optional[dict] = None) -> Result:
    t0 = time.time(); mark = _usage_mark(llm)
    res = Result(system="full" if (interp and verifier) else ("no_verify" if interp else "no_interp"), qid=qid)
    try:
        lg = load_graph(name, lg_path, description or "")
        if not interp:
            lg.strict = True
            card = raw_card(lg.facts)
            card["note"] = "No semantics inference available: decide directed/weighted yourself from the file facts and description."
            card["reciprocity_of_rows"] = round(lg.facts.reciprocity, 4)
            card["n_rows"] = lg.facts.n_rows
            # without an interpretation stage the graph is loaded with the file's literal shape: directed as listed
            lg = load_graph(name, lg_path, "", override={"directed": True if lg.facts.reciprocity < 0.98 else False})
            lg.strict = True
        else:
            card = profile_card(lg, with_inference=True)
        system = PLANNER_SYSTEM.format(catalogue=catalogue_text())
        user = f"PROFILE CARD:\n{json.dumps(card, default=str)}\n\n{user_message(os.path.basename(lg_path), description, question)}"
        reply = llm.chat(system, user)
        plan = Plan.model_validate(parse_json(reply.text))
        # the planner may override the interpretation (e.g. settle an 'ask' using the description);
        # without an interpretation stage this is the only way semantics get set
        if True:
            ov = {}
            for k in ("directed", "weighted"):
                v = plan.interpretation.get(k)
                if isinstance(v, bool) and v != getattr(lg.semantics, k):
                    if k == "weighted" and v and not lg.facts.weight_column_numeric:
                        continue
                    ov[k] = v
            if ov:
                strict = lg.strict
                lg = load_graph(name, lg_path, description or "", override=ov)
                lg.strict = strict
        problems = validate(plan, lg, budget_ops)
        if problems:
            fix = llm.chat(system, user + f"\n\nYour previous plan had these problems, return a corrected full plan:\n" + "\n".join(problems) + f"\n\nPrevious plan:\n{plan.model_dump_json()}")
            plan = Plan.model_validate(parse_json(fix.text))
            problems = validate(plan, lg, budget_ops)
            if problems:
                res.error = "invalid plan after repair: " + "; ".join(problems)
        notes = auto_tier(plan, lg, budget_ops)
        res.plan = plan.model_dump()
        res.assumptions = dict(plan.interpretation)
        res.clarification = plan.clarification
        res.ops_used = [s.op for s in plan.steps]
        if res.error:
            return _finish(res, llm, t0, mark)
        if plan.clarification and not plan.steps:
            res.answer = None
            return _finish(res, llm, t0, mark)
        ctx, trace = execute(plan, lg, cache=cache)
        res.trace = trace
        findings = verify(plan, ctx) if verifier else []
        bad = [f for f in findings if not f["ok"] and f["severity"] == "error"]
        if bad and verifier:
            fix = llm.chat(system, user + "\n\nThe plan executed but verification failed; return a corrected full plan:\n" +
                           "\n".join(f"{f['check']}: {f['detail']}" for f in bad) + f"\n\nPrevious plan:\n{plan.model_dump_json()}")
            plan2 = Plan.model_validate(parse_json(fix.text))
            if not validate(plan2, lg, budget_ops):
                auto_tier(plan2, lg, budget_ops)
                ctx, trace = execute(plan2, lg, cache=cache)
                plan, res.plan, res.trace = plan2, plan2.model_dump(), trace
                res.ops_used = [s.op for s in plan.steps]
                findings = verify(plan, ctx)
        res.findings = findings
        res.answer = _jsonable(extract_answer(plan, ctx))
        res.assumptions = dict(plan.interpretation)
        res.assumptions.setdefault("directed", lg.g.is_directed())
        res.assumptions["tier_notes"] = notes
        # variants: other measures computed in the plan (any top_k not the answer step)
        if plan.answer:
            for st in plan.steps:
                if st.id != plan.answer.step and st.op in ("top_k",) :
                    src = next((s for s in plan.steps if s.id == st.args.get("map")), None)
                    res.variants.append({"label": f"by {src.op if src else st.args.get('map')}", "answer": _jsonable(summarise(ctx.values[st.id], 100))})
        if verifier:
            disagreements = [f for f in findings if not f["ok"] and "disagree" in f["detail"]]
            if disagreements and not res.clarification:
                res.assumptions["verifier_note"] = "; ".join(f["detail"] for f in disagreements)
        # prose
        tr = json.dumps(trace, default=str)[:6000]
        prose = llm.chat(INTERPRETER_SYSTEM, f"Question: {question}\nPlan rationale: {plan.rationale}\nInterpretation: {json.dumps(res.assumptions, default=str)}\nTrace: {tr}\nFindings: {json.dumps(findings)[:3000]}\nExtracted answer: {json.dumps(res.answer, default=str)[:2000]}")
        res.explanation = prose.text.strip()
    except Exception as e:  # noqa: BLE001
        res.error = f"{type(e).__name__}: {e}"
    return _finish(res, llm, t0, mark)


# =========================================================================== FLAT tool calling
FLAT_SYSTEM = """You are a graph analysis agent. You answer questions about a graph file by calling tools, one call per message.
First call load_graph to build the graph (you must decide directed/weighted yourself; call file_facts first if unsure). Then call analysis tools. When you have the answer, return the final JSON.

TOOLS (call as {{"tool": "<name>", "args": {{...}}}}):
- file_facts() -> statistics about the raw file (rows, reciprocity of rows, weight column, sample rows)
- load_graph(directed: bool, weighted: bool) -> loads the graph as "G"
{catalogue}
Tool results come back as JSON. Arguments of type Graph/NodeMap/Partition/NodeSet refer to the id you gave with "save_as". Every tool call may include "save_as": "<id>" to name its result for later tools.

When done, reply with {{"final": <answer object>}} where the answer object is:
{schema}
Reply with exactly one JSON object per message: either a tool call or a final answer. Do not compute anything yourself; every number must come from a tool result.
"""


def run_flat(qid: str, lg_path: str, name: str, description: Optional[str], question: str, llm: LLM, max_turns: int = 12, cache: Optional[dict] = None) -> Result:
    t0 = time.time(); mark = _usage_mark(llm)
    res = Result(system="flat", qid=qid)
    system = FLAT_SYSTEM.format(catalogue=catalogue_text(), schema=ANSWER_SCHEMA)
    convo = user_message(os.path.basename(lg_path), description, question)
    facts, _ = read_raw(lg_path)
    lg: Optional[LoadedGraph] = None
    ctx: Optional[Ctx] = None
    try:
        for turn in range(max_turns):
            reply = llm.chat(system, convo)
            try:
                msg = parse_json(reply.text)
            except Exception:
                convo += f"\n\nASSISTANT: {reply.text}\nTOOL: error: reply was not a JSON object"
                continue
            if "final" in msg:
                fin = msg["final"] if isinstance(msg["final"], dict) else {"answer": msg["final"]}
                res.answer = fin.get("answer"); res.assumptions = fin.get("assumptions", {}) or {}
                res.clarification = fin.get("clarification"); res.variants = fin.get("variants", []) or []
                res.explanation = reply.text
                break
            tool = msg.get("tool"); args = msg.get("args", {}) or {}; save_as = msg.get("save_as")
            res.ops_used.append(tool)
            try:
                if tool == "file_facts":
                    out = {k: v for k, v in asdict(facts).items() if k not in ("sha1",)}
                elif tool == "load_graph":
                    lg = load_graph(name, lg_path, "", override={"directed": bool(args.get("directed", False)), "weighted": bool(args.get("weighted", False)) and facts.weight_column_numeric})
                    lg.strict = True
                    ctx = Ctx(lg)
                    out = {"loaded": True, "n": lg.g.vcount(), "m": lg.g.ecount(), "directed": lg.g.is_directed(), "weighted": "weight" in lg.g.es.attributes()}
                elif tool in CATALOGUE:
                    if ctx is None:
                        raise ValueError("call load_graph first")
                    val = CATALOGUE[tool].fn(ctx, **args)
                    if save_as:
                        ctx.values[save_as] = val
                    out = summarise(val, 25)
                else:
                    raise ValueError(f"unknown tool {tool}")
                out_s = json.dumps(out, default=str)
            except Exception as e:  # noqa: BLE001
                out_s = json.dumps({"error": f"{type(e).__name__}: {e}"})
            convo += f"\n\nASSISTANT: {json.dumps(msg)}\nTOOL: {out_s[:4000]}"
        else:
            res.error = "max turns reached without a final answer"
        if lg is not None:
            res.assumptions.setdefault("directed", lg.g.is_directed())
        res.trace = convo[-8000:]
    except Exception as e:  # noqa: BLE001
        res.error = f"{type(e).__name__}: {e}"
    return _finish(res, llm, t0, mark)


# =========================================================================== CODE GENERATION
CODEGEN_SYSTEM = """You are a graph data analyst. Answer the question by writing one complete Python 3 script that reads the graph file and prints exactly one line of JSON to stdout at the end.
Available libraries: networkx 3.x, igraph 1.0 (python-igraph), numpy, pandas. Prefer networkx for clarity; use igraph for expensive algorithms on large graphs.
The file path is given in the message. Decide directed/weighted from the file facts and the description. For large graphs, use approximations (e.g. networkx betweenness_centrality with k samples) rather than letting the script run for more than a few minutes.
The JSON line must be the answer object:
{schema}
Reply with only the script inside a ```python fence."""


def run_codegen(qid: str, lg_path: str, name: str, description: Optional[str], question: str, llm: LLM, timeout: int = 600, cache: Optional[dict] = None) -> Result:
    t0 = time.time(); mark = _usage_mark(llm)
    res = Result(system="codegen", qid=qid)
    facts, _ = read_raw(lg_path)
    fx = {k: v for k, v in asdict(facts).items() if k not in ("sha1",)}
    user = f"File path: {os.path.abspath(lg_path)}\nFile facts: {json.dumps(fx, default=str)}\n{user_message(os.path.basename(lg_path), description, question)}"
    system = CODEGEN_SYSTEM.format(schema=ANSWER_SCHEMA)
    try:
        reply = llm.chat(system, user)
        code = _extract_code(reply.text)
        out, err, rc = _run_script(code, timeout)
        if rc != 0 or not _last_json(out):
            fix = llm.chat(system, user + f"\n\nYour previous script failed (exit {rc}). stderr:\n{err[-3000:]}\nstdout:\n{out[-1500:]}\nReturn a corrected full script.")
            code = _extract_code(fix.text)
            out, err, rc = _run_script(code, timeout)
        res.plan = code
        res.trace = {"stdout": out[-3000:], "stderr": err[-3000:], "rc": rc}
        fin = _last_json(out)
        if fin is None:
            res.error = f"script produced no JSON answer (rc={rc}): {err[-500:]}"
        else:
            res.answer = fin.get("answer"); res.assumptions = fin.get("assumptions", {}) or {}
            res.clarification = fin.get("clarification"); res.variants = fin.get("variants", []) or []
        # weak plan signal: which library functions were called
        import re
        res.ops_used = sorted(set(re.findall(r"nx\.([a-zA-Z_]+)\(|\.([a-z_]+)\(", code) and [a or b for a, b in re.findall(r"nx\.([a-zA-Z_]+)\(|\.([a-z_]+)\(", code)]))
    except Exception as e:  # noqa: BLE001
        res.error = f"{type(e).__name__}: {e}"
    return _finish(res, llm, t0, mark)


def _extract_code(text: str) -> str:
    import re
    m = re.search(r"```python\s*(.*?)```", text, re.S) or re.search(r"```\s*(.*?)```", text, re.S)
    return m.group(1) if m else text


def _run_script(code: str, timeout: int) -> tuple[str, str, int]:
    with tempfile.TemporaryDirectory() as d:
        p = os.path.join(d, "script.py")
        with open(p, "w") as f:
            f.write(code)
        try:
            r = subprocess.run([sys.executable, "-I", p], capture_output=True, text=True, timeout=timeout, cwd=d)
            return r.stdout, r.stderr, r.returncode
        except subprocess.TimeoutExpired as e:
            return (e.stdout or b"").decode() if isinstance(e.stdout, bytes) else (e.stdout or ""), f"timeout after {timeout}s", 124


def _last_json(out: str) -> Optional[dict]:
    for line in reversed(out.strip().splitlines()):
        line = line.strip()
        if line.startswith("{"):
            try:
                return json.loads(line)
            except json.JSONDecodeError:
                continue
    return None


SYSTEMS = {
    "full": lambda *a, **k: run_full(*a, interp=True, verifier=True, **k),
    "no_verify": lambda *a, **k: run_full(*a, interp=True, verifier=False, **k),
    "no_interp": lambda *a, **k: run_full(*a, interp=False, verifier=True, **k),
    "flat": run_flat,
    "codegen": run_codegen,
}
