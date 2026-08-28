import sys

sys.path.insert(
    0,
    "/home/jbishop6/sdtoolbox/Python3/sdtoolbox"
)

import cantera as ct
from sdtoolbox.postshock import CJspeed, PostShock_eq

# ------------------------------------------------------------
# Fresh reactant state -- this comes directly from your input
# ------------------------------------------------------------

T1 = 300.0
P1 = 1.0e5

# Stoichiometric H2/O2
# This is the same mixture as your Y_H2 = 0.1119,
# Y_O2 = 0.8881, just specified in mole proportions.
q = "H2:2 O2:1"

# IMPORTANT:
# Use the Cantera mechanism corresponding to your Davis mechanism
mech = "/home/jbishop6/PeleC/Submodules/PelePhysics/Mechanisms/Davis/mechanism.yaml"

# ------------------------------------------------------------
# CJ SPEED
# ------------------------------------------------------------

D_CJ = CJspeed(P1, T1, q, mech)

print(f"CJ speed = {D_CJ:.3f} m/s")


# ------------------------------------------------------------
# FRESH MIXTURE DENSITY
# ------------------------------------------------------------

gas1 = ct.Solution(mech)
gas1.TPX = T1, P1, q

rho1 = gas1.density


# ------------------------------------------------------------
# EQUILIBRIUM POST-DETONATION CJ STATE
# ------------------------------------------------------------

gas2 = PostShock_eq(
    D_CJ,
    P1,
    T1,
    q,
    mech
)

rho2 = gas2.density
T2   = gas2.T
P2   = gas2.P


# ------------------------------------------------------------
# LAB-FRAME PRODUCT VELOCITY
#
# Mass conservation across moving wave:
#
# rho1 * D = rho2 * (D - u2)
#
# Therefore:
#
# u2 = D * (1 - rho1/rho2)
# ------------------------------------------------------------

u2 = D_CJ * (1.0 - rho1 / rho2)


print()
print("Fresh state:")
print(f"rho1 = {rho1:.8e} kg/m^3")
print(f"T1   = {T1:.3f} K")
print(f"P1   = {P1:.8e} Pa")

print()
print("CJ product state:")
print(f"rho2 = {rho2:.8e} kg/m^3")
print(f"T2   = {T2:.3f} K")
print(f"P2   = {P2:.8e} Pa")
print(f"u2   = {u2:.3f} m/s")

print()
print("CJ speed:")
print(f"D_CJ = {D_CJ:.3f} m/s")

print()
print("Major product mass fractions:")

for species, Y in zip(gas2.species_names, gas2.Y):
    if Y > 1.0e-5:
        print(f"{species:8s} {Y:.8f}")
