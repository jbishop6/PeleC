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
def extract_thrust_from_plotfile(plotfile_dir, outlet_x=0.99, tolerance=0.05):
    """
    Extract thrust from simulation output
    
    Args:
        plotfile_dir: path to plotfile directory
        outlet_x: x-location of outlet (fraction of domain, 0-1)
        tolerance: tolerance for finding outlet cells (fraction of domain)
    """
    import yt
    from unyt import m
    
    ds = yt.load(plotfile_dir)
    ad = ds.all_data()
    
    # Get data arrays (these come with unyt units from yt)
    x_raw = ad["x"]
    rho_raw = ad["density"]
    vx_raw = ad["x_velocity"]
    
    # Convert to SI and strip units completely using .v (value) attribute
    x_m = x_raw.to("m").v  # Convert to meters, then extract numpy array
    rho_kg = rho_raw.to("kg/m**3").v  # Convert to kg/m³, extract array
    vx_m = vx_raw.to("m/s").v  # Convert to m/s, extract array
    
    # Get cell size in y-direction
    dy = float((ds.domain_width[1] / ds.domain_dimensions[1]).to("m"))
    
    # Domain info (for debugging)
    x_min = float(ds.domain_left_edge[0].to("m"))
    x_max = float(ds.domain_right_edge[0].to("m"))
    domain_length = x_max - x_min
    
    print(f"[DEBUG] Domain x-range: [{x_min:.3f}, {x_max:.3f}] m")
    print(f"[DEBUG] Domain length: {domain_length:.3f} m")
    print(f"[DEBUG] Data x-range: [{x_m.min():.3f}, {x_m.max():.3f}] m")
    
    # Calculate outlet position in meters
    outlet_x_m = x_min + outlet_x * domain_length
    tolerance_m = tolerance * domain_length
    
    print(f"[DEBUG] Looking for outlet at x = {outlet_x_m:.3f} m ± {tolerance_m:.3f} m")
    
    # Create mask for cells near outlet
    mask = np.abs(x_m - outlet_x_m) < tolerance_m
    
    n_cells = np.sum(mask)
    print(f"[DEBUG] Found {n_cells} cells near outlet")
    
    if n_cells == 0:
        print(f"[WARNING] No cells found near outlet_x = {outlet_x}")
        print(f"[WARNING] Try increasing tolerance (current: {tolerance})")
        print(f"[WARNING] Or adjust outlet_x position (current: {outlet_x})")
        raise RuntimeError(f"No cells found near outlet_x={outlet_x} with tolerance={tolerance}")
    
    # Extract outlet values
    rho_out = rho_kg[mask]
    vx_out = vx_m[mask]
    
    # Compute thrust: F = ∫(ρu² + p) dA
    # Simplified: F ≈ Σ(ρu²) * dy (momentum flux)
    # Note: You may want to add pressure term if available
    thrust = float(np.sum(rho_out * vx_out**2) * dy)
    
    print(f"[INFO] Computed thrust: {thrust:.6e} N")
    print(f"[INFO] Mean outlet velocity: {np.mean(vx_out):.3f} m/s")
    print(f"[INFO] Mean outlet density: {np.mean(rho_out):.3f} kg/m³")
    
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
