file(REMOVE_RECURSE
  "libsundials_generic.a"
  "libsundials_generic.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_generic_static.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
