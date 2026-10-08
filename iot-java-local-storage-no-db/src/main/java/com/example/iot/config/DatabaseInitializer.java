package com.example.iot.config;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

/**
 * 兼容旧工程类名。
 * 当前版本已经完全取消外部数据库依赖，此类不会创建数据库、不会连接 RDS。
 */
@Component
public class DatabaseInitializer {
    private static final Logger log = LoggerFactory.getLogger(DatabaseInitializer.class);

    public DatabaseInitializer() {
        log.info("[DB] 外部数据库功能已禁用，当前使用本地文件持久化，无需数据库访问权限。");
    }
}
