package com.example.iot.model;

import java.time.LocalDateTime;

public record AlertRecord(
    Long id,
    Integer projectId,
    String projectName,
    String alertType,
    String severity,
    String fieldName,
    String fieldValue,
    String threshold,
    String message,
    Integer resolved,
    LocalDateTime createdAt,
    LocalDateTime resolvedAt) {
}