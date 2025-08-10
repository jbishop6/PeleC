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

# --- do NOT use SUNDIALS for this case (must be before include) ---
USE_SUNDIALS   = FALSE
PELE_USE_MAGMA = FALSE
PELE_USE_KLU   = FALSE

# keep PelePhysics from pulling BlackBoxFunction
EXCLUDE_PKGS += Source/Utility/BlackBoxFunction

# case glue
Bpack := ./Make.package
Blocs := .
PELE_HOME := ../../..
include $(PELE_HOME)/Exec/Make.PeleC

# --------- post-include: purge anything SUNDIALS sneaks in ----------

# drop any macro that might trigger AMReX SUNDIALS
DEFINES := $(filter-out -DAMREX_USE_SUNDIALS,$(DEFINES))

# remove AMReX Extern/SUNDIALS directory from header search & VPATH
INCLUDE_LOCATIONS := $(filter-out $(AMREX_HOME)/Src/Extern/SUNDIALS,$(INCLUDE_LOCATIONS))
VPATH_LOCATIONS   := $(filter-out $(AMREX_HOME)/Src/Extern/SUNDIALS,$(VPATH_LOCATIONS))

# remove any other sundials paths/libs/rpaths
INCLUDE_LOCATIONS := $(filter-out %/sundials/%,$(INCLUDE_LOCATIONS))
LIB_LOCATIONS     := $(filter-out %/sundials/% %/ThirdParty/INSTALL/%,$(LIB_LOCATIONS))
LIBRARIES         := $(filter-out -lsundials_%,$(LIBRARIES))
LDFLAGS           := $(filter-out -Wl,-rpath,%/sundials/% -Wl,-rpath,%/ThirdParty/INSTALL/%,$(LDFLAGS))

# exclude ALL reactor + AMReX sundials TUs by full path (no guessing)
EXCLUDE_FILES += \
  $(PELE_PHYSICS_HOME)/Source/Reactions/ReactorCvode.cpp \
  $(PELE_PHYSICS_HOME)/Source/Reactions/ReactorCvodeUtils.cpp \
  $(PELE_PHYSICS_HOME)/Source/Reactions/ReactorCvodeJacobian.cpp \
  $(PELE_PHYSICS_HOME)/Source/Reactions/ReactorCvodePreconditioner.cpp \
  $(PELE_PHYSICS_HOME)/Source/Reactions/ReactorCvodeCustomLinSolver.cpp \
  $(PELE_PHYSICS_HOME)/Source/Reactions/ReactorBDF.cpp \
  $(PELE_PHYSICS_HOME)/Source/Reactions/ReactorArkode.cpp \
  $(PELE_PHYSICS_HOME)/Source/Reactions/ReactorRK64.cpp \
  $(AMREX_HOME)/Src/Extern/SUNDIALS/AMReX_Sundials_Core.cpp \
  $(AMREX_HOME)/Src/Extern/SUNDIALS/AMReX_SUNMemory.cpp \
  $(AMREX_HOME)/Src/Extern/SUNDIALS/AMReX_NVector_MultiFab.cpp

# also strip whole directories from the auto source lists (belt & suspenders)
CEXE_sources := $(filter-out %/Source/Reactions/%,$(CEXE_sources))
CEXE_headers := $(filter-out %/Source/Reactions/%,$(CEXE_headers))
CEXE_sources := $(filter-out %/Src/Extern/SUNDIALS/%,$(CEXE_sources))
CEXE_headers := $(filter-out %/Src/Extern/SUNDIALS/%,$(CEXE_headers))
