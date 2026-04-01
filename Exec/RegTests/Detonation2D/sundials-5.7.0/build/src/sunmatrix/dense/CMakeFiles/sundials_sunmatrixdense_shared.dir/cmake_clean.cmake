file(REMOVE_RECURSE
  ".3"
  "libsundials_sunmatrixdense.pdb"
  "libsundials_sunmatrixdense.so"
  "libsundials_sunmatrixdense.so.3"
  "libsundials_sunmatrixdense.so.3.7.0"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_sunmatrixdense_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
