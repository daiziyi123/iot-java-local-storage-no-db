package com.example.iot.service;

import com.example.iot.model.ProjectConfig;
import com.example.iot.repository.ProjectRepository;
import com.example.iot.tcp.DeviceConnectionManager;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

@Service
public class ProjectService {

  private final ProjectRepository repository;
  private final DeviceConnectionManager connectionManager;

  public ProjectService(ProjectRepository repository, DeviceConnectionManager connectionManager) {
    this.repository = repository;
    this.connectionManager = connectionManager;
  }

  public List<Map<String, Object>> listAll() {
    List<ProjectConfig> list;
    try {
      list = repository.findAll();
    } catch (Exception e) {
      list = new ArrayList<>();
    }
    return list.stream().map(p -> toMap(p)).toList();
  }

  public Map<String, Object> detail(int id) {
    return repository.findById(id)
        .map(this::toMap)
        .orElseGet(() -> {
          Map<String, Object> empty = new LinkedHashMap<>();
          empty.put("error", "Project not found");
          return empty;
        });
  }

  public int save(Map<String, Object> body) {
    int id = Integer.parseInt(String.valueOf(body.getOrDefault("id", 0)));
    String name = String.valueOf(body.getOrDefault("name", "Project " + id));
    String icon = String.valueOf(body.getOrDefault("icon", "◇"));
    String accent = String.valueOf(body.getOrDefault("accent", "#2f7ad9"));
    String desc = String.valueOf(body.getOrDefault("description", ""));
    int status = Integer.parseInt(String.valueOf(body.getOrDefault("status", 1)));
    @SuppressWarnings("unchecked")
    List<String> fields = (List<String>) body.getOrDefault("fields", new ArrayList<String>());
    @SuppressWarnings("unchecked")
    List<Map<String, String>> commands = (List<Map<String, String>>) body.getOrDefault("commands",
        new ArrayList<Map<String, String>>());
    ProjectConfig p = new ProjectConfig(id, name, icon, accent, desc, fields, commands, status, null, null);
    return repository.upsert(p);
  }

  public int delete(int id) {
    return repository.delete(id);
  }

  private Map<String, Object> toMap(ProjectConfig p) {
    Map<String, Object> m = new LinkedHashMap<>();
    m.put("id", p.id());
    m.put("name", p.name());
    m.put("icon", p.icon());
    m.put("accent", p.accent());
    m.put("description", p.description());
    m.put("fields", p.fields());
    m.put("commands", p.commands());
    m.put("status", p.status());
    m.put("online", connectionManager.isOnline(p.id()));
    if (p.createdAt() != null)
      m.put("createdAt", p.createdAt().toString());
    if (p.updatedAt() != null)
      m.put("updatedAt", p.updatedAt().toString());
    return m;
  }
}
