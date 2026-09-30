#!/bin/bash

# ============================================================
# Calculate induction time for Brown 1999 Baurle validation case
# Definition: time corresponding to maximum dT/dt
# ============================================================

RESULTS_DIR="$HOME/PeleC/Exec/RegTests/Detonation1D_Precursor/validation_Brown1999_Baurle_1291K_3.31atm_30.4us_2026-09-30_111533"

cd "$RESULTS_DIR" || exit 1

echo "Analyzing:"
echo "$RESULTS_DIR"
echo

python3 - <<'PY'
import numpy as np

# Read PeleC temperature history
data = np.genfromtxt(
    "temperature_history.csv",
    delimiter=",",
    skip_header=1
)

t = data[:, 0]   # seconds
T = data[:, 1]   # Kelvin

# Calculate dT/dt
dTdt = np.gradient(T, t)

# Induction time = time of maximum temperature rise rate
i = np.argmax(dTdt)

tau_s = t[i]
tau_us = tau_s * 1.0e6

print("========================================")
print("Baurle-Mech Induction-Time Result")
print("========================================")
print(f"Induction time       = {tau_s:.8e} s")
print(f"Induction time       = {tau_us:.3f} us")
print(f"Temperature at tau   = {T[i]:.3f} K")
print(f"Maximum dT/dt        = {dTdt[i]:.8e} K/s")
print()
print("Brown experiment     = 30.400 us")
print(f"Difference            = {tau_us - 30.4:+.3f} us")
print(f"Ratio (model/exp)     = {tau_us / 30.4:.4f}")

# Save result
with open("induction_time_result.txt", "w") as f:
    f.write("Baurle-Mech Brown 1999 validation\n")
    f.write("T = 1291 K\n")
    f.write("P = 3.31 atm\n")
    f.write("Mixture = C2H4 + 3 O2 + 12 N2\n\n")
    f.write(f"Predicted induction time = {tau_us:.6f} us\n")
    f.write("Experimental induction time = 30.400000 us\n")
    f.write(f"Difference = {tau_us - 30.4:+.6f} us\n")
    f.write(f"Model/experiment ratio = {tau_us / 30.4:.6f}\n")
    f.write(f"Temperature at max dT/dt = {T[i]:.6f} K\n")
    f.write(f"Maximum dT/dt = {dTdt[i]:.8e} K/s\n")
PY
