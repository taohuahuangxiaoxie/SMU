#include "Utility/Version.h"

namespace Utility {

// 版本号定义
#define LIBUTILITY_VERSION_MAJOR 1
#define LIBUTILITY_VERSION_MINOR 0
#define LIBUTILITY_VERSION_PATCH 0

const char* GetVersionString() {
    static const char* version = "1.0.0";
    return version;
}

int GetVersionMajor() {
    return LIBUTILITY_VERSION_MAJOR;
}

int GetVersionMinor() {
    return LIBUTILITY_VERSION_MINOR;
}

int GetVersionPatch() {
    return LIBUTILITY_VERSION_PATCH;
}

} // namespace Utility

