file(REMOVE_RECURSE
  ".3"
  "libsundials_sunmatrixsparse.pdb"
  "libsundials_sunmatrixsparse.so"
  "libsundials_sunmatrixsparse.so.3"
  "libsundials_sunmatrixsparse.so.3.7.0"
)

# Per-language clean rules from dependency scanning.
foreach(lang C)
  include(CMakeFiles/sundials_sunmatrixsparse_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
