import os
#SCRATCH_BASE = "/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs"
#os.makedirs("lhs_samples", exist_ok=True)
#os.makedirs("logs", exist_ok=True)  # If needed
import re
import matplotlib
matplotlib.use('Agg')  # Headless backend for HPC
import matplotlib.pyplot as plt
import numpy as np
import csv
import subprocess
import yt
import sys
import concurrent.futures
import uuid
import shutil
import tempfile
import time
import pandas as pd
from glob import glob
from concurrent.futures import ProcessPoolExecutor
from unyt import cm
from scipy.stats.qmc import LatinHypercube
from sklearn.gaussian_process import GaussianProcessRegressor
from sklearn.gaussian_process.kernels import Matern
from scipy.stats import norm

# Allow custom output directory via CLI
#if len(sys.argv) > 1:
#    SCRATCH_BASE = sys.argv[1]
#else:
  #  SCRATCH_BASE = "/mmfs1/scratch/jbishop6/PeleC/Exec/RegTests/Detonation2D/outputs"

# Ensure the base directory exists
#os.makedirs(SCRATCH_BASE, exist_ok=True)

#os.makedirs("lhs_samples", exist_ok=True)
#os.makedirs("logs", exist_ok=True)  # If needed

# Shared across all optimization runs
# Shared input and result files — read-only
SHARED_BASE = os.path.expanduser("~/PeleC/Exec/RegTests/Detonation2D/outputs_new")
LHS_INPUT_CSV = os.path.join(SHARED_BASE, "lhs_samples", "lhs_input.csv")
LHS_RESULTS_CSV = os.path.join(SHARED_BASE, "results_log.csv")   # <- This is important

# New job-specific scratch output dir
SCRATCH_BASE = os.path.expanduser("~/PeleC/Exec/RegTests/Detonation2D/outputs_new")
os.makedirs(SCRATCH_BASE, exist_ok=True)

#os.makedirs(os.path.join(output_base, "lhs_samples"), exist_ok=True)





print("[DEBUG] Python script started", file=sys.stderr)

# Setup paths
INP_FILE = "inputs.detonation.threebranch.inp"
SIM_EXECUTABLE = "/home/jbishop6/PeleC/Exec/RegTests/Detonation2D/PeleC2d.gnu.ex"
RESULTS_LOG = os.path.join(SCRATCH_BASE, "results_log.csv")

# Create empty results_log.csv with headers (if not already there)
if not os.path.exists(RESULTS_LOG):
    with open(RESULTS_LOG, "w") as f:
        f.write("iteration,geo.Z,geo.X,geo.H,geo.W,thrust_avg,thrust_std,thrust_max,thrust_min\n")

# Defining functions for LHS
# Ensuring they are a valid combination of inputs so PeleC doesn't crash
def is_valid_input(x):
    geo_X, geo_H, geo_Z, geo_W = x
    return (geo_X + geo_W <= 1.0) and (geo_H + geo_Z <= 2.0)
    
def generate_valid_lhs_samples(bounds, n_samples=5, max_attempts_per_sample=500):
    dim = bounds.shape[0]
    valid_samples = []

    while len(valid_samples) < n_samples:
        attempts = 0
        found_valid = False

        while attempts < max_attempts_per_sample:
            # Generate 1 LHS sample
            sampler = LatinHypercube(d=dim)
            lhs_unit = sampler.random(n=1)
            candidate = bounds[:, 0] + lhs_unit[0] * (bounds[:, 1] - bounds[:,0])

            if is_valid_input(candidate):
                valid_samples.append(candidate)
                found_valid = True
                break

            attempts += 1

    if not found_valid:
        raise RuntimeError(f"Failed to find valid sample after {max_attempts_per_sample} attempts.")
    return np.array(valid_samples)
                           
