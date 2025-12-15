#include "Utility/Compression.h"
#include "zstd/zstd.h"
#include <cstring>

namespace Utility::Compression {

// 内部辅助函数：流式解压（Zstd）
static std::vector<char> DecompressStreamingZstd(const std::vector<char>& compressed);

// Zstd 压缩实现
static std::vector<char> CompressZstd(const std::vector<char>& data, int level) {
    if (data.empty()) {
        return std::vector<char>();
    }

    // 验证压缩级别
    if (level < 1 || level > ZSTD_maxCLevel()) {
        level = 3; // 使用默认级别
    }

    // 计算压缩后的最大大小
    size_t const dstCapacity = ZSTD_compressBound(data.size());
    std::vector<char> dst(dstCapacity);

    // 执行压缩
    size_t const compressedSize = ZSTD_compress(
        dst.data(), dstCapacity,
        data.data(), data.size(),
        level
    );

    // 检查压缩是否成功
    if (ZSTD_isError(compressedSize)) {
        return std::vector<char>();
    }

    // 调整向量大小到实际压缩后的大小
    dst.resize(compressedSize);
    return dst;
}

// Zstd 解压实现（已知原始大小）
static std::vector<char> DecompressZstd(const std::vector<char>& compressed, size_t originalSize) {
    if (compressed.empty() || originalSize == 0) {
        return std::vector<char>();
    }

    std::vector<char> dst(originalSize);

    // 执行解压
    size_t const decompressedSize = ZSTD_decompress(
        dst.data(), originalSize,
        compressed.data(), compressed.size()
    );

    // 检查解压是否成功
    if (ZSTD_isError(decompressedSize)) {
        return std::vector<char>();
    }

    // 验证解压后的大小是否匹配
    if (decompressedSize != originalSize) {
        return std::vector<char>();
    }

    return dst;
}

// Zstd 解压实现（自动检测原始大小）
static std::vector<char> DecompressAutoZstd(const std::vector<char>& compressed) {
    if (compressed.empty()) {
        return std::vector<char>();
    }

    // 尝试从压缩帧头获取原始大小
    unsigned long long const frameContentSize = ZSTD_getFrameContentSize(
        compressed.data(), compressed.size()
    );

    if (ZSTD_isError(frameContentSize)) {
        return std::vector<char>();
    }

    // 如果无法从帧头获取大小，使用流式解压
    if (frameContentSize == ZSTD_CONTENTSIZE_UNKNOWN) {
        return DecompressStreamingZstd(compressed);
    }

    if (frameContentSize == ZSTD_CONTENTSIZE_ERROR) {
        return std::vector<char>();
    }

    // 已知原始大小，直接解压
    std::vector<char> dst(static_cast<size_t>(frameContentSize));

    size_t const decompressedSize = ZSTD_decompress(
        dst.data(), frameContentSize,
        compressed.data(), compressed.size()
    );

    if (ZSTD_isError(decompressedSize)) {
        return std::vector<char>();
    }

    if (static_cast<size_t>(decompressedSize) != static_cast<size_t>(frameContentSize)) {
        return std::vector<char>();
    }

    dst.resize(decompressedSize);
    return dst;
}

// Zstd 流式解压实现
static std::vector<char> DecompressStreamingZstd(const std::vector<char>& compressed) {
    // 创建解压流
    ZSTD_DStream* dstream = ZSTD_createDStream();
    if (dstream == nullptr) {
        return std::vector<char>();
    }

    // 初始化解压流
    size_t initResult = ZSTD_initDStream(dstream);
    if (ZSTD_isError(initResult)) {
        ZSTD_freeDStream(dstream);
        return std::vector<char>();
    }

    std::vector<char> dst;
    const size_t outBufferSize = ZSTD_DStreamOutSize();
    std::vector<char> outBuffer(outBufferSize);

    ZSTD_inBuffer input = {compressed.data(), compressed.size(), 0};
    ZSTD_outBuffer output = {outBuffer.data(), outBufferSize, 0};

    // 循环解压直到完成
    while (input.pos < input.size) {
        output.pos = 0;

        size_t const ret = ZSTD_decompressStream(dstream, &output, &input);

        if (ZSTD_isError(ret)) {
            ZSTD_freeDStream(dstream);
            return std::vector<char>();
        }

        // 将解压的数据追加到结果中
        if (output.pos > 0) {
            dst.insert(dst.end(), outBuffer.begin(), outBuffer.begin() + output.pos);
        }

        // 如果返回0，表示帧已完全解压
        if (ret == 0) {
            break;
        }
    }

    ZSTD_freeDStream(dstream);
    return dst;
}

// 公共接口实现
std::vector<char> Compress(const std::vector<char>& data, Algorithm algorithm, int level) {
    switch (algorithm) {
        case Algorithm::Zstd:
            return CompressZstd(data, level);
        default:
            return std::vector<char>();
    }
}

std::vector<char> Decompress(const std::vector<char>& compressed, size_t originalSize, Algorithm algorithm) {
    switch (algorithm) {
        case Algorithm::Zstd:
            return DecompressZstd(compressed, originalSize);
        default:
            return std::vector<char>();
    }
}

std::vector<char> DecompressAuto(const std::vector<char>& compressed, Algorithm algorithm) {
    switch (algorithm) {
        case Algorithm::Zstd:
            return DecompressAutoZstd(compressed);
        default:
            return std::vector<char>();
    }
}

} // namespace Utility::Compression
