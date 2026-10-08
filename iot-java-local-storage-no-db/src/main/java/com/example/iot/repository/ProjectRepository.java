package com.example.iot.repository;

import com.example.iot.model.ProjectConfig;
import com.example.iot.storage.LocalStorageSupport;
import com.fasterxml.jackson.core.type.TypeReference;
import jakarta.annotation.PostConstruct;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

@Repository
public class ProjectRepository {

  private static final String FILE_NAME = "projects.json";

  private final LocalStorageSupport storage;
  private final Map<Integer, ProjectConfig> projects = new LinkedHashMap<>();

  public ProjectRepository(LocalStorageSupport storage) {
    this.storage = storage;
  }

  @PostConstruct
  public synchronized void load() {
    projects.clear();
    List<ProjectConfig> saved = storage.readArray(FILE_NAME, new TypeReference<List<ProjectConfig>>() {});
    if (saved.isEmpty()) saved = defaultProjects();
    for (ProjectConfig p : saved) projects.put(p.id(), p);
    persist();
  }

  public synchronized List<ProjectConfig> findAll() {
    return projects.values().stream().sorted(Comparator.comparing(ProjectConfig::id)).toList();
  }

  public synchronized Optional<ProjectConfig> findById(int id) {
    return Optional.ofNullable(projects.get(id));
  }

  public synchronized int upsert(ProjectConfig p) {
    ProjectConfig old = projects.get(p.id());
    LocalDateTime now = LocalDateTime.now();
    ProjectConfig saved = new ProjectConfig(
        p.id(), p.name(), p.icon(), p.accent(), p.description(),
        p.fields() == null ? new ArrayList<>() : new ArrayList<>(p.fields()),
        p.commands() == null ? new ArrayList<>() : new ArrayList<>(p.commands()),
        p.status() == null ? 1 : p.status(),
        old == null || old.createdAt() == null ? now : old.createdAt(), now);
    projects.put(saved.id(), saved);
    persist();
    return 1;
  }

  public synchronized int delete(int id) {
    ProjectConfig removed = projects.remove(id);
    if (removed != null) persist();
    return removed == null ? 0 : 1;
  }

  private void persist() {
    storage.writeArray(FILE_NAME, findAll());
  }

  private List<ProjectConfig> defaultProjects() {
    LocalDateTime now = LocalDateTime.now();
    List<ProjectConfig> list = new ArrayList<>();
    list.add(project(0, "基础 IoT 控制", "▣", "#2f9bff", "基础传感器接入、状态采集与远程控制实验",
        List.of("temperature", "humidity", "light"),
        cmds(new String[][]{{"A","LED 红灯开启"},{"E","LED 关闭"},{"B","风扇 50%"},{"F","风扇关闭"},{"D","窗帘 100%"},{"G","窗帘 0%"},{"C","显示温度"}}), now));
    list.add(project(13, "智能宠物屋", "⌂", "#27d2c3", "环境监测、宠物驻留分析与自动喂食控制",
        List.of("temperature", "humidity", "distance", "in_nest", "stay_seconds", "feed_reminder"),
        cmds(new String[][]{{"H","执行喂食"},{"Q","查询状态"}}), now));
    list.add(project(15, "智能窗户", "▦", "#9b6dff", "环境感知、窗帘联动与光照温湿度协同控制",
        List.of("temperature", "humidity", "light", "pir_detected", "led_brightness", "curtain_pos", "usage_rate", "curtain_runtime"),
        cmds(new String[][]{{"O","打开窗户"},{"C","关闭窗户"},{"A","自动模式"}}), now));
    list.add(project(16, "智能垃圾桶", "♲", "#38aaff", "满溢检测、人员感知、桶盖与压缩装置控制",
        List.of("distance", "capacity", "person_detected", "lid_open", "alarm", "compressing"),
        cmds(new String[][]{{"O","打开桶盖"},{"C","关闭桶盖"},{"R","恢复自动"}}), now));
    list.add(project(17, "智能农场", "♧", "#29dc8e", "光温湿监测、作物生长分析与自动灌溉控制",
        List.of("light", "temperature", "humidity", "height", "irrigating", "shade_closed"),
        cmds(new String[][]{{"I","开始灌溉"},{"S","停止灌溉"},{"R","恢复自动"}}), now));
    list.add(project(20, "智能教室", "▤", "#ab6cff", "环境监测、设备联动、能耗分析与智能控制",
        List.of("temperature", "humidity", "light", "pir_detected", "mode", "fan_speed", "led_brightness", "curtain_pos", "usage", "fan_runtime", "light_runtime"),
        cmds(new String[][]{{"M","模式切换"},{"F","风扇控制"},{"L","灯光控制"},{"O","开启自动模式"}}), now));
    list.add(project(25, "智能无人机", "✣", "#ffb934", "飞行状态监控、环境采集、动力档位与模式控制",
        List.of("altitude", "temperature", "humidity", "power_gear", "alarm_flag", "flight_mode", "flight_time"),
        cmds(new String[][]{{"T","起飞"},{"L","降落"},{"H","悬停"},{"U","上升"},{"D","下降"}}), now));
    return list;
  }

  private ProjectConfig project(int id, String name, String icon, String accent, String desc,
                                List<String> fields, List<Map<String, String>> commands, LocalDateTime now) {
    return new ProjectConfig(id, name, icon, accent, desc, fields, commands, 1, now, now);
  }

  private List<Map<String, String>> cmds(String[][] values) {
    List<Map<String, String>> result = new ArrayList<>();
    for (String[] v : values) {
      Map<String, String> m = new LinkedHashMap<>();
      m.put("cmd", v[0]);
      m.put("text", v[1]);
      result.add(m);
    }
    return result;
  }
}
