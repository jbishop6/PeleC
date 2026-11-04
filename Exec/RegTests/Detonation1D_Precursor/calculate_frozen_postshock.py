
import cantera as ct
from sdtoolbox.postshock import PostShock_fr

# --------------------------------------------------
# CHEMICAL MECHANISM
# --------------------------------------------------

MECHANISM = (
    "/home/jbishop6/PeleC/Submodules/PelePhysics/"
    "Mechanisms/BaurleEklund_C2H4_7s3r_Ar/mechanism.yaml"
)

# --------------------------------------------------
# INITIAL UNBURNED MIXTURE
# --------------------------------------------------

T1 = 295.0       # K
P1 = 1.0e5       # Pa

COMPOSITION = "C2H4:1 O2:3 N2:11.28"

# Previously calculated CJ velocity
D_CJ = 1855.50803897  # m/s

# --------------------------------------------------
# FROZEN POST-SHOCK CALCULATION
# --------------------------------------------------

gas = PostShock_fr(
    D_CJ,
    P1,
    T1,
    COMPOSITION,
    MECHANISM
)

# --------------------------------------------------
# RESULTS
# --------------------------------------------------

print("======================================")
print("FROZEN POST-SHOCK CONDITIONS")
print("======================================")

print(f"CJ shock velocity: {D_CJ:.4f} m/s")
print(f"Initial temperature: {T1:.2f} K")
print(f"Initial pressure: {P1 / 1e5:.4f} bar")

print()
print(f"Frozen post-shock temperature: {gas.T:.4f} K")
print(f"Frozen post-shock pressure: {gas.P / 1e5:.4f} bar")
print(f"Frozen post-shock density: {gas.density:.6f} kg/m^3")

print()
print("Frozen post-shock composition:")
for name, fraction in zip(gas.species_names, gas.Y):
    if fraction > 1e-10:
        print(f"{name}: {fraction:.8e}")
