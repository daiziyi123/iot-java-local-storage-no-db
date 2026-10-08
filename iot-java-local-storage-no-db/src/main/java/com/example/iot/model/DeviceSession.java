package com.example.iot.model;

import java.time.LocalDateTime;

public record DeviceSession(
    Long id,
    Integer projectId,
    String projectName,
    String remoteIp,
    Integer remotePort,
    LocalDateTime connectedAt,
    LocalDateTime disconnectedAt,
    Integer packetsReceived,
    Long bytesReceived,
    Integer commandsSent,
    Integer durationSec,
    String status) {
}