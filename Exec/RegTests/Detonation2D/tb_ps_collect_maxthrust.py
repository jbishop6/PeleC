import os
import re
import csv
from glob import glob

import numpy as np
import yt

# === CONFIG: change this if your TB_PS path moves ===
TB_ROOT = "/Users/jenniferbishop/Documents/UCCS/Combustion_Research/Results/Comparison_TBvsTB3/run_2_26/TB_vs_TB3"

# Output CSV (Excel can open this directly)
OUTPUT_CSV = os.path.join(TB_ROOT, "TB_PS_thrust_max_summary.csv")

H = 0.1880923
X = 0.3269623
P_REF_CGS = 1e6 #dyn/cm^2

def parse_geometry_from_rundir(rundir_name):
    """
    Parse branch type Z and W from a run directory name (In this case L = Z in the name) like:
      run_2B_L0.080_W0.040
    Returns (branch_type, Z, W) as floats, or (None, None, None, None) if parse fails.
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
        # convert "0p08" -> "0.08"
        return float(s.replace("p", "."))

    try:
        Z = de_p(m.group("Z"))
        W = de_p(m.group("W"))
        return branch_type, Z, W
    except Exception:
        return branch_type, None, None

  


def get_all_plotfiles(run_dir):
    """
    Return sorted list of plt* directories inside a run directory.
    """
    plot_dirs = sorted(glob(os.path.join(run_dir, "plt*")))
    if not plot_dirs:
        raise RuntimeError(f"No plotfiles found in {run_dir}")
    return plot_dirs


def extract_thrust_from_plotfile(plotfile_dir, outlet_x=0.9):
    """
    Compute thrust from a single plotfile using a plane-like slice:

      - x is streamwise direction
      - integrate ρ u^2 over y at one x-plane
      - tolerance in x is ~ one cell (dx)
      - Result is N per unit depth (per cm) in the out-of-plane direction.
    """
    ds = yt.load(plotfile_dir)
    ad = ds.all_data()

    # Fields in CGS
    x_cm = ad["x"].to("cm").v
    rho = ad["density"].to("g/cm**3").v
    vx = ad["x_velocity"].to("cm/s").v
    p_cgs = ad["pressure"].v

    # Cell sizes
    dx_cm = float((ds.domain_width[0] / ds.domain_dimensions[0]).to("cm"))
    dy_cm = float((ds.domain_width[1] / ds.domain_dimensions[1]).to("cm"))

    # Domain info
    x_min = float(ds.domain_left_edge[0].to("cm"))
    x_max = float(ds.domain_right_edge[0].to("cm"))
    Lx = x_max - x_min

    # Desired plane location
    outlet_x_cm = x_min + outlet_x * Lx

    # Use ~1 column of cells in x
    tolerance_cm = 0.51 * dx_cm

    mask = np.abs(x_cm - outlet_x_cm) < tolerance_cm
    n_cells = int(np.sum(mask))
    if n_cells == 0:
        raise RuntimeError(f"No cells near x={outlet_x_cm:.3e} cm in {plotfile_dir}")

    rho_out = rho[mask]
    vx_out = vx[mask]
    p_out = p_cgs[mask]

    # Area-averaged outlet pressure (dyn/cm^2)
    p_bar_out = float(np.mean(p_out))

    
    # Thrust per unit depth:
    # F_2D = Σ(ρ u_x^2 + (p - P_ref) dy   [g/s^2] ~ dyne/cm (per depth)
    integrand = rho_out * vx_out**2 + (p_out - P_REF_CGS)
    thrust_dyne = float(np.sum(integrand) * dy_cm)

    # dyne -> N  (this is effectively N per cm depth)
    thrust_N = thrust_dyne / 1e5

    if not np.isfinite(thrust_N) or not np.isfinite(p_bar_out):
        print(f"[DEBUG] NaN detected in {plotfile_dir}: thrust_N={thrust_N}, p_bar_out={p_bar_out}")
    
    return thrust_N, p_bar_out


def analyze_run_max_thrust(run_dir, outlet_x=0.8):
    """
    For a given run directory:
      - loop over all plt* files
      - compute thrust for each
      - return max thrust over time, plus avg and std for reference
    """
    plotfiles = get_all_plotfiles(run_dir)

    thrust_values = []
    pout_values = []
    for pf in plotfiles:
        try:
            F, p_bar_out = extract_thrust_from_plotfile(pf, outlet_x=outlet_x)
            thrust_values.append(F)
            pout_values.append(p_bar_out)
        except Exception as e:
            print(f"[WARN] Skipping {pf}: {e}")
            continue

    if not thrust_values:
        raise RuntimeError(f"No thrust values computed for {run_dir}")

    thrust_values = np.array(thrust_values)
    thrust_max = float(np.max(thrust_values))
    thrust_avg = float(np.mean(thrust_values))
    thrust_std = float(np.std(thrust_values))

    # delta_P calculation:
    deltaP_cgs = np.mean(pout_values) - P_REF_CGS # dyn/cm^2
    deltaP_Pa = deltaP_cgs * 0.1 # Pa
    deltaP_kPa = deltaP_cgs * 1e-4 # kPa   

    print(f"[DEBUG] {run_dir}: thrust_max={thrust_max}, thrust_avg={thrust_avg}, thrust_std={thrust_std}, deltaP_kPa={deltaP_kPa}")

    return thrust_max, thrust_avg, thrust_std, deltaP_kPa, len(thrust_values)


def main():
    # Find all run_* directories in TB_ROOT
    run_dirs = sorted(
        d for d in glob(os.path.join(TB_ROOT, "run_*"))
        if os.path.isdir(d)
    )

    if not run_dirs:
        print(f"[FATAL] No run_* directories found in {TB_ROOT}")
        return

    print(f"[INFO] Found {len(run_dirs)} run_* directories in TB_ROOT.")
    print(f"[INFO] Output CSV will be: {OUTPUT_CSV}")

    # Prepare CSV
    with open(OUTPUT_CSV, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([
            "run_dir",
            "geo.Z", "geo.X", "geo.H", "geo.W",
            "thrust_max_N_per_cm",
            "thrust_avg_N_per_cm",
            "thrust_std_N_per_cm", "deltaP_kPa",
            "num_samples",
        ])

        for run_dir in run_dirs:
            run_name = os.path.basename(run_dir)
            print(f"\n[INFO] Processing {run_name} ...")

            branch_type, Z, W = parse_geometry_from_rundir(run_name)
            if Z is None:
                print(f"[WARN] Could not parse geometry from {run_name}, logging as blanks.")

            try:
                thrust_max, thrust_avg, thrust_std, deltaP_kPa, n_samples = analyze_run_max_thrust(run_dir)
            except Exception as e:
                print(f"[ERROR] Failed to analyze {run_name}: {e}")
                # Log a row with NaNs so you see the failure in the spreadsheet
                writer.writerow([
                    run_name,
                    Z, X, H, W,
                    "NaN", "NaN", "NaN", "NaN", 0,
                ])
                continue

            print(
                f"[RESULT] {run_name}: "
                f"max={thrust_max:.4f} N/cm, "
                f"avg={thrust_avg:.4f} N/cm, "
                f"std={thrust_std:.4f} N/cm, "
                f"delta_P={deltaP_kPa:.4f} kPa,"
                f"samples={n_samples}"
            )

            writer.writerow([
                run_name,
                Z, X, H, W,
                thrust_max,
                thrust_avg,
                thrust_std,
                deltaP_kPa,
                n_samples,
            ])

    print("\n[INFO] Done.")
    print(f"[INFO] Summary written to: {OUTPUT_CSV}")


if __name__ == "__main__":
    main()
