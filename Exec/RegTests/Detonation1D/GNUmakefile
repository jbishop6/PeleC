# Basic settings
DIM = 3
COMP = gnu
PRECISION = DOUBLE

USE_MPI = FALSE
USE_OMP = FALSE
DEBUG = FALSE

Eos_Model := Fuego
Chemistry_Model := Davis
Transport_Model := Simple

# 🛑 Place this BEFORE including Make.PeleC!
EXCLUDE_PKGS += Source/Utility/BlackBoxFunction

# Make system
Bpack := ./Make.package
Blocs := .
EXCLUDE_PKGS := BlackBoxFunction
PELE_HOME := ../../..
include $(PELE_HOME)/Exec/Make.PeleC

Blocs := . ../../../Source ../../../Source/Params/param_includes
