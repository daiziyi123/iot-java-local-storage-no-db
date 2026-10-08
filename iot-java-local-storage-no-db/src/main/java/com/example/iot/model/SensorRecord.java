package com.example.iot.model;

import java.time.LocalDateTime;
import java.util.Map;

public record SensorRecord(
        Long id,
        Integer projectId,
        String projectName,
        Map<String, Object> sensorData,
        LocalDateTime createdAt
) {}
