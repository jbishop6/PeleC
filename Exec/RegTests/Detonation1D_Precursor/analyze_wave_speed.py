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

# First-frame search window around the initial interface
INITIAL_SEARCH_MIN = 0.0045
INITIAL_SEARCH_MAX = 0.0070

# Maximum physically allowed wave speed for tracking
# This is intentionally much larger than D_CJ so we do not
# force the answer to be near the CJ speed.
MAX_SPEED_ALLOWED = 10000.0   # m/s

# Stop tracking before the wave reaches the right boundary
X_STOP = 0.090

# Ignore the earliest startup transient when fitting speed
FIT_START_TIME = 1.0e-8


# ============================================================
# FIND PLOTFILES
# ============================================================

plotfiles = sorted(
    (
        p for p in RESULTS_DIR.glob("plt*")
        if p.is_dir()
    ),
    key=lambda p: int(p.name.replace("plt", ""))
)

print("\nFirst 20 plotfiles in analysis order:")

for p in plotfiles[:20]:
    print(p.name)

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

previous_front = None
previous_time = None

pressure_field = None

for n, plotfile in enumerate(plotfiles):

    ds = yt.load(str(plotfile))

    # --------------------------------------------------------
    # Simulation time
    # --------------------------------------------------------
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

    pressure = np.asarray(cg[pressure_field])

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
    #
    # Use the raw AMReX/PeleC coordinates.
    # The precursor input defines x = 0.0 to 0.10 m.
    # Avoid yt unit conversion here because it may apply
    # BoxLib default unit assumptions.
    # --------------------------------------------------------
    
    xlo = float(ds.domain_left_edge[0])
    xhi = float(ds.domain_right_edge[0])
    
    nx = len(p_x)
    
    dx = (xhi - xlo) / nx
    
    x = xlo + (np.arange(nx) + 0.5) * dx

# --------------------------------------------------------
# Track the SAME wave front from one plotfile to the next
# --------------------------------------------------------

    if previous_front is None:
    
        # First plotfile:
        # search only near the known initial interface
        mask = (
            (x >= INITIAL_SEARCH_MIN) &
            (x <= INITIAL_SEARCH_MAX)
        )
    
    else:
    
        # Time elapsed since previous plotfile
        dt_local = time - previous_time
    
        # Maximum distance the front could reasonably move forward
        max_forward_distance = MAX_SPEED_ALLOWED * dt_local
        
       # Always search several cells ahead because the front may move
        # less than one grid cell between closely spaced plotfiles.
        min_forward_distance = 8.0 * dx
        
        forward_distance = max(
            max_forward_distance,
            min_forward_distance
        )
        
        # Allow only ONE cell behind the previous front for numerical
        # jitter. The physical leading front should propagate rightward.
        search_min = previous_front - dx
        
        search_max = previous_front + forward_distance
            
        mask = (
            (x >= search_min) &
            (x <= search_max)
        )
    
    # Make sure the search window actually contains cells
    if not np.any(mask):
        raise RuntimeError(
            f"No cells found in tracking window at t={time:.6e} s"
        )
    
    x_search = x[mask]
    p_search = p_x[mask]
    
    # Need at least a few points for gradient calculation
    if len(x_search) < 3:
        raise RuntimeError(
            f"Tracking window too small at t={time:.6e} s"
        )
    
   # Pressure gradient
    dpdx = np.gradient(p_search, x_search)
    
    # --------------------------------------------------------
    # Identify the LEADING strong pressure drop
    # --------------------------------------------------------
    
    # Most negative pressure gradient in this local window
    min_gradient = np.min(dpdx)

    # If there is no negative pressure gradient in the search window,
    # then our assumed right-moving leading pressure drop is absent.
    if min_gradient >= 0.0:
        candidate_indices = np.array([], dtype=int)
    
    else:
        gradient_threshold = 0.25 * min_gradient
    
        candidate_indices = np.where(
            dpdx <= gradient_threshold
        )[0]

    if len(candidate_indices) == 0:
        print(
            f"\nNo pressure-front candidates found at "
            f"t = {time:.6e} s."
        )
        
        if previous_front is None:
            print("Previous front position = None (no front tracked yet)")
        else:
            print(
                f"Previous front position = "
                f"{previous_front:.6f} m"
            )
        
        print(
            f"Search window = "
            f"{x_search.min():.6f} to {x_search.max():.6f} m"
        )
        print(
            f"Minimum dp/dx = "
            f"{min_gradient:.6e}"
        )
        print(
            "Stopping front tracking at this point."
        )
        break
        
    # The detonation front is the LEADING/rightmost strong
    # pressure drop.
    shock_index = candidate_indices[-1]
    
    x_front = x_search[shock_index]
    p_front = p_search[shock_index]
    
    # --------------------------------------------------------
    # Enforce right-moving front
    # --------------------------------------------------------

    if previous_front is not None:
    
        # Allow at most one cell of apparent backward motion
        # due to spatial discretization.
        if x_front < previous_front - dx:
            raise RuntimeError(
                f"Front moved backward unexpectedly at "
                f"t={time:.6e} s: "
                f"{previous_front:.6f} -> {x_front:.6f} m"
            )
    
        # Save this timestep's result
        times.append(time)
        front_positions.append(x_front)
        front_pressures.append(p_front)
        
        # Update tracker for the next plotfile
        previous_front = x_front
        previous_time = time
        
        print(
            f"{plotfile.name:12s}  "
            f"t = {time:12.5e} s   "
            f"x_front = {x_front:10.6f} m"
        )
    
    # Stop before the front interacts with the right boundary
    if x_front >= X_STOP:
        print(
            f"\nFront reached x = {x_front:.6f} m. "
            "Stopping before right-boundary interaction."
        )
        break
# ============================================================
# CONVERT TO NUMPY ARRAYS
# ============================================================

times = np.asarray(times)
front_positions = np.asarray(front_positions)
front_pressures = np.asarray(front_pressures)


# ============================================================
# REMOVE t = 0 / EARLY TRANSIENT FOR FIT
# ============================================================

fit_mask = (
    (times >= FIT_START_TIME) &
    (front_positions < X_STOP)
)

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
