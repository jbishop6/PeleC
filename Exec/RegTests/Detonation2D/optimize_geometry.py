import os
import re
import matplotlib.pyplot as plt
import numpy as np
import csv
import subprocess
import yt
from glob import glob
import sys
from unyt import cm  # Import units to fix unyt UnitOperationError

print("[DEBUG] Python script started", file=sys.stderr)

# Setup paths
INP_FILE = "inputs.detonation.threebranch.inp"
SIM_EXECUTABLE = "./PeleC2d.gnu.ex"
RESULTS_LOG = "results_log.csv"

# Parameters to optimize
geo_Z = 0.1
geo_X = 0.4
geo_H = 0.1


# === Modify geometry and plotfile output path in .inp file ===
def modify_geometry_params(path, Z, X, H, plot_dir):
    with open(path, "r") as f:
        lines = f.readlines()

    updated_lines = []
    plot_line_found = False
    for line in lines:
        if line.strip().startswith("geo.Z"):
            updated_lines.append(f"geo.Z = {Z}\n")
        elif line.strip().startswith("geo.X"):
            updated_lines.append(f"geo.X = {X}\n")
        elif line.strip().startswith("geo.H"):
            updated_lines.append(f"geo.H = {H}\n")
        elif line.strip().startswith("amr.plot_file"):
            updated_lines.append(f"amr.plot_file = {plot_dir}/plt\n")
            plot_line_found = True
        else:
            updated_lines.append(line)

    # Append plot_file line if it wasn't found
    if not plot_line_found:
        updated_lines.append(f"\namr.plot_file = {plot_dir}/plt\n")

    with open(path, "w") as f:
        f.writelines(updated_lines)

    print(f"[INFO] Updated geometry: Z={Z}, X={X}, H={H}")
    print(f"[INFO] Plot file output set to: {plot_dir}/plt")


# === Run simulation ===
def run_simulation(executable, inp_file):
    print(f"[INFO] Running: {executable} {inp_file}")
    result = subprocess.run([executable, inp_file], capture_output=True, text=True)

    if result.returncode != 0:
        print("[ERROR] Simulation failed:")
        print(result.stderr)
        raise RuntimeError("Simulation run failed")

    print("[INFO] Simulation completed successfully")


# === Get plotfile directory (e.g., plt00000) ===
def get_plotfile_path(base_dir):
    plot_dirs = sorted(glob(os.path.join(base_dir, "plt*")))
    if not plot_dirs:
        raise RuntimeError(f"No plotfiles found in {base_dir}.")
    return plot_dirs[-1]


# === Extract thrust from plotfile ===
def extract_thrust_from_plotfile(plotfile_dir, tolerance=1e-4):
    ds = yt.load(plotfile_dir)
    ad = ds.all_data()

    x = ad["x"]
    rho = ad["density"]
    vx = ad["x_velocity"]

    # Convert to SI units
    x /= 100                      # cm → m
    rho = rho * (100**3) / 1000   # g/cm³ → kg/m³
    vx /= 100                     # cm/s → m/s

    # Auto-detect max x position and pick outlet just inside domain
    x_max_cm = x.max().to("cm")
    outlet_x_val = x_max_cm - 1e-4 * cm   # 0.0001 cm inside the boundary
    tolerance_val = 1e-4 * cm             # tolerance in cm

    print(f"[DEBUG] x range: {x.min().to('cm')} to {x_max_cm}")
    print(f"[DEBUG] Using outlet_x = {outlet_x_val} with tolerance = {tolerance_val}")

    # Mask and extract
    mask = np.abs(x - outlet_x_val) < tolerance_val
    if not np.any(mask):
        raise RuntimeError("No cells found near outlet_x")

    rho_out = rho[mask]
    vx_out = vx[mask]

    dy = float((ds.domain_width[1] / ds.domain_dimensions[1]).to("m"))  # cm → m

    thrust = np.sum(rho_out * vx_out**2) * dy
    print(f"[INFO] Computed thrust from {plotfile_dir}: {thrust:.3e} N")
    return thrust

# === Log results ===
def log_results(Z, X, H, thrust):
    file_exists = os.path.isfile(RESULTS_LOG)
    with open(RESULTS_LOG, "a", newline="") as csvfile:
        writer = csv.writer(csvfile)
        if not file_exists:
            writer.writerow(["geo.Z", "geo.X", "geo.H", "max_thrust"])
        writer.writerow([Z, X, H, thrust])
    print(f"[INFO] Logged results to {RESULTS_LOG}")


# === MAIN WORKFLOW ===
if __name__ == "__main__":
    run_id = f"Z{geo_Z}_X{geo_X}_H{geo_H}".replace(".", "p")
    output_dir = os.path.join("outputs", f"run_{run_id}")
    os.makedirs(output_dir, exist_ok=True)

    modify_geometry_params(INP_FILE, geo_Z, geo_X, geo_H, output_dir)
    run_simulation(SIM_EXECUTABLE, INP_FILE)
    plotfile_dir = get_plotfile_path(output_dir)
    max_thrust = extract_thrust_from_plotfile(plotfile_dir)
    log_results(geo_Z, geo_X, geo_H, max_thrust)
