package com.example.iot.controller;

import com.example.iot.service.ProjectService;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api/project")
public class ProjectController {

  private final ProjectService service;

  public ProjectController(ProjectService service) {
    this.service = service;
  }

  @GetMapping
  public List<Map<String, Object>> list() {
    try {
      return service.listAll();
    } catch (Exception e) {
      List<Map<String, Object>> fallback = new ArrayList<>();
      Map<String, Object> m = new LinkedHashMap<>();
      m.put("error", e.getMessage());
      fallback.add(m);
      return fallback;
    }
  }

  @GetMapping("/{id}")
  public ResponseEntity<Map<String, Object>> detail(@PathVariable int id) {
    try {
      Map<String, Object> m = service.detail(id);
      if (m.containsKey("error"))
        return ResponseEntity.status(404).body(m);
      return ResponseEntity.ok(m);
    } catch (Exception e) {
      return ResponseEntity.status(500).body(Map.of("error", e.getMessage()));
    }
  }

  @PostMapping
  public ResponseEntity<Map<String, Object>> createOrUpdate(@RequestBody Map<String, Object> body) {
    try {
      if (!body.containsKey("id") || !body.containsKey("name")) {
        return ResponseEntity.badRequest().body(Map.of("status", "error", "message", "id and name are required"));
      }
      int rows = service.save(body);
      return ResponseEntity.ok(Map.of("status", "ok", "affected", rows));
    } catch (Exception e) {
      return ResponseEntity.status(500).body(Map.of("status", "error", "message", e.getMessage()));
    }
  }

  @DeleteMapping("/{id}")
  public ResponseEntity<Map<String, Object>> delete(@PathVariable int id) {
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