package com.example.iot.service;

import com.example.iot.model.SensorRecord;
import com.example.iot.repository.SensorRecordRepository;
import com.example.iot.tcp.DeviceConnectionManager;
import org.springframework.stereotype.Service;

import java.time.format.DateTimeFormatter;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

@Service
public class SensorService {

    private static final DateTimeFormatter FORMATTER = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

    private final SensorRecordRepository repository;
    private final DeviceConnectionManager connectionManager;
    private final AlertService alertService;

    public SensorService(SensorRecordRepository repository,
                         DeviceConnectionManager connectionManager,
                         AlertService alertService) {
        this.repository = repository;
        this.connectionManager = connectionManager;
        this.alertService = alertService;
    }

    public long save(int projectId, String projectName, Map<String, Object> sensorData) {
        long id = repository.save(projectId, projectName, sensorData);
        try {
            alertService.detectAndSave(projectId, projectName, sensorData);
        } catch (Exception ignored) {}
        return id;
    }

    public Map<String, Object> latest(int projectId) {
        // 设备可能分多条报文上报（如环境传感器、设备状态各一条），取最近若干条合并字段，
        // 这样温度/湿度和模式/风扇速度等能在同一张"实时数据"卡片里一起显示。
        List<SensorRecord> recent = repository.findRecentByProjectId(projectId, 10);
        if (recent.isEmpty()) {
            Map<String, Object> empty = new LinkedHashMap<>();
            empty.put("project", projectId);
            empty.put("online", connectionManager.isOnline(projectId));
            empty.put("tcpConnected", connectionManager.isTcpConnected(projectId));
            empty.put("timestamp", "");
            appendLastCmd(empty, projectId);
            return empty;
        }
        // findRecentByProjectId 按时间倒序返回：从旧到新合并，新值覆盖旧值
        Map<String, Object> merged = new LinkedHashMap<>();
        for (int i = recent.size() - 1; i >= 0; i--) {
            SensorRecord r = recent.get(i);
            if (r.sensorData() != null) merged.putAll(r.sensorData());
        }
        SensorRecord latest = recent.get(0); // 最新一条，取其 id / 时间
        Map<String, Object> result = new LinkedHashMap<>(merged);
        result.put("id", latest.id());
        result.put("project", latest.projectId());
        result.put("name", latest.projectName());
        result.put("timestamp", latest.createdAt() == null ? "" : latest.createdAt().format(FORMATTER));
        result.put("online", connectionManager.isOnline(projectId));
        result.put("tcpConnected", connectionManager.isTcpConnected(projectId));
        appendLastCmd(result, projectId);
        return result;
    }

    public List<Map<String, Object>> history(int projectId, int limit) {
        boolean online = connectionManager.isOnline(projectId);
        return repository.findRecentByProjectId(projectId, limit)
                .stream()
                .map(record -> toApiMap(record, online))
                .toList();
    }

    public long count() {
        return repository.count();
    }

    /** 附带最近一条已被设备取走执行的命令回执，供前端确认排队命令是否执行 */
    private void appendLastCmd(Map<String, Object> result, int projectId) {
        DeviceConnectionManager.Delivery d = connectionManager.getLastDelivered(projectId);
        if (d != null) {
            result.put("lastCmd", d.cmd());
            result.put("lastCmdSeq", d.seq());
            result.put("lastCmdAt", d.deliveredAtMs());
        }
    }

    private Map<String, Object> toApiMap(SensorRecord record, boolean online) {
        Map<String, Object> result = new LinkedHashMap<>(record.sensorData());
        result.put("id", record.id());
        result.put("project", record.projectId());
        result.put("name", record.projectName());
        result.put("timestamp", record.createdAt() == null ? "" : record.createdAt().format(FORMATTER));
        result.put("online", online);
        result.put("tcpConnected", connectionManager.isTcpConnected(record.projectId()));
        appendLastCmd(result, record.projectId());
        return result;
    }
}
