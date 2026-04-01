file(REMOVE_RECURSE
  ".5"
  "libsundials_ida.pdb"
  "libsundials_ida.so"
  "libsundials_ida.so.5"
  "libsundials_ida.so.5.7.0"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_ida_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
