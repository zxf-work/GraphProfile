"""LLM client with two backends.

- "sdk": the official Anthropic Python SDK (used when ANTHROPIC_API_KEY or an `ant auth` profile exists).
- "cli": `claude -p` headless mode, which is the only authenticated path inside a Claude Code cloud session.

Both return the same Reply record so the rest of the prototype does not care which backend ran.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import time
from dataclasses import dataclass, field
from typing import Any, Optional

DEFAULT_MODEL = os.environ.get("T2R_MODEL", "claude-opus-5-5")


@dataclass
class Reply:
    text: str
    input_tokens: int = 0
    output_tokens: int = 0
    cache_read_tokens: int = 0
    cost_usd: float = 0.0
    seconds: float = 0.0
    backend: str = ""
    raw: Any = None


@dataclass
class Usage:
    calls: int = 0
    input_tokens: int = 0
    output_tokens: int = 0
    cost_usd: float = 0.0
    seconds: float = 0.0

    def add(self, r: Reply) -> None:
        self.calls += 1
        self.input_tokens += r.input_tokens + r.cache_read_tokens
        self.output_tokens += r.output_tokens
        self.cost_usd += r.cost_usd
        self.seconds += r.seconds


class LLM:
    def __init__(self, model: str = DEFAULT_MODEL, backend: Optional[str] = None, effort: str = "medium"):
        self.model = model
        self.effort = effort
        self.backend = backend or self._detect()
        self.usage = Usage()
        self._sdk = None
        if self.backend == "sdk":
            import anthropic  # type: ignore
            self._sdk = anthropic.Anthropic()

    @staticmethod
    def _detect() -> str:
        if os.environ.get("ANTHROPIC_API_KEY"):
            try:
                import anthropic  # noqa: F401
                return "sdk"
            except ImportError:
                pass
        return "cli"

    def chat(self, system: str, user: str, max_tokens: int = 8000) -> Reply:
        t0 = time.time()
        if self.backend == "sdk":
            r = self._chat_sdk(system, user, max_tokens)
        else:
            r = self._chat_cli(system, user)
        r.seconds = time.time() - t0
        r.backend = self.backend
        self.usage.add(r)
        return r

    # price table for the CLI path, which does not always report cost: $/MTok input, output
    _PRICES = {"claude-opus-5-5": (4.0, 20.0), "claude-sonnet-5-5": (2.0, 10.0), "claude-haiku-5-5": (0.10, 0.50)}

    def _chat_sdk(self, system: str, user: str, max_tokens: int) -> Reply:
        resp = self._sdk.messages.create(
            model=self.model, max_tokens=max_tokens, system=system,
            output_config={"effort": self.effort},
            messages=[{"role": "user", "content": user}],
        )
        text = "".join(b.text for b in resp.content if b.type == "text")
        u = resp.usage
        pin, pout = self._PRICES.get(self.model, (4.0, 20.0))
        cr = getattr(u, "cache_read_input_tokens", 0) or 0
        cost = (u.input_tokens * pin + cr * pin * 0.1 + u.output_tokens * pout) / 1e6
        return Reply(text=text, input_tokens=u.input_tokens, output_tokens=u.output_tokens, cache_read_tokens=cr, cost_usd=cost, raw=resp.model_dump() if hasattr(resp, "model_dump") else None)

    def _chat_cli(self, system: str, user: str) -> Reply:
        cmd = ["claude", "-p", "--output-format", "json", "--model", self.model, "--system-prompt", system,
               "--tools", "", "--max-turns", "1", "--no-session-persistence"]
        env = dict(os.environ)
        env.pop("CLAUDECODE", None)  # allow nesting
        for attempt in range(3):
            p = subprocess.run(cmd, input=user, capture_output=True, text=True, env=env, timeout=600)
            try:
                d = json.loads(p.stdout)
                break
            except json.JSONDecodeError:
                if attempt == 2:
                    raise RuntimeError(f"claude cli failed: rc={p.returncode} stderr={p.stderr[-500:]} stdout={p.stdout[-500:]}")
                time.sleep(2 * (attempt + 1))
        u = d.get("usage", {})
        return Reply(text=d.get("result", ""), input_tokens=u.get("input_tokens", 0) + u.get("cache_creation_input_tokens", 0),
                     output_tokens=u.get("output_tokens", 0), cache_read_tokens=u.get("cache_read_input_tokens", 0),
                     cost_usd=float(d.get("total_cost_usd", 0.0)), raw=d)


def parse_json(text: str) -> Any:
    """Extract the first JSON object from a model reply (tolerates ```json fences and prose)."""
    text = text.strip()
    m = re.search(r"```(?:json)?\s*(\{.*?\})\s*```", text, re.S)
    if m:
        text = m.group(1)
    else:
        i = text.find("{")
        if i >= 0:
            # find matching brace
            depth = 0
            for j in range(i, len(text)):
                if text[j] == "{":
                    depth += 1
                elif text[j] == "}":
                    depth -= 1
                    if depth == 0:
                        text = text[i:j + 1]
                        break
    return json.loads(text)
