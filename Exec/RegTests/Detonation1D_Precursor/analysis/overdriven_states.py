#!/usr/bin/env python3
"""Driver (left-state) conditions for an overdriven detonation.

For f = D/D_CJ > 1 this follows the STRONG branch of the equilibrium
Hugoniot and prints prob.T_l / prob.p_l / prob.u_l in PeleC's CGS units.

    python3 overdriven_states.py [path/to/mechanism.yaml]

Overdriving raises T, p AND the piston velocity u together.  Setting only
p (or setting T to the von Neumann value, which is cooler than the CJ
products) does not overdrive the wave.
"""
import os, sys
import numpy as np, cantera as ct
from scipy.optimize import brentq, minimize_scalar

HERE = os.path.dirname(os.path.abspath(__file__))
MECH = sys.argv[1] if len(sys.argv) > 1 else os.path.normpath(os.path.join(
    HERE, "../../../../Submodules/PelePhysics/Mechanisms/"
          "BaurleEklund_C2H4_7s3r_Ar/mechanism.yaml"))
X0 = "C2H4:0.065445, O2:0.196335, N2:0.738220"; T1, P1 = 295.0, 1.0e5

g = ct.Solution(MECH); g.TPX = T1, P1, X0
v1, h1, Y1 = 1/g.density, g.enthalpy_mass, g.Y.copy()
gq = ct.Solution(MECH)
def hug(v2):
    def res(T):
        gq.Y = Y1; gq.TD = T, 1/v2; gq.equilibrate("TV")
        return gq.enthalpy_mass - h1 - 0.5*(gq.P - P1)*(v1 + v2)
    try:
        if res(400.)*res(6000.) > 0: return None
        T2 = brentq(res, 400., 6000., xtol=1e-5)
    except Exception:
        return None
    gq.Y = Y1; gq.TD = T2, 1/v2; gq.equilibrate("TV")
    return gq.T, gq.P
def D_of(v2):
    s = hug(v2)
    if s is None or v2 >= v1: return np.inf
    n = (s[1]-P1)/(v1-v2)
    return v1*np.sqrt(n) if n > 0 else np.inf
v_cj = minimize_scalar(D_of, bounds=(0.3*v1, 0.95*v1), method="bounded",
                       options={"xatol": 1e-10}).x
D_CJ = D_of(v_cj)
print(f"D_CJ = {D_CJ:.1f} m/s\n")
print(f"{'f':>5s} {'D (m/s)':>9s} {'prob.T_l (K)':>13s} {'prob.p_l (dyn/cm2)':>19s} "
      f"{'prob.u_l (cm/s)':>16s}  {'p (atm)':>8s}")
for f in (1.0, 1.1, 1.2, 1.3, 1.5):
    D = f*D_CJ; m = D/v1
    def R(v2):
        s = hug(v2)
        return np.nan if s is None else s[1] - (P1 + m*m*(v1 - v2))
    grid = np.linspace(0.12*v1, v_cj*0.999999, 400)
    vals = np.array([R(v) for v in grid]); ok = np.isfinite(vals)
    grid, vals = grid[ok], vals[ok]
    sc = np.where(np.diff(np.sign(vals)) != 0)[0]
    if sc.size == 0:
        print(f"{f:5.2f}  (CJ tangency / no strong root)"); continue
    v2 = brentq(R, grid[sc[0]], grid[sc[0]+1], xtol=1e-14)
    T2, p2 = hug(v2); up = D*(1 - v2/v1)
    print(f"{f:5.2f} {D:9.1f} {T2:13.1f} {p2*10:19.6e} {up*100:16.6e}  {p2/101325:8.2f}")
