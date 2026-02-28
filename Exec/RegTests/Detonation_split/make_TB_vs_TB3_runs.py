#!/usr/bin/env python3
import os
import shutil
import itertools
import re
import subprocess

# --- user choices -------------------------------------------------
# geometric sweep for third branch
lengths = [0.08, 0.12, 0.16]   # Z values (third-branch length)
widths  = [0.04, 0.08]         # W values (third-branch width)

# Tag so Slurm jobs mark as new config
CONFIG_TAG = "newconfig"

# Base directory where all runs will be created
base_dir = "TB_vs_TB3"

# Filenames in the *current* Detonation2D directory
INPUT_FILE_2B = "input_detonation_two_branch.inp"
INPUT_FILE_3B = "input_detonation_three_branch.inp"
EXECUTABLE_NAME = "PeleC2d.gnu.ex"   # change if your exe name is different

# Any extra static files PeleC needs (add to this list)
COMMON_FILES = [
    EXECUTABLE_NAME,
    "probin",    # comment out or remove if you don't use probin
    # add other shared files here if needed
]

# ------------------------------------------------------------------
os.makedirs(base_dir, exist_ok=True)


def get_active_jobnames():
    """
    Query Slurm for all jobs owned by this user and return their names as a list.
    """
    user = os.getenv("USER", "")
    try:
        result = subprocess.run(
            ["squeue", "-u", user, "-h", "-o", "%j"],
            check=True, capture_output=True, text=True
        )
        names = [line.strip() for line in result.stdout.splitlines() if line.strip()]
        print(f"[INFO] Active jobs for {user}: {names}")
        return names
    except Exception as e:
        print(f"[WARN] Could not query squeue for active jobs: {e}")
        return []


def case_tag_running(tag, active_names):
    """
    Return True if any active job name contains this geometry tag.
    E.g. tag="L0.080_W0.040" will match "2B_L0.080_W0.040" or similar.
    (Currently unused, but kept for reference.)
    """
    for name in active_names:
        if tag in name:
            return True
    return False


def job_running(job_name, active_names):
    """
    Return True if this exact job name is active.
    """
    return job_name in active_names


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
    """
    script_path = os.path.join(path, "run_job.sh")
    with open(script_path, "w") as f:
        f.write(f"""#!/bin/bash
#SBATCH --job-name={job_name}
#SBATCH -p compute-long
#SBATCH --output={job_name}.out
#SBATCH --error={job_name}.err
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8

# Run from the directory containing this script
cd "$(dirname "$0")"

echo "Job $SLURM_JOB_ID running on $(hostname) in $(pwd)"
date

echo "Launching ./{exe_name} {input_filename}"
srun ./{exe_name} {input_filename}

echo "Job $SLURM_JOB_ID finished at $(date)"
""")
    os.chmod(script_path, 0o755)


def setup_run_dir(run_dir, input_file_source, input_file_target_name):
    """
    Create run_dir and copy required files from the current Detonation2D folder.
    Assumes we've already checked that this specific job is not currently running.
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


# ------------------------------------------------------------
# Main: check current Slurm jobs and skip any already running
# ------------------------------------------------------------
active_names = get_active_jobnames()

# keep track of (script_path, job_name, case_tag) so we can write submit_all.sh
all_job_scripts = []

for Z_val, W_val in itertools.product(lengths, widths):
    case_tag = f"L{Z_val:.3f}_W{W_val:.3f}"

    # ----------------- 2-branch case -----------------
    job_name_2b = f"{CONFIG_TAG}_2B_{case_tag}_newconfig1"
    run2 = os.path.join(base_dir, f"run_{CONFIG_TAG}_2B_{case_tag}")

    if job_running(job_name_2b, active_names):
        print(f"[INFO] {job_name_2b} already has an active job; skipping setup for {job_name_2b}.")
    else:
        setup_run_dir(run2, INPUT_FILE_2B, INPUT_FILE_2B)
        edit_geometry_inputs(run2, Z_val, W_val, INPUT_FILE_2B)
        make_job_script(run2, job_name_2b, INPUT_FILE_2B, EXECUTABLE_NAME)
        all_job_scripts.append((os.path.join(run2, "run_job.sh"), job_name_2b, case_tag))

    # ----------------- 3-branch case -----------------
    job_name_3b = f"{CONFIG_TAG}_3B_{case_tag}_newconfig1"
    run3 = os.path.join(base_dir, f"run_{CONFIG_TAG}_3B_{case_tag}")

    if job_running(job_name_3b, active_names):
        print(f"[INFO] {job_name_3b} already has an active job; skipping setup for {job_name_3b}.")
    else:
        setup_run_dir(run3, INPUT_FILE_3B, INPUT_FILE_3B)
        edit_geometry_inputs(run3, Z_val, W_val, INPUT_FILE_3B)
        make_job_script(run3, job_name_3b, INPUT_FILE_3B, EXECUTABLE_NAME)
        all_job_scripts.append((os.path.join(run3, "run_job.sh"), job_name_3b, case_tag))

# write a master submit script
submit_all = os.path.join(base_dir, "submit_all.sh")
with open(submit_all, "w") as f:
    f.write("#!/bin/bash\n\n")
    for script_path, job_name, case_tag in all_job_scripts:
        run_dir = os.path.dirname(script_path)
        script_name = os.path.basename(script_path)
        # At submission time, check by *job name* so 2B and 3B don't block each other
        f.write(f"""# {job_name}
if squeue -u $USER -h -o "%j" | grep -q "{job_name}"; then
    echo "Skipping {job_name}: a job named {job_name} is already in the queue"
else
    cd "{run_dir}" && sbatch "{script_name}" && cd - > /dev/null
fi

""")
os.chmod(submit_all, 0o755)

print("All cases created (skipping any jobs that already have active jobs).")
print("To submit the remaining cases:")
print(f"  cd {base_dir}")
print("  ./submit_all.sh")
