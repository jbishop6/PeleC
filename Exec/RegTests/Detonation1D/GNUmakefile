cat > GNUmakefile << 'EOF'
# --- basic build knobs ---
DIM = 3
COMP = gnu
PRECISION = DOUBLE
USE_MPI = FALSE
USE_OMP = FALSE
DEBUG = FALSE

# --- physics choices ---
Eos_Model := Fuego
Chemistry_Model := Davis
Transport_Model := Simple

# --- keep SUNDIALS libs OFF for this case ---
USE_SUNDIALS = FALSE

# --- exclude packages we don't need ---
EXCLUDE_PKGS += Reactions SUNDIALS BlackBoxFunction

# --- standard glue (do NOT override Blocs here) ---
Bpack := ./Make.package
Blocs := .
PELE_HOME := ../../..
include $(PELE_HOME)/Exec/Make.PeleC

# Add SUNDIALS *headers only* so PelePhysics headers that mention them can compile
INCLUDE_LOCATIONS += -I$(PELE_HOME)/Submodules/PelePhysics/Submodules/sundials/install/include
INCLUDE_LOCATIONS += -I$(PELE_HOME)/Submodules/PelePhysics/Submodules/sundials/include
EOF
