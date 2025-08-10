# --- basic build knobs ---
DIM       = 3
COMP      = gnu
PRECISION = DOUBLE
USE_MPI   = FALSE
USE_OMP   = FALSE
DEBUG     = FALSE

# --- physics choices (no chemistry for this case) ---
Eos_Model       := Fuego
Chemistry_Model := Null
Transport_Model := Simple

# --- do NOT use SUNDIALS for this case ---
USE_SUNDIALS = FALSE
PELE_USE_MAGMA = FALSE
PELE_USE_KLU   = FALSE

# --- keep PelePhysics from pulling in BlackBoxFunction ---
# (must be BEFORE including Make.PeleC)
EXCLUDE_PKGS += Source/Utility/BlackBoxFunction

# --- case glue ---
Bpack := ./Make.package
Blocs := .
PELE_HOME := ../../..
include $(PELE_HOME)/Exec/Make.PeleC

# --------------- post-include sanitizing (belt & suspenders) ---------------

# Drop any SUNDIALS define a package may have set
DEFINES := $(filter-out -DAMREX_USE_SUNDIALS,$(DEFINES))

# Purge SUNDIALS headers & paths that packages might have added
INCLUDE_LOCATIONS := $(filter-out %/sundials/% %/Src/Extern/SUNDIALS%,$(INCLUDE_LOCATIONS))
LIB_LOCATIONS     := $(filter-out %/sundials/% %/ThirdParty/INSTALL/%,$(LIB_LOCATIONS))
LIBRARIES         := $(filter-out -lsundials_%,$(LIBRARIES))
LDFLAGS           := $(filter-out -Wl,-rpath,%/sundials/%,$(LDFLAGS))

# Keep ANY reactions & AMReX SUNDIALS translation units out of the build
EXCLUDE_FILES += \
  ReactorCvode.cpp ReactorCvodeUtils.cpp ReactorCvodeJacobian.cpp \
  ReactorCvodePreconditioner.cpp ReactorCvodeCustomLinSolver.cpp \
  ReactorBDF.cpp ReactorArkode.cpp ReactorRK64.cpp \
  AMReX_Sundials_Core.cpp AMReX_SUNMemory.cpp AMReX_NVector_MultiFab.cpp

# Also strip whole directories from what the case tries to compile (extra safe)
CEXE_sources := $(filter-out %/Source/Reactions/%,$(CEXE_sources))
CEXE_headers := $(filter-out %/Source/Reactions/%,$(CEXE_headers))
CEXE_sources := $(filter-out %/Src/Extern/SUNDIALS/%,$(CEXE_sources))
CEXE_headers := $(filter-out %/Src/Extern/SUNDIALS/%,$(CEXE_headers))
