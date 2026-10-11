package com.example.iot.tcp;

import org.springframework.stereotype.Component;

import java.time.Instant;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

/**
 * [修复增强] TCP 通信日志管理器：环形缓冲区保存最近 N 条收发记录，供前端展示。
 * 配合 TcpDeviceServer 在收到设备数据 / 发送响应时调用 addLog()。
 * 前端 GET /api/comm-logs 拉取最近通信。
 */
@Component
public class CommunicationLogManager {

    private static final int MAX_SIZE = 200; // 最多保留 200 条，避免内存膨胀
    private static final DateTimeFormatter FMT =
            DateTimeFormatter.ofPattern("HH:mm:ss").withZone(ZoneId.systemDefault());

    private final LinkedList<LogEntry> buffer = new LinkedList<>();

    public synchronized void addLog(Direction dir, int projectId, String remoteAddr, String content) {
        // content 可能很长（HTTP 头+body），截断到 300 字符显示
        String trimmed = content == null ? "" :
                (content.length() > 300 ? content.substring(0, 300) + "…" : content);
        buffer.add(new LogEntry(Instant.now().toEpochMilli(), dir, projectId, remoteAddr, trimmed));
        while (buffer.size() > MAX_SIZE) buffer.pollFirst();
    }

    public synchronized List<LogEntry> getLogs() {
        return new ArrayList<>(buffer);
    }

    public synchronized void clear() {
        buffer.clear();
    }

    public enum Direction { RECEIVED, SENT }

    public record LogEntry(
            long timestampMs,
            Direction direction,    // RECEIVED=收 / SENT=发
            int projectId,
            String remoteAddr,      // 如 192.168.43.19:65522
            String content          // 收发内容（截断后）
    ) {
        public String time() {
            return FMT.format(Instant.ofEpochMilli(timestampMs));
        }
    }
}
