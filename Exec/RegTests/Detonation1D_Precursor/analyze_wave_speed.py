# ============================================================
# analyze_wave_speed.py
#
# Determine the propagation speed of the leading pressure wave
# in the PeleC quasi-1D detonation precursor.
#
# The measured PeleC wave speed is compared against the
# theoretical CJ speed calculated using SDToolbox/Davis.
# ============================================================

from pathlib import Path

import numpy as np
import matplotlib.pyplot as plt
import yt


# ============================================================
# USER SETTINGS
# ============================================================

# IMPORTANT:
# Point this ONLY to the results directory for the run
# you want to analyze.
RESULTS_DIR = Path(
    "/home/jbishop6/PeleC/Exec/RegTests/Detonation1D_Precursor/"
    "results_2026-08-28_172816"
)

# Theoretical CJ speed from SDToolbox + Davis mechanism
D_CJ = 2834.958187   # m/s

# Original location of the two-state interface
X_INITIAL = 0.005    # m

# Ignore a few cells immediately around the initial interface
# when looking for the pressure front.
X_SEARCH_MIN = 0.0051

# Optional upper search bound
X_SEARCH_MAX = 0.095

# Skip some very early data when performing the linear fit.
#
# At extremely early times, the initialized discontinuity may
# relax before establishing its propagation behavior.
#
# Change this later if needed after inspecting the results.
FIT_START_TIME = 1.0e-8   # s


# ============================================================
# FIND PLOTFILES
# ============================================================

plotfiles = sorted(
    p for p in RESULTS_DIR.glob("plt*")
    if p.is_dir()
)

if len(plotfiles) == 0:
    raise RuntimeError(
        f"No plt* directories found in:\n{RESULTS_DIR}"
    )

print("\n========================================")
print("PELEC WAVE SPEED ANALYSIS")
print("========================================")
print(f"Results directory : {RESULTS_DIR}")
print(f"Number plotfiles  : {len(plotfiles)}")
print(f"First plotfile    : {plotfiles[0].name}")
print(f"Last plotfile     : {plotfiles[-1].name}")
print(f"Theoretical D_CJ  : {D_CJ:.3f} m/s")
print("========================================\n")


# ============================================================
# HELPER: FIND PRESSURE FIELD
# ============================================================

def find_pressure_field(ds):
    """
    Find the pressure field in the PeleC plotfile.
    """

    candidates = [
        ("boxlib", "pressure"),
        ("boxlib", "Pressure"),
        ("gas", "pressure"),
    ]

    all_fields = list(ds.field_list) + list(ds.derived_field_list)

    for field in candidates:
        if field in all_fields:
            return field

    # Fallback: look for anything named pressure
    for field in all_fields:
        if field[1].lower() == "pressure":
            return field

    print("\nAvailable fields:")
    for field in ds.field_list:
        print(field)

    raise RuntimeError(
        "Could not find a pressure field in the plotfile."
    )


# ============================================================
# LOOP THROUGH PLOTFILES
# ============================================================

times = []
front_positions = []
front_pressures = []

pressure_field = None

