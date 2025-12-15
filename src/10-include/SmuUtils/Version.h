#pragma once

/**
 * @file Version.h
 * @brief SmuUtils 版本信息接口
 */

namespace SmuUtils {

/**
 * @brief 获取版本字符串
 * @return 版本字符串，格式为 "主版本号.次版本号.修订号"，例如 "1.0.0"
 */
const char* GetVersionString();

/**
 * @brief 获取主版本号
 * @return 主版本号
 */
int GetVersionMajor();

/**
 * @brief 获取次版本号
 * @return 次版本号
 */
int GetVersionMinor();

/**
 * @brief 获取修订号
 * @return 修订号
 */
int GetVersionPatch();

} // namespace SmuUtils

