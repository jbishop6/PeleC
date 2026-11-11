
from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import yt

# --------------------------------------------------
# USER SETTINGS
# --------------------------------------------------

RESULTS_DIR = Path(
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D_Precursor/"
    "validation_Brown1999_Baurle_1291K_3.31atm_30.4us_2026-11-09_1242500"
)

PLOTFILES = ["plt02000", "plt50000", "plt98000"]

SPECIES = [
    "C2H4",
    "O2",
    "CO",
    "CO2",
    "H2O",
    "H2",
    "N2",
]

# --------------------------------------------------
# CREATE FIGURES
# --------------------------------------------------

fig_main, axes_main = plt.subplots(
    3, 1, figsize=(11, 10), sharex=True
)

fig_species, axes_species = plt.subplots(
    4, 2, figsize=(14, 13), sharex=True
)

axes_species = axes_species.flatten()

# --------------------------------------------------
# LOAD PLOTFILES
# --------------------------------------------------

for name in PLOTFILES:

    ds = yt.load(str(RESULTS_DIR / name))

    print(f"\nProcessing {name}")
    print("Available fields:", ds.field_list)

    cg = ds.covering_grid(
        level=0,
        left_edge=ds.domain_left_edge,
        dims=ds.domain_dimensions
    )

    nx = int(ds.domain_dimensions[0])

    xlo = float(ds.domain_left_edge[0])
    xhi = float(ds.domain_right_edge[0])

    x_cm = xlo + (
        np.arange(nx) + 0.5
    ) * (xhi - xlo) / nx

    def profile(field):
        data = np.asarray(cg[field])
        return data.mean(axis=tuple(range(1, data.ndim)))

    # --------------------------------------------------
    # THERMODYNAMIC PROFILES
    # --------------------------------------------------

    pressure = profile(("boxlib", "pressure")) / 1.0e6
    temperature = profile(("boxlib", "Temp"))
    rho = profile(("boxlib", "density"))

    label = f"{float(ds.current_time) * 1e6:.1f} µs"

    axes_main[0].plot(x_cm, pressure, label=label)
    axes_main[1].plot(x_cm, temperature, label=label)

    # --------------------------------------------------
    # SPECIES MASS FRACTIONS
    # --------------------------------------------------

    for i, species in enumerate(SPECIES):

        field = ("boxlib", f"rho_{species}")

        if field not in ds.field_list:
            print(f"WARNING: Missing field {field}")
            continue

        rho_species = profile(field)

        # Convert species density to mass fraction
        Y_species = rho_species / rho

        # Original ethylene plot
        if species == "C2H4":
            axes_main[2].plot(
                x_cm, Y_species, label=label
            )

        # Individual species subplot
        axes_species[i].plot(
            x_cm, Y_species, label=label
        )

        axes_species[i].set_title(species)
        axes_species[i].set_ylabel("Mass fraction")
        axes_species[i].grid(True)

# --------------------------------------------------
# FORMAT MAIN FIGURE
# --------------------------------------------------

axes_main[0].set_ylabel("Pressure [bar]")
axes_main[1].set_ylabel("Temperature [K]")
axes_main[2].set_ylabel("Ethylene mass fraction")
axes_main[2].set_xlabel("Position [cm]")

for ax in axes_main:
    ax.legend()
    ax.grid(True)

fig_main.suptitle("PeleC Detonation Profiles")

fig_main.tight_layout()

fig_main.savefig(
    RESULTS_DIR / "detonation_profiles.png",
    dpi=300
)

# --------------------------------------------------
# FORMAT SPECIES FIGURE
# --------------------------------------------------

for ax in axes_species:
    ax.set_xlabel("Position [cm]")

# Hide unused eighth subplot
axes_species[-1].axis("off")

# Shared legend
handles, labels = axes_species[0].get_legend_handles_labels()

fig_species.legend(
    handles,
    labels,
    loc="upper center",
    ncol=len(PLOTFILES)
)

fig_species.suptitle(
    "PeleC Species Mass Fraction Profiles",
    fontsize=16
)

fig_species.tight_layout(rect=[0, 0, 1, 0.94])

fig_species.savefig(
    RESULTS_DIR / "detonation_species_profiles.png",
    dpi=300
)

plt.close("all")

print("\nSaved:")
print("detonation_profiles.png")
print("detonation_species_profiles.png")
