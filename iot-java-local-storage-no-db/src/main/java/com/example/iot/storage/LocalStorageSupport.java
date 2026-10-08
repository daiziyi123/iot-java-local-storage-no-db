package com.example.iot.storage;

import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;
import jakarta.annotation.PostConstruct;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.AtomicMoveNotSupportedException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.nio.file.StandardOpenOption;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * 本地文件持久化支持。
 *
 * 不连接 MySQL / RDS，不需要数据库账号、白名单或数据库权限。
 * 配置、告警、设备会话保存为 JSON；高频传感器数据使用 JSONL 追加写入。
 */
@Component
public class LocalStorageSupport {

    private static final Logger log = LoggerFactory.getLogger(LocalStorageSupport.class);

    private final ObjectMapper objectMapper;
    private final Path root;

    public LocalStorageSupport(ObjectMapper objectMapper,
                               @Value("${iot.storage.dir:./data}") String storageDir) {
        this.objectMapper = objectMapper;
        this.root = Path.of(storageDir).toAbsolutePath().normalize();
    }

    @PostConstruct
    public void init() {
        try {
            Files.createDirectories(root);
            log.info("[Storage] 本地数据目录已就绪: {}", root);
        } catch (IOException e) {
            throw new IllegalStateException("无法创建本地数据目录: " + root, e);
        }
    }

    public String displayPath() {
        return root.toString();
    }

    public boolean isWritable() {
        try {
            Files.createDirectories(root);
            return Files.isWritable(root);
        } catch (Exception e) {
            return false;
        }
    }

    public Path path(String fileName) {
        return root.resolve(fileName).normalize();
    }

    public <T> List<T> readArray(String fileName, TypeReference<List<T>> type) {
        Path file = path(fileName);
        if (!Files.exists(file)) return new ArrayList<>();
        try {
            List<T> result = objectMapper.readValue(file.toFile(), type);
            return result == null ? new ArrayList<>() : new ArrayList<>(result);
        } catch (Exception e) {
            log.warn("[Storage] 读取 {} 失败，将按空数据处理: {}", file, e.getMessage());
            return new ArrayList<>();
        }
    }

    public void writeArray(String fileName, Collection<?> data) {
        Path file = path(fileName);
        Path tmp = path(fileName + ".tmp");
        try {
            Files.createDirectories(root);
            objectMapper.writerWithDefaultPrettyPrinter().writeValue(tmp.toFile(), data);
            try {
                Files.move(tmp, file, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tmp, file, StandardCopyOption.REPLACE_EXISTING);
            }
        } catch (Exception e) {
            try { Files.deleteIfExists(tmp); } catch (Exception ignored) {}
            throw new IllegalStateException("写入本地数据文件失败: " + file, e);
        }
    }

    public <T> List<T> readJsonLines(String fileName, Class<T> type, int maxRecords) {
        Path file = path(fileName);
        if (!Files.exists(file)) return new ArrayList<>();

        int safeMax = Math.max(1, maxRecords);
        ArrayDeque<T> ring = new ArrayDeque<>(safeMax);
        try (BufferedReader reader = Files.newBufferedReader(file, StandardCharsets.UTF_8)) {
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.isBlank()) continue;
                try {
                    T value = objectMapper.readValue(line, type);
                    if (ring.size() >= safeMax) ring.removeFirst();
                    ring.addLast(value);
                } catch (Exception badLine) {
                    log.warn("[Storage] 跳过损坏的 JSONL 数据行: {}", badLine.getMessage());
                }
            }
        } catch (IOException e) {
            log.warn("[Storage] 读取 {} 失败: {}", file, e.getMessage());
        }
        return new ArrayList<>(ring);
    }

    public void appendJsonLine(String fileName, Object value) {
        Path file = path(fileName);
        try {
            Files.createDirectories(root);
            String json = objectMapper.writeValueAsString(value);
            Files.writeString(file, json + System.lineSeparator(), StandardCharsets.UTF_8,
                    StandardOpenOption.CREATE, StandardOpenOption.WRITE, StandardOpenOption.APPEND);
        } catch (Exception e) {
            throw new IllegalStateException("追加本地传感器记录失败: " + file, e);
        }
    }

    public void rewriteJsonLines(String fileName, Collection<?> values) {
        Path file = path(fileName);
        Path tmp = path(fileName + ".tmp");
        try {
            Files.createDirectories(root);
            try (BufferedWriter writer = Files.newBufferedWriter(tmp, StandardCharsets.UTF_8,
                    StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING, StandardOpenOption.WRITE)) {
                for (Object value : values) {
                    writer.write(objectMapper.writeValueAsString(value));
                    writer.newLine();
                }
            }
            try {
                Files.move(tmp, file, StandardCopyOption.REPLACE_EXISTING, StandardCopyOption.ATOMIC_MOVE);
            } catch (AtomicMoveNotSupportedException e) {
                Files.move(tmp, file, StandardCopyOption.REPLACE_EXISTING);
            }
        } catch (Exception e) {
            try { Files.deleteIfExists(tmp); } catch (Exception ignored) {}
            throw new IllegalStateException("压缩本地传感器数据失败: " + file, e);
        }
    }
}
