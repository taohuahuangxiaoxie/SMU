#pragma once

#include <vector>
#include <cstddef>

/**
 * @file Compression.h
 * @brief SmuUtils 压缩和解压缩功能接口
 */

namespace SmuUtils::Compression {

/**
 * @brief 压缩算法类型
 */
enum class Algorithm {
    Zstd = 0  // Zstandard 压缩算法
};

/**
 * @brief 压缩数据
 * @param data 待压缩的数据
 * @param algorithm 压缩算法，默认为 Zstd
 * @param level 压缩级别 (1-22)，默认值为 3。级别越高压缩率越高但速度越慢
 * @return 压缩后的数据。如果压缩失败，返回空向量
 */
std::vector<char> Compress(const std::vector<char>& data, 
                          Algorithm algorithm = Algorithm::Zstd, 
                          int level = 3);

/**
 * @brief 解压数据（已知原始大小）
 * @param compressed 压缩的数据
 * @param originalSize 原始数据大小
 * @param algorithm 压缩算法，默认为 Zstd
 * @return 解压后的数据。如果解压失败，返回空向量
 * @note 如果已知原始大小，使用此函数性能更好
 */
std::vector<char> Decompress(const std::vector<char>& compressed, 
                             size_t originalSize,
                             Algorithm algorithm = Algorithm::Zstd);

/**
 * @brief 解压数据（自动检测原始大小）
 * @param compressed 压缩的数据
 * @param algorithm 压缩算法，默认为 Zstd
 * @return 解压后的数据。如果解压失败，返回空向量
 * @note 此函数会自动从压缩帧头中获取原始大小，如果无法获取则使用流式解压
 */
std::vector<char> DecompressAuto(const std::vector<char>& compressed,
                                 Algorithm algorithm = Algorithm::Zstd);

} // namespace SmuUtils::Compression

