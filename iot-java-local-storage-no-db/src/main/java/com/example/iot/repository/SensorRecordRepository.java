package com.example.iot.repository;

import com.example.iot.model.SensorRecord;
import com.example.iot.storage.LocalStorageSupport;
import jakarta.annotation.PostConstruct;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Repository;

import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicLong;

@Repository
public class SensorRecordRepository {

    private static final String FILE_NAME = "sensor-records.jsonl";

    private final LocalStorageSupport storage;
    private final int maxRecords;
    private final List<SensorRecord> records = new ArrayList<>();
    private final AtomicLong idSequence = new AtomicLong(0);
    private long writesSinceCompact = 0;

    public SensorRecordRepository(LocalStorageSupport storage,
                                  @Value("${iot.storage.max-sensor-records:5000}") int maxRecords) {
        this.storage = storage;
        this.maxRecords = Math.max(200, maxRecords);
    }

    @PostConstruct
    public synchronized void load() {
        records.clear();
        records.addAll(storage.readJsonLines(FILE_NAME, SensorRecord.class, maxRecords));
        long maxId = records.stream().map(SensorRecord::id).filter(java.util.Objects::nonNull)
                .max(Comparator.naturalOrder()).orElse(0L);
        idSequence.set(maxId);
    }

    public synchronized long save(int projectId, String projectName, Map<String, Object> sensorData) {
        SensorRecord record = new SensorRecord(
                idSequence.incrementAndGet(),
                projectId,
                projectName,
                new java.util.LinkedHashMap<>(sensorData),
                LocalDateTime.now()
        );
        records.add(record);
        if (records.size() > maxRecords) {
            records.remove(0);
        }
        storage.appendJsonLine(FILE_NAME, record);

        writesSinceCompact++;
        if (writesSinceCompact >= 1000) {
            storage.rewriteJsonLines(FILE_NAME, records);
            writesSinceCompact = 0;
        }
        return record.id();
    }

    public synchronized Optional<SensorRecord> findLatestByProjectId(int projectId) {
        for (int i = records.size() - 1; i >= 0; i--) {
            SensorRecord r = records.get(i);
            if (r.projectId() != null && r.projectId() == projectId) return Optional.of(r);
        }
        return Optional.empty();
    }

    public synchronized List<SensorRecord> findRecentByProjectId(int projectId, int limit) {
        int safeLimit = Math.max(1, Math.min(limit, 500));
        List<SensorRecord> result = new ArrayList<>(safeLimit);
        for (int i = records.size() - 1; i >= 0 && result.size() < safeLimit; i--) {
            SensorRecord r = records.get(i);
            if (r.projectId() != null && r.projectId() == projectId) result.add(r);
        }
        return result;
    }

    public synchronized long count() {
        return records.size();
    }
}
