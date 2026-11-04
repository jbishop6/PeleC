
import cantera as ct
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# --------------------------------------------------
# USER SETTINGS
# --------------------------------------------------

MECHANISM = "/home/jbishop6/PeleC/Submodules/PelePhysics/Mechanisms/BaurleEklund_C2H4_7s3r_Ar/mechanism.yaml"

# --------------------------------------------------
# FROZEN POST-SHOCK CONDITIONS
# --------------------------------------------------

T0 = 1630.2099       # K
P0 = 35.0512e5       # Pa

COMPOSITION = "C2H4:1 O2:3 N2:11.28"

# Same fresh mixture as PeleC
COMPOSITION = "C2H4:1, O2:3, N2:11.28"

# Start with a longer window than the PeleC run.
END_TIME = 1.0e-2      # 10 milliseconds
MAX_TIME_STEP = 1.0e-6 # 1 microsecond

# --------------------------------------------------
# INITIALIZE GAS
# --------------------------------------------------

gas = ct.Solution(MECHANISM)
gas.TPX = T0, P0, COMPOSITION

print("Mechanism:", MECHANISM)
print("Initial temperature:", gas.T, "K")
print("Initial pressure:", gas.P / 1e5, "bar")
print("Initial ethylene mass fraction:",
      gas["C2H4"].Y[0])

# --------------------------------------------------
# CREATE ADIABATIC CONSTANT-VOLUME REACTOR
# --------------------------------------------------

reactor = ct.IdealGasReactor(gas, energy="on")
network = ct.ReactorNet([reactor])
network.max_time_step = MAX_TIME_STEP

times = [0.0]
temperatures = [reactor.T]
pressures = [reactor.thermo.P]
fuel = [reactor.thermo["C2H4"].Y[0]]

# --------------------------------------------------
# INTEGRATE CHEMISTRY
# --------------------------------------------------

while network.time < END_TIME:

    network.step()

    times.append(network.time)
    temperatures.append(reactor.T)
    pressures.append(reactor.thermo.P)
    fuel.append(reactor.thermo["C2H4"].Y[0])

times = np.array(times)
temperatures = np.array(temperatures)
pressures = np.array(pressures)
fuel = np.array(fuel)

# --------------------------------------------------
# CALCULATE IGNITION DELAY
# --------------------------------------------------

dTdt = np.gradient(temperatures, times)
index = np.argmax(dTdt)

tau_ign = times[index]
peak_dTdt = dTdt[index]
temperature_rise = temperatures.max() - T0

print("\n===================================")
print("IGNITION DELAY RESULTS")
print("===================================")
print(f"Maximum temperature: {temperatures.max():.2f} K")
print(f"Temperature rise: {temperature_rise:.2f} K")
print(f"Peak dT/dt: {peak_dTdt:.4e} K/s")

if temperature_rise >= 100.0 and peak_dTdt > 0:
    print(f"Ignition delay: {tau_ign:.6e} s")
    print(f"Ignition delay: {tau_ign * 1e6:.3f} microseconds")
else:
    print("No clear ignition detected in the simulation window.")
    print("The maximum dT/dt is not a reliable ignition delay.")

# --------------------------------------------------
# PLOT RESULTS
# --------------------------------------------------

fig, axes = plt.subplots(3, 1, figsize=(9, 10), sharex=True)

time_ms = times * 1e3

axes[0].plot(time_ms, temperatures)
axes[0].set_ylabel("Temperature [K]")

axes[1].plot(time_ms, pressures / 1e5)
axes[1].set_ylabel("Pressure [bar]")

axes[2].plot(time_ms, fuel)
axes[2].set_ylabel("Ethylene mass fraction")
axes[2].set_xlabel("Time [ms]")

for ax in axes:
    ax.grid(True)

plt.tight_layout()
plt.savefig("ignition_delay_profiles.png", dpi=300)

print("\nSaved ignition_delay_profiles.png")
