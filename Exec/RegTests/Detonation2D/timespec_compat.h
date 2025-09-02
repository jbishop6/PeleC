#pragma once
// Minimal compatibility shim for older glibc headers when used with newer libstdc++.
// We only provide a declaration so <ctime>'s `using ::timespec_get;` finds a symbol.
// We do NOT define the function to avoid linkage/storage-class conflicts.
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Redeclaring the same prototype is safe; different storage class (e.g., 'static')
// would not be — so we avoid that entirely here.
int timespec_get(struct timespec* ts, int base);

#ifdef __cplusplus
}
#endif
