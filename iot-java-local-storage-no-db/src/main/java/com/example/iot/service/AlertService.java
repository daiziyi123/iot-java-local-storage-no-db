package com.example.iot.service;

import com.example.iot.model.AlertRecord;
import com.example.iot.repository.AlertRepository;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

@Service
public class AlertService {

  private static final Logger log = LoggerFactory.getLogger(AlertService.class);

  private final AlertRepository repository;

  public AlertService(AlertRepository repository) {
    this.repository = repository;
  }

  public List<Map<String, Object>> listAll(Integer projectId, String severity, Boolean unresolvedOnly, int limit) {
    List<AlertRecord> list;
    try {
      if (projectId != null) {
        list = repository.findByProject(projectId, limit);
      } else if (Boolean.TRUE.equals(unresolvedOnly)) {
        list = repository.findUnresolved(limit);
      } else {
        list = repository.findAll(limit);
      }
    } catch (Exception e) {
      list = new ArrayList<>();
    }
    if (severity != null && !severity.isBlank()) {
      String s = severity.toLowerCase();
      list = list.stream().filter(a -> s.equals(a.severity())).toList();
    }
    return list.stream().map(this::toMap).toList();
  }

  public Map<String, Object> summary() {
    Map<String, Object> m = new LinkedHashMap<>();
    long total = 0, danger = 0, warning = 0, info = 0, unresolved = 0;
    try {
      List<AlertRecord> all = repository.findAll(500);
      total = all.size();
      unresolved = repository.countUnresolved();
      for (AlertRecord a : all) {
        switch (a.severity() == null ? "" : a.severity()) {
          case "danger" -> danger++;
          case "warning" -> warning++;
          case "info" -> info++;
        }
      }
    } catch (Exception ignored) {
    }
    m.put("total", total);
    m.put("unresolved", unresolved);
    m.put("danger", danger);
    m.put("warning", warning);
    m.put("info", info);
    return m;
  }

  public int resolve(long id) {
    return repository.resolve(id);
  }

  public int resolveByProject(int projectId) {
    return repository.resolveAllByProject(projectId);
  }

  public int resolveAll() {
    return repository.resolveAll();
  }

  public int delete(long id) {
    return repository.delete(id);
  }

  /**
   * 由 SensorService 调用：每次写入传感器数据后检测阈值并生成告警
   */
  public void detectAndSave(int projectId, String projectName, Map<String, Object> data) {
    try {
      // 温度
      if (data.get("temperature") != null) {
        double t = Double.parseDouble(String.valueOf(data.get("temperature")));
        if (t > 35)
          saveAlert(projectId, projectName, "temperature", "danger", "temperature", String.valueOf(t), ">35",
              "温度过高：" + t + "°C");
        else if (t > 30)
          saveAlert(projectId, projectName, "temperature", "warning", "temperature", String.valueOf(t), ">30",
              "温度偏高：" + t + "°C");
        else if (t < 5)
          saveAlert(projectId, projectName, "temperature", "warning", "temperature", String.valueOf(t), "<5",
              "温度过低：" + t + "°C");
      }
      // 湿度
      if (data.get("humidity") != null) {
        double h = Double.parseDouble(String.valueOf(data.get("humidity")));
        if (h > 85)
          saveAlert(projectId, projectName, "humidity", "warning", "humidity", String.valueOf(h), ">85",
              "湿度过高：" + h + "%RH");
        else if (h < 20)
          saveAlert(projectId, projectName, "humidity", "warning", "humidity", String.valueOf(h), "<20",
              "湿度过低：" + h + "%RH");
      }
      // 通用 alarm 标志
      if (data.get("alarm") != null && truthy(data.get("alarm"))) {
        saveAlert(projectId, projectName, "alarm_flag", "danger", "alarm", "1", "", "设备上报报警标志位，请检查现场");
      }
      if (data.get("alarm_flag") != null && truthy(data.get("alarm_flag"))) {
        saveAlert(projectId, projectName, "alarm_flag", "danger", "alarm_flag", "1", "", "设备上报异常报警，请检查现场");
      }
      // 垃圾桶满溢
      if (data.get("capacity") != null) {
        double c = Double.parseDouble(String.valueOf(data.get("capacity")));
        if (c >= 95)
          saveAlert(projectId, projectName, "capacity", "danger", "capacity", String.valueOf(c), ">=95",
              "垃圾桶已满：" + c + "%");
      }
    } catch (Exception e) {
      log.warn("告警检测异常: {}", e.getMessage());
    }
  }

  private void saveAlert(int pid, String pname, String type, String severity,
      String field, String value, String threshold, String message) {
    AlertRecord a = new AlertRecord(null, pid, pname, type, severity, field, value, threshold, message, 0,
        LocalDateTime.now(), null);
    repository.save(a);
  }

  private boolean truthy(Object v) {
    return v == Boolean.TRUE || "1".equals(String.valueOf(v)) || "true".equalsIgnoreCase(String.valueOf(v));
  }

  private Map<String, Object> toMap(AlertRecord a) {
    Map<String, Object> m = new LinkedHashMap<>();
    m.put("id", a.id());
    m.put("projectId", a.projectId());
    m.put("projectName", a.projectName());
    m.put("alertType", a.alertType());
    m.put("severity", a.severity());
    m.put("fieldName", a.fieldName());
    m.put("fieldValue", a.fieldValue());
    m.put("threshold", a.threshold());
    m.put("message", a.message());
    m.put("resolved", a.resolved());
    if (a.createdAt() != null)
      m.put("createdAt", a.createdAt().toString());
    if (a.resolvedAt() != null)
      m.put("resolvedAt", a.resolvedAt().toString());
    return m;
  }
}