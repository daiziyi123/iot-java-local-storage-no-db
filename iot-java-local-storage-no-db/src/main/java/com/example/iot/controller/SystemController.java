package com.example.iot.controller;

import com.example.iot.service.SensorService;
import com.example.iot.storage.LocalStorageSupport;
import com.example.iot.tcp.DeviceConnectionManager;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.LinkedHashMap;
import java.util.Map;

@RestController
@RequestMapping("/api/system")
public class SystemController {

    private final LocalStorageSupport storage;
    private final SensorService sensorService;
    private final DeviceConnectionManager connectionManager;
    private final int tcpPort;

    public SystemController(LocalStorageSupport storage,
                            SensorService sensorService,
                            DeviceConnectionManager connectionManager,
                            @Value("${iot.tcp.port:1025}") int tcpPort) {
        this.storage = storage;
        this.sensorService = sensorService;
        this.connectionManager = connectionManager;
        this.tcpPort = tcpPort;
    }

    @GetMapping("/health")
    public Map<String, Object> health() {
        Map<String, Object> result = new LinkedHashMap<>();
        result.put("service", "ok");
        result.put("databaseRequired", false);
        result.put("database", "disabled");
        result.put("storage", "local-file");
        result.put("storageWritable", storage.isWritable());
        result.put("storagePath", storage.displayPath());
        result.put("sensorRecords", sensorService.count());
        result.put("onlineDevices", connectionManager.onlineCount());
        result.put("tcpPort", tcpPort);
        return result;
    }
}
