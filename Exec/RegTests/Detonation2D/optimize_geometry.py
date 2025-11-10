import os
import re
import matplotlib.pyplot as plt
import numpy as np
import csv
import subprocess
import yt
from glob import glob

# Setup paths
INP_FILE = "inputs.detonation.threebranch.inp"
SIM_EXECUTABLE = "./PeleC2d.gnu.ex"
RESULTS_LOG = "results_log.csv"
PLOT_PREFIX = "thrust_plot"
PLOTFILE_GLOB = "plt*"

# Parameters to optimize
geo_Z = 0.1
geo_X = 0.4
geo_H = 0.1

# Parameters to test (manually for now)
def modifiy_geometry_params(path, Z, X, H):
  with open(path, 'r') as f:
    lines = f.readlines()

  updated_lines = []
  for line in lines:
    if line.strip().startswith("geo.Z"):
      updated_lines.append(f"geo.Z = {Z}\n")
    elif line.strip().startswith("geo.X"):
      updated_lines.append(f"geo.X = {X}\n")
    elif line.strip().startswith("geo.H"):
      updated_lines.append(f"geo.H = {H}\n")
    else:
      updated_lines.append(line)

  with open(path, 'w') as f:
    f.writelines(updated_lines)

  print(f"[INFO] Updated geometry: Z={Z}, X={X}, H={H}")

# Run simulation
def run_simulation(executable, inp_file):
  print(f"[INFO] Running: {executable} {inp_file}")
  result = subprocess.run([executable, inp_file], capture_output=True text=True)

  if result.returncode != 0:
    print("[ERROR Simulation failed:")
    print(result.stderr)
    raise RuntimeError("Simulation run failed")

  print("[INFO] Simulation completed successfully")

# Get plotfile path
  def get_plotfile_path(base_dir):
    plot_dirs = sorted(glob(os.path.join(base_dir, "plt*"))
    if not plot_dirs:
      raise RuntimeError(f"No plotfiles found in {base_dir}.")
    return plot_dirs[-1]

# Extract thrust from plotfile using yt

def extract_thrust_from_plotfile(plotfile_dir, outlet_x=1.0, tolerance=1e-4):
  ds = yt.load(plotfile_dir)
  ad = ds.all_data()

  x = ad["x]
  rho = ad["density"]
  vx = ad["x_velocity"]

  mask = np.abs(x - outlet_x) < tolerance
  if not np.any(mask)
    raise RunTimeError("No cells found near outlet_x")

  rho_out = rho[mask]
  vx_out = vx[mask]
  dy = float(ds.domain_width[1] / ds.domain_dimensions[1])

  thrust = np.sum(rho_out * vx_out**2) * dy
  print(f"[INFO] Computed thrust from {plotfile_dir}: {thrust:.3e}")
  return thrust

# Log results to CSV
def log_results(Z, X, H, thrust):
  file_exists = os.path.isfile(RESULTS_LOG)
  with open(RESULTS_LOG, 'a', newline=' ') as csvfile:
    writer = csv.writer(csvfile)
    if not file_exists:
      writer.writerow(["geo.Z", "geo.X", "geo.H", "max_thrust"])
    writer.writerow([Z, X, H, thrust])
  print(f" [INFO] Logged results to {RESULTS_LOG}")


# Main Workflow
if __name__ == "__main__":
  # Create run-specific output directory
  run_id = f"Z{geo_Z}_X{geo_X}_H{geo_H}".replace('.', 'p')
  output_dir = os.path.join("outputs", f"run_{run_id}")
  os.makedirs(output_dir, exist_ok=True)

  # Modify input with geometry + plotfile path
  modify_geometry_params(INP_FILE, geo_Z, geo_X, geo_H, output_dir)

  # Run the simulation
  run_simulation(SIM_EXECUTABLE, INP_FILE)

  # Extract thrust from generated plotfile
  plotfile_dir = get_plotfile_path(output_dir)
  max_thrust = extract_thrust_from_plotfile(plotfile_dir)

  # Log results
  log_results(geo_Z, geo_X, geo_H, max_thrust)
  
  
