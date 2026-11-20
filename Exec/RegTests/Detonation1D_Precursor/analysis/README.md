# Detonation1D_Precursor — detonation initiation study

Analysis of why the original CJ-driver setup failed to detonate, and what
makes it work.  All numbers below are for `BaurleEklund_C2H4_7s3r_Ar`,
stoichiometric C2H4-air at 295 K / 1 bar.

## Reference states (from `znd_cj.py`)

| quantity | value |
|---|---|
| D_CJ | 1855.5 m/s  (literature ~1825; see caveat below) |
| CJ | 3027.4 K, 18.72 atm |
| von Neumann (frozen shock at D_CJ) | 1630.2 K, 34.59 atm |
| ZND induction length, Delta_i | 443 um  (tau = 1.26 us) |
| 95% / 99% C2H4 consumed | 1200 um / 2208 um |
| ignition delay at von Neumann | 0.84 us |

Caveat: the CJ tangency search converges to M2 ~ 0.973 rather than exactly
1.000, so D_CJ is good to roughly +/- 30 m/s.  The equilibrium
thermodynamics are sound (adiabatic flame temperature 2400 K matches
expectation), so the literature value is a fair cross-check.

## What was wrong with the original setup

`input.detonation1D.inp` initialised the driver at the CJ state and the
wave never detonated.  It settled into a steady **0.674 D_CJ** shock with
the reaction zone trailing **2.5 mm** behind the shock — a decoupled
shock/deflagration complex, not a detonation.

The cause was not resolution.  Delta_i = 443 um spans 35 cells at
dx = 12.5 um, which is adequate.  The cause was that the driver produced
only a 942 K post-shock state, where the ignition delay is **747 us**
(vs 0.84 us at the required 1630 K von Neumann state) — an 890x gap.  At
1250 m/s that is 934 mm of travel before ignition, in a 100 mm domain.
The shocked gas could never ignite.

## What works

Overdrive the driver and let it relax.  `overdriven_states.py` prints the
strong-branch Hugoniot states; `input.detonation1D_od12.inp` uses f = 1.2:

    prob.T_l = 3520.6          # K
    prob.p_l = 4.187943e7      # dyn/cm^2  (41.33 atm)
    prob.u_l = 1.563660e5      # cm/s      (1563.7 m/s)

Overdriving raises T, p **and** the piston velocity u together.  Note the
von Neumann temperature (1630 K) is *lower* than the CJ product
temperature (3027 K) — the von Neumann state is shocked unburned gas and
is not a driver state.

With this driver the reaction zone attaches (62 um behind the shock) and a
von Neumann pressure spike appears.

## Boundary condition matters

* `pelec.lo_bc = "FOExtrap"` keeps feeding driver gas in — the wave stays
  piston-supported at f ~ 1.18 indefinitely.
* `pelec.lo_bc = "NoSlipWall"` makes the driver finite.  A rarefaction
  leaves the wall and overtakes the front at ~10.5 us, after which the
  overdrive decays.

## Result: the detonation gallops

With the wall BC the wave does not settle at CJ.  It cycles:
decay to ~0.88 D_CJ with the reaction zone stretching to ~2000 um
(decoupling), then a violent re-ignition (peak pressure far above the CJ
von Neumann value, reaction zone collapsing to ~20 um), then decay again.
Period ~19 us, which is ~3.6 cm of travel.

A resolution check (dx 12.5 -> 6.25 um, 35 -> 71 cells per Delta_i) shows
the galloping **survives refinement** — the two runs overlay exactly
through 19 us and the cycle is reproduced, not damped.  So the mode is
physical, not numerical.  Individual cycle amplitude and phase are *not*
grid-converged, though: the fine run re-ignites earlier (31 vs 40 us) and
less violently (78 vs 196 atm peak).  That is expected for a galloping
detonation, where re-initiation is exponentially sensitive to the local
temperature history.

## Open items

* The 10 cm domain only holds ~2 gallop cycles before the front exits at
  ~43 us.  A 30 cm domain (`amr.n_cell = 24000 8`, `stop_time = 1.5e-4`)
  would give ~8 cycles and let you measure period/amplitude statistics.
* A third resolution (dx = 3.125 um) would show whether cycle amplitude is
  converging.
* All of this is a property of a 3-reaction skeletal mechanism, which is
  likely more unstable than real ethylene-air chemistry.

## Running the scripts

`znd_cj.py` and `overdriven_states.py` need Cantera + scipy;
`postprocess.py` needs yt.  Run `postprocess.py` from the case directory.

    python3 analysis/znd_cj.py                 # writes analysis/znd_data.npz
    python3 analysis/overdriven_states.py      # driver-state table
    python3 analysis/postprocess.py plt_od     # D/D_CJ history -> plt_od_history.png

`postprocess.py` accepts several prefixes to overlay runs, discards
plotfiles whose front has left the domain, and splits interleaved runs by
mtime.  **Use a distinct `amr.plot_file` per run** — reusing a prefix makes
AMReX rename only the steps both runs produced, leaving stale directories
from the earlier run interleaved with the new ones.

Figures in the parent directory: `znd_structure.png`, `znd_species.png`,
`znd_ignition_delay.png`, `znd_lengthscales.png` (theory), and
`long_history.png`, `long_znd_compare.png` (simulation).
