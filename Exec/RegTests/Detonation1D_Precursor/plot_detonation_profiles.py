
from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
import yt

# --------------------------------------------------
# USER SETTINGS
# --------------------------------------------------

RESULTS_DIR = Path(
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D_Precursor/"
    "validation_Brown1999_Baurle_1291K_3.31atm_30.4us_2026-11-02_095632"
)

PLOTFILES = ["plt02000", "plt50000", "plt98000"]

# --------------------------------------------------
# LOAD AND PLOT
# --------------------------------------------------

fig, axes = plt.subplots(3, 1, figsize=(10, 10), sharex=True)

for name in PLOTFILES:
    ds = yt.load(str(RESULTS_DIR / name))

    print(f"\n{name}")
    print("Available fields:", ds.field_list)

    cg = ds.covering_grid(
        level=0,
        left_edge=ds.domain_left_edge,
        dims=ds.domain_dimensions
    )

    nx = int(ds.domain_dimensions[0])

    # PeleC coordinates are in cm
    xlo = float(ds.domain_left_edge[0])
    xhi = float(ds.domain_right_edge[0])

    x_cm = xlo + (
        np.arange(nx) + 0.5
    ) * (xhi - xlo) / nx

    def profile(field):
        data = np.asarray(cg[field])
        return data.mean(axis=tuple(range(1, data.ndim)))

    # These field names must match the plotfile.
    pressure = profile(("boxlib", "pressure")) / 1.0e6
    temperature = profile(("boxlib", "Temp"))

    # Species may be stored as density-weighted fractions.
    rho = profile(("boxlib", "density"))
    rho_c2h4 = profile(("boxlib", "rho_C2H4"))
    y_c2h4 = rho_c2h4 / rho

    label = f"{float(ds.current_time) * 1e6:.1f} µs"

    axes[0].plot(x_cm, pressure, label=label)
    axes[1].plot(x_cm, temperature, label=label)
    axes[2].plot(x_cm, y_c2h4, label=label)

axes[0].set_ylabel("Pressure [bar]")
axes[1].set_ylabel("Temperature [K]")
axes[2].set_ylabel("Ethylene mass fraction")
axes[2].set_xlabel("Position [cm]")

for ax in axes:
    ax.legend()
    ax.grid(True)

axes[0].set_title("PeleC Detonation Profiles")

plt.tight_layout()
plt.savefig(RESULTS_DIR / "detonation_profiles.png", dpi=300)
plt.show()
