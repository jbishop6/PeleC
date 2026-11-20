#!/usr/bin/env python3
"""CJ state, von Neumann state, ZND reaction-zone structure and ignition
delays for the BaurleEklund_C2H4_7s3r_Ar mechanism.

Needs Cantera + scipy.  Writes znd_data.npz next to itself.

    python3 znd_cj.py [path/to/mechanism.yaml]

Note: the CJ tangency search converges to M2 ~ 0.973 rather than exactly
1.000, so D_CJ carries roughly +/- 30 m/s.  Literature for stoichiometric
C2H4-air at 295 K / 1 bar is ~1825 m/s.
"""
import os, sys
import numpy as np, cantera as ct
from scipy.optimize import brentq, minimize_scalar
from scipy.integrate import solve_ivp

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_MECH = os.path.normpath(os.path.join(
    HERE, "../../../../Submodules/PelePhysics/Mechanisms/"
          "BaurleEklund_C2H4_7s3r_Ar/mechanism.yaml"))
MECH = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_MECH

# unburned state, matching prob.T_r / prob.p_r and the stoichiometric mixture
X0 = "C2H4:0.065445, O2:0.196335, N2:0.738220"
T1, P1 = 295.0, 1.0e5          # K, Pa   (1e6 dyn/cm^2 = 1e5 Pa)
Ru = ct.gas_constant

g = ct.Solution(MECH); g.TPX = T1, P1, X0
v1, h1, Y1 = 1.0/g.density, g.enthalpy_mass, g.Y.copy()
W1, W = g.mean_molecular_weight, g.molecular_weights
print(f"[1] unburned  T={T1} K  p={P1/1e5:.3f} bar  rho={1/v1:.5f} kg/m3  a={g.sound_speed:.1f} m/s")

gq = ct.Solution(MECH)
def hug_eq(v2):
    """equilibrium Hugoniot point at specific volume v2 -> (T, p, a)"""
    def res(T2):
        gq.Y = Y1; gq.TD = T2, 1.0/v2; gq.equilibrate("TV")
        return gq.enthalpy_mass - h1 - 0.5*(gq.P - P1)*(v1 + v2)
    try:
        if res(400.0)*res(6000.0) > 0: return None
        T2 = brentq(res, 400.0, 6000.0, xtol=1e-5)
    except Exception:
        return None
    gq.Y = Y1; gq.TD = T2, 1.0/v2; gq.equilibrate("TV")
    return gq.T, gq.P, gq.sound_speed

def D_of(v2):
    s = hug_eq(v2)
    if s is None or v2 >= v1: return np.inf
    num = (s[1] - P1)/(v1 - v2)
    return v1*np.sqrt(num) if num > 0 else np.inf

r = minimize_scalar(D_of, bounds=(0.3*v1, 0.95*v1), method="bounded",
                    options={"xatol": 1e-10})
v_cj = r.x; D_CJ = D_of(v_cj); Tcj, Pcj, acj = hug_eq(v_cj)
print(f"[2] CJ        D_CJ={D_CJ:.1f} m/s  p_CJ={Pcj/101325:.2f} atm  "
      f"T_CJ={Tcj:.1f} K  rho2/rho1={v1/v_cj:.3f}")

# --- von Neumann: frozen-composition shock jump at D_CJ -------------------
gf = ct.Solution(MECH); m = D_CJ/v1
def vn_res(v2):
    p2 = P1 + m**2*(v1 - v2); T2 = p2*v2*W1/Ru
    gf.TPY = T2, p2, Y1
    return gf.enthalpy_mass - h1 - 0.5*m**2*(v1**2 - v2**2)
v_vn = brentq(vn_res, 0.05*v1, 0.995*v1, xtol=1e-14)
p_vn = P1 + m**2*(v1 - v_vn); T_vn = p_vn*v_vn*W1/Ru
gf.TPY = T_vn, p_vn, Y1; w_vn = m*v_vn
print(f"[3] von Neumann  T={T_vn:.1f} K  p={p_vn/101325:.2f} atm  "
      f"rho/rho1={v1/v_vn:.3f}  M={w_vn/gf.sound_speed:.4f}")