for n, plotfile in enumerate(plotfiles):

    ds = yt.load(str(plotfile))

    # --------------------------------------------------------
    # Simulation time
    # --------------------------------------------------------

    try:
        time = float(ds.current_time.to("s"))
    except Exception:
        time = float(ds.current_time)

    # --------------------------------------------------------
    # Determine pressure field on first plotfile
    # --------------------------------------------------------

    if pressure_field is None:
        pressure_field = find_pressure_field(ds)
        print(f"Using pressure field: {pressure_field}\n")

    # --------------------------------------------------------
    # Make a uniform grid covering the base domain
    #
    # Current precursor has amr.max_level = 0, so this gives
    # the complete solution directly.
    # --------------------------------------------------------

    dims = ds.domain_dimensions

    cg = ds.covering_grid(
        level=0,
        left_edge=ds.domain_left_edge,
        dims=dims
    )

    # --------------------------------------------------------
    # Extract pressure
    # --------------------------------------------------------

    pressure = cg[pressure_field]

    try:
        pressure = pressure.to("Pa").d
    except Exception:
        pressure = np.asarray(pressure)

    # --------------------------------------------------------
    # Average across all transverse directions.
    #
    # This makes the thin 2-D case effectively a 1-D profile.
    # --------------------------------------------------------

    if pressure.ndim == 3:
        p_x = np.mean(pressure, axis=(1, 2))
    elif pressure.ndim == 2:
        p_x = np.mean(pressure, axis=1)
    else:
        p_x = pressure

    # --------------------------------------------------------
    # Construct x cell-center coordinates
    # --------------------------------------------------------

    xlo = float(ds.domain_left_edge[0].to("m"))
    xhi = float(ds.domain_right_edge[0].to("m"))

    nx = len(p_x)

    dx = (xhi - xlo) / nx

    x = xlo + (np.arange(nx) + 0.5) * dx

    # --------------------------------------------------------
    # Search only in the physically relevant portion
    # --------------------------------------------------------

    mask = (
        (x >= X_SEARCH_MIN) &
        (x <= X_SEARCH_MAX)
    )

    x_search = x[mask]
    p_search = p_x[mask]

    # --------------------------------------------------------
    # Find the leading pressure front
    #
    # For a right-moving detonation:
    #
    #     high pressure | shock | low pressure
    #
    # pressure therefore falls sharply with increasing x.
    #
    # The shock location is approximated by the most negative
    # value of dp/dx.
    # --------------------------------------------------------

    dpdx = np.gradient(p_search, x_search)

    shock_index = np.argmin(dpdx)

    x_front = x_search[shock_index]
    p_front = p_search[shock_index]

    times.append(time)
    front_positions.append(x_front)
    front_pressures.append(p_front)

    print(
        f"{plotfile.name:12s}  "
        f"t = {time:12.5e} s   "
        f"x_front = {x_front:10.6f} m"
    )


# ============================================================
# CONVERT TO NUMPY ARRAYS
# ============================================================

times = np.asarray(times)
front_positions = np.asarray(front_positions)
front_pressures = np.asarray(front_pressures)


# ============================================================
# REMOVE t = 0 / EARLY TRANSIENT FOR FIT
# ============================================================

fit_mask = times >= FIT_START_TIME

t_fit = times[fit_mask]
x_fit = front_positions[fit_mask]

if len(t_fit) < 2:
    raise RuntimeError(
        "Not enough points remain for the linear fit. "
        "Reduce FIT_START_TIME."
    )


# ============================================================
# LINEAR FIT:
#
# x_front = D_sim * t + b
#
# Therefore slope = simulated wave speed
# ============================================================

coefficients = np.polyfit(t_fit, x_fit, 1)

D_sim = coefficients[0]
intercept = coefficients[1]

x_prediction = (
    D_sim * t_fit + intercept
)


# ============================================================
# FIT QUALITY
# ============================================================

residuals = x_fit - x_prediction

ss_res = np.sum(residuals**2)

ss_tot = np.sum(
    (x_fit - np.mean(x_fit))**2
)

if ss_tot > 0.0:
    r_squared = 1.0 - ss_res / ss_tot
else:
    r_squared = np.nan


# ============================================================
# COMPARE AGAINST THEORETICAL CJ SPEED
# ============================================================

percent_difference = (
    (D_sim - D_CJ) / D_CJ
) * 100.0

absolute_percent_difference = abs(percent_difference)


# ============================================================
# ALSO CALCULATE INTERVAL-BY-INTERVAL SPEED
# ============================================================

dt = np.diff(times)
dx_front = np.diff(front_positions)

instantaneous_speed = np.divide(
    dx_front,
    dt,
    out=np.full_like(dx_front, np.nan),
    where=dt != 0
)

instantaneous_time = (
    0.5 * (times[:-1] + times[1:])
)


# ============================================================
# PRINT RESULTS
# ============================================================

print("\n========================================")
print("WAVE SPEED RESULTS")
print("========================================")

print(
    f"Theoretical CJ speed : "
    f"{D_CJ:.3f} m/s"
)

print(
    f"PeleC fitted speed   : "
    f"{D_sim:.3f} m/s"
)

print(
    f"Percent difference   : "
    f"{percent_difference:.3f} %"
)

print(
    f"Absolute difference  : "
    f"{absolute_percent_difference:.3f} %"
)

print(
    f"R^2 of linear fit    : "
    f"{r_squared:.6f}"
)

print(
    f"Fit begins at        : "
    f"{FIT_START_TIME:.3e} s"
)

