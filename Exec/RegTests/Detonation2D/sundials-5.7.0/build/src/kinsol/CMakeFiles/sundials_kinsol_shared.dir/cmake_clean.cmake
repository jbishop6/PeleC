file(REMOVE_RECURSE
  ".5"
  "libsundials_kinsol.pdb"
  "libsundials_kinsol.so"
  "libsundials_kinsol.so.5"
  "libsundials_kinsol.so.5.7.0"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_kinsol_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
