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

# --- do NOT use sundials for this case ---
USE_SUNDIALS = FALSE

# --- exclude all SUNDIALS/Reactor translation units explicitly ---
EXCLUDE_FILES += ReactorCvode.cpp ReactorCvodeUtils.cpp ReactorCvodeJacobian.cpp \
                 ReactorCvodePreconditioner.cpp ReactorCvodeCustomLinSolver.cpp \
                 ReactorBDF.cpp ReactorArkode.cpp ReactorRK64.cpp \
                 AMReX_Sundials_Core.cpp AMReX_SUNMemory.cpp AMReX_NVector_MultiFab.cpp

# --- standard glue ---
Bpack := ./Make.package
Blocs := .
PELE_HOME := ../../..
include $(PELE_HOME)/Exec/Make.PeleC

# --- hard-disable SUNDIALS & reactions after the core includes ---
# Drop the AMReX_SUNDIALS macro if any package set it
DEFINES := $(filter-out -DAMREX_USE_SUNDIALS,$(DEFINES))

# Nuke any sundials include/lib paths that packages might have added
INCLUDE_LOCATIONS := $(filter-out %/sundials/%,$(INCLUDE_LOCATIONS))
LIB_LOCATIONS     := $(filter-out %/sundials/%,$(LIB_LOCATIONS))
LIBRARIES         := $(filter-out -lsundials_%,$(LIBRARIES))

# Make absolutely sure no PelePhysics reaction sources are compiled
CEXE_sources := $(filter-out %/Source/Reactions/%,$(CEXE_sources))
CEXE_headers := $(filter-out %/Source/Reactions/%,$(CEXE_headers))

# (Optional but helpful) also filter any AMReX SUNDIALS externs that slipped in
CEXE_sources := $(filter-out %AMReX_Sundials_% %AMReX_NVector_MultiFab.cpp,$(CEXE_sources))


# --- hard-disable SUNDIALS link step ---
USE_SUNDIALS = FALSE
PELE_USE_MAGMA = FALSE
PELE_USE_KLU   = FALSE

# Remove any SUNDIALS libraries/paths that Make.ThirdParty may have added
LIBRARIES := $(filter-out -lsundials_cvode -lsundials_arkode -lsundials_core -lsundials_nvecserial,$(LIBRARIES))
LIB_LOCATIONS := $(filter-out %/ThirdParty/INSTALL/%,$(LIB_LOCATIONS))
