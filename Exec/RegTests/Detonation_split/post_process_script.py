import os
import re
import csv
from glob import glob

import numpy as np
import yt
import matplotlib.pyplot as plt

# === CONFIG: point this directly to the ONE run folder ===
RUN_DIR = "/Users/jenniferbishop/Documents/UCCS/Combustion_Research/Results/Comparison_TBvsTB3/run_3_31"

# Output summary CSV
OUTPUT_CSV = os.path.join(RUN_DIR, "thrust_summary.csv")

# Folder to store plots and time-history CSV
PLOT_DIR = os.path.join(RUN_DIR, "time_history_plots")
os.makedirs(PLOT_DIR, exist_ok=True)

H = 0.1880923
X = 0.3269623
P_REF_CGS = 1e6  # dyn/cm^2


def parse_geometry_from_rundir(rundir_name):
    """
    Parse branch type, Z, and W from a run directory name like:
      run_2B_L0.080_W0.040
    Returns (branch_type, Z, W) as floats, or (None, None, None) if parse fails.
    """
    if "2B" in rundir_name:
        branch_type = "2B"
    elif "3B" in rundir_name:
        branch_type = "3B"
    else:
        branch_type = None

    pattern = r"L(?P<Z>[0-9._]+)_W(?P<W>[^_]+)"
    m = re.search(pattern, rundir_name)
    if not m:
        return branch_type, None, None

    def de_p(s):
        return float(s.replace("p", "."))

    try:
        Z = de_p(m.group("Z"))
        W = de_p(m.group("W"))
        return branch_type, Z, W
    except Exception:
        return branch_type, None, None


def extract_thrust_and_pressure_from_plotfile(plotfile_dir: str, outlet_x=0.95):
    """
    Compute thrust and outlet pressure from a single plotfile.

    Returns
    -------
    time_s : float
        Dataset current time in seconds.
    thrust_N_per_cm : float
        Thrust per unit depth in N/cm.
    p_bar_out_kPa : float
        Area-averaged outlet pressure in kPa.
    deltaP_kPa : float
        Outlet pressure rise relative to P_REF_CGS in kPa.
    """
    ds = yt.load(plotfile_dir)
    ad = ds.all_data()

    x_coord = ad["x"].v
    y_coord = ad["y"].v
    rho = ad["density"].to("g/cm**3").v
    vx = ad["x_velocity"].to("cm/s").v
    p_cgs = ad["pressure"].v

    dx_cm = float((ds.domain_width[0] / ds.domain_dimensions[0]).to("cm"))
    dy_cm = float((ds.domain_width[1] / ds.domain_dimensions[1]).to("cm"))

    x_min = float(ds.domain_left_edge[0])
    x_max = float(ds.domain_right_edge[0])
    Lx = x_max - x_min

    outlet_x_coord = x_min + outlet_x * Lx
    tolerance_coord = 0.51 * (x_max - x_min) / ds.domain_dimensions[0]

    y_min = 0.905
    y_max = 1.095

    mask = (
        (np.abs(x_coord - outlet_x_coord) < tolerance_coord)
        & (y_coord >= y_min)
        & (y_coord <= y_max)
    )

    n_cells = int(np.sum(mask))
    if n_cells == 0:
        raise RuntimeError(
            f"No cells in outlet window near x={outlet_x_coord:.6f} "
            f"and y=[{y_min:.6f}, {y_max:.6f}] in {plotfile_dir}"
        )

    rho_out = rho[mask]
    vx_out = vx[mask]
    p_out = p_cgs[mask]

    p_bar_out_cgs = float(np.mean(p_out))
    p_bar_out_kPa = p_bar_out_cgs * 1e-4

    deltaP_cgs = p_bar_out_cgs - P_REF_CGS
    deltaP_kPa = deltaP_cgs * 1e-4

    integrand = rho_out * vx_out**2 + (p_out - P_REF_CGS)
    thrust_dyne_per_cm = float(np.sum(integrand) * dy_cm)
    thrust_N_per_cm = thrust_dyne_per_cm / 1e5

    time_s = float(ds.current_time.to("s"))

    if not np.isfinite(thrust_N_per_cm) or not np.isfinite(p_bar_out_kPa):
        raise RuntimeError(f"Non-finite values in {plotfile_dir}")

    return time_s, thrust_N_per_cm, p_bar_out_kPa, deltaP_kPa


