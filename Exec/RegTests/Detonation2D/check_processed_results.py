import os
import pandas as pd

SCRATCH_BASE = "/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs"
results_csv = os.path.join(SCRATCH_BASE, "lhs_results.csv")

logged_keys = set()
if os.path.exists(results_csv):
    df = pd.read_csv(results_csv)
    for _, row in df.iterrows():
        key = tuple(round(row[c], 8) for c in ["geo.Z", "geo.X", "geo.H", "geo.W"])
        logged_keys.add(key)

unlogged_dirs = []
for d in os.listdir(SCRATCH_BASE):
    if not d.startswith("run_Z"):
        continue
    parts = d.split("_")
    try:
        geo_vals = [float(p[1:].replace("p", ".")) for p in parts[1:]]
        key = tuple(round(v, 8) for v in geo_vals)
        if key not in logged_keys:
            unlogged_dirs.append(d)
    except Exception as e:
        print(f"Could not parse {d}: {e}")

print(f"🔍 Unlogged run folders found: {len(unlogged_dirs)}")
for d in unlogged_dirs:
    print(" -", d)