# === Modify geometry and plotfile output path in .inp file ===
def modify_geometry_params(path, Z, X, H, W, plot_dir):
    """
    Modify ONLY geometry parameters, preserving ALL other content
    Uses regex for maximum robustness
    """
    import re
    
    # Read entire file as a single string
    with open(path, "r") as f:
        content = f.read()
    
    # Store original for debugging
    original_lines = len(content.splitlines())
    
    # Check if file has required parameters
    if "max_step" not in content:
        raise RuntimeError(f"Original file {path} is missing max_step!")
    if "stop_time" not in content:
        raise RuntimeError(f"Original file {path} is missing stop_time!")
    
    # Replace parameters using regex (preserves everything else)
    content = re.sub(r'geo\.Z\s*=\s*[\d.eE+-]+', f'geo.Z = {Z}', content)
    content = re.sub(r'geo\.X\s*=\s*[\d.eE+-]+', f'geo.X = {X}', content)
    content = re.sub(r'geo\.H\s*=\s*[\d.eE+-]+', f'geo.H = {H}', content)
    content = re.sub(r'geo\.W\s*=\s*[\d.eE+-]+', f'geo.W = {W}', content)
    content = re.sub(r'amr\.plot_file\s*=\s*\S+', f'amr.plot_file = {plot_dir}/plt', content)
    
    # Write back
    with open(path, "w") as f:
        f.write(content)
    
    # Verify the modification worked
    modified_lines = len(content.splitlines())
    
    if modified_lines < 10:
        raise RuntimeError(f"Modified file only has {modified_lines} lines! Something went wrong.")
    
    if "max_step" not in content:
        raise RuntimeError(f"max_step missing after modification!")
    if "stop_time" not in content:
        raise RuntimeError(f"stop_time missing after modification!")
    
    print(f"[DEBUG] Modified {path}: {original_lines} -> {modified_lines} lines", file=sys.stderr)
    
    # Print first 10 lines for verification
    print(f"[DEBUG] First 10 lines of modified file:", file=sys.stderr)
    for i, line in enumerate(content.splitlines()[:10], 1):
        print(f"  {i}: {line}", file=sys.stderr)
            
# === Run simulation ===
def run_simulation(executable, inp_file):
    print(f"[INFO] Running: {executable} {inp_file}")
    result = subprocess.run([executable, inp_file], capture_output=True, text=True)
    if result.returncode != 0:
        print("[ERROR] Simulation failed:")
        print(result.stderr)
        raise RuntimeError("Simulation run failed")
    print("[INFO] Simulation completed successfully")


# === Get ALL plotfile directories ===
def get_all_plotfiles(base_dir):
    """Get all plotfile directories sorted by time"""
    plot_dirs = sorted(glob(os.path.join(base_dir, "plt*")))
    if not plot_dirs:
        raise RuntimeError(f"No plotfiles found in {base_dir}.")
    print(f"[INFO] Found {len(plot_dirs)} plotfiles")
    return plot_dirs


# === Extract thrust from single plotfile ===
def extract_thrust_from_plotfile(plotfile_dir, outlet_x=0.95, tolerance=0.10):
    """
    Extract thrust from simulation output (CGS units)
    Returns thrust in Newtons
    """
    ds = yt.load(plotfile_dir)
    ad = ds.all_data()
    
    # Get data in CGS units (cm, g, cm/s)
    x_cm = ad["x"].to("cm").v
    rho_cgs = ad["density"].to("g/cm**3").v
    vx_cgs = ad["x_velocity"].to("cm/s").v
    
    # Get cell size in y-direction (in cm)
    dy_cm = float((ds.domain_width[1] / ds.domain_dimensions[1]).to("cm"))
    
    # Domain info
    x_min = float(ds.domain_left_edge[0].to("cm"))
    x_max = float(ds.domain_right_edge[0].to("cm"))
    domain_length = x_max - x_min
    
    # Calculate outlet position in cm
    outlet_x_cm = x_min + outlet_x * domain_length
    tolerance_cm = tolerance * domain_length
    
    # Create mask
    mask = np.abs(x_cm - outlet_x_cm) < tolerance_cm
    n_cells = np.sum(mask)
    
    if n_cells == 0:
        raise RuntimeError(f"No cells found near outlet in {plotfile_dir}")
    
    # Extract outlet values (in CGS)
    rho_out = rho_cgs[mask]
    vx_out = vx_cgs[mask]
    
    # Compute thrust: F = Σ(ρu²) * dy
    # Units: (g/cm³) * (cm/s)² * cm = g·cm/s² = dyne
    thrust_dyne = float(np.sum(rho_out * vx_out**2) * dy_cm)
    
    # Convert dyne to Newton: 1 N = 10^5 dyne
    thrust_N = thrust_dyne / 1e5
    
    return thrust_N


