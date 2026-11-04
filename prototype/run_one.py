"""Run one (system, question) pair and write the Result as JSON. Used by run_experiment.py for isolation."""
import argparse, json, os, sys
HERE = os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0, HERE)
from dataclasses import asdict
from t2r.llm import LLM
from t2r.systems import SYSTEMS

ap = argparse.ArgumentParser()
ap.add_argument("--system", required=True); ap.add_argument("--qid", required=True); ap.add_argument("--out", required=True)
ap.add_argument("--model", default=os.environ.get("T2R_MODEL", "claude-opus-5-5"))
a = ap.parse_args()
q = next(x for x in json.load(open(os.path.join(HERE, "bench", "questions.json"))) if x["id"] == a.qid)
llm = LLM(model=a.model)
res = SYSTEMS[a.system](a.qid, q["file"], q["dataset"], q["description"], q["question"], llm=llm)
d = asdict(res)
os.makedirs(os.path.dirname(a.out), exist_ok=True)
json.dump(d, open(a.out, "w"), indent=1, default=str)
print(json.dumps({"system": a.system, "qid": a.qid, "answer": d["answer"], "error": d["error"], "cost": d["cost_usd"]}, default=str)[:300])
