import numpy as np
import sys
from optimize_geometry import run_pelec_and_extract_thrust

if __name__ == "__main__":
    sample_file = sys.argv[1]
    sample = np.loadtxt(sample_file)
    geo_Z, geo_X, geo_H, geo_W = sample

    run_pelec_and_extract_thrust(
        geo_Z, geo_X, geo_H, geo_W,
        "inputs.detonation.threebranch.inp",
        "./PeleC2d.gnu.ex"
    )
