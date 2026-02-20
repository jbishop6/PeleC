import os
import glob
import argparse
import yt
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages


def load_ray(fname):

    print(f"Loading: {fname}")
    fields = [
        "x",
        "y",
        "z",
        "density",
        "Temp",
        "Y(HO2)",
    ]
    ds = yt.load(fname)
    lengths = ds.domain_right_edge - ds.domain_left_edge

    start = [
        ds.domain_left_edge[0] + 0.5 * lengths[0],
        ds.domain_left_edge[1] + 0.5 * lengths[1],
        ds.domain_left_edge[2],
    ]
    end = [
        ds.domain_left_edge[0] + 0.5 * lengths[0],
        ds.domain_left_edge[1] + 0.5 * lengths[1],
        ds.domain_right_edge[2],
    ]
    ray = ds.r[start:end]
    srt = np.argsort(ray[("boxlib", "x")])

    df = pd.DataFrame({f: np.array(ray[("boxlib", f)][srt]) for f in fields})
    df.sort_values(by=["z"], inplace=True)
    return df


if __name__ == "__main__":

    pltfiles_10 = sorted(glob.glob(os.path.join(".", "plt10_*")))
    df_10 = load_ray(pltfiles_10[-1])

    pltfiles_01 = sorted(glob.glob(os.path.join(".", "plt01_*")))
    df_01 = load_ray(pltfiles_01[-1])

    z10_max = df_10.loc[df_10["Y(HO2)"].idxmax(), "z"]
    z01_max = df_01.loc[df_01["Y(HO2)"].idxmax(), "z"]
    print(f"flame location for dx = 10 dy {z10_max}")
    print(f"flame location for dx = 0.1 dy {z01_max}")

    fname = "plots.pdf"
    with PdfPages(fname) as pdf:
        plt.figure("Temp")
        plt.plot(df_10.z, df_10.Temp, label="dx / 10 = dy = dz")
        plt.plot(df_01.z, df_01.Temp, label="10 dx = dy = dz", ls="--")
        plt.xlabel("z (cm)")
        plt.ylabel("T (K)")
        plt.xlim([2.5, 3.5])
        plt.legend(frameon=False)
        plt.tight_layout()
        pdf.savefig(dpi=300)

        plt.figure("Y(HO2)")
        plt.plot(df_10.z, df_10["Y(HO2)"], label="dx / 10 = dy = dz")
        plt.plot(df_01.z, df_01["Y(HO2)"], label="10 dx = dy = dz", ls="--")
        plt.xlabel("z (cm)")
        plt.ylabel("Y(HO2) (-)")
        plt.xlim([2.5, 3.5])
        plt.legend(frameon=False)
        plt.tight_layout()
        pdf.savefig(dpi=300)
