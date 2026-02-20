import pandas as pd
import numpy as np
import os
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib._pylab_helpers import Gcf
from scipy.special import erf


def get_ncell(ngrid, factor):
    return np.array(([ngrid] * 3)) * np.array(factor)


npoints = [16, 32, 64, 128]
factors = [[1, 1, 1], [2, 1, 1], [1, 2, 1], [1, 1, 2]]

# for grid in grid:
alpha = 7.5362000690846e00

pltfile = "plt00400"

futils_path = "../../../Build/Submodules/PelePhysics/Submodules/amrex/Tools/Plotfile"
fextract = f"{futils_path}/amrex_fextract"

for factor in factors:
    error = {}
    for ng_num, ngrid in enumerate(npoints):
        os.system("rm -r plt*0")
        ncell = get_ncell(ngrid, factor)
        ncell_str = " ".join(str(x) for x in ncell)
        error[ncell_str] = {}
        os.system(
            f"mpirun -n 8 ./PeleC3d.llvm.MPI.ex masscons-isothermal.inp amr.n_cell={ncell_str}"
        )
        for idir in ["0", "1"]:
            os.system(f"{fextract} -d {idir} -s slice{idir} {pltfile}")
            fname = f"slice{idir}"

            with open(fname, "r") as f:
                junk = f.readline()
                timetext = f.readline()
                time = float(timetext.split()[-1])

            # remove first 2 lines, and # sign on third line
            with open(fname, "r") as f:
                lines = f.readlines()
            lines = [lines[2].lstrip("#").lstrip()] + lines[3:]
            with open(fname, "w") as f:
                f.writelines(lines)

            data = pd.read_csv(fname, sep="\\s+")

            coord = {"0": "x", "1": "y", "2": "z"}[idir]

            data["zeta1"] = (data[coord] + 0.5) / np.sqrt(4 * alpha * time)
            data["zeta2"] = (0.5 - data[coord]) / np.sqrt(4 * alpha * time)

            dT = {"0": 100, "1": 50, "2": 30}[idir]

            data["Tdelta2"] = -dT * erf(data["zeta2"])
            data["Tdelta1"] = dT * erf(data["zeta1"])
            data["Ttrue"] = 700.0 + data["Tdelta2"] + data["Tdelta1"]

            plt.figure(f"{idir}-{factor}")
            plt.plot(
                data[coord],
                data["Temp"],
                "-",
                label=f"dir = {idir}, [nx, ny, nz] = {ncell_str}",
            )
            if ng_num == len(npoints) - 1:
                plt.plot(data[coord], data["Ttrue"], "k:", label="analytical")
            plt.legend(frameon=False)
            plt.xlabel(coord + " (cm)")
            plt.ylabel("T (K)")
            error[ncell_str][idir + "max"] = np.max(
                np.abs(data["Ttrue"] - data["Temp"])
            )  # np.linalg.norm( data['Ttrue'  ] - data['Temp'], ord=2) / np.sqrt(len(data[coord]))
            error[ncell_str][idir + "mse"] = np.linalg.norm(
                data["Ttrue"] - data["Temp"], ord=2
            ) / np.sqrt(len(data[coord]))
            error[ncell_str]["nx"] = ncell[0]
            error[ncell_str]["ny"] = ncell[1]

    error = pd.DataFrame(error).T
    print(error)
    plt.figure(f"error-{factor}")
    plt.loglog(error.nx, error["0mse"], "r-o", label="x-direction")
    plt.loglog(error.ny, error["1mse"], "b-o", label="y-direction")
    plt.loglog(error.nx, 100 * (1.0 / error.nx) ** 2, "k:", label="2nd order")
    plt.ylabel("Error")
    plt.xlabel("N")
    plt.xlim([8, 512])
    plt.legend(frameon=False)

fname = "isothermal-plots.pdf"
with PdfPages(fname) as pdf:
    for manager in Gcf.get_all_fig_managers():
        fig = manager.canvas.figure
        pdf.savefig(fig, dpi=300)