print("========================================\n")


# ============================================================
# SAVE FRONT POSITION DATA
# ============================================================

csv_file = RESULTS_DIR / "wave_front_positions.csv"

data = np.column_stack(
    (
        times,
        front_positions,
        front_pressures
    )
)

np.savetxt(
    csv_file,
    data,
    delimiter=",",
    header="time_s,x_front_m,pressure_front_Pa",
    comments=""
)


# ============================================================
# SAVE SPEED BETWEEN EACH PAIR OF PLOTFILES
# ============================================================

speed_csv = RESULTS_DIR / "wave_speed_history.csv"

speed_data = np.column_stack(
    (
        instantaneous_time,
        instantaneous_speed
    )
)

np.savetxt(
    speed_csv,
    speed_data,
    delimiter=",",
    header="time_s,wave_speed_m_per_s",
    comments=""
)


# ============================================================
# SAVE SUMMARY TEXT FILE
# ============================================================

summary_file = RESULTS_DIR / "wave_speed_summary.txt"

with open(summary_file, "w") as f:

    f.write(
        "========================================\n"
    )
    f.write(
        "PELEC DETONATION WAVE SPEED ANALYSIS\n"
    )
    f.write(
        "========================================\n\n"
    )

    f.write(
        f"Results directory:\n"
        f"{RESULTS_DIR}\n\n"
    )

    f.write(
        f"Number of plotfiles = "
        f"{len(plotfiles)}\n"
    )

    f.write(
        f"Fit start time = "
        f"{FIT_START_TIME:.8e} s\n\n"
    )

    f.write(
        f"Theoretical Davis CJ speed = "
        f"{D_CJ:.8f} m/s\n"
    )

    f.write(
        f"PeleC measured wave speed = "
        f"{D_sim:.8f} m/s\n"
    )

    f.write(
        f"Percent difference = "
        f"{percent_difference:.6f} %\n"
    )

    f.write(
        f"Absolute percent difference = "
        f"{absolute_percent_difference:.6f} %\n"
    )

    f.write(
        f"R^2 = "
        f"{r_squared:.8f}\n"
    )


# ============================================================
# PLOT FRONT POSITION VS TIME
# ============================================================

plt.figure(figsize=(8, 6))

plt.plot(
    times,
    front_positions,
    "o",
    markersize=4,
    label="PeleC front position"
)

plt.plot(
    t_fit,
    x_prediction,
    "-",
    label=(
        f"Linear fit: "
        f"D = {D_sim:.1f} m/s"
    )
)

# Theoretical CJ trajectory from initial interface
x_cj = (
    X_INITIAL +
    D_CJ * times
)

plt.plot(
    times,
    x_cj,
    "--",
    label=(
        f"Theoretical CJ: "
        f"{D_CJ:.1f} m/s"
    )
)

plt.xlabel("Time [s]")
plt.ylabel("Leading pressure-front position [m]")
plt.title("PeleC Wave-Front Propagation")

plt.legend()
plt.grid()

plt.tight_layout()

position_plot = (
    RESULTS_DIR /
    "wave_front_position_vs_time.png"
)

plt.savefig(
    position_plot,
    dpi=300
)

plt.close()


# ============================================================
# PLOT INSTANTANEOUS SPEED VS TIME
# ============================================================

plt.figure(figsize=(8, 6))

plt.plot(
    instantaneous_time,
    instantaneous_speed,
    "o-",
    markersize=4,
    label="PeleC interval speed"
)

plt.axhline(
    D_CJ,
    linestyle="--",
    label=(
        f"Theoretical CJ = "
        f"{D_CJ:.1f} m/s"
    )
)

plt.xlabel("Time [s]")
plt.ylabel("Wave speed [m/s]")
plt.title("PeleC Wave-Speed History")

plt.legend()
plt.grid()

plt.tight_layout()

speed_plot = (
    RESULTS_DIR /
    "wave_speed_vs_time.png"
)

plt.savefig(
    speed_plot,
    dpi=300
)

plt.close()


# ============================================================
# FINISHED
# ============================================================

print("Files written:")
print(f"  {csv_file}")
print(f"  {speed_csv}")
print(f"  {summary_file}")
print(f"  {position_plot}")
print(f"  {speed_plot}")
print()
