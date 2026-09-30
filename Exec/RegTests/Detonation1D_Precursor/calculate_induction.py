import numpy as np

# Read extracted PeleC temperature history
data = np.genfromtxt(
    "temperature_history.csv",
    delimiter=",",
    skip_header=1
)

t = data[:, 0]       # seconds
T = data[:, 1]       # Kelvin

# Calculate temperature rise rate
dTdt = np.gradient(T, t)

# Schultz & Shepherd induction-time definition:
# time corresponding to maximum dT/dt
i = np.argmax(dTdt)

tau_s = t[i]
tau_us = tau_s * 1.0e6

print(f"Induction time: {tau_s:.8e} s")
print(f"Induction time: {tau_us:.3f} us")
print(f"Temperature at max dT/dt: {T[i]:.3f} K")
print(f"Maximum dT/dt: {dTdt[i]:.8e} K/s")
