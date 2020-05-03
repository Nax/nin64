#if defined(__cplusplus)
# define NIN64_API_EXTERN extern "C"
#else
# define NIN64_API_EXTERN
#endif

#if defined(WIN32) || defined(_WIN32)
# if defined(NIN64_STATIC)
#  define NIN64_API_LINKAGE
# else
#  if defined(NIN64_DLL)
#   define NIN64_API_LINKAGE __declspec(dllexport)
#  else
#   define NIN64_API_LINKAGE __declspec(dllimport)
#  endif
# endif
#else
# define NIN64_API_LINKAGE
#endif

#define NIN64_API NIN64_API_EXTERN NIN64_API_LINKAGE
