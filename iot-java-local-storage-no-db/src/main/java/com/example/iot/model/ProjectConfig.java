package com.example.iot.model;

import java.time.LocalDateTime;
import java.util.List;
import java.util.Map;

public record ProjectConfig(
    Integer id,
    String name,
    String icon,
    String accent,
    String description,
    List<String> fields,
    List<Map<String, String>> commands,
    Integer status,
    LocalDateTime createdAt,
    LocalDateTime updatedAt) {
}