# --- basic build knobs ---
DIM = 3
COMP = gnu
PRECISION = DOUBLE
USE_MPI = FALSE
USE_OMP  = FALSE
DEBUG    = FALSE

# --- physics choices ---
Eos_Model        := Fuego
Chemistry_Model  := Davis
Transport_Model  := Simple

# --- turn SUNDIALS off for this case ---
USE_SUNDIALS = FALSE

# --- exclude optional packages we don't need for the Sod-derived 1D case ---
EXCLUDE_PKGS += Reactions SUNDIALS BlackBoxFunction

# --- standard glue (do NOT override Blocs) ---
Bpack := ./Make.package
Blocs := .
PELE_HOME := ../../..
include $(PELE_HOME)/Exec/Make.PeleC

# Add SUNDIALS headers only (no linking) so headers that mention them still compile
INCLUDE_LOCATIONS += -I$(PELE_HOME)/Submodules/PelePhysics/Submodules/sundials/install/include
INCLUDE_LOCATIONS += -I$(PELE_HOME)/Submodules/PelePhysics/Submodules/sundials/include
