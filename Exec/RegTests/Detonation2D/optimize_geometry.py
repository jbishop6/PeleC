import os
os.makedirs("lhs_samples", exist_ok=True)
os.makedirs("logs", exist_ok=True)  # If needed
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
from glob import glob
from concurrent.futures import ProcessPoolExecutor
from unyt import cm
from scipy.stats.qmc import LatinHypercube
from sklearn.gaussian_process import GaussianProcessRegressor
from sklearn.gaussian_process.kernels import Matern
from scipy.stats import norm

print("[DEBUG] Python script started", file=sys.stderr)

# Setup paths
INP_FILE = "inputs.detonation.threebranch.inp"
SIM_EXECUTABLE = "./PeleC2d.gnu.ex"
RESULTS_LOG = "results_log.csv"

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
def analyze_thrust_timeseries(plotfiles, output_dir):
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
    create_thrust_plot(results, output_dir)
    
    return {
        'thrust_avg': thrust_avg,
        'thrust_std': thrust_std,
        'thrust_max': thrust_max,
        'thrust_min': thrust_min,
        'thrust_timeseries': results
        }


# === Create thrust evolution plot ===
def create_thrust_plot(results, output_dir):
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
    ax.fill_between(times, thrust_avg-thrust_std, thrust_avg+thrust_std,
        alpha=0.2, color='r', label=f'±1σ: {thrust_std:.2f} N')
    
    ax.set_xlabel('Time (μs)', fontsize=12, fontweight='bold')
    ax.set_ylabel('Thrust (N)', fontsize=12, fontweight='bold')
    ax.set_title(f'Thrust Evolution (Z={geo_Z}, X={geo_X}, H={geo_H})', 
                fontsize=14, fontweight='bold')
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
    output_dir = os.path.join("outputs", f"run_{run_id}")
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

# === MAIN WORKFLOW ===

if __name__ == "__main__":
    # Parameters to optimize
    # geo_Z = 0.1
    # geo_X = 0.4
    # geo_H = 0.1

    # Creating initial samples for Bayesian Optimization
    # In this case there are no prior data points and thus need to generate some
    # This will be done by using Latin Hypercube Sampling (so random guesses are made within bounds)

    # Setting bounds based on physical constraints in PeleC
    bounds = np.array([
        [0.05, 0.5],   # Z parameter (third branch length)
        [0.15, 0.6],   # X parameter (channel circumference)
        [0.1, 0.6],    # H parameter (separator height)
        [0.05, 0.25]   # W parameter (third branch thickness)
    ])

    init_samp_num = 5  # Initial number of samples

    # Generate valid LHS samples
    job_ids = []
    X_init = generate_valid_lhs_samples(bounds, n_samples=init_samp_num)

    Y_init = []
    # Submit each LHS sample as a separate SLURM job
    for i, x in enumerate(X_init):
        sample_file = f"lhs_samples/sample_{i}.txt"
        np.savetxt(sample_file, x)

        # Submit SLURM job using sbatch
        result = subprocess.run(["sbatch", "run_sample.sh", sample_file], capture_output=True, text=True)
        print(result.stdout.strip())

        # Extract job ID
        match = re.search(r'Submitted batch job (\d+)', result.stdout)
        if match:
            job_ids.append(match.group(1))

    # Wait for all LHS jobs to finish
    wait_for_jobs_to_finish(job_ids)

    # Read results from CSV
    Y_init = []
    with open("results_log.csv") as f:
        for line in f:
            if "thrust_max" in line:
                continue
            *_, thrust_max = line.strip().split(",")
            Y_init.append(float(thrust_max))

    if len(Y_init) == 0:
        raise RuntimeError("All initial simulations failed. Cannot train GP.")

    # Generate 1,000 different possible candidates for the system
    def generate_valid_candidates(bounds, n_candidates=1000):
        candidates = []
        while len(candidates) < n_candidates:
            x = np.random.uniform(bounds[:, 0], bounds[:, 1])
            if is_valid_input(x):
                candidates.append(x)
        return np.array(candidates)

    # Computing the expected improvement
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

    X_data = X_init.copy()
    Y_data = Y_init.copy()

    max_iters = 15  # Starting at 15 since computationally expensive
    tol = 1e-2  # For early stage of testing

    # Bayesian Optimization Loop
    for iteration in range(max_iters):
        print(f"\n-- Iteration {iteration + 1} --")

        # Step 1: Fit GP on current data
        kernel = Matern(nu=2.5)
        gp = GaussianProcessRegressor(
            kernel=kernel,
            alpha=1e-6,
            normalize_y=True
        )
        gp.fit(X_data, Y_data)

        # Step 2: Generate valid candidate geometries
        X_candidates = generate_valid_candidates(bounds, n_candidates=1000)

        # Step 3: Compute expected improvement with candidates
        Y_best = max(Y_data)
        ei = expected_improvement(X_candidates, gp, Y_best)

        # Step 4: Pick the best candidate based on expected improvement
        best_index = np.argmax(ei)
        x_next = X_candidates[best_index]

        # Step 5: Run PeleC simulation on the new candidate
        geo_Z, geo_X, geo_H, geo_W = x_next
        thrust_stats = run_pelec_and_extract_thrust(
            geo_Z, geo_X, geo_H, geo_W,
            INP_FILE, SIM_EXECUTABLE,
            iteration=iteration + 1
        )
        y_next = thrust_stats['thrust_max']

        # Step 6: Append new result to dataset
        X_data = np.vstack((X_data, x_next))
        Y_data.append(y_next)

        # Step 7: Check for convergence
        improvement = abs(y_next - Y_best)
        print(f"[INFO] Improvement: {improvement:.6f} N")

        if improvement < tol:
            print("[INFO] Optimization has converged.")
            break

    print("\n=== Optimization complete ===")
    print(f"Best thrust achieved: {max(Y_data):.3f} N")
    print(f"Number of simulations run: {len(Y_data)}")
