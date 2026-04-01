file(REMOVE_RECURSE
  ".3.7.0"
  "libsundials_sunlinsolband.pdb"
  "libsundials_sunlinsolband.so"
  "libsundials_sunlinsolband.so.3.7.0"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_sunlinsolband_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
