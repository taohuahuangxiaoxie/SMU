#pragma once

/**
 * @file SmuUtils.h
 * @brief SmuUtils 主头文件，包含所有功能模块
 * 
 * 使用示例：
 * @code
 * #include <SmuUtils/SmuUtils.h>
 * 
 * // 获取版本信息
 * auto version = SmuUtils::GetVersionString();
 * 
 * // 压缩数据
 * std::vector<char> data = {...};
 * auto compressed = SmuUtils::Compression::Compress(data, SmuUtils::Compression::Algorithm::Zstd);
 * 
 * // 解压数据
 * auto decompressed = SmuUtils::Compression::DecompressAuto(compressed, SmuUtils::Compression::Algorithm::Zstd);
 * @endcode
 */

#include "Version.h"
#include "Compression.h"

