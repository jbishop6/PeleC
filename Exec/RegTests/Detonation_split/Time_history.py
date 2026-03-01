import pandas as pd
import matplotlib.pyplot as plt
import numpy as np


def find_finite(numbers,limit):
    # Filter out infinite values and values larger than "limit"
    finite_numbers = [n if n < limit else float('nan') for n in numbers]
    return finite_numbers

# 1. Load the data
# We skip the '#' symbol but use the rest of the first line as the header
filename = 'probe_data_2d.dat'

# 1. Manually extract the header since pandas ignores it with comment='#'
with open(filename, 'r') as f:
    header_line = f.readline()
    # Remove '#' and leading/trailing whitespace, then split by space
    column_names = header_line.lstrip('#').strip().split()

# 2. Read the data, skipping the first row (the header) 
# We use names=column_names to restore the headers
df = pd.read_csv(filename, sep=r'\s+', comment='#', names=column_names)

# Adjust the indices below if your 'ncomp' differs.
time = df.iloc[:, 0]

# Adjust these labels/indices based on your specific 'ncomp' value
p0_temp = df.filter(like='P0_Temp') # or df.iloc[:, some_index]
p1_temp = df.filter(like='P1_Temp')
p0_pres = df.filter(like='P0_Pressure')
p1_pres = df.filter(like='P1_Pressure')
p0_rhoE = df.filter(like='P0_rho_E')
p1_rhoE = df.filter(like='P1_rho_E')
p0_rho  = df.filter(like='P0_density')
p1_rho  = df.filter(like='P1_density')
p0_rhoU = df.filter(like='P0_xmom')
p1_rhoU = df.filter(like='P1_xmom')
# Data that need further maniplutation, I chose to turn into np.array
p0_temp = p0_temp.iloc[:, :1].to_numpy()
p1_temp = p1_temp.iloc[:, :1].to_numpy()
p0_pres = p0_pres.iloc[:, :1].to_numpy()
p1_pres = p1_pres.iloc[:, :1].to_numpy()
p0_rho  = p0_rho.iloc[:, :1].to_numpy()
p1_rho  = p1_rho.iloc[:, :1].to_numpy()
p0_rhoU = p0_rhoU.iloc[:, :1].to_numpy()
p1_rhoU = p1_rhoU.iloc[:, :1].to_numpy()
p0_temp  = p0_temp.reshape((-1,))
p1_temp  = p1_temp.reshape((-1,))
p0_pres  = p0_pres.reshape((-1,))
p1_pres  = p1_pres.reshape((-1,))
p0_rho  = p0_rho.reshape((-1,))
p1_rho  = p1_rho.reshape((-1,))
p0_rhoU = p0_rhoU.reshape((-1,))
p1_rhoU = p1_rhoU.reshape((-1,))
# Clean up non-physical numbers due to negative energy
p0_temp  = [n if n > 0 else float('nan') for n in p0_temp]
p1_temp  = [n if n > 0 else float('nan') for n in p1_temp]
p0_pres = find_finite(p0_pres,5e6)
p1_pres = find_finite(p1_pres,5e6)
p0_rho  = find_finite(p0_rho,0.205e-3)
p1_rho  = find_finite(p1_rho,0.25e-3)
p0_rho  = [n if n > 0 else float('nan') for n in p0_rho]
p1_rho  = [n if n > 0 else float('nan') for n in p1_rho]
p0_U    = np.divide(p0_rhoU, p0_rho)
p1_U    = np.divide(p1_rhoU, p1_rho)

# Make plots ----------------------------------------------
# Make first figure
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 8), sharex=True)

# Plot Temperatures
ax1.plot(time, p0_temp, label='Probe 0 Temp', color='red')
ax1.plot(time, p1_temp, label='Probe 1 Temp', color='darkblue')
ax1.set_ylabel('Temperature [K]')
ax1.set_ylim(0,4000)
ax1.legend()
ax1.grid(True, linestyle='--')

# Plot Pressures
ax2.plot(time, p0_pres, label='Probe 0 Pressure', color='red')
ax2.plot(time, p1_pres, label='Probe 1 Pressure', color='darkblue')
ax2.set_xlabel('Time [s]')
ax2.set_ylabel('Pressure [Ba]')
ax2.set_ylim(0,5e5)
ax2.legend()
ax2.grid(True, linestyle='--')

# Plot rho_E
ax3.plot(time, p0_rhoE, label='Probe 0 rho E', color='red')
ax3.plot(time, p1_rhoE, label='Probe 1 rho E', color='darkblue')
ax3.set_ylabel('rho E [erg/cm^3]')
ax3.legend()
ax3.grid(True, linestyle='--')

plt.tight_layout()
plt.savefig('probe_plot.png')
# plt.show()
plt.close()

# Make second figure
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 8), sharex=True)

# Plot density
ax1.plot(time, p0_rho, label='Probe 0 rho', color='red')
ax1.plot(time, p1_rho, label='Probe 1 rho', color='darkblue')
ax1.set_ylabel('Density [g/cm^3]')
ax1.set_ylim(0,0.2e-3)
ax1.legend()
ax1.grid(True, linestyle='--')

# Plot xmom
ax2.plot(time, p0_rhoU, label='Probe 0 rho U', color='red')
ax2.plot(time, p1_rhoU, label='Probe 1 rho U', color='darkblue')
ax2.set_xlabel('Time [s]')
ax2.set_ylabel('x-momentum [g-cm/s]')
# ax2.set_ylim(0,5e5)
ax2.legend()
ax2.grid(True, linestyle='--')

# Plot x-vel
ax3.plot(time, p0_U, label='Probe 0 U', color='red')
ax3.plot(time, p1_U, label='Probe 1 U', color='darkblue')
ax3.set_ylabel('x-velocity [cm/s]')
ax3.legend()
ax3.grid(True, linestyle='--')

plt.tight_layout()
plt.savefig('probe_plot_2.png')
# plt.show()
plt.close()
