# AMReX
DIM = 3
COMP = gnu
PRECISION = DOUBLE

# Profiling
PROFILE = FALSE
TINY_PROFILE = FALSE
COMM_PROFILE = FALSE
TRACE_PROFILE = FALSE
MEM_PROFILE = FALSE
USE_GPROF = FALSE

# Performance
USE_MPI = FALSE
USE_OMP = FALSE
USE_CUDA = FALSE
USE_HIP = FALSE
USE_SYCL = FALSE

# Debugging
DEBUG = FALSE
FSANITIZER = FALSE
THREAD_SANITIZER = FALSE

# PeleC
PELE_CVODE_FORCE_YCORDER = FALSE
PELE_USE_MAGMA = FALSE
PELE_COMPILE_AJACOBIAN = FALSE
Eos_Model := GammaLaw
Transport_Model := Constant
Chemistry_Model := Null

# GNU Make
Bpack := ./Make.package
Blocs := .
PELE_HOME := ../../..
include $(PELE_HOME)/Exec/Make.PeleC
PELE_USE_SUNDIALS := FALSE

EXCLUDE_FILES += ReactorCvodeCustomLinSolver.cpp
EXCLUDE_FILES += ReactorCvodeJacobian.cpp
EXCLUDE_FILES += ReactorCvodePreconditioner.cpp
EXCLUDE_FILES += ReactorCvodeUtils.cpp
EXCLUDE_FILES += AMReX_SUNMemory.cpp
EXCLUDE_FILES += AMReX_Sundials_Core.cpp
EXCLUDE_PKGS += Source/Utility/BlackBoxFunction


