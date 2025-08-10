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