# === Analyze thrust over all timesteps ===
def analyze_thrust_timeseries(plotfiles, output_dir, geo_Z=None, geo_X=None, geo_H=None, geo_W=None):
    """
    Analyze thrust from all plotfiles and compute statistics
    """
    print(f"\n{'='*60}")
    print("ANALYZING THRUST TIME SERIES")
    print(f"{'='*60}\n")
    
    results = []
    
    for i, pf in enumerate(plotfiles):
        try:
            ds = yt.load(pf)
            time_s = float(ds.current_time.to("s"))
            time_us = time_s * 1e6
            
            thrust_N = extract_thrust_from_plotfile(pf)
            
            results.append({
                'plotfile': os.path.basename(pf),
                'time_s': time_s,
                'time_us': time_us,
                'thrust_N': thrust_N
                })
            
            print(f"[{i+1:3d}/{len(plotfiles)}] t={time_us:7.2f} μs | F={thrust_N:8.3f} N")
            
        except Exception as e:
            print(f"[ERROR] Failed to process {pf}: {e}")
            continue
    
    if not results:
        raise RuntimeError("No thrust data extracted from any plotfile")
    
    # Calculate statistics
    thrust_values = [r['thrust_N'] for r in results]
    thrust_avg = np.mean(thrust_values)
    thrust_std = np.std(thrust_values)
    thrust_max = np.max(thrust_values)
    thrust_min = np.min(thrust_values)
    
    print(f"\n{'='*60}")
    print("THRUST STATISTICS")
    print(f"{'='*60}")
    print(f"Average thrust:  {thrust_avg:.3f} N")
    print(f"Std deviation:   {thrust_std:.3f} N")
    print(f"Maximum thrust:  {thrust_max:.3f} N")
    print(f"Minimum thrust:  {thrust_min:.3f} N")
    print(f"Coefficient of variation: {(thrust_std/thrust_avg)*100:.1f}%")
    print(f"{'='*60}\n")
    
    # Save detailed time series to CSV
    timeseries_csv = os.path.join(output_dir, "thrust_timeseries.csv")
    with open(timeseries_csv, 'w', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=results[0].keys())
        writer.writeheader()
        writer.writerows(results)
    print(f"[INFO] Time series saved to: {timeseries_csv}")
    
    # Save summary statistics
    summary_txt = os.path.join(output_dir, "thrust_summary.txt")
    with open(summary_txt, 'w') as f:
        f.write(f"THRUST ANALYSIS SUMMARY\n")
        f.write(f"{'='*40}\n\n")
        f.write(f"Geometry Parameters:\n")
        f.write(f"  Z = {geo_Z}\n")
        f.write(f"  X = {geo_X}\n")
        f.write(f"  H = {geo_H}\n")
        f.write(f"  W = {geo_W}\n\n")
        f.write(f"Thrust Statistics:\n")
        f.write(f"  Average:  {thrust_avg:.6f} N\n")
        f.write(f"  Std Dev:  {thrust_std:.6f} N\n")
        f.write(f"  Maximum:  {thrust_max:.6f} N\n")
        f.write(f"  Minimum:  {thrust_min:.6f} N\n")
        f.write(f"  CV:       {(thrust_std/thrust_avg)*100:.2f}%\n\n")
        f.write(f"Number of samples: {len(results)}\n")
    print(f"[INFO] Summary saved to: {summary_txt}")
    
    # Create thrust evolution plot
    create_thrust_plot(results, output_dir, geo_Z, geo_X, geo_H)
    
    return {
        'thrust_avg': thrust_avg,
        'thrust_std': thrust_std,
        'thrust_max': thrust_max,
        'thrust_min': thrust_min,
        'thrust_timeseries': results
        }


