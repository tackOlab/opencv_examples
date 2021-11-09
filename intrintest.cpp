#include <x86intrin.h>

#include <cstdint>
#include <cstdio>

// Check windows
#if _MSC_VER
#if _WIN32 || _WIN64
#if _WIN64
#define ENV64BIT
#else
#define ENV32BIT
#endif
#endif
// Check GCC
#elif defined(__GNUC__)
#if defined(__x86_64__)
#define ENV64BIT
#else
#define ENV32BIT
#endif
#endif

uint32_t lzcnt(const uint32_t x) {
#if defined(_MSC_VER)
  return __lzcnt(x);
#elif defined(__MINGW32__) || defined(__MINGW64__)
  return __builtin_clz(x);
#elif defined(__ARM_FEATURE_CLZ)
  return __clz(x);
#else
#if defined(__AVX2__)
  return _lzcnt_u32(x);
#else
  return 31 - int_log2(x);
#endif
#endif
}

int main() {
#if defined(ENV64BIT)
  printf(" [TRACE] 64 Bits - x86_64\n");
#else
  printf(" [TRACE] 32 Bits - x86\n");
#endif
  uint32_t val = 0;
  printf("lzcnt of %u = %u\n", val, lzcnt(val));
  val = 1;
  for (int i = 0; i < 4; i++) {
    printf("lzcnt of %u = %u\n", val << i, lzcnt(val << i));
  }
}