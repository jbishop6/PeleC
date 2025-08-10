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

# --- block SUNDIALS & Reactions BEFORE including Make.PeleC ---
EXCLUDE_PKGS += $(AMREX_HOME)/Src/Extern/SUNDIALS
EXCLUDE_PKGS += $(PELE_PHYSICS_HOME)/Source/Reactions

# Also exclude common SUNDIALS/React files (belt & suspenders)
EXCLUDE_FILES += AMReX_Sundials_Core.cpp AMReX_SUNMemory.cpp AMReX_NVector_MultiFab.cpp
EXCLUDE_FILES += ReactorCvode.cpp ReactorCvodeUtils.cpp ReactorCvodeJacobian.cpp \
                 ReactorCvodePreconditioner.cpp ReactorCvodeCustomLinSolver.cpp \
                 ReactorBDF.cpp ReactorArkode.cpp ReactorRK64.cpp

# --- standard glue ---
Bpack := ./Make.package
Blocs := .
PELE_HOME := ../../..
include $(PELE_HOME)/Exec/Make.PeleC

# --- strip any leftovers after include ---
AMREX_USE_SUNDIALS = FALSE
DEFINES := $(filter-out -DAMREX_USE_SUNDIALS,$(DEFINES))
INCLUDE_LOCATIONS := $(filter-out %/sundials/% %/Src/Extern/SUNDIALS%,$(INCLUDE_LOCATIONS))
LIB_LOCATIONS     := $(filter-out %/sundials/% %/ThirdParty/INSTALL/%,$(LIB_LOCATIONS))
LIBRARIES         := $(filter-out -lsundials_%,$(LIBRARIES))
CEXE_sources := $(filter-out %/Source/Reactions/% %/Src/Extern/SUNDIALS/%,$(CEXE_sources))
CEXE_headers := $(filter-out %/Source/Reactions/% %/Src/Extern/SUNDIALS/%,$(CEXE_headers))
