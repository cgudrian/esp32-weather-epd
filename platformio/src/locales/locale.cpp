#include "config.h"

#define str(s) #s
#define X_LOCALE_INC(code) str(locale_ ## code.inc)
#define LOCALE_INC(code) X_LOCALE_INC(code)

#include LOCALE_INC(LOCALE)
