# AMReX
DIM = 1
COMP = gnu
PRECISION = DOUBLE

# Force SUNDIALS headers visible to every compile unit (AMReX included)
# CPPFLAGS += -I/opt/ohpc/pub/anaconda/anaconda3/pkgs/sundials-7.3.0-h13d7cbe_0/include
USE_REACT = TRUE

# Profiling
PROFILE = FALSE
TINY_PROFILE = FALSE
COMM_PROFILE = FALSE
TRACE_PROFILE = FALSE
MEM_PROFILE = FALSE
USE_GPROF = FALSE

# Performance
USE_MPI = TRUE
USE_OMP = FALSE
USE_CUDA = FALSE
USE_HIP = FALSE
USE_SYCL = FALSE
USE_EB = FALSE

# Debugging
DEBUG = FALSE
FSANITIZER = FALSE
THREAD_SANITIZER = FALSE

# PeleC
PELE_CVODE_FORCE_YCORDER = FALSE
PELE_USE_MAGMA = FALSE
PELE_COMPILE_AJACOBIAN = FALSE
Eos_Model := Fuego
Chemistry_Model := Davis
Transport_Model := Simple

# GNU Make
Bpack := ./Make.package
Blocs := .
PELE_HOME := ../../..

include $(PELE_HOME)/Exec/Make.PeleC

# GEOM_SRC := pelec-detonation/Source/Geometry.cpp
