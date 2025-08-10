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
# Disable SUNDIALS entirely (MUST be before include)
# ------------------------------
USE_SUNDIALS = FALSE
PELE_USE_KLU = FALSE
PELE_USE_MAGMA = FALSE

# Exclude SUNDIALS and Reactions packages so their .cpp never enter CEXE_sources
EXCLUDE_PKGS += $(AMREX_HOME)/Src/Extern/SUNDIALS
EXCLUDE_PKGS += $(PELE_PHYSICS_HOME)/Source/Reactions

# ------------------------------
# Standard glue
# ------------------------------
Bpack := ./Make.package
Blocs := .
PELE_HOME := ../../..
include $(PELE_HOME)/Exec/Make.PeleC

# ------------------------------
# Scrub all SUNDIALS leftovers computed during include
# (AMReX precomputes rpath flags; nuke them explicitly)
# ------------------------------
DEFINES          := $(filter-out -DAMREX_USE_SUNDIALS,$(DEFINES))
INCLUDE_LOCATIONS:= $(filter-out %/sundials/% %/Extern/SUNDIALS/%,$(INCLUDE_LOCATIONS))
LIB_LOCATIONS    := $(filter-out %/sundials/% %/ThirdParty/INSTALL/%,$(LIB_LOCATIONS))
RPATH_LOCATIONS  :=
LIBRARIES        := $(filter-out -lsundials_%,$(LIBRARIES))

# These are the important ones — AMReX uses this on the link line.
amrex_rpath_flags :=
# Belt & suspenders: strip any stray -Wl,-rpath added elsewhere.
LDFLAGS   := $(filter-out -Wl,-rpath=%,$(LDFLAGS))
LINKFLAGS := $(filter-out -Wl,-rpath=%,$(LINKFLAGS))

# Also ensure no SUNDIALS/Reaction sources slipped in
CEXE_sources := $(filter-out %/Source/Reactions/% %/Src/Extern/SUNDIALS/%,$(CEXE_sources))
CEXE_headers := $(filter-out %/Source/Reactions/% %/Src/Extern/SUNDIALS/%,$(CEXE_headers))
