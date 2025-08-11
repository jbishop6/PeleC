# --- basic build knobs ---
DIM = 3
COMP = gnu
PRECISION = DOUBLE
USE_MPI = FALSE
USE_OMP  = FALSE
DEBUG    = FALSE

# --- physics choices (no chemistry) ---
Eos_Model        := Fuego
Chemistry_Model  := Null
Transport_Model  := Simple

# --- standard glue ---
Bpack := ./Make.package
Blocs := .
PELE_HOME := ../../..
INCLUDE_LOCATIONS := $(abspath ./stubs) $(INCLUDE_LOCATIONS)
INCLUDE_LOCATIONS += $(abspath $(CURDIR)/stubs)
include $(PELE_HOME)/Exec/Make.PeleC

# ========= hard-disable BlackBoxFunction / manifold, and SUNDIALS =========

# 1) Drop the preprocessor flag that enables manifold in EOS if present
DEFINES := $(filter-out -DPELE_USE_MANIFOLD,$(DEFINES))

# 2) Remove BlackBoxFunction package paths added by Make.PeleC
UTILITY_HOME := $(PELE_PHYSICS_HOME)/Source/Utility
BboxDir := $(UTILITY_HOME)/BlackBoxFunction
Bpack := $(filter-out $(BboxDir)/Make.package,$(Bpack))
Blocs := $(filter-out $(BboxDir),$(Blocs))
INCLUDE_LOCATIONS := $(filter-out $(BboxDir),$(INCLUDE_LOCATIONS))
VPATH_LOCATIONS   := $(filter-out $(BboxDir),$(VPATH_LOCATIONS))

# 3) Ensure none of the SUNDIALS wrappers or CVODE files get compiled
EXCLUDE_FILES += AMReX_Sundials_Core.cpp AMReX_SUNMemory.cpp AMReX_NVector_MultiFab.cpp
EXCLUDE_FILES += ReactorCvode.cpp ReactorCvodeUtils.cpp ReactorCvodeJacobian.cpp \
                 ReactorCvodePreconditioner.cpp ReactorCvodeCustomLinSolver.cpp \
                 ReactorBDF.cpp ReactorArkode.cpp ReactorRK64.cpp

# 4) Nuke any SUNDIALS include/lib paths & link flags that packages might add
DEFINES          := $(filter-out -DAMREX_USE_SUNDIALS,$(DEFINES))
INCLUDE_LOCATIONS:= $(filter-out %/sundials/% %/Src/Extern/SUNDIALS/%,$(INCLUDE_LOCATIONS))
LIB_LOCATIONS    := $(filter-out %/sundials/% %/ThirdParty/INSTALL/%,$(LIB_LOCATIONS))
LIBRARIES        := $(filter-out -lsundials_%,$(LIBRARIES))
