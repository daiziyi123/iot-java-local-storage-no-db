package com.example.iot.controller;

import com.example.iot.service.DeviceService;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api/device")
public class DeviceController {

  private final DeviceService service;

  public DeviceController(DeviceService service) {
    this.service = service;
  }

  @GetMapping
  public List<Map<String, Object>> list(
      @RequestParam(required = false) Integer projectId,
      @RequestParam(required = false) Boolean onlineOnly,
      @RequestParam(defaultValue = "100") int limit) {
    try {
      return service.listAll(projectId, onlineOnly, limit);
    } catch (Exception e) {
      return new ArrayList<>();
    }
  }

  @GetMapping("/summary")
  public Map<String, Object> summary() {
    try {
      return service.summary();
    } catch (Exception e) {
      return Map.of("error", e.getMessage());
    }
  }

  @PostMapping("/{id}/offline")
  public ResponseEntity<Map<String, Object>> markOffline(@PathVariable long id) {
    try {
      int rows = service.markOfflineById(id);
      if (rows == 0)
        return ResponseEntity.status(404).body(Map.of("status", "error", "message", "Not found"));
      return ResponseEntity.ok(Map.of("status", "ok", "affected", rows));
    } catch (Exception e) {
      return ResponseEntity.status(500).body(Map.of("status", "error", "message", e.getMessage()));
    }
  }
}