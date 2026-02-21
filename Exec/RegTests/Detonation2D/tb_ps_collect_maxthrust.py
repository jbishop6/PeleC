import os
import re
import csv
from glob import glob

import numpy as np
import yt

# === CONFIG: change this if your TB_PS path moves ===
TB_ROOT = "/Users/jenniferbishop/Documents/UCCS/Combustion_Research/Results/Parameter_Sweep/TB_PS"

# Output CSV (Excel can open this directly)
OUTPUT_CSV = os.path.join(TB_ROOT, "TB_PS_thrust_max_summary.csv")


def parse_geometry_from_rundir(rundir_name):
    """
    Parse Z, X, H, W from a run directory name like:
      run_Z0p08_X0p326962268_H0p188092301_W0p084630218
    Returns (Z, X, H, W) as floats, or (None, None, None, None) if parse fails.
    """
    pattern = r"run_Z(?P<Z>[^_]+)_X(?P<X>[^_]+)_H(?P<H>[^_]+)_W(?P<W>[^_]+)"
    m = re.search(pattern, rundir_name)
    if not m:
        return None, None, None, None

    def de_p(s):
        # convert "0p08" -> "0.08"
        return float(s.replace("p", "."))

    try:
        Z = de_p(m.group("Z"))
        X = de_p(m.group("X"))
        H = de_p(m.group("H"))
        W = de_p(m.group("W"))
        return Z, X, H, W
    except Exception:
        return None, None, None, None


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

    # F_2D = Σ(ρ u^2) dy   [g/s^2] ~ dyne/cm (per depth)
    thrust_dyne = float(np.sum(rho_out * vx_out**2) * dy_cm)

    # dyne -> N  (this is effectively N per cm depth)
    thrust_N = thrust_dyne / 1e5
    return thrust_N


def analyze_run_max_thrust(run_dir, outlet_x=0.8):
    """
    For a given run directory:
      - loop over all plt* files
      - compute thrust for each
      - return max thrust over time, plus avg and std for reference
    """
    plotfiles = get_all_plotfiles(run_dir)

    thrust_values = []
    for pf in plotfiles:
        try:
            F = extract_thrust_from_plotfile(pf, outlet_x=outlet_x)
            thrust_values.append(F)
        except Exception as e:
            print(f"[WARN] Skipping {pf}: {e}")
            continue

    if not thrust_values:
        raise RuntimeError(f"No thrust values computed for {run_dir}")

    thrust_values = np.array(thrust_values)
    thrust_max = float(np.max(thrust_values))
    thrust_avg = float(np.mean(thrust_values))
    thrust_std = float(np.std(thrust_values))

    return thrust_max, thrust_avg, thrust_std, len(thrust_values)


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
            "thrust_std_N_per_cm",
            "num_samples",
        ])

        for run_dir in run_dirs:
            run_name = os.path.basename(run_dir)
            print(f"\n[INFO] Processing {run_name} ...")

            Z, X, H, W = parse_geometry_from_rundir(run_name)
            if Z is None:
                print(f"[WARN] Could not parse geometry from {run_name}, logging as blanks.")

            try:
                thrust_max, thrust_avg, thrust_std, n_samples = analyze_run_max_thrust(run_dir)
            except Exception as e:
                print(f"[ERROR] Failed to analyze {run_name}: {e}")
                # Log a row with NaNs so you see the failure in the spreadsheet
                writer.writerow([
                    run_name,
                    Z, X, H, W,
                    "NaN", "NaN", "NaN", 0,
                ])
                continue

            print(
                f"[RESULT] {run_name}: "
                f"max={thrust_max:.4f} N/cm, "
                f"avg={thrust_avg:.4f} N/cm, "
                f"std={thrust_std:.4f} N/cm, "
                f"samples={n_samples}"
            )

            writer.writerow([
                run_name,
                Z, X, H, W,
                thrust_max,
                thrust_avg,
                thrust_std,
                n_samples,
            ])

    print("\n[INFO] Done.")
    print(f"[INFO] Summary written to: {OUTPUT_CSV}")


if __name__ == "__main__":
    main()