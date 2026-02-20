import pandas as pd
import numpy as np
import os
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
import numpy.testing as npt

inputs = ["mol-1", "mol-2", "plm", "ppm", "isothermal", "isothermal-whydro"]

ncells = ["8 8 8", "16 8 8", "8 16 8", "8 8 16"]
threshold = 1e-13

for ncell in ncells:
    for iname in inputs:
        os.system("rm -r datlog plt*0")
        os.system(
            f"./PeleC3d.llvm.MPI.ex masscons-{iname}.inp max_step=200 amr.n_cell={ncell}"
        )
        data = pd.read_fwf("datlog")
        data["rel_mass_change"] = (data["mass"] - data["mass"][0]) / data["mass"][0]
        plt.figure(f"mass-{ncell}")
        plt.plot(
            data.index,
            data["rel_mass_change"],
            label=f"{iname}, [nx, ny, nz] = {ncell}",
        )

        npt.assert_array_less(
            np.abs(data["rel_mass_change"]),
            threshold,
            err_msg=f"Relative mass change for test {iname} (ncell = {ncell}) exceeds {threshold}",
        )

        if iname != "isothermal-whydro":
            data["rel_energy_change"] = (data["rho_E"] - data["rho_E"][0]) / data[
                "rho_E"
            ][0]
            plt.figure(f"energy-{ncell}")
            plt.plot(
                data.index,
                data["rel_energy_change"],
                label=f"{iname}, [nx, ny, nz] = {ncell}",
            )

            npt.assert_array_less(
                np.abs(data["rel_energy_change"]),
                threshold,
                err_msg=f"Relative energy change for test {iname} (ncell = {ncell}) exceeds {threshold}",
            )


fname = "masscons-plots.pdf"
with PdfPages(fname) as pdf:
    for ncell in ncells:
        plt.figure(f"mass-{ncell}")
        plt.xlabel("Timestep")
        plt.ylabel("Relative Mass Change")
        plt.ylim([-1e-14, 1e-14])
        plt.legend(frameon=False)
        plt.tight_layout()
        pdf.savefig(dpi=300)

        plt.figure(f"energy-{ncell}")
        plt.xlabel("Timestep")
        plt.ylabel("Relative Energy Change")
        plt.ylim([-1e-14, 1e-14])
        plt.legend(frameon=False)
        plt.tight_layout()
        pdf.savefig(dpi=300)
