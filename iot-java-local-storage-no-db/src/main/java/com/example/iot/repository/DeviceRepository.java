package com.example.iot.repository;

import com.example.iot.model.DeviceSession;
import com.example.iot.storage.LocalStorageSupport;
import com.fasterxml.jackson.core.type.TypeReference;
import jakarta.annotation.PostConstruct;
import org.springframework.stereotype.Repository;

import java.time.Duration;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicLong;

@Repository
public class DeviceRepository {

  private static final String FILE_NAME = "device-sessions.json";

  private final LocalStorageSupport storage;
  private final List<DeviceSession> sessions = new ArrayList<>();
  private final AtomicLong idSequence = new AtomicLong(0);

  public DeviceRepository(LocalStorageSupport storage) {
    this.storage = storage;
  }

  @PostConstruct
  public synchronized void load() {
    sessions.clear();
    sessions.addAll(storage.readArray(FILE_NAME, new TypeReference<List<DeviceSession>>() {}));
    idSequence.set(sessions.stream().map(DeviceSession::id).filter(java.util.Objects::nonNull)
        .max(Comparator.naturalOrder()).orElse(0L));
    // 程序重启后旧 TCP 连接已经不存在，避免显示“假在线”。
    boolean changed = false;
    for (int i = 0; i < sessions.size(); i++) {
      DeviceSession s = sessions.get(i);
      if ("online".equals(s.status())) {
        sessions.set(i, offlineCopy(s));
        changed = true;
      }
    }
    if (changed) persist();
  }

  public synchronized List<DeviceSession> findAll(int limit) {
    int safe = Math.max(1, Math.min(limit, 500));
    return sessions.stream().sorted(Comparator.comparing(DeviceSession::id).reversed()).limit(safe).toList();
  }

  public synchronized List<DeviceSession> findOnline() {
    return sessions.stream().filter(s -> "online".equals(s.status()))
        .sorted(Comparator.comparing(DeviceSession::id).reversed()).toList();
  }

  public synchronized List<DeviceSession> findByProject(int projectId, int limit) {
    int safe = Math.max(1, Math.min(limit, 500));
    return sessions.stream().filter(s -> s.projectId() != null && s.projectId() == projectId)
        .sorted(Comparator.comparing(DeviceSession::id).reversed()).limit(safe).toList();
  }

  public synchronized Optional<Long> create(DeviceSession s) {
    long id = idSequence.incrementAndGet();
    DeviceSession saved = new DeviceSession(id, s.projectId(), s.projectName(), s.remoteIp(), s.remotePort(),
        s.connectedAt() == null ? LocalDateTime.now() : s.connectedAt(), null,
        0, 0L, 0, 0, "online");
    sessions.add(saved);
    persist();
    return Optional.of(id);
  }

  public synchronized int updateStats(long id, int packets, long bytes, int commands, int duration) {
    for (int i = 0; i < sessions.size(); i++) {
      DeviceSession s = sessions.get(i);
      if (s.id() != null && s.id() == id) {
        int elapsed = duration;
        if (s.connectedAt() != null && "online".equals(s.status())) {
          elapsed = Math.max(elapsed, (int) Math.max(0, Duration.between(s.connectedAt(), LocalDateTime.now()).getSeconds()));
        }
        DeviceSession updated = new DeviceSession(s.id(), s.projectId(), s.projectName(), s.remoteIp(), s.remotePort(),
            s.connectedAt(), s.disconnectedAt(),
            nz(s.packetsReceived()) + Math.max(0, packets),
            nz(s.bytesReceived()) + Math.max(0L, bytes),
            nz(s.commandsSent()) + Math.max(0, commands),
            Math.max(nz(s.durationSec()), elapsed), s.status());
        sessions.set(i, updated);
        persist();
        return 1;
      }
    }
    return 0;
  }

  public synchronized int markOffline(long id) {
    for (int i = 0; i < sessions.size(); i++) {
      DeviceSession s = sessions.get(i);
      if (s.id() != null && s.id() == id) {
        sessions.set(i, offlineCopy(s));
        persist();
        return 1;
      }
    }
    return 0;
  }

  public synchronized int markAllOffline() {
    int changed = 0;
    for (int i = 0; i < sessions.size(); i++) {
      DeviceSession s = sessions.get(i);
      if ("online".equals(s.status())) {
        sessions.set(i, offlineCopy(s));
        changed++;
      }
    }
    if (changed > 0) persist();
    return changed;
  }

  public synchronized long countOnline() {
    return sessions.stream().filter(s -> "online".equals(s.status())).count();
  }

  private DeviceSession offlineCopy(DeviceSession s) {
    LocalDateTime now = LocalDateTime.now();
    int duration = nz(s.durationSec());
    if (s.connectedAt() != null) {
      duration = Math.max(duration, (int) Math.max(0, Duration.between(s.connectedAt(), now).getSeconds()));
    }
    return new DeviceSession(s.id(), s.projectId(), s.projectName(), s.remoteIp(), s.remotePort(),
        s.connectedAt(), now, s.packetsReceived(), s.bytesReceived(), s.commandsSent(), duration, "offline");
  }

  private static int nz(Integer v) { return v == null ? 0 : v; }
  private static long nz(Long v) { return v == null ? 0L : v; }

  private void persist() {
    storage.writeArray(FILE_NAME, sessions);
  }
}
