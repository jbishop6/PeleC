#!/bin/bash

# Brown 1999 GRI-Mech validation case:
# T = 1291 K
# P = 3.31 atm
# Experimental induction time = 30.4 us

RESULTS_DIR="validation_Brown1999_GRI_1291K_3.31atm_30.4us_2026-09-27_163146"

FEXTRACT=~/PeleC/Submodules/PelePhysics/Submodules/amrex/Tools/Plotfile/fextract.gnu.ex
FTIME=~/PeleC/Submodules/PelePhysics/Submodules/amrex/Tools/Plotfile/ftime.gnu.ex

# Make sure we are pointing to the correct results directory
if [ ! -d "$RESULTS_DIR" ]; then
    echo "ERROR: Results directory not found:"
    echo "$RESULTS_DIR"
    exit 1
fi

echo "Extracting temperature history from:"
echo "$RESULTS_DIR"

# Create the CSV INSIDE the validation results directory
echo "time_s,temp_K" > "$RESULTS_DIR/temperature_history.csv"

for plt in "$RESULTS_DIR"/plt*; do
    if [ -d "$plt" ]; then

        # Get simulation time
        time=$($FTIME "$plt" | awk '{print $4}')

        # Extract Temp from this plotfile
        $FEXTRACT -v Temp -s "${plt}.slice" "$plt" > /dev/null

        # Grab Temp from the first spatial point
        temp=$(awk '!/^#/ {print $2; exit}' "${plt}.slice")

        # Save time and temperature
        echo "$time,$temp" >> "$RESULTS_DIR/temperature_history.csv"

        # Remove temporary slice file
        rm "${plt}.slice"
    fi
done

echo "Done."
echo "Temperature history saved to:"
echo "$RESULTS_DIR/temperature_history.csv"
