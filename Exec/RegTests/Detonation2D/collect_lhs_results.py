import os
import csv
import re
from optimize_geometry import run_pelec_and_extract_thrust

SCRATCH_BASE = "/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs"
RESULTS_FILE = os.path.join(SCRATCH_BASE, "lhs_results.csv")

run_dirs = [
    d for d in os.listdir(SCRATCH_BASE)
    if d.startswith("run_Z") and os.path.isdir(os.path.join(SCRATCH_BASE, d))
]

# Load already-processed folders
processed = set()
if os.path.exists(RESULTS_FILE):
    with open(RESULTS_FILE) as f:
        reader = csv.DictReader(f)
        for row in reader:
            processed.add(f"run_Z{row['geo.Z']}_X{row['geo.X']}_H{row['geo.H']}_W{row['geo.W']}".replace('.', 'p'))

with open(RESULTS_FILE, "a", newline="") as f:
    writer = csv.writer(f)
    # Write header if file was empty
    if os.stat(RESULTS_FILE).st_size == 0:
        writer.writerow(["geo.Z", "geo.X", "geo.H", "geo.W", "thrust_avg", "thrust_std", "thrust_max", "thrust_min"])

    for run_dir in run_dirs:
        if run_dir in processed:
            continue

        match = re.match(r"run_Z(.*?)_X(.*?)_H(.*?)_W(.*)", run_dir)
        if not match:
            print(f"[SKIP] Folder name not in expected format: {run_dir}")
            continue

        geo_Z, geo_X, geo_H, geo_W = [s.replace('p', '.') for s in match.groups()]
        geo_vals = list(map(float, [geo_Z, geo_X, geo_H, geo_W]))

        print(f"[INFO] Processing {run_dir}...")
        try:
            thrust_stats = run_pelec_and_extract_thrust(
                *geo_vals,
                input_file="inputs.detonation.threebranch.inp",
                executable="./PeleC2d.gnu.ex",  # Path here doesn't matter, it won't rerun
                iteration=None,
                run_directory=os.path.join(SCRATCH_BASE, run_dir)
            )

            writer.writerow([
                geo_Z, geo_X, geo_H, geo_W,
                thrust_stats['thrust_avg'],
                thrust_stats['thrust_std'],
                thrust_stats['thrust_max'],
                thrust_stats['thrust_min']
            ])
            print(f"[✅] Added results for {run_dir}")
        except Exception as e:
            print(f"[ERROR] Failed to extract from {run_dir}: {e}")
