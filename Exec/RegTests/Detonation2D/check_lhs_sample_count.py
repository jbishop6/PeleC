import pandas as pd
import sys
import os

# Set the path to your results file
lhs_results = "/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs/lhs_results.csv"

# Threshold
REQUIRED_SAMPLES = 5

if not os.path.exists(lhs_results):
    print("[CHECK] Results file not found. Need to generate samples.")
    sys.exit(1)

try:
    df = pd.read_csv(lhs_results)

    # Filter out rows with invalid or zero thrust
    valid = df[df["thrust_max"] > 0.0]
    count = len(valid)

    print(f"[CHECK] Found {count} usable LHS samples")

    if count < REQUIRED_SAMPLES:
        print("[CHECK] Not enough valid samples — need more.")
        sys.exit(1)
    else:
        print("[CHECK] Sufficient valid samples available.")
        sys.exit(0)

except Exception as e:
    print(f"[CHECK] Error reading or processing CSV: {e}")
    sys.exit(1)
