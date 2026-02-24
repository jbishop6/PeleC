#!/usr/bin/env python3
import os
import shutil
import itertools
import re

# --- user choices -------------------------------------------------
# geometric sweep for third branch
lengths = [0.08, 0.12, 0.16]   # Z values (third-branch length)
widths  = [0.04, 0.08]         # W values (third-branch width)

# Base directory where all runs will be created
base_dir = "TB_vs_TB3"

# Filenames in the *current* Detonation2D directory
INPUT_FILE_2B = "inputs.detonation.twobranch.inp"
INPUT_FILE_3B = "inputs.detonation.threebranch.inp"
EXECUTABLE_NAME = "PeleC2d.gnu.ex"   # change if your exe name is different

# Any extra static files PeleC needs (add to this list)
COMMON_FILES = [
    EXECUTABLE_NAME,
    "probin",    # if you use probin; delete if not
    # add other shared files here if needed
]

# ------------------------------------------------------------------
os.makedirs(base_dir, exist_ok=True)


def edit_geometry_inputs(run_dir, Z_val, W_val, input_filename):
    """
    Edit geometry parameters + plotfile path inside the given run directory.

    Assumes your input file has lines like:
        geo.Z = ...
        geo.W = ...
    and an amr.plot_file line.
    """
    path = os.path.join(run_dir, input_filename)

    if not os.path.exists(path):
        raise FileNotFoundError(f"Input file '{input_filename}' not found in {run_dir}")

    with open(path, "r") as f:
        content = f.read()

    # Set geometry parameters: Z_val -> geo.Z, W_val -> geo.W
    content = re.sub(r'geo\.Z\s*=\s*[\d.eE+-]+', f'geo.Z = {Z_val}', content)
    content = re.sub(r'geo\.W\s*=\s*[\d.eE+-]+', f'geo.W = {W_val}', content)

    # Redirect plot output into this run directory: <run_dir>/plt...
    plot_path = os.path.abspath(os.path.join(run_dir, "plt"))
    content = re.sub(
        r'amr\.plot_file\s*=\s*\S+',
        f'amr.plot_file = {plot_path}',
        content
    )

    with open(path, "w") as f:
        f.write(content)

    print(f"[INFO] Updated {path}: geo.Z={Z_val}, geo.W={W_val}, plot_file={plot_path}")


def make_job_script(path, job_name, input_filename, exe_name):
    """
    Create a Slurm batch script in 'path' for this case.
    Customize the SBATCH options and module loads to match Incline.
    """
    script_path = os.path.join(path, "run_job.sh")
    with open(script_path, "w") as f:
        f.write(f"""#!/bin/bash
#SBATCH --job-name={job_name}
#SBATCH --output={job_name}.out
#SBATCH --error={job_name}.err
#SBATCH --time=04:00:00
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8

module load pelec   # <-- CUSTOMIZE: modules you actually use

# run the code
srun ./{exe_name} {input_filename}
""")
    os.chmod(script_path, 0o755)


# keep track of all job script paths so we can write a submit_all.sh
all_job_scripts = []


def setup_run_dir(run_dir, input_file_source, input_file_target_name):
    """
    Create run_dir and copy required files from the current Detonation2D folder.
    """
    # fresh directory
    if os.path.exists(run_dir):
        shutil.rmtree(run_dir)
    os.makedirs(run_dir, exist_ok=True)

    # copy common files (exe, probin, etc.)
    for fname in COMMON_FILES:
        if os.path.exists(fname):
            shutil.copy(fname, os.path.join(run_dir, os.path.basename(fname)))
        else:
            print(f"[WARN] Common file '{fname}' not found in current directory")

    # copy the appropriate input file
    shutil.copy(input_file_source, os.path.join(run_dir, input_file_target_name))


# main loop over (Z, W)
for Z_val, W_val in itertools.product(lengths, widths):
    case_tag = f"L{Z_val:.3f}_W{W_val:.3f}"

    # ----------------- 2-branch case -----------------
    run2 = os.path.join(base_dir, f"run_2B_{case_tag}")
    setup_run_dir(run2, INPUT_FILE_2B, INPUT_FILE_2B)

    # patch geometry + plotfile path
    edit_geometry_inputs(run2, Z_val, W_val, INPUT_FILE_2B)
    make_job_script(run2, f"2B_{case_tag}", INPUT_FILE_2B, EXECUTABLE_NAME)
    all_job_scripts.append(os.path.join(run2, "run_job.sh"))

    # ----------------- 3-branch case -----------------
    run3 = os.path.join(base_dir, f"run_3B_{case_tag}")
    setup_run_dir(run3, INPUT_FILE_3B, INPUT_FILE_3B)

    # patch geometry + plotfile path
    edit_geometry_inputs(run3, Z_val, W_val, INPUT_FILE_3B)
    make_job_script(run3, f"3B_{case_tag}", INPUT_FILE_3B, EXECUTABLE_NAME)
    all_job_scripts.append(os.path.join(run3, "run_job.sh"))


# write a master submit script
submit_all = os.path.join(base_dir, "submit_all.sh")
with open(submit_all, "w") as f:
    f.write("#!/bin/bash\n\n")
    for script in all_job_scripts:
        # This cd / sbatch / cd - pattern keeps sbatch paths clean
        f.write(
            f"cd {os.path.dirname(script)} && sbatch {os.path.basename(script)} && cd - > /dev/null\n"
        )
os.chmod(submit_all, 0o755)

print("All cases created. To submit everything:")
print(f"  cd {base_dir}")
print("  ./submit_all.sh")
