/* engine.h for libcurl */

#include <libwolfssl/openssl/err.h>

#undef HAVE_OPENSSL_ENGINE_H

/* ENGINE_load_builtin_engines not needed, as all builtin engines are already
   loaded into memory and used on startup. */
#define ENGINE_load_builtin_engines()

