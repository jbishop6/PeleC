file(REMOVE_RECURSE
  ".5"
  "libsundials_cvode.pdb"
  "libsundials_cvode.so"
  "libsundials_cvode.so.5"
  "libsundials_cvode.so.5.7.0"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_cvode_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