# --- ZND reaction zone, integrated in particle time ----------------------
gz = ct.Solution(MECH); ns = gz.n_species
def rhs(t, y):
    w, T = y[0], y[1]; Yv = np.clip(y[2:2+ns], 0, 1); rho = m/w
    gz.TDY = T, rho, Yv
    dY = gz.net_production_rates*W/rho
    cp, Wbar = gz.cp_mass, gz.mean_molecular_weight
    hi = gz.partial_molar_enthalpies/W
    sigdot = float(np.dot(Wbar/W - hi/(cp*T), dY))
    M = w/gz.sound_speed; eta = 1.0 - M*M
    eta = np.sign(eta)*max(abs(eta), 1e-8)
    dw = w*sigdot/eta
    dT = (-w*dw - float(np.dot(hi, dY)))/cp
    return np.concatenate(([dw], [dT], dY, [w]))
def sonic(t, y):
    gz.TDY = y[1], m/y[0], np.clip(y[2:2+ns], 0, 1)
    return 0.9985 - y[0]/gz.sound_speed
sonic.terminal = True; sonic.direction = -1

y0 = np.concatenate(([w_vn], [T_vn], Y1, [0.0]))
sol = solve_ivp(rhs, [0, 2e-3], y0, method="LSODA", events=sonic,
                rtol=1e-9, atol=1e-14, max_step=1e-6)
t, wv, Tv, Yv, xv = sol.t, sol.y[0], sol.y[1], sol.y[2:2+ns], sol.y[2+ns]
rhov = m/wv
pv = np.empty_like(Tv); sig = np.empty_like(Tv); Mv = np.empty_like(Tv)
for i in range(Tv.size):
    gz.TDY = Tv[i], rhov[i], np.clip(Yv[:, i], 0, 1)
    pv[i] = gz.P; Mv[i] = wv[i]/gz.sound_speed
    dY = gz.net_production_rates*W/rhov[i]
    s = gz.mean_molecular_weight/W - (gz.partial_molar_enthalpies/W)/(gz.cp_mass*Tv[i])
    sig[i] = float(np.dot(s, dY))
ipk = int(np.argmax(sig)); Delta_i = xv[ipk]
iC = gz.species_index("C2H4"); frac = 1.0 - Yv[iC]/Y1[iC]
xat = lambda f: xv[min(int(np.searchsorted(frac, f)), xv.size-1)]
print(f"[4] ZND   induction length (peak thermicity) = {Delta_i*1e6:.1f} um "
      f"(tau={t[ipk]:.3e} s)")
print(f"          50% / 95% / 99% C2H4 consumed at "
      f"{xat(0.5)*1e6:.0f} / {xat(0.95)*1e6:.0f} / {xat(0.99)*1e6:.0f} um")

# --- constant-volume ignition delay --------------------------------------
def tau_ign(T0, p0):
    gi = ct.Solution(MECH); gi.TPX = T0, p0, X0
    rr = ct.IdealGasReactor(gi); sim = ct.ReactorNet([rr])
    tt, TT = [0.0], [gi.T]
    while sim.time < 1e-1:
        sim.step(); tt.append(sim.time); TT.append(rr.T)
        if rr.T > T0 + 1500: break
    tt, TT = np.array(tt), np.array(TT)
    return np.nan if TT[-1] < T0 + 100 else tt[int(np.argmax(np.gradient(TT, tt)))]
tv = tau_ign(T_vn, p_vn)
print(f"[5] ignition delay at von Neumann ({T_vn:.0f} K, {p_vn/101325:.1f} atm) "
      f"= {tv:.3e} s")

np.savez(os.path.join(HERE, "znd_data.npz"),
         x=xv, t=t, T=Tv, p=pv, M=Mv, sig=sig, Y=Yv,
         names=np.array(gz.species_names), D_CJ=D_CJ, T_vn=T_vn, p_vn=p_vn,
         Tcj=Tcj, Pcj=Pcj, Delta_i=Delta_i, tau_vn=tv)
print(f"    wrote {os.path.join(HERE,'znd_data.npz')}")