def get_all_plotfiles(run_dir, max_plt=27000):
    """
    Return sorted list of plt* directories inside the run directory,
    keeping only plotfiles with index <= max_plt.
    """
    all_plot_dirs = sorted(glob(os.path.join(run_dir, "plt*")))

    filtered_plot_dirs = []
    for pf in all_plot_dirs:
        name = os.path.basename(pf)
        m = re.match(r"^plt(\d+)$", name)
        if not m:
            continue

        plt_num = int(m.group(1))
        if plt_num <= max_plt:
            filtered_plot_dirs.append(pf)

    if not filtered_plot_dirs:
        raise RuntimeError(
            f"No plotfiles found in {run_dir} with plot index <= {max_plt}"
        )

    return filtered_plot_dirs


def extract_plot_index(plotfile_dir):
    """
    Extract integer plotfile index from name like plt27000.
    """
    name = os.path.basename(plotfile_dir)
    m = re.match(r"^plt(\d+)$", name)
    return int(m.group(1)) if m else None


def analyze_run_time_history(run_dir, outlet_x=0.95):
    """
    For the given run directory:
      - loop over all plt* files
      - compute thrust and pressure metrics at each plotfile
      - return a list of rows for time-history plotting
    """
    plotfiles = get_all_plotfiles(run_dir)

    rows = []
    for pf in plotfiles:
        try:
            time_s, thrust_N_per_cm, p_bar_out_kPa, deltaP_kPa = extract_thrust_and_pressure_from_plotfile(
                pf, outlet_x=outlet_x
            )
            rows.append({
                "plotfile": os.path.basename(pf),
                "plot_index": extract_plot_index(pf),
                "time_s": time_s,
                "time_us": time_s * 1e6,
                "thrust_N_per_cm": thrust_N_per_cm,
                "thrust_N_per_m": thrust_N_per_cm * 100.0,
                "p_bar_out_kPa": p_bar_out_kPa,
                "deltaP_kPa": deltaP_kPa,
            })
        except Exception as e:
            print(f"[WARN] Skipping {pf}: {e}")
            continue

    if not rows:
        raise RuntimeError(f"No usable plotfiles found for {run_dir}")

    rows = sorted(rows, key=lambda r: r["time_s"])
    return rows


def summarize_time_history(rows):
    """
    Return summary metrics from time-history rows.
    """
    thrust_values = np.array([r["thrust_N_per_cm"] for r in rows])
    deltaP_values = np.array([r["deltaP_kPa"] for r in rows])

    thrust_max = float(np.max(thrust_values))
    thrust_avg = float(np.mean(thrust_values))
    thrust_std = float(np.std(thrust_values))
    time_avg_deltaP_kPa = float(np.mean(deltaP_values))
    n_samples = len(rows)

    return thrust_max, thrust_avg, thrust_std, time_avg_deltaP_kPa, n_samples


def save_time_history_csv(rows, run_name):
    out_csv = os.path.join(PLOT_DIR, f"{run_name}_time_history.csv")
    with open(out_csv, "w", newline="") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                "plotfile",
                "plot_index",
                "time_s",
                "time_us",
                "thrust_N_per_cm",
                "thrust_N_per_m",
                "p_bar_out_kPa",
                "deltaP_kPa",
            ],
        )
        writer.writeheader()
        writer.writerows(rows)
    return out_csv


