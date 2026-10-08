package com.example.iot.controller;

import com.example.iot.service.AlertService;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api/alert")
public class AlertController {

  private final AlertService service;

  public AlertController(AlertService service) {
    this.service = service;
  }

  @GetMapping
  public List<Map<String, Object>> list(
      @RequestParam(required = false) Integer projectId,
      @RequestParam(required = false) String severity,
      @RequestParam(required = false) Boolean unresolvedOnly,
      @RequestParam(defaultValue = "100") int limit) {
    try {
      return service.listAll(projectId, severity, unresolvedOnly, limit);
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

  @PostMapping("/{id}/resolve")
  public ResponseEntity<Map<String, Object>> resolve(@PathVariable long id) {
    try {
      int rows = service.resolve(id);
      if (rows == 0)
        return ResponseEntity.status(404).body(Map.of("status", "error", "message", "Not found"));
      return ResponseEntity.ok(Map.of("status", "ok", "affected", rows));
    } catch (Exception e) {
      return ResponseEntity.status(500).body(Map.of("status", "error", "message", e.getMessage()));
    }
  }

  @PostMapping("/resolve-all")
  public ResponseEntity<Map<String, Object>> resolveAll() {
    try {
      int rows = service.resolveAll();
      return ResponseEntity.ok(Map.of("status", "ok", "affected", rows));
    } catch (Exception e) {
      return ResponseEntity.status(500).body(Map.of("status", "error", "message", e.getMessage()));
    }
  }

  @PostMapping("/project/{pid}/resolve-all")
  public ResponseEntity<Map<String, Object>> resolveByProject(@PathVariable int pid) {
    try {
      int rows = service.resolveByProject(pid);
      return ResponseEntity.ok(Map.of("status", "ok", "affected", rows));
    } catch (Exception e) {
      return ResponseEntity.status(500).body(Map.of("status", "error", "message", e.getMessage()));
    }
  }

  @DeleteMapping("/{id}")
  public ResponseEntity<Map<String, Object>> delete(@PathVariable long id) {
    try {
      int rows = service.delete(id);
      if (rows == 0)
        return ResponseEntity.status(404).body(Map.of("status", "error", "message", "Not found"));
      return ResponseEntity.ok(Map.of("status", "ok", "affected", rows));
    } catch (Exception e) {
      return ResponseEntity.status(500).body(Map.of("status", "error", "message", e.getMessage()));
    }
  }
}
