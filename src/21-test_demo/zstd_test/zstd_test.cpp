#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "zstd/zstd.h"
#include <chrono>
#include <vector>
#include <string>
#include <fstream>
#include <thread>

bool runTag = true;
std::string appname = "zstd_test";

// 设置spdlog参数配置
void initlog()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);

    // 设置目录
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>
                                    ("logs/zstd_test.log", 1024 * 1024 * 10, 3);
    file_sink->set_level(spdlog::level::info);

    auto logger = std::make_shared<spdlog::logger>
                (appname, spdlog::sinks_init_list{console_sink, file_sink});
    logger->set_level(spdlog::level::debug);

#if _WIN32
    logger->set_pattern("zstd_test: [%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
#else
    logger->set_pattern("zstd_test: [%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
#endif

    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(5));
}

/**
 * @brief 压缩数据
 * @param src 源数据
 * @param compressionLevel 压缩级别 (1-22, 默认3)
 * @return 压缩后的数据
 */
std::vector<char> compressData(const std::vector<char>& src, int compressionLevel = ZSTD_CLEVEL_DEFAULT)
{
    size_t const dstCapacity = ZSTD_compressBound(src.size());
    std::vector<char> dst(dstCapacity);
    
    auto start = std::chrono::high_resolution_clock::now();
    size_t const compressedSize = ZSTD_compress(dst.data(), dstCapacity, 
                                                 src.data(), src.size(), 
                                                 compressionLevel);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    if (ZSTD_isError(compressedSize)) {
        SPDLOG_ERROR("压缩失败: {}", ZSTD_getErrorName(compressedSize));
        return std::vector<char>();
    }
    
    dst.resize(compressedSize);
    double ratio = (1.0 - (double)compressedSize / src.size()) * 100.0;
    SPDLOG_INFO("压缩完成: 原始大小={} bytes, 压缩后大小={} bytes, 压缩率={:.2f}%, 耗时={} 微秒", 
                src.size(), compressedSize, ratio, duration.count());
    
    return dst;
}

/**
 * @brief 解压数据
 * @param src 压缩的数据
 * @param originalSize 原始数据大小
 * @return 解压后的数据
 */
std::vector<char> decompressData(const std::vector<char>& src, size_t originalSize)
{
    std::vector<char> dst(originalSize);
    
    auto start = std::chrono::high_resolution_clock::now();
    size_t const decompressedSize = ZSTD_decompress(dst.data(), originalSize,
                                                     src.data(), src.size());
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    if (ZSTD_isError(decompressedSize)) {
        SPDLOG_ERROR("解压失败: {}", ZSTD_getErrorName(decompressedSize));
        return std::vector<char>();
    }
    
    if (decompressedSize != originalSize) {
        SPDLOG_WARN("解压后大小不匹配: 期望={}, 实际={}", originalSize, decompressedSize);
    }
    
    SPDLOG_INFO("解压完成: 压缩大小={} bytes, 解压后大小={} bytes, 耗时={} 微秒",
                src.size(), decompressedSize, duration.count());
    
    return dst;
}

/**
 * @brief 测试基本压缩和解压功能
 */
void testBasicCompression()
{
    SPDLOG_INFO("========== 开始基本压缩测试 ==========");
    
    // 生成测试数据
    std::string testData = "这是一个测试数据。This is a test data. ";
    std::vector<char> originalData;
    
    // 重复数据以增加压缩效果
    for (int i = 0; i < 1000; i++) {
        originalData.insert(originalData.end(), testData.begin(), testData.end());
    }
    
    SPDLOG_INFO("原始数据大小: {} bytes", originalData.size());
    
    // 测试不同压缩级别
    for (int level = 1; level <= 5; level++) {
        SPDLOG_INFO("--- 测试压缩级别 {} ---", level);
        std::vector<char> compressed = compressData(originalData, level);
        
        if (!compressed.empty()) {
            std::vector<char> decompressed = decompressData(compressed, originalData.size());
            
            if (!decompressed.empty() && decompressed == originalData) {
                SPDLOG_INFO("压缩级别 {} 测试通过: 数据完整性验证成功", level);
            } else {
                SPDLOG_ERROR("压缩级别 {} 测试失败: 数据不匹配", level);
            }
        }
    }
    
    SPDLOG_INFO("========== 基本压缩测试完成 ==========");
}

