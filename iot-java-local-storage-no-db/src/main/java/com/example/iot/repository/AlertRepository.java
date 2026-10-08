package com.example.iot.repository;

import com.example.iot.model.AlertRecord;
import com.example.iot.storage.LocalStorageSupport;
import com.fasterxml.jackson.core.type.TypeReference;
import jakarta.annotation.PostConstruct;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.atomic.AtomicLong;

@Repository
public class AlertRepository {

  private static final String FILE_NAME = "alerts.json";

  private final LocalStorageSupport storage;
  private final List<AlertRecord> records = new ArrayList<>();
  private final AtomicLong idSequence = new AtomicLong(0);

  public AlertRepository(LocalStorageSupport storage) {
    this.storage = storage;
  }

  @PostConstruct
  public synchronized void load() {
    records.clear();
    records.addAll(storage.readArray(FILE_NAME, new TypeReference<List<AlertRecord>>() {}));
    idSequence.set(records.stream().map(AlertRecord::id).filter(java.util.Objects::nonNull)
        .max(Comparator.naturalOrder()).orElse(0L));
  }

  public synchronized List<AlertRecord> findAll(int limit) {
    int safe = Math.max(1, Math.min(limit, 500));
    return records.stream().sorted(Comparator.comparing(AlertRecord::id).reversed()).limit(safe).toList();
  }

  public synchronized List<AlertRecord> findByProject(int projectId, int limit) {
    int safe = Math.max(1, Math.min(limit, 500));
    return records.stream()
        .filter(a -> a.projectId() != null && a.projectId() == projectId)
        .sorted(Comparator.comparing(AlertRecord::id).reversed())
        .limit(safe).toList();
  }

  public synchronized List<AlertRecord> findUnresolved(int limit) {
    int safe = Math.max(1, Math.min(limit, 500));
    return records.stream()
        .filter(a -> a.resolved() == null || a.resolved() == 0)
        .sorted(Comparator.comparing(AlertRecord::id).reversed())
        .limit(safe).toList();
  }

  public synchronized int save(AlertRecord a) {
    AlertRecord saved = new AlertRecord(
        idSequence.incrementAndGet(), a.projectId(), a.projectName(), a.alertType(), a.severity(),
        a.fieldName(), a.fieldValue(), a.threshold(), a.message(),
        a.resolved() == null ? 0 : a.resolved(),
        a.createdAt() == null ? LocalDateTime.now() : a.createdAt(), a.resolvedAt());
    records.add(saved);
    persist();
    return 1;
  }

  public synchronized int resolve(long id) {
    for (int i = 0; i < records.size(); i++) {
      AlertRecord a = records.get(i);
      if (a.id() != null && a.id() == id) {
        if (a.resolved() != null && a.resolved() == 1) return 1;
        records.set(i, copyResolved(a));
        persist();
        return 1;
      }
    }
    return 0;
  }

  public synchronized int resolveAllByProject(int projectId) {
    int changed = 0;
    for (int i = 0; i < records.size(); i++) {
      AlertRecord a = records.get(i);
      if (a.projectId() != null && a.projectId() == projectId && (a.resolved() == null || a.resolved() == 0)) {
        records.set(i, copyResolved(a));
        changed++;
      }
    }
    if (changed > 0) persist();
    return changed;
  }

  public synchronized int resolveAll() {
    int changed = 0;
    for (int i = 0; i < records.size(); i++) {
      AlertRecord a = records.get(i);
      if (a.resolved() == null || a.resolved() == 0) {
        records.set(i, copyResolved(a));
        changed++;
      }
    }
    if (changed > 0) persist();
    return changed;
  }

  public synchronized int delete(long id) {
    boolean removed = records.removeIf(a -> a.id() != null && a.id() == id);
    if (removed) persist();
    return removed ? 1 : 0;
  }

  public synchronized long countUnresolved() {
    return records.stream().filter(a -> a.resolved() == null || a.resolved() == 0).count();
  }

  private AlertRecord copyResolved(AlertRecord a) {
    return new AlertRecord(a.id(), a.projectId(), a.projectName(), a.alertType(), a.severity(),
        a.fieldName(), a.fieldValue(), a.threshold(), a.message(), 1, a.createdAt(), LocalDateTime.now());
  }

  private void persist() {
    storage.writeArray(FILE_NAME, records);
  }
}
