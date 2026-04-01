file(REMOVE_RECURSE
  ".3.7.0"
  "libsundials_sunlinsolspfgmr.pdb"
  "libsundials_sunlinsolspfgmr.so"
  "libsundials_sunlinsolspfgmr.so.3.7.0"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_sunlinsolspfgmr_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