# === Create thrust evolution plot ===
def create_thrust_plot(results, output_dir, geo_Z=None, geo_X=None, geo_H=None):
    """
    Create plot showing thrust evolution over time
    """
    times = [r['time_us'] for r in results]
    thrust = [r['thrust_N'] for r in results]
    
    thrust_avg = np.mean(thrust)
    thrust_std = np.std(thrust)
    
    fig, ax = plt.subplots(figsize=(10, 6))
    
    ax.plot(times, thrust, 'b-', linewidth=2, marker='o', 
            markersize=4, label='Instantaneous Thrust')
    ax.axhline(thrust_avg, color='r', linestyle='--', 
               linewidth=2, label=f'Average: {thrust_avg:.2f} N')
    ax.fill_between(times, thrust_avg - thrust_std, thrust_avg + thrust_std,
                    alpha=0.2, color='r', label=f'±1σ: {thrust_std:.2f} N')
    
    ax.set_xlabel('Time (μs)', fontsize=12, fontweight='bold')
    ax.set_ylabel('Thrust (N)', fontsize=12, fontweight='bold')

    # Safe title formatting
    if geo_Z is not None and geo_X is not None and geo_H is not None:
        title = f'Thrust Evolution (Z={geo_Z:.3f}, X={geo_X:.3f}, H={geo_H:.3f})'
    else:
        title = "Thrust Evolution"
    
    ax.set_title(title, fontsize=14, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.legend(fontsize=10)
    
    plt.tight_layout()
    
    plot_path = os.path.join(output_dir, "thrust_evolution.png")
    plt.savefig(plot_path, dpi=150, bbox_inches='tight')
    plt.close()
    
    print(f"[INFO] Thrust plot saved to: {plot_path}")



# === Log results ===
def log_results(Z, X, H, W, thrust_stats, iteration=None):
    """
    Log results to CSV with both average and peak thrust
    """
    file_exists = os.path.isfile(RESULTS_LOG)
    with open(RESULTS_LOG, "a", newline="") as csvfile:
        writer = csv.writer(csvfile)
        if not file_exists:
            writer.writerow(["iteration","geo.Z", "geo.X", "geo.H", "geo.W",
                        "thrust_avg", "thrust_std", "thrust_max", "thrust_min"])
        writer.writerow([
            iteration if iteration is not None else "",       
            Z, X, H, W,
            thrust_stats['thrust_avg'],
            thrust_stats['thrust_std'],
            thrust_stats['thrust_max'],
            thrust_stats['thrust_min']])
    print(f"[INFO] Logged results to {RESULTS_LOG}")

# ===    RUN PELEC AND EXTRACT RESULTS == 
def run_pelec_and_extract_thrust(geo_Z, geo_X, geo_H, geo_W, INP_FILE, SIM_EXECUTABLE, iteration=None):
      # Create unique output directory for results
    run_id = f"Z{geo_Z}_X{geo_X}_H{geo_H}_W{geo_W}".replace(".", "p")
    output_dir = os.path.join(SCRATCH_BASE, f"run_{run_id}")
    output_dir = os.path.abspath(output_dir)
    print(f"[DEBUG] Absolute output path: {output_dir}", file=sys.stderr)
    os.makedirs(output_dir, exist_ok=True)

    print(f"\n{'='*60}")
    print(f"RDE OPTIMIZATION RUN: {run_id}")
    print(f"{'='*60}\n")

    # === CREATE TEMP WORKING DIRECTORY
    temp_work_dir = tempfile.mkdtemp(prefix="pelec_run_")

    # === COPY input file to a temp version
    unique_suffix = uuid.uuid4().hex[:8]
    temp_inp_file = os.path.join(temp_work_dir, f"temp_input_{unique_suffix}.inp")
    shutil.copy(INP_FILE, temp_inp_file)

    # === COPY executable into temp dir
    executable_name = os.path.basename(SIM_EXECUTABLE)
    temp_exec_path = os.path.join(temp_work_dir, executable_name)
    shutil.copy(SIM_EXECUTABLE, temp_exec_path)

    try:
        # Modify input file to update geometry
        modify_geometry_params(temp_inp_file, geo_Z, geo_X, geo_H, geo_W, output_dir)

        # Run simulation in the temp working directory
        subprocess.run([f"./{executable_name}", temp_inp_file], cwd=temp_work_dir, check=True)

        # Analyze the output results
        plotfiles = get_all_plotfiles(output_dir)
        thrust_stats = analyze_thrust_timeseries(plotfiles, output_dir)

        # Log the results
        log_results(geo_Z, geo_X, geo_H, geo_W, thrust_stats, iteration=iteration)

        print(f"\n{'='*60}")
        print("RUN COMPLETE")
        print(f"{'='*60}")
        print(f"Average thrust: {thrust_stats['thrust_avg']:.3f} N")
        print(f"Peak thrust:    {thrust_stats['thrust_max']:.3f} N")
        print(f"Results saved to: {output_dir}")
        print(f"{'='*60}\n")

        return thrust_stats

    finally:
        # Clean up: Remove temp working dir and its contents
        shutil.rmtree(temp_work_dir)

def wait_for_jobs_to_finish(job_ids, check_interval=30):
    """
    Wait until all given SLURM jobs are completed
    """
    while True:
        # Check job status using squeue
        result = subprocess.run(['squeue', '-u', os.getenv("USER")], stdout=subprocess.PIPE, text=True)
        running_jobs = result.stdout

        still_running = [job_id for job_id in job_ids if job_id in running_jobs]
        if not still_running:
            break

        print(f"[INFO] Waiting for {len(still_running)} jobs to finish...")
        time.sleep(check_interval)
# Evaluate PeleC for each
def run_single_lhs_sample(x):
    geo_Z, geo_X, geo_H, geo_W = x
    print(f"[DEBUG] Starting LHS simulation for Z={geo_Z}, X={geo_X}, H={geo_H}, W={geo_W}", file=sys.stderr)
    try:
        result = run_pelec_and_extract_thrust(geo_Z, geo_X, geo_H, geo_W, INP_FILE, SIM_EXECUTABLE)
        return result
    except Exception as e:
        print(f"[ERROR] Failed LHS run for {x} — Exception: {e}", file=sys.stderr)
        return None
        
def extract_thrust_from_existing_output(Z, X, H, W, run_directory):
    try:
        plotfiles = get_all_plotfiles(run_directory)
        thrust_stats = analyze_thrust_timeseries(plotfiles, run_directory)

        log_results(Z, X, H, W, thrust_stats, iteration=None)

        return thrust_stats
    except Exception as e:
        print(f"[ERROR] Failed to extract thrust from {run_directory}: {e}")
        return None


# === MAIN WORKFLOW ===
    # Parameters to optimize
    # geo_Z = 0.1
    # geo_X = 0.4
    # geo_H = 0.1

    # Creating initial samples for Bayesian Optimization
    # In this case there are no prior data points and thus need to generate some
    # This will be done by using Latin Hypercube Sampling (so random guesses are made within bounds)

   # === MAIN WORKFLOW ===
def main():
    lhs_input_csv = LHS_INPUT_CSV
    lhs_results_csv = LHS_RESULTS_CSV

    # Step 0: Create lhs_input.csv if it doesn't exist
    if not os.path.exists(lhs_input_csv):
        print("[INFO] lhs_input.csv not found — generating LHS samples...")
        subprocess.run(["python", "run_lhs_sampling.py"], check=True)

    # Step 1: Parse how many input samples exist
    with open(lhs_input_csv) as f:
        input_lines = [line.strip() for line in f if line.strip()]
        n_input_samples = len(input_lines)

    print(f"[INFO] Found {n_input_samples - 1} samples in lhs_input.csv")

    # Step 2: Check how many results have been processed
    existing_results = set()
    if os.path.exists(lhs_results_csv):
        df_results = pd.read_csv(lhs_results_csv)
        for _, row in df_results.iterrows():
            key = tuple(round(row[c], 8) for c in ["geo.Z", "geo.X", "geo.H", "geo.W"])
            existing_results.add(key)
    else:
        df_results = pd.DataFrame()

    # Step 3: Search output folders for runs not yet in lhs_results.csv
    #all_run_dirs = [
        #d for d in os.listdir(SCRATCH_BASE)
        #if d.startswith("run_Z") and os.path.isdir(os.path.join(SCRATCH_BASE, d))
    #]

    #new_results_added = 0
    #for run_dir in all_run_dirs:
        #match = re.search(r"Z([\d]+p[\d]+)_X([\d]+p[\d]+)_H([\d]+p[\d]+)_W([\d]+p[\d]+)", run_dir)
        #if not match:
           # continue
        #geo_vals = [float(s.replace("p", ".")) for s in match.groups()]
        #key = tuple(round(x, 8) for x in geo_vals)
        #if key in existing_results:
            #continue

        #full_path = os.path.join(SCRATCH_BASE, run_dir)
        #try:
            #thrust_stats = analyze_thrust_timeseries(get_all_plotfiles(full_path), full_path)
            #log_results(*geo_vals, thrust_stats)
            #new_results_added += 1
        #except Exception as e:
            #print(f"[WARN] Skipping {run_dir}: {e}")

    #total_results = len(existing_results) + new_results_added
    #print(f"[INFO] Total LHS results available: {total_results}")

    total_results = len(existing_results)

    if total_results < 5:
        print(f"[INFO] Not enough LHS results ({total_results}/5). Launching additional sampling...")
        subprocess.run(["python", "run_lhs_sampling.py"], check=True)
        subprocess.run(["sbatch", "run_sample.sh"], check=True)
        print("[INFO] LHS job array submitted. Please rerun this script after jobs complete.")
        return  #  Exit early

    #  Reload updated results file
    df = pd.read_csv(lhs_results_csv)

    #  Now use *all* available data
    X_init = df[["geo.Z", "geo.X", "geo.H", "geo.W"]].values
    Y_init = df["thrust_max"].values

    if len(Y_init) < 5:
        raise RuntimeError(" ERROR: Still less than 5 usable LHS samples. Aborting.")

    # === Freeze data here ===
    X_data = X_init.copy()
    Y_data = list(Y_init.copy())  # Ensure it's mutable

    # === Bayesian Optimization Loop ===
    def generate_valid_candidates(bounds, n_candidates=1000):
        candidates = []
        while len(candidates) < n_candidates:
            x = np.random.uniform(bounds[:, 0], bounds[:, 1])
            if is_valid_input(x):
                candidates.append(x)
        return np.array(candidates)

    def expected_improvement(X, gp, Y_best, xi=0.01):
        mu, sigma = gp.predict(X, return_std=True)
        sigma = sigma.reshape(-1, 1)
        mu = mu.reshape(-1, 1)
        with np.errstate(divide='warn'):
            imp = mu - Y_best - xi
            Z = imp / sigma
            ei = imp * norm.cdf(Z) + sigma * norm.pdf(Z)
            ei[sigma == 0.0] = 0.0
        return ei.ravel()

    max_iters = 15
    tol = 1e-2
    bounds = np.array([
        [0.08, 0.15],   # Z
        [0.30, 0.50],   # X
        [0.10, 0.20],   # H
        [0.08, 0.15]    # W
    ])

    try:
        for iteration in range(max_iters):
            print("\n" + "="*60)
            print(f"[ITERATION {iteration + 1}/{max_iters}] Starting Bayesian Optimization step")
            print("="*60)

            # Debug shapes
            print(f"[DEBUG] X_data shape: {X_data.shape}")
            print(f"[DEBUG] Y_data shape: {len(Y_data)}")

            # Print current best
            print(f"[INFO] Current best thrust: {max(Y_data):.4f} N")


            # Sanity check
            if np.isnan(X_data).any() or np.isnan(Y_data).any():
                raise ValueError("NaNs found in X_data or Y_data!")
            if X_data.shape[0] != len(Y_data):
                raise ValueError("Mismatch between number of samples in X and Y")

            kernel = Matern(nu=2.5)
            gp = GaussianProcessRegressor(kernel=kernel, alpha=1e-6, normalize_y=True)

            print("[DEBUG] Fitting GP model...")
            gp.fit(X_data, Y_data)
            print("[DEBUG] GP fit complete.")

            X_candidates = generate_valid_candidates(bounds, n_candidates=1000)
            Y_best = max(Y_data)
            ei = expected_improvement(X_candidates, gp, Y_best)

            best_index = np.argmax(ei)
            x_next = X_candidates[best_index]
            geo_Z, geo_X, geo_H, geo_W = x_next

            print(f"[INFO] Running BO sample: Z={geo_Z}, X={geo_X}, H={geo_H}, W={geo_W}")

            try:
                thrust_stats = run_pelec_and_extract_thrust(
                    geo_Z, geo_X, geo_H, geo_W,
                    INP_FILE, SIM_EXECUTABLE,
                    iteration=iteration + 1
                )
                y_next = thrust_stats['thrust_max']

                X_data = np.vstack((X_data, x_next))
                Y_data.append(y_next)

                improvement = abs(y_next - Y_best)
                print(f"[INFO] Improvement: {improvement:.6f} N")

                if improvement < tol:
                    print("[INFO] Optimization has converged.")
                    break
            except Exception as e:
                print(f"[WARN] BO sample failed: {e}")

    except Exception as fatal:
        print(f"[FATAL] Optimization crashed: {fatal}", file=sys.stderr)
        import traceback
        traceback.print_exc()

    print("\n=== Optimization complete ===")
    print(f"Best thrust achieved: {max(Y_data):.3f} N")
    print(f"Number of simulations run: {len(Y_data)}")


if __name__ == "__main__":
    main()
