#!/usr/bin/env python3
"""Front tracking and D/D_CJ history from PeleC plotfiles.

    python3 postprocess.py <plotfile-prefix> [more-prefixes...]

e.g.  python3 postprocess.py plt_od_long

Needs yt.  Run it from the case directory.  Writes <prefix>_history.png.

Two gotchas this script handles, both learned the hard way:
  * plotfiles whose front has left the domain are discarded (the front
    position pins at the boundary and the computed speed goes to zero);
  * if a prefix was reused across runs, AMReX renames only the steps that
    both runs produced, so stale bare-named directories from the earlier
    run survive and interleave.  Files are split into runs by mtime.
    USE A DISTINCT amr.plot_file PER RUN AND THIS CANNOT BITE YOU.
"""
import glob, os, re, sys
import numpy as np, yt, matplotlib
matplotlib.use("Agg"); import matplotlib.pyplot as plt
yt.set_log_level(50)

D_CJ = 1855.5          # m/s, from znd_cj.py
AMB = 1.0e6            # dyn/cm^2, unburned pressure
DOMAIN_X = 10.0        # cm; must match geometry.prob_hi
EXIT_X = 0.95*DOMAIN_X
SURF, INK, INK2, GRID = "#fcfcfb", "#0b0b0b", "#52514e", "#d8d7d2"
CAT = ["#2a78d6", "#eb6834", "#1baf7a", "#eda100"]
plt.rcParams.update({"figure.facecolor": SURF, "axes.facecolor": SURF,
 "savefig.facecolor": SURF, "text.color": INK, "axes.labelcolor": INK,
 "xtick.color": INK2, "ytick.color": INK2, "axes.edgecolor": GRID,
 "grid.color": GRID, "grid.linewidth": 0.6, "axes.titleweight": "bold",
 "font.size": 10, "legend.frameon": False, "figure.dpi": 150,
 "lines.linewidth": 1.6})

def t_of(d):
    L = open(d+"/Header").read().split("\n"); n = int(L[1])
    return float(L[2+n+1])
def load(d):
    ds = yt.load(d)
    cg = ds.covering_grid(level=0, left_edge=ds.domain_left_edge,
                          dims=ds.domain_dimensions)
    gg = lambda f: np.array(cg["boxlib", f])[:, 0, 0]
    rho = gg("density"); nx = rho.size
    return dict(t=t_of(d), nx=nx, x=(np.arange(nx)+0.5)*(DOMAIN_X/nx),
                T=gg("Temp"), p=gg("pressure"),
                YC2H4=gg("rho_C2H4")/rho)
def xfront(L):
    p, x = L["p"], L["x"]
    i = np.where(p > 2*AMB)[0]
    if i.size == 0: return np.nan
    j = i.max()
    if j+1 >= p.size: return x[j]
    return x[j] + (2*AMB - p[j])/(p[j+1] - p[j])*(x[j+1] - x[j])

def newest_run(prefix):
    """bare-named dirs belonging to the most recent run with this prefix"""
    bare = [d for d in glob.glob(prefix+"*")
            if os.path.isdir(d) and re.fullmatch(re.escape(prefix)+r"\d+", d)]
    if not bare: return []
    t0 = max(os.path.getmtime(d) for d in bare)
    # keep files written within 12 h of the newest -> one run
    return sorted([d for d in bare if t0 - os.path.getmtime(d) < 43200], key=t_of)

prefixes = sys.argv[1:] or ["plt"]
if not any(newest_run(p) for p in prefixes):
    sys.exit(f"no plotfiles found for: {', '.join(prefixes)}  "
             f"(run this from the case directory)")
fig, axs = plt.subplots(3, 1, figsize=(7.8, 8.6), sharex=True)
plotted = 0
for pi, prefix in enumerate(prefixes):
    ds = newest_run(prefix)
    if not ds:
        print(f"{prefix}: no plotfiles"); continue
    plotted += 1
    ts, xs, pk, gap = [], [], [], []
    for d in ds:
        L = load(d); xf = xfront(L)
        ts.append(L["t"]); xs.append(xf); pk.append(L["p"].max()/1.01325e6)
        j = np.argmax(np.abs(np.gradient(L["YC2H4"], L["x"])))
        gap.append((xf - L["x"][j])*1e4 if np.isfinite(xf) else np.nan)
    ts, xs = np.array(ts), np.array(xs)
    pk, gap = np.array(pk), np.array(gap)
    keep = xs < EXIT_X
    nex = int(np.argmin(keep)) if (~keep).any() else len(xs)
    if nex < len(xs):
        print(f"{prefix}: front leaves domain at t={ts[nex-1]*1e6:.1f} us; "
              f"discarding {len(xs)-nex} of {len(xs)} files")
    ts, xs, pk, gap = ts[:nex], xs[:nex], pk[:nex], gap[:nex]
    v = np.diff(xs)/np.diff(ts)/100.0
    tm = 0.5*(ts[1:] + ts[:-1]); f = v/D_CJ; g = np.isfinite(f)
    dx = DOMAIN_X/load(ds[0])["nx"]*1e4
    lab = f"{prefix}  (dx = {dx:.2f} µm)"
    print(f"{prefix}: dx={dx:.2f} um  D/D_CJ min={f[g].min():.3f} "
          f"max={f[g].max():.3f} mean={f[g].mean():.3f} std={f[g].std():.3f}")
    c = CAT[pi % len(CAT)]
    axs[0].plot(tm[g]*1e6, f[g], color=c, marker="o", ms=3.6, mec=SURF,
                mew=0.6, label=lab)
    axs[1].plot(ts*1e6, pk, color=c, marker="o", ms=3.2, mec=SURF, mew=0.6)
    axs[2].plot(ts*1e6, gap, color=c, marker="o", ms=3.2, mec=SURF, mew=0.6)
for ax, (lab, ref, rl) in zip(axs, [
        ("D / D$_{CJ}$", 1.0, "CJ"),
        ("peak pressure  (atm)", 34.59, "CJ von Neumann 34.6 atm"),
        ("reaction zone offset  (µm)", 443.0, "ZND $\\Delta_i$ = 443 µm")]):
    ax.axhline(ref, color=INK2, lw=1.0, ls=(0, (4, 3)))
    ax.set_ylabel(lab); ax.grid(True); ax.set_axisbelow(True)
    ax.annotate(rl, (1, ref), xycoords=("axes fraction", "data"),
                xytext=(-4, 5), textcoords="offset points", ha="right",
                fontsize=9, color=INK2)
    for s in ("top", "right"): ax.spines[s].set_visible(False)
axs[2].set_yscale("log"); axs[2].set_xlabel("time  (µs)")
if plotted: axs[0].legend(loc="upper left", fontsize=9)
fig.tight_layout()
out = f"{(sys.argv[1] if len(sys.argv)>1 else 'plt')}_history.png"
fig.savefig(out, bbox_inches="tight"); print("wrote", out)
