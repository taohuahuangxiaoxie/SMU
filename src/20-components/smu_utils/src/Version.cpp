#include "SmuUtils/Version.h"

namespace SmuUtils {

// 版本号定义
#define SMU_UTILS_VERSION_MAJOR 1
#define SMU_UTILS_VERSION_MINOR 0
#define SMU_UTILS_VERSION_PATCH 0

const char* GetVersionString() {
    static const char* version = "1.0.0";
    return version;
}

int GetVersionMajor() {
    return SMU_UTILS_VERSION_MAJOR;
}

int GetVersionMinor() {
    return SMU_UTILS_VERSION_MINOR;
}

int GetVersionPatch() {
    return SMU_UTILS_VERSION_PATCH;
}

} // namespace SmuUtils