def make_case_plots(rows, run_name):
    """
    Save thrust-vs-time and deltaP-vs-time plots.
    """
    time_us = np.array([r["time_us"] for r in rows])
    thrust_N_per_m = np.array([r["thrust_N_per_m"] for r in rows])
    deltaP_kPa = np.array([r["deltaP_kPa"] for r in rows])

    i_peak = int(np.argmax(thrust_N_per_m))
    t_peak = time_us[i_peak]

    plt.figure(figsize=(8, 5))
    plt.plot(time_us, thrust_N_per_m, 'o-', linewidth=2, markersize=6)
    plt.axvline(t_peak, linestyle='--', linewidth=1.5, label=f'Peak @ {t_peak:.3f} μs')
    plt.xlabel("Time [μs]")
    plt.ylabel("Thrust per unit depth [N/m]")
    plt.title(f"Thrust vs Time: {run_name}")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    thrust_path = os.path.join(PLOT_DIR, f"{run_name}_thrust_vs_time.png")
    plt.savefig(thrust_path, dpi=300)
    plt.close()

    plt.figure(figsize=(8, 5))
    plt.plot(time_us, deltaP_kPa, 'o-', linewidth=2, markersize=6)
    plt.axvline(t_peak, linestyle='--', linewidth=1.5, label=f'Peak thrust @ {t_peak:.3f} μs')
    plt.xlabel("Time [μs]")
    plt.ylabel(r"$\Delta P$ [kPa]")
    plt.title(f"Outlet Pressure Rise vs Time: {run_name}")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    dp_path = os.path.join(PLOT_DIR, f"{run_name}_deltaP_vs_time.png")
    plt.savefig(dp_path, dpi=300)
    plt.close()

    return thrust_path, dp_path


def main():
    if not os.path.isdir(RUN_DIR):
        print(f"[FATAL] Run directory does not exist: {RUN_DIR}")
        return

    run_name = os.path.basename(RUN_DIR)
    print(f"[INFO] Processing single run directory: {RUN_DIR}")
    print(f"[INFO] Output CSV will be: {OUTPUT_CSV}")
    print(f"[INFO] Time-history plots will be saved to: {PLOT_DIR}")

    branch_type, Z, W = parse_geometry_from_rundir(run_name)
    if Z is None:
        print(f"[WARN] Could not parse geometry from {run_name}, logging blanks.")

    with open(OUTPUT_CSV, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([
            "run_dir",
            "geo.Z", "geo.X", "geo.H", "geo.W",
            "thrust_max_N_per_cm",
            "thrust_avg_N_per_cm",
            "thrust_std_N_per_cm",
            "time_avg_deltaP_kPa",
            "num_samples",
            "time_history_csv",
            "thrust_plot",
            "deltaP_plot",
        ])

        try:
            rows = analyze_run_time_history(RUN_DIR, outlet_x=0.95)
            thrust_max, thrust_avg, thrust_std, time_avg_deltaP_kPa, n_samples = summarize_time_history(rows)
            time_csv = save_time_history_csv(rows, run_name)
            thrust_plot, deltaP_plot = make_case_plots(rows, run_name)
        except Exception as e:
            print(f"[ERROR] Failed to analyze {run_name}: {e}")
            writer.writerow([
                run_name,
                Z, X, H, W,
                "NaN", "NaN", "NaN", "NaN", 0,
                "NaN", "NaN", "NaN",
            ])
            return

        print(
            f"[RESULT] {run_name}: "
            f"max={thrust_max:.4f} N/cm, "
            f"avg={thrust_avg:.4f} N/cm, "
            f"std={thrust_std:.4f} N/cm, "
            f"time-avg delta_P={time_avg_deltaP_kPa:.4f} kPa, "
            f"samples={n_samples}"
        )

        writer.writerow([
            run_name,
            Z, X, H, W,
            thrust_max,
            thrust_avg,
            thrust_std,
            time_avg_deltaP_kPa,
            n_samples,
            time_csv,
            thrust_plot,
            deltaP_plot,
        ])

    print("\n[INFO] Done.")
    print(f"[INFO] Summary written to: {OUTPUT_CSV}")
    print(f"[INFO] Plots written to: {PLOT_DIR}")


if __name__ == "__main__":
    main()
