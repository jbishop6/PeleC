# Basic settings
DIM = 3          # PeleC is multi-D only; do 1D by setting Ny=Nz=1 in inputs
COMP = gnu
PRECISION = DOUBLE

USE_MPI = FALSE
USE_OMP  = FALSE
DEBUG    = FALSE

# Physics for a Sod-like case (no chemistry)
Eos_Model := GammaLaw
Chemistry_Model := Null
Transport_Model := Constant   # or Simple

# Standard boilerplate
Bpack := ./Make.package
Blocs := .
PELE_HOME := ../../..
include $(PELE_HOME)/Exec/Make.PeleC
