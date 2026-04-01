file(REMOVE_RECURSE
  ".3.7.0"
  "libsundials_sunlinsolpcg.pdb"
  "libsundials_sunlinsolpcg.so"
  "libsundials_sunlinsolpcg.so.3.7.0"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_sunlinsolpcg_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