/**
 * @brief 测试大文件压缩
 */
void testLargeDataCompression()
{
    SPDLOG_INFO("========== 开始大文件压缩测试 ==========");
    
    // 生成较大的测试数据 (10MB)
    const size_t dataSize = 10 * 1024 * 1024;
    std::vector<char> largeData(dataSize);
    
    // 填充一些模式数据
    for (size_t i = 0; i < dataSize; i++) {
        largeData[i] = static_cast<char>(i % 256);
    }
    
    SPDLOG_INFO("生成大文件数据: {} bytes ({} MB)", 
                dataSize, dataSize / (1024 * 1024));
    
    // 压缩
    std::vector<char> compressed = compressData(largeData, 3);
    
    if (!compressed.empty()) {
        // 解压
        std::vector<char> decompressed = decompressData(compressed, largeData.size());
        
        if (!decompressed.empty() && decompressed == largeData) {
            SPDLOG_INFO("大文件压缩测试通过: 数据完整性验证成功");
        } else {
            SPDLOG_ERROR("大文件压缩测试失败: 数据不匹配");
        }
    }
    
    SPDLOG_INFO("========== 大文件压缩测试完成 ==========");
}

/**
 * @brief 测试文本数据压缩
 */
void testTextCompression()
{
    SPDLOG_INFO("========== 开始文本数据压缩测试 ==========");
    
    // 读取或生成文本数据
    std::string textData = R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. 
Sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. 
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris 
nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in 
reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla 
pariatur. Excepteur sint occaecat cupidatat non proident, sunt in 
culpa qui officia deserunt mollit anim id est laborum.)";
    
    // 重复文本以增加数据量
    std::vector<char> textVector;
    for (int i = 0; i < 10000; i++) {
        textVector.insert(textVector.end(), textData.begin(), textData.end());
    }
    
    SPDLOG_INFO("文本数据大小: {} bytes", textVector.size());
    
    // 压缩
    std::vector<char> compressed = compressData(textVector, 3);
    
    if (!compressed.empty()) {
        // 解压
        std::vector<char> decompressed = decompressData(compressed, textVector.size());
        
        if (!decompressed.empty() && decompressed == textVector) {
            SPDLOG_INFO("文本数据压缩测试通过: 数据完整性验证成功");
        } else {
            SPDLOG_ERROR("文本数据压缩测试失败: 数据不匹配");
        }
    }
    
    SPDLOG_INFO("========== 文本数据压缩测试完成 ==========");
}

/**
 * @brief 显示 zstd 版本信息
 */
void showVersionInfo()
{
    unsigned version = ZSTD_versionNumber();
    const char* versionString = ZSTD_versionString();
    
    SPDLOG_INFO("ZSTD 版本号: {}", version);
    SPDLOG_INFO("ZSTD 版本字符串: {}", versionString);
    SPDLOG_INFO("最大压缩级别: {}", ZSTD_maxCLevel());
}

int main()
{
    initlog();
    
    SPDLOG_INFO("========== ZSTD 测试程序启动 ==========");
    
    // 显示版本信息
    showVersionInfo();
    
    // 运行测试
    testBasicCompression();
    testTextCompression();
    testLargeDataCompression();
    
    // 循环测试
    int count = 0;
    while (runTag) {
        count++;
        SPDLOG_INFO("========== 第 {} 次循环测试 ==========", count);
        
        testBasicCompression();
        
        std::this_thread::sleep_for(std::chrono::seconds(10));
        
        // 限制循环次数，避免无限运行
        if (count >= 5) {
            SPDLOG_INFO("达到最大循环次数，退出测试");
            break;
        }
    }
    
    SPDLOG_INFO("========== ZSTD 测试程序结束 ==========");
    
    return 0;
}

