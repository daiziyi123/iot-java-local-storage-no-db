package com.example.iot.tcp;

import org.springframework.stereotype.Component;

import java.io.OutputStream;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

@Component
public class DeviceConnectionManager {

    /** 心跳宽限期：最近 N 秒内有数据上报即视为在线（兼容 ESP32-C3 短连接上报场景） */
    private static final long ONLINE_GRACE_MILLIS = 90_000L;

    private final Map<Integer, Socket> sockets = new ConcurrentHashMap<>();
    /** projectId -> 最近一次收到数据的时间戳（毫秒） */
    private final Map<Integer, Long> lastActiveMap = new ConcurrentHashMap<>();

    /** 排队等待下发的命令（携带序号） */
    public record PendingCmd(String cmd, long seq) {}

    /** 命令送达回执：设备取走命令时记录 */
    public record Delivery(String cmd, long seq, long deliveredAtMs) {}

    private final Map<Integer, PendingCmd> pendingCommands = new ConcurrentHashMap<>();
    /** projectId -> 递增命令序号 */
    private final Map<Integer, Long> cmdSeqMap = new ConcurrentHashMap<>();
    /** projectId -> 最近一条被设备取走的命令回执 */
    private final Map<Integer, Delivery> lastDeliveredMap = new ConcurrentHashMap<>();

    public void register(int projectId, Socket socket) {
        Socket old = sockets.put(projectId, socket);
        if (old != null && old != socket) {
            try { old.close(); } catch (Exception ignored) {}
        }
        touch(projectId);
    }

    /** 每次收到设备上报数据时刷新活跃时间 */
    public void touch(int projectId) {
        lastActiveMap.put(projectId, System.currentTimeMillis());
    }

    public void unregister(Socket socket) {
        sockets.entrySet().removeIf(entry -> entry.getValue() == socket);
    }

    /**
     * 业务在线：TCP 长连接存活，或心跳宽限期内有过数据上报。
     */
    public boolean isOnline(int projectId) {
        if (isTcpConnected(projectId)) {
            return true;
        }
        Long lastActive = lastActiveMap.get(projectId);
        return lastActive != null && System.currentTimeMillis() - lastActive <= ONLINE_GRACE_MILLIS;
    }

    /**
     * 真实 TCP 连接是否存活（只有此时才能实时下发命令）。
     */
    public boolean isTcpConnected(int projectId) {
        Socket socket = sockets.get(projectId);
        return socket != null && socket.isConnected() && !socket.isClosed();
    }

    public int onlineCount() {
        Set<Integer> projectIds = new HashSet<>(sockets.keySet());
        projectIds.addAll(lastActiveMap.keySet());
        int count = 0;
        for (Integer pid : projectIds) if (isOnline(pid)) count++;
        return count;
    }

    /** 命令排队：返回该命令的序号，前端用它确认设备是否已取走执行 */
    public long enqueueCommand(int projectId, String cmd) {
        long seq = cmdSeqMap.merge(projectId, 1L, Long::sum);
        pendingCommands.put(projectId, new PendingCmd(cmd, seq));
        return seq;
    }

    /**
     * 设备上报时调用：取出排队命令，并记录"已送达设备"回执。
     * 前端通过 getLastDelivered 轮询确认命令已被设备取走执行。
     */
    public PendingCmd pollPendingCommand(int projectId) {
        PendingCmd pc = pendingCommands.remove(projectId);
        if (pc != null) {
            lastDeliveredMap.put(projectId, new Delivery(pc.cmd(), pc.seq(), System.currentTimeMillis()));
        }
        return pc;
    }

    public Delivery getLastDelivered(int projectId) {
        return lastDeliveredMap.get(projectId);
    }

    public synchronized void sendCommand(int projectId, String cmd) throws Exception {
        Socket socket = sockets.get(projectId);
        if (socket == null || socket.isClosed()) {
            throw new IllegalStateException("Device not connected");
        }
        OutputStream out = socket.getOutputStream();
        out.write(cmd.getBytes(StandardCharsets.UTF_8));
        out.flush();
    }

    /** [增强] 直接向指定 projectId 的 socket 写任意原始字节数据 */
    public synchronized void sendRaw(int projectId, String raw) throws Exception {
        Socket socket = sockets.get(projectId);
        if (socket == null || socket.isClosed()) {
            throw new IllegalStateException("Device not connected");
        }
        OutputStream out = socket.getOutputStream();
        out.write(raw.getBytes(StandardCharsets.UTF_8));
        out.flush();
    }

    /** 获取当前在线 socket 的远端地址，用于日志展示 */
    public String getRemoteAddr(int projectId) {
        Socket socket = sockets.get(projectId);
        return socket == null ? null : String.valueOf(socket.getRemoteSocketAddress());
    }
}
