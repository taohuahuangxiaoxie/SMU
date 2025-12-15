#pragma once

/**
 * @file Utility.h
 * @brief Utility 主头文件，包含所有功能模块
 * 
 * 使用示例：
 * @code
 * #include <Utility/Utility.h>
 * 
 * // 获取版本信息
 * auto version = Utility::GetVersionString();
 * 
 * // 压缩数据
 * std::vector<char> data = {...};
 * auto compressed = Utility::Compression::Compress(data, Utility::Compression::Algorithm::Zstd);
 * 
 * // 解压数据
 * auto decompressed = Utility::Compression::DecompressAuto(compressed, Utility::Compression::Algorithm::Zstd);
 * @endcode
 */

#include "Version.h"
#include "Compression.h"

