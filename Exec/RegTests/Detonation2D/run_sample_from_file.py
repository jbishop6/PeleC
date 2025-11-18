import sys
import numpy as np
from optimize_geometry import run_pelec_and_extract_thrust

sample_path = sys.argv[1]
Z, X, H, W = np.loadtxt(sample_path)

INP_FILE = "inputs.detonation.threebranch.inp"
EXECUTABLE = "./PeleC2d.gnu.ex"

run_pelec_and_extract_thrust(Z, X, H, W, INP_FILE, EXECUTABLE)
