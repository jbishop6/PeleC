from pathlib import Path
import sys
import csv
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import cantera as ct

# --------------------------------------------------
# PATHS
# --------------------------------------------------

BASE_DIR = Path(__file__).resolve().parent

SDTOOLBOX_DIR = Path(
    "/home/jbishop6/sdtoolbox/Python3"
)

MECH = (
    "/home/jbishop6/PeleC/Submodules/PelePhysics/"
    "Mechanisms/BaurleEklund_C2H4_7s3r_Ar/mechanism.yaml"
)

sys.path.insert(0, str(SDTOOLBOX_DIR))

from sdtoolbox.postshock import CJspeed, PostShock_fr
from sdtoolbox.znd import zndsolve

# --------------------------------------------------
# INITIAL CONDITIONS (SI)
# --------------------------------------------------

T1 = 295.0
P1 = 1.0e5
q = "C2H4:1 O2:3 N2:11.28"

# Integration settings
T_END = 5.0e-5
MAX_STEP = 1.0e-8
REL_TOL = 1.0e-8
ABS_TOL = 1.0e-12

# --------------------------------------------------
# CJ SPEED AND FROZEN SHOCK
# --------------------------------------------------

gas1 = ct.Solution(MECH)
gas1.TPX = T1, P1, q

D_CJ = CJspeed(P1, T1, q, MECH)

gas_shock = PostShock_fr(
    D_CJ, P1, T1, q, MECH
)

rho1 = gas1.density
rho_vn = gas_shock.density

# Shock-frame gas velocity
U_vn = D_CJ * rho1 / rho_vn

print(f"CJ speed: {D_CJ:.6f} m/s")
print(f"Frozen shock T: {gas_shock.T:.3f} K")
print(f"Frozen shock P: {gas_shock.P / 1e5:.3f} bar")

# --------------------------------------------------
# ZND INTEGRATION
# --------------------------------------------------

znd = zndsolve(
    gas_shock,
    gas1,
    D_CJ,
    t_end=T_END,
    max_step=MAX_STEP,
    relTol=REL_TOL,
    absTol=ABS_TOL,
    advanced_output=True,
)

print("Returned ZND fields:", list(znd.keys()))

# --------------------------------------------------
# EXTRACT PROFILES
# --------------------------------------------------

x = np.asarray(znd["distance"]).reshape(-1)
t = np.asarray(znd["time"]).reshape(-1)
T = np.asarray(znd["T"]).reshape(-1)
P = np.asarray(znd["P"]).reshape(-1)
rho = np.asarray(znd["rho"]).reshape(-1)
U = np.asarray(znd["U"]).reshape(-1)

# Species are expected as mass fractions.
species = np.asarray(znd["species"])

if species.shape == (gas1.n_species, len(x)):
    species = species.T
elif species.shape != (len(x), gas1.n_species):
    raise ValueError(
        f"Unexpected species array shape: {species.shape}"
    )

if not all(
    len(a) == len(x) for a in (t, T, P, rho, U)
):
    raise ValueError("ZND profile arrays have different lengths")

if not np.all(np.isfinite(species)):
    raise ValueError("Nonfinite species mass fractions")

if np.min(species) < -1e-8:
    raise ValueError("Negative species mass fractions")

if not np.allclose(
    species.sum(axis=1), 1.0, atol=1e-5
):
    raise ValueError("Species mass fractions do not sum to one")

# Convert shock-frame velocity to lab-frame velocity.
u_lab = D_CJ - U

# --------------------------------------------------
# EXPORT CSV (SI UNITS)
# --------------------------------------------------

csv_path = BASE_DIR / "ZND_profile_Baurle.csv"

with open(csv_path, "w", newline="") as f:
    writer = csv.writer(f)

    writer.writerow(
        ["distance_m", "time_s", "T_K", "P_Pa",
         "rho_kg_m3", "u_lab_m_s"]
        + [f"Y_{s}" for s in gas1.species_names]
    )

    for i in range(len(x)):
        writer.writerow(
            [x[i], t[i], T[i], P[i], rho[i], u_lab[i]]
            + list(species[i])
        )

# --------------------------------------------------
# SUMMARY
# --------------------------------------------------

summary_path = BASE_DIR / "ZND_results_Baurle.txt"

with open(summary_path, "w") as f:
    f.write("BAURLE-EKLUND ZND CALCULATION\n\n")
    f.write(f"T1 = {T1:.6f} K\n")
    f.write(f"P1 = {P1:.6e} Pa\n")
    f.write(f"Mixture = {q}\n")
    f.write(f"D_CJ = {D_CJ:.9f} m/s\n\n")

    f.write("FROZEN POST-SHOCK STATE\n")
    f.write(f"T_vN = {gas_shock.T:.6f} K\n")
    f.write(f"P_vN = {gas_shock.P:.6e} Pa\n")
    f.write(f"rho_vN = {rho_vn:.9f} kg/m3\n")
    f.write(f"U_vN = {U_vn:.6f} m/s\n\n")

    f.write("ZND INTEGRATION\n")
    f.write(f"Final time = {t[-1]:.9e} s\n")
    f.write(f"Final distance = {x[-1]:.9e} m\n")
    f.write(f"Final temperature = {T[-1]:.6f} K\n")
    f.write(f"Final pressure = {P[-1]:.6e} Pa\n")
    f.write(f"Final lab velocity = {u_lab[-1]:.6f} m/s\n")
    f.write(f"Number of points = {len(x)}\n")

    for key in ("ind_time", "ind_len", "exo_time", "exo_len"):
        if key in znd:
            f.write(f"{key} = {znd[key]}\n")

# --------------------------------------------------
# PLOTS
# --------------------------------------------------

fig, axes = plt.subplots(
    4, 1, figsize=(10, 12), sharex=True
)

x_mm = x * 1000.0

axes[0].plot(x_mm, P / 1e5)
axes[0].set_ylabel("Pressure [bar]")

axes[1].plot(x_mm, T)
axes[1].set_ylabel("Temperature [K]")

axes[2].plot(x_mm, u_lab)
axes[2].set_ylabel("Lab velocity [m/s]")

for name in ["C2H4", "O2", "CO", "CO2", "H2O"]:
    if name in gas1.species_names:
        idx = gas1.species_index(name)
        axes[3].plot(
            x_mm, species[:, idx], label=name
        )

axes[3].set_ylabel("Mass fraction")
axes[3].set_xlabel("Distance behind shock [mm]")
axes[3].legend()

for ax in axes:
    ax.grid(True, alpha=0.3)

fig.suptitle("Baurle-Eklund ZND Detonation Structure")
fig.tight_layout()

fig.savefig(
    BASE_DIR / "ZND_profiles_Baurle.png",
    dpi=200
)

plt.close(fig)

print("\nZND calculation complete.")
print(f"Final distance: {x[-1] * 1000:.4f} mm")
print(f"Final temperature: {T[-1]:.2f} K")
print(f"CSV saved: {csv_path}")
