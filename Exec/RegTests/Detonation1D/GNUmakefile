# ------------------------------
# Basic build knobs
# ------------------------------
DIM       = 3
COMP      = gnu
PRECISION = DOUBLE
USE_MPI   = FALSE
USE_OMP   = FALSE
DEBUG     = FALSE

# ------------------------------
# Physics choices (no chemistry)
# ------------------------------
Eos_Model        := Fuego
Chemistry_Model  := Null
Transport_Model  := Simple

# ------------------------------
# Disable SUNDIALS entirely (must be BEFORE include)
# ------------------------------
USE_SUNDIALS = FALSE
PELE_USE_KLU = FALSE
PELE_USE_MAGMA = FALSE

# ------------------------------
# Exclude all SUNDIALS and Reactions source
# ------------------------------
EXCLUDE_PKGS += $(AMREX_HOME)/Src/Extern/SUNDIALS
EXCLUDE_PKGS += $(PELE_PHYSICS_HOME)/Source/Reactions
EXCLUDE_FILES += AMReX_Sundials_Core.cpp AMReX_SUNMemory.cpp AMReX_NVector_MultiFab.cpp \
                 ReactorCvode.cpp ReactorCvodeUtils.cpp ReactorCvodeJacobian.cpp \
                 ReactorCvodePreconditioner.cpp ReactorCvodeCustomLinSolver.cpp \
                 ReactorBDF.cpp ReactorArkode.cpp ReactorRK64.cpp

# ------------------------------
# Standard glue
# ------------------------------
Bpack := ./Make.package
Blocs := .
PELE_HOME := ../../..
include $(PELE_HOME)/Exec/Make.PeleC

# ------------------------------
# Scrub any SUNDIALS leftovers after include
# ------------------------------
DEFINES := $(filter-out -DAMREX_USE_SUNDIALS,$(DEFINES))
INCLUDE_LOCATIONS := $(filter-out %/sundials/% %/Src/Extern/SUNDIALS%,$(INCLUDE_LOCATIONS))
LIB_LOCATIONS     := $(filter-out %/sundials/% %/ThirdParty/INSTALL/%,$(LIB_LOCATIONS))
RPATH_LOCATIONS   := $(filter-out %/sundials/% %/ThirdParty/INSTALL/%,$(RPATH_LOCATIONS))
LIBRARIES         := $(filter-out -lsundials_%,$(LIBRARIES))
CEXE_sources := $(filter-out %/Source/Reactions/% %/Src/Extern/SUNDIALS/%,$(CEXE_sources))
CEXE_headers := $(filter-out %/Source/Reactions/% %/Src/Extern/SUNDIALS/%,$(CEXE_headers))

# Force RPATH off entirely
USE_RPATH = FALSE
