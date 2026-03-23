import os
from glob import glob

import yt
import numpy as np
import matplotlib.pyplot as plt


RUN_DIR = "."   # or full path to the run folder


def get_all_plotfiles(base_dir):
    plot_dirs = sorted(glob(os.path.join(base_dir, "plt*")))
    if not plot_dirs:
        raise RuntimeError(f"No plotfiles found in {base_dir}.")
    return plot_dirs


def extract_thrust_from_plotfile(plotfile_dir, outlet_x=0.95, tolerance=0.10):
    """
    Exact same outlet/mask logic as optimization script.
    Returns the same numeric value as before, but for a 2D case
    you will interpret/report it as force per unit depth.
    """
    ds = yt.load(plotfile_dir)
    ad = ds.all_data()

    x_cm = ad["x"].to("cm").v
    rho_cgs = ad["density"].to("g/cm**3").v
    vx_cgs = ad["x_velocity"].to("cm/s").v

    dy_cm = float((ds.domain_width[1] / ds.domain_dimensions[1]).to("cm"))

    x_min = float(ds.domain_left_edge[0].to("cm"))
    x_max = float(ds.domain_right_edge[0].to("cm"))
    domain_length = x_max - x_min

    outlet_x_cm = x_min + outlet_x * domain_length
    tolerance_cm = tolerance * domain_length

    mask = np.abs(x_cm - outlet_x_cm) < tolerance_cm

    if np.sum(mask) == 0:
        raise RuntimeError(f"No cells found near outlet in {plotfile_dir}")

    rho_out = rho_cgs[mask]
    vx_out = vx_cgs[mask]

    thrust_dyne = float(np.sum(rho_out * vx_out**2) * dy_cm)

    # keep exact same numeric conversion as original script
    thrust_per_unit_depth = thrust_dyne / 1e5

    return thrust_per_unit_depth


def main():
    plotfiles = get_all_plotfiles(RUN_DIR)

    results = []
    for pf in plotfiles:
        ds = yt.load(pf)
        time_s = float(ds.current_time.to("s"))
        time_us = time_s * 1e6

        thrust_val = extract_thrust_from_plotfile(pf)

        results.append({
            "plotfile": os.path.basename(pf),
            "time_us": time_us,
            "thrust": thrust_val
        })

        print(f"{os.path.basename(pf):20s}  t={time_us:8.3f} us  thrust={thrust_val:10.5f} N/m")

    times = np.array([r["time_us"] for r in results])
    thrust = np.array([r["thrust"] for r in results])

    thrust_avg = np.mean(thrust)
    thrust_std = np.std(thrust)

    fig, ax = plt.subplots(figsize=(10, 6))

    ax.plot(times, thrust, 'b-', linewidth=2, marker='o',
            markersize=4, label='Instantaneous Thrust')
    ax.axhline(thrust_avg, color='r', linestyle='--',
               linewidth=2, label=f'Average: {thrust_avg:.2f} N/m')
    ax.fill_between(times, thrust_avg - thrust_std, thrust_avg + thrust_std,
                    alpha=0.2, color='r', label=f'±1σ: {thrust_std:.2f} N/m')

    ax.set_xlabel('Time (μs)', fontsize=12, fontweight='bold')
    ax.set_ylabel('Thrust (N/m)', fontsize=12, fontweight='bold')
    ax.set_title('Thrust Evolution', fontsize=14, fontweight='bold')
    ax.grid(True, alpha=0.3)
    ax.legend(fontsize=10)

    plt.tight_layout()
    plt.savefig(os.path.join(RUN_DIR, "thrust_evolution_fixed.png"),
                dpi=150, bbox_inches='tight')
    plt.show()


if __name__ == "__main__":
    main()
