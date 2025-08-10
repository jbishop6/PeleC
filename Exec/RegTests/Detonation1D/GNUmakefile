# --- basic build knobs ---
DIM = 3
COMP = gnu
PRECISION = DOUBLE
USE_MPI = FALSE
USE_OMP  = FALSE
DEBUG    = FALSE

# --- physics choices (you can flip to Davis later) ---
Eos_Model        := Fuego
Chemistry_Model  := Null
Transport_Model  := Simple

# --- point to the *installed* SUNDIALS, not the source tree ---
PELE_HOME := ../../..
# These three must come BEFORE including Make.PeleC
SUNDIALS_HOME := $(PELE_HOME)/Submodules/PelePhysics/ThirdParty/INSTALL/gnu
USE_SUNDIALS  = TRUE
PELE_USE_KLU  = FALSE
PELE_USE_MAGMA= FALSE

# standard glue
Bpack := ./Make.package
Blocs := .
include $(PELE_HOME)/Exec/Make.PeleC

# ---- after include: make sure only the installed headers/libs are used ----
# drop any source-tree sundials include dirs that might have crept in
INCLUDE_LOCATIONS := $(filter-out %/Submodules/sundials/include,$(INCLUDE_LOCATIONS))
# keep installed include (SUNDIALS_HOME/include) at the end for clarity
INCLUDE_LOCATIONS += $(SUNDIALS_HOME)/include
# ensure we link only against installed libs
LIB_LOCATIONS := $(filter-out %/Submodules/sundials/lib%,$(LIB_LOCATIONS))
LIB_LOCATIONS += $(SUNDIALS_HOME)/lib
