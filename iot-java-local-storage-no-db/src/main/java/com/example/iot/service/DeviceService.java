package com.example.iot.service;

import com.example.iot.model.DeviceSession;
import com.example.iot.repository.DeviceRepository;
import com.example.iot.tcp.DeviceConnectionManager;
import jakarta.annotation.PreDestroy;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.net.Socket;
import java.time.Duration;
import java.time.LocalDateTime;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

@Service
public class DeviceService {

  private static final Logger log = LoggerFactory.getLogger(DeviceService.class);

  private final DeviceRepository repository;
  private final DeviceConnectionManager connectionManager;

  /** projectId -> sessionId 映射，用于更新在线状态 */
  private final Map<Integer, Long> sessionIdMap = new ConcurrentHashMap<>();

  public DeviceService(DeviceRepository repository, DeviceConnectionManager connectionManager) {
    this.repository = repository;
    this.connectionManager = connectionManager;
  }

  public List<Map<String, Object>> listAll(Integer projectId, Boolean onlineOnly, int limit) {
    List<DeviceSession> list;
    try {
      if (projectId != null) {
        list = repository.findByProject(projectId, limit);
      } else {
        list = repository.findAll(limit);
      }
    } catch (Exception e) {
      list = new ArrayList<>();
    }
    // 在线状态以实时心跳判定为准（不依赖文件里的 status 字段）
    List<Map<String, Object>> mapped = list.stream().map(this::toMap).toList();
    if (Boolean.TRUE.equals(onlineOnly)) {
      // 在线视图：按 projectId 去重，每个在线项目只保留最新一条会话
      Map<Integer, Map<String, Object>> latestByProject = new LinkedHashMap<>();
      for (Map<String, Object> m : mapped) {
        if (!"online".equals(m.get("status"))) continue;
        Integer pid = (Integer) m.get("projectId");
        Map<String, Object> exist = latestByProject.get(pid);
        if (exist == null || ((Number) m.getOrDefault("id", 0L)).longValue()
            > ((Number) exist.getOrDefault("id", 0L)).longValue()) {
          latestByProject.put(pid, m);
        }
      }
      return new ArrayList<>(latestByProject.values());
    }
    return mapped;
  }

  public Map<String, Object> summary() {
    Map<String, Object> m = new LinkedHashMap<>();
    long online = 0, offline = 0, total = 0, totalPackets = 0, totalBytes = 0, totalCmds = 0;
    try {
      List<DeviceSession> all = repository.findAll(500);
      total = all.size();
      // 在线设备数 = 心跳宽限期内活跃过的项目数（按 projectId 去重）
      online = connectionManager.onlineCount();
      offline = Math.max(0, total - online);
      for (DeviceSession s : all) {
        totalPackets += s.packetsReceived() == null ? 0 : s.packetsReceived();
        totalBytes += s.bytesReceived() == null ? 0 : s.bytesReceived();
        totalCmds += s.commandsSent() == null ? 0 : s.commandsSent();
      }
    } catch (Exception ignored) {
    }
    m.put("total", total);
    m.put("online", online);
    m.put("offline", offline);
    m.put("packetsReceived", totalPackets);
    m.put("bytesReceived", totalBytes);
    m.put("commandsSent", totalCmds);
    return m;
  }

  /**
   * TCP 设备连接时调用
   */
  public void onConnected(int projectId, String projectName, Socket socket) {
    try {
      String ip = socket.getInetAddress().getHostAddress();
      int port = socket.getPort();
      DeviceSession s = new DeviceSession(null, projectId, projectName, ip, port,
          LocalDateTime.now(), null, 0, 0L, 0, 0, "online");
      Optional<Long> idOpt = repository.create(s);
      idOpt.ifPresent(id -> sessionIdMap.put(projectId, id));
      log.info("[Device] project={} connected from {}:{}", projectId, ip, port);
    } catch (Exception e) {
      log.warn("[Device] onConnected 失败: {}", e.getMessage());
    }
  }

  /**
   * TCP 断开时调用
   * 注意：不移除 sessionIdMap —— ESP32-C3 短连接周期上报时，两次上报之间连接会断开，
   * 保留最近会话映射可以让后续上报继续累计包数/字节数；超过心跳宽限期后新上报会创建新会话。
   */
  public void onDisconnected(int projectId) {
    Long sid = sessionIdMap.get(projectId);
    if (sid != null)
      repository.markOffline(sid);
    log.info("[Device] project={} disconnected", projectId);
  }

  /**
   * 设备上报数据时更新统计
   */
  public void onPacketReceived(int projectId, int packets, long bytes) {
    Long sid = sessionIdMap.get(projectId);
    if (sid == null)
      return;
    int dur = 0;
    repository.updateStats(sid, packets, bytes, 0, dur);
  }

  /**
   * 下发命令时更新统计
   */
  public void onCommandSent(int projectId) {
    Long sid = sessionIdMap.get(projectId);
    if (sid == null)
      return;
    repository.updateStats(sid, 0, 0L, 1, 0);
  }

  public int markOfflineById(long id) {
    return repository.markOffline(id);
  }

  @PreDestroy
  public void shutdown() {
    try {
      repository.markAllOffline();
    } catch (Exception ignored) {
    }
  }

  private Map<String, Object> toMap(DeviceSession s) {
    Map<String, Object> m = new LinkedHashMap<>();
    m.put("id", s.id());
    m.put("projectId", s.projectId());
    m.put("projectName", s.projectName());
    m.put("remoteIp", s.remoteIp());
    m.put("remotePort", s.remotePort());
    // 在线状态只由实时心跳判定（TCP 存活 或 90 秒宽限期内有上报），不看文件里的历史 status
    boolean onlineNow = connectionManager.isOnline(s.projectId());
    m.put("status", onlineNow ? "online" : "offline");
    m.put("packetsReceived", s.packetsReceived());
    m.put("bytesReceived", s.bytesReceived());
    m.put("commandsSent", s.commandsSent());
    m.put("durationSec", s.durationSec());
    if (s.connectedAt() != null) {
      m.put("connectedAt", s.connectedAt().toString());
      if (onlineNow) {
        m.put("elapsedSec", Duration.between(s.connectedAt(), LocalDateTime.now()).getSeconds());
      }
    }
    if (s.disconnectedAt() != null)
      m.put("disconnectedAt", s.disconnectedAt().toString());
    return m;
  }
}
