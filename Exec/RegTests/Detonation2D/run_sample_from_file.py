import sys
import numpy as np
from optimize_geometry import run_pelec_and_extract_thrust

sample_file = sys.argv[1]
sample = np.loadtxt(sample_file)

geo_Z, geo_X, geo_H, geo_W = sample
run_pelec_and_extract_thrust(geo_Z, geo_X, geo_H, geo_W, "inputs.detonation.threebranch.inp", "./PeleC2d.gnu.ex")
