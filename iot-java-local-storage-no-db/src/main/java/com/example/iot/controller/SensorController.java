package com.example.iot.controller;

import com.example.iot.service.DeviceService;
import com.example.iot.service.SensorService;
import com.example.iot.tcp.DeviceConnectionManager;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api")
public class SensorController {

    private final SensorService sensorService;
    private final DeviceConnectionManager connectionManager;
    private final DeviceService deviceService;

    public SensorController(SensorService sensorService,
                            DeviceConnectionManager connectionManager,
                            DeviceService deviceService) {
        this.sensorService = sensorService;
        this.connectionManager = connectionManager;
        this.deviceService = deviceService;
    }

    @GetMapping("/sensor/{pid}")
    public Map<String, Object> latest(@PathVariable int pid) {
        try {
            return sensorService.latest(pid);
        } catch (Exception e) {
            java.util.Map<String, Object> fallback = new java.util.LinkedHashMap<>();
            fallback.put("project", pid);
            fallback.put("online", connectionManager.isOnline(pid));
            fallback.put("timestamp", "");
            fallback.put("_db_error", e.getMessage());
            return fallback;
        }
    }

    @GetMapping("/sensor/{pid}/history")
    public List<Map<String, Object>> history(@PathVariable int pid,
                                             @RequestParam(defaultValue = "20") int limit) {
        try {
            return sensorService.history(pid, limit);
        } catch (Exception e) {
            return List.of(Map.of(
                    "project", pid,
                    "online", connectionManager.isOnline(pid),
                    "timestamp", "",
                    "_db_error", e.getMessage()
            ));
        }
    }

    @PostMapping("/command/{pid}")
    public ResponseEntity<Map<String, Object>> command(@PathVariable int pid,
                                                        @RequestBody Map<String, Object> body) {
        String cmd = String.valueOf(body.getOrDefault("cmd", ""));
        if (cmd.length() != 1) {
            return ResponseEntity.badRequest().body(Map.of(
                    "status", "error",
                    "message", "命令必须是单个字符"
            ));
        }

        // 命令一律排队，随设备"下次上报的 HTTP 响应"下发：{"status":"OK","cmd":"X"}
        //
        // 原因：现场设备为 GD32 + ESP32-C3 普通 AT 指令模式（AT+CIPSEND），并非透传模式。
        // 直接往 TCP 裸写单字符时，ESP32-C3 会以 "+IPD,0,1:X" 异步上报串口，
        // 而 GD32 只在上报后读取一次串口，异步 +IPD 会打乱其 AT 响应状态机（实测会导致连接 reset）。
        // 设备每次上报后必然读取 HTTP 响应，因此把命令放在响应 JSON 里是唯一时序确定、可靠的通道。
        if (!connectionManager.isOnline(pid)) {
            return ResponseEntity.status(503).body(Map.of(
                    "status", "error",
                    "message", "设备离线（超过 90 秒未上报），命令未发出"
            ));
        }
        long seq = connectionManager.enqueueCommand(pid, cmd);
        return ResponseEntity.ok(Map.of(
                "status", "ok",
                "delivered", "queued",
                "seq", seq,
                "message", "命令 " + cmd + " 已排队，将在设备下次上报时（约几秒内）自动下发"
        ));
    }
}
