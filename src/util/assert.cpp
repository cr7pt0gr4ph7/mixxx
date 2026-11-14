#include "util/assert.h"

#include <QtDebug>

#ifdef MIXXX_DEBUG_ASSERTIONS_ENABLED
void mixxx_debug_assert(const char* assertion, const char* file, int line, const char* function) {
    qCritical("%s: \"%s\" in function %s at %s:%d",
            kDebugAssertPrefix,
            assertion,
            function,
            file,
            line);
}

bool mixxx_debug_assert_return_true(
        const char* assertion,
        const char* file,
        int line,
        const char* function) {
    mixxx_debug_assert(assertion, file, line, function);
    return true;
}
#endif

void mixxx_release_assert(const char* assertion, const char* file, int line, const char* function) {
    qFatal("ASSERT: \"%s\" in function %s at %s:%d", assertion, function, file, line);
}
