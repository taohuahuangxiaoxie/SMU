#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "model/model_sample.h"
#include <chrono>
#include <future>

bool runTag = true;
std::string appname = "wcdb_test";

// 设置表wcdb宏
WCDB_CPP_ORM_IMPLEMENTATION_BEGIN(ModelSample);
WCDB_CPP_SYNTHESIZE(id);
WCDB_CPP_SYNTHESIZE(name);
WCDB_CPP_SYNTHESIZE(age);
WCDB_CPP_SYNTHESIZE(email);
WCDB_CPP_SYNTHESIZE(phone);
WCDB_CPP_SYNTHESIZE(address);
WCDB_CPP_SYNTHESIZE(city);
WCDB_CPP_SYNTHESIZE(state);
WCDB_CPP_SYNTHESIZE(country);
WCDB_CPP_SYNTHESIZE(created_at);
WCDB_CPP_SYNTHESIZE(updated_at);
WCDB_CPP_SYNTHESIZE(add_1);

WCDB_CPP_PRIMARY_AUTO_INCREMENT(id);
WCDB_CPP_ORM_IMPLEMENTATION_END;

// 设置spdlog参数配置
void initlog()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);

    // 设置目录
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>
                                    ("logs/wcdb_test.log", 1024 * 1024 * 10, 3);
    file_sink->set_level(spdlog::level::info);

    auto logger = std::make_shared<spdlog::logger>
                (appname, spdlog::sinks_init_list{console_sink, file_sink});
    logger->set_level(spdlog::level::debug);

#if _WIN32
    logger->set_pattern("wcdb_test: [%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");
#else
    logger->set_pattern("wcdb_test: [%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%s:%#] %v");
#endif

    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(5));
}

void insertData(WCDB::Database &db)
{
    // 插入1000条数据
    std::vector<ModelSample> models;
    for (int i = 0; i < 1000; i++) {
        ModelSample model;
        model.id = 0;
        model.isAutoIncrement = true;
        model.name = "John Doe " + std::to_string(i);
        model.age = i;
        model.email = "john.doe@example.com " + std::to_string(i);
        model.phone = "1234567890 " + std::to_string(i);
        model.add_1 = "add_1_ " + std::to_string(i);
        models.push_back(model);
    }
    auto ret = db.insertObjects<ModelSample>(models, TABLE_NAME_SAMPLE);
    if (ret) {
        SPDLOG_INFO("插入数据成功");
    } else {
        SPDLOG_ERROR("插入数据失败");
    }
}

/**
 * @brief 查询数据
 * @param db 数据库对象
 */
void queryData(WCDB::Database &db)
{
    auto allObjects = db.getAllObjects<ModelSample>(TABLE_NAME_SAMPLE);
    int countInterval = 100;
    if (allObjects.hasValue()) {
        size_t totalCount = allObjects.value().size();
        SPDLOG_INFO("查询到数据总数: {}", totalCount);
        
        // 每隔 countInterval 条打印一次数据内容
        for (size_t i = 0; i < totalCount; i++) {
            if (i % countInterval == 0) {
                const ModelSample &model = allObjects.value()[i];
                SPDLOG_INFO("第 {} 条数据: id={}, name={}, age={}, email={}, phone={}, add1={}", 
                           i + 1, model.id, model.name, model.age, model.email, model.phone, model.add_1);
            }
        }
        
        // 打印最后一条数据（如果不是1000的倍数）
        if (totalCount > 0 && (totalCount - 1) % countInterval != 0) {
            const ModelSample &lastModel = allObjects.value()[totalCount - 1];
            SPDLOG_INFO("最后一条数据 (第 {} 条): id={}, name={}, age={}, email={}, phone={}, add1={}", 
                       totalCount, lastModel.id, lastModel.name, lastModel.age, 
                       lastModel.email, lastModel.phone, lastModel.add_1);
        }
    } else {
        SPDLOG_ERROR("查询数据失败");
    }
}

/**
 * @brief 处理数据库损坏情况
 * @param db 损坏的数据库对象
 */
void handleDatabaseCorruption(WCDB::Database &db)
{
    SPDLOG_WARN("检测到数据库损坏，开始修复流程");
    
    SPDLOG_INFO("准备调用retrieve()进行修复...");
    
    // 同步调用retrieve()进行修复
    // 注意：如果没有备份文件或文件头完全损坏，retrieve可能无法开始修复
    double recoveryRate = db.retrieve([](double progress, double increment) -> bool {
        SPDLOG_INFO("数据库修复进度回调被调用: progress={:.6f} ({:.2f}%), increment={:.6f} ({:.2f}%)", 
                   progress, progress * 100, increment, increment * 100);
        return true; // 返回true继续修复
    });
    
    SPDLOG_INFO("retrieve()调用完成，返回恢复率: {}", recoveryRate);
    if (recoveryRate > 0 && recoveryRate <= 1.0) {
        SPDLOG_INFO("数据库修复成功，恢复率: {:.2f}%", recoveryRate * 100);
        // retrieve()成功时会自动恢复表结构（备份文件包含CREATE TABLE语句）
    } else {
        // 修复失败，转存并重建数据库
        SPDLOG_WARN("数据库修复失败（恢复率: {}），开始转存并重建数据库", recoveryRate);
        if (db.deposit()) {
            if (!db.createTable<ModelSample>(TABLE_NAME_SAMPLE)) {
                SPDLOG_ERROR("创建表失败");
            }
            SPDLOG_INFO("数据库已转存并重建，表结构已自动创建");

        } else {
            SPDLOG_ERROR("数据库转存失败");
        }
    }
}

/**
 * @brief 初始化数据库保护机制
 * @param db 数据库对象
 */
void initDatabaseProtection(WCDB::Database &db)
{
    // 1. 开启自动备份
    db.enableAutoBackup(true);
    SPDLOG_INFO("已开启数据库自动备份");
    
    // 执行初始备份
    if (db.backup()) {
        SPDLOG_INFO("初始备份成功");
    } else {
        SPDLOG_WARN("初始备份失败，但继续执行");
    }
    
    // 2. 设置损坏通知回调
    db.setNotificationWhenCorrupted([](WCDB::Database &corruptedDb) {
        handleDatabaseCorruption(corruptedDb);
    });
}

int main() 
{
    initlog();

    // 初始化数据库
    WCDB::Database db("./test.db");
    
    // 初始化数据库保护机制
    initDatabaseProtection(db);
    
    // 检查数据库是否已损坏
#if 0
    if (db.isAlreadyCorrupted()) {
        SPDLOG_WARN("数据库已被标记为损坏，开始修复");
        handleDatabaseCorruption(db);
    }

    if (db.checkIfCorrupted()) {
        SPDLOG_WARN("数据库已损坏，开始修复");
        handleDatabaseCorruption(db);
    }
#endif

    // 创建表结构
    if (!db.createTable<ModelSample>(TABLE_NAME_SAMPLE)) {
        SPDLOG_ERROR("创建表失败");
        //return -1;
    }

    // 插入数据
    insertData(db);

    // 查询数据
    int count = 0;
    while (runTag) {
        count++;
        if (count % 5 == 0) {
            //insertData(db);
        }
        SPDLOG_INFO("查询数据第{}次", count);
        queryData(db);
        
       std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}