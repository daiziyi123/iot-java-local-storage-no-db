package com.example.iot.controller;

import com.example.iot.tcp.CommunicationLogManager;
import com.example.iot.tcp.DeviceConnectionManager;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

/**
 * [增强] TCP 通信日志 API，供前端展示 TCP 服务器与板子之间的收发记录。
 */
@RestController
@RequestMapping("/api")
public class CommunicationController {

    private final CommunicationLogManager commLogManager;
    private final DeviceConnectionManager connectionManager;

    public CommunicationController(CommunicationLogManager commLogManager,
                                   DeviceConnectionManager connectionManager) {
        this.commLogManager = commLogManager;
        this.connectionManager = connectionManager;
    }

    @GetMapping("/comm-logs")
    public ResponseEntity<List<Map<String, Object>>> logs(
            @RequestParam(defaultValue = "100") int limit) {
        List<CommunicationLogManager.LogEntry> entries = commLogManager.getLogs();
        List<Map<String, Object>> result = new ArrayList<>();
        // entries 按时间升序，前端要最新的先，倒序
        int start = Math.max(0, entries.size() - Math.min(limit, 200));
        for (int i = entries.size() - 1; i >= start; i--) {
            CommunicationLogManager.LogEntry e = entries.get(i);
            Map<String, Object> m = new LinkedHashMap<>();
            m.put("time", e.time());
            m.put("timestamp", e.timestampMs());
            m.put("direction", e.direction() == CommunicationLogManager.Direction.RECEIVED ? "RECEIVED" : "SENT");
            m.put("directionLabel", e.direction() == CommunicationLogManager.Direction.RECEIVED ? "← 收" : "→ 发");
            m.put("projectId", e.projectId());
            m.put("remoteAddr", e.remoteAddr());
            m.put("content", e.content());
            result.add(m);
        }
        return ResponseEntity.ok(result);
    }

    @GetMapping("/comm-logs/clear")
    public ResponseEntity<Map<String, Object>> clear() {
        commLogManager.clear();
        return ResponseEntity.ok(Map.of("status", "ok"));
    }

    /**
     * [增强] 后端直接向板子的 TCP socket 写原始数据。
     * 板子 ESP32-C3 会通过 UART 以 +IPD 形式收到。
     *
     * 调用方式（任选）：
     *   POST /api/tcp-send?projectId=0&data=xxx      （query param，需要 URL 编码）
     *   POST /api/tcp-send  body: {"projectId":0,"data":"xxx"}  （JSON body，推荐）
     */
    @PostMapping("/tcp-send")
    public ResponseEntity<Map<String, Object>> tcpSend(
            @RequestParam(value = "projectId", required = false) Integer queryProjectId,
            @RequestParam(value = "data", required = false) String queryData,
            @RequestBody(required = false) Map<String, Object> body) {

        int projectId = queryProjectId != null ? queryProjectId : 0;
        String data = queryData;

        // 如果 query param 没给，从 JSON body 里取
        if (data == null && body != null) {
            Object pidObj = body.get("projectId");
            if (pidObj != null) projectId = Integer.parseInt(String.valueOf(pidObj));
            Object dataObj = body.get("data");
            if (dataObj != null) data = String.valueOf(dataObj);
        }

        if (data == null || data.isEmpty()) {
            return ResponseEntity.status(400).body(Map.of(
                    "status", "error",
                    "message", "缺少 data 参数，用法: POST /api/tcp-send?projectId=0&data=xxx"
            ));
        }

        String remoteAddr = connectionManager.getRemoteAddr(projectId);
        if (remoteAddr == null) {
            return ResponseEntity.status(503).body(Map.of(
                    "status", "error",
                    "message", "设备离线，projectId=" + projectId
            ));
        }

        try {
            connectionManager.sendRaw(projectId, data);
            commLogManager.addLog(CommunicationLogManager.Direction.SENT,
                    projectId, remoteAddr, data);
            return ResponseEntity.ok(Map.of(
                    "status", "ok",
                    "sent", data,
                    "remote", remoteAddr
            ));
        } catch (Exception e) {
            return ResponseEntity.status(500).body(Map.of(
                    "status", "error",
                    "message", e.getMessage()
            ));
        }
    }
}
