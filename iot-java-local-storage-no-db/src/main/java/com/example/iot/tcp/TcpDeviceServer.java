package com.example.iot.tcp;

import com.example.iot.service.SensorService;
import com.example.iot.service.DeviceService;
import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.ObjectMapper;
import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

@Component
public class TcpDeviceServer {

    private static final Logger log = LoggerFactory.getLogger(TcpDeviceServer.class);
    private static final byte[] HEADER_END = "\r\n\r\n".getBytes(StandardCharsets.US_ASCII);

    @Value("${iot.tcp.port:1025}")
    private int port;

    private final SensorService sensorService;
    private final DeviceConnectionManager connectionManager;
    private final DeviceService deviceService;
    private final ObjectMapper objectMapper;
    private final ExecutorService pool = Executors.newCachedThreadPool();

    private volatile boolean running = true;
    private ServerSocket serverSocket;

    public TcpDeviceServer(SensorService sensorService,
                           DeviceConnectionManager connectionManager,
                           DeviceService deviceService,
                           ObjectMapper objectMapper) {
        this.sensorService = sensorService;
        this.connectionManager = connectionManager;
        this.deviceService = deviceService;
        this.objectMapper = objectMapper;
    }

    @PostConstruct
    public void start() {
        Thread thread = new Thread(this::acceptLoop, "iot-tcp-accept");
        thread.setDaemon(true);
        thread.start();
    }

    private void acceptLoop() {
        try (ServerSocket ss = new ServerSocket(port)) {
            this.serverSocket = ss;
            log.info("[TCP] Listening on port {} ...", port);
            while (running) {
                Socket socket = ss.accept();
                socket.setKeepAlive(true);
                // 主动 TCP 保活探测：空闲 30s 开始探测，每 10s 一次，3 次无应答判定死链。
                // 默认 keepalive 要 2 小时才探测，手机热点/NAT 老化或设备掉电后会长时间挂着假连接。
                try {
                    socket.setOption(jdk.net.ExtendedSocketOptions.TCP_KEEPIDLE, 30);
                    socket.setOption(jdk.net.ExtendedSocketOptions.TCP_KEEPINTERVAL, 10);
                    socket.setOption(jdk.net.ExtendedSocketOptions.TCP_KEEPCOUNT, 3);
                } catch (Throwable ignored) {
                    // 旧系统不支持细粒度 keepalive 时退回系统默认值（setKeepAlive 已开启）
                }
                log.info("[TCP] New connection from {}", socket.getRemoteSocketAddress());
                pool.submit(() -> handleClient(socket));
            }
        } catch (Exception e) {
            if (running) {
                log.error("TCP server stopped unexpectedly", e);
            }
        }
    }

    private void handleClient(Socket socket) {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        try (socket; InputStream in = socket.getInputStream()) {
            socket.setSoTimeout(3000); // 读超时 3 秒，避免设备没发完整数据时一直阻塞
            byte[] temp = new byte[4096];
            while (running && !socket.isClosed()) {
                int n;
                try {
                    n = in.read(temp);
                } catch (java.net.SocketTimeoutException ste) {
                    // 读超时：尝试处理缓冲区中已经收到的数据（兼容设备分片上报 / 没发 Content-Length 的情况）
                    n = 0;
                }
                if (n < 0) break;
                if (n > 0) buffer.write(temp, 0, n);

                byte[] bytes = buffer.toByteArray();
                if (bytes.length == 0) continue;

                int consumed = 0;

                while (true) {
                    int headerEnd = indexOf(bytes, HEADER_END, consumed);
                    if (headerEnd < 0) {
                        // 没找到 \r\n\r\n：可能是设备只发了裸 JSON（没 HTTP 头）
                        // 兼容旧 Python 服务的裸 JSON 报文：直接尝试解析整个缓冲区
                        String maybeJson = new String(bytes, consumed, bytes.length - consumed, StandardCharsets.UTF_8).trim();
                        if (maybeJson.startsWith("{") && maybeJson.endsWith("}")) {
                            byte[] body = Arrays.copyOfRange(bytes, consumed, bytes.length);
                            processRequest("", body, socket);
                            consumed = bytes.length;
                        }
                        break;
                    }

                    int bodyStart = headerEnd + HEADER_END.length;
                    String headerText = new String(bytes, consumed, headerEnd - consumed, StandardCharsets.UTF_8);
                    int contentLength = contentLength(headerText);

                    if (contentLength < 0) {
                        // 没有 Content-Length：用 \r\n\r\n 之后到缓冲区末尾的所有数据作为 body
                        // （与旧 Python 服务完全一致，Python 之前就是这么处理的）
                        contentLength = bytes.length - bodyStart;
                    }

                    if (bytes.length - bodyStart < contentLength) {
                        // 声明的 Content-Length 超过已收到的数据：不再死等，按已收数据处理（兼容 ESP32 分片 / CL 算错）
                        // 仅当至少有一个完整 JSON 起止位时处理，避免处理半截 JSON
                        String bodyStr = new String(bytes, bodyStart, bytes.length - bodyStart, StandardCharsets.UTF_8).trim();
                        if (bodyStr.startsWith("{") && bodyStr.endsWith("}")) {
                            byte[] body = Arrays.copyOfRange(bytes, bodyStart, bytes.length);
                            processRequest(headerText, body, socket);
                            consumed = bytes.length;
                        }
                        break;
                    }

                    byte[] body = Arrays.copyOfRange(bytes, bodyStart, bodyStart + contentLength);
                    processRequest(headerText, body, socket);
                    consumed = bodyStart + contentLength;

                    if (consumed >= bytes.length) break;
                }

                if (consumed > 0) {
                    buffer.reset();
                    if (consumed < bytes.length) {
                        buffer.write(bytes, consumed, bytes.length - consumed);
                    }
                }
            }
        } catch (Exception e) {
            // 异常类型用于定位断连发起方：
            // SocketException "Connection reset" = 设备/网络侧发 RST（WiFi 抖动、设备重启、NAT 老化）
            // SocketException "Broken pipe" = 写响应时连接已不存在
            // SocketTimeoutException 已在循环内被吞掉（只继续等待），正常不会到这里
            log.info("[TCP] Connection closed: {} ({}: {})",
                    socket.getRemoteSocketAddress(),
                    e.getClass().getSimpleName(),
                    e.getMessage());
        } finally {
            try {
                String remote = String.valueOf(socket.getRemoteSocketAddress());
                for (Map.Entry<Integer, Long> e : currentProjectId.entrySet()) {
                    if (remote.equals(currentSocketAddress.get(e.getKey()))) {
                        deviceService.onDisconnected(e.getKey());
                    }
                }
            } catch (Exception ignored) {}
            connectionManager.unregister(socket);
        }
    }

    /** 当前 handleClient 线程关联的 projectId -> sessionId，socket 关闭时用 */
    private final java.util.concurrent.ConcurrentHashMap<Integer, Long> currentProjectId =
            new java.util.concurrent.ConcurrentHashMap<>();
    private final java.util.concurrent.ConcurrentHashMap<Integer, String> currentSocketAddress =
            new java.util.concurrent.ConcurrentHashMap<>();

    private void processRequest(String headers, byte[] body, Socket socket) {
        try {
            // 与旧 Python 服务一致：在整段 headers 任意位置匹配 "POST /device/update"
            // （Python 用的是 if "POST /device/update" in headers:）
            // 同时兼容设备没发 HTTP 头、只发裸 JSON 的情况（headers 为空字符串时直接当 body 处理）
            if (!headers.contains("POST /device/update") && !headers.isEmpty()) {
                sendHttp(socket, 404, "Not Found");
                return;
            }

            // body 可能带尾随 \r\n 或空字符，trim 掉避免 Jackson 解析失败
            String bodyStr = new String(body, StandardCharsets.UTF_8).trim();
            if (bodyStr.isEmpty()) {
                log.warn("[TCP] Empty body from {}", socket.getRemoteSocketAddress());
                sendHttp(socket, 400, "Bad Request");
                return;
            }

            Map<String, Object> json = objectMapper.readValue(bodyStr, new TypeReference<>() {});
            Object projectObj = json.remove("project");
            int projectId = projectObj == null ? 0 : Integer.parseInt(String.valueOf(projectObj));
            Object nameObj = json.remove("name");
            String projectName = nameObj == null ? defaultProjectName(projectId) : String.valueOf(nameObj);

            Map<String, Object> sensorData = new LinkedHashMap<>(json);
            boolean wasRegistered = connectionManager.isOnline(projectId);
            connectionManager.register(projectId, socket);
            if (!wasRegistered) {
                deviceService.onConnected(projectId, projectName, socket);
                currentProjectId.put(projectId, -1L);
                currentSocketAddress.put(projectId, String.valueOf(socket.getRemoteSocketAddress()));
            }

            long recordId = sensorService.save(projectId, projectName, sensorData);
            deviceService.onPacketReceived(projectId, 1, body.length);
            // 刷新在线心跳：短连接设备在上报间隔内仍判定为在线
            connectionManager.touch(projectId);

            log.info("[Project {}] Sensor saved locally, id={}: {}", projectId, recordId, sensorData);
            sendHttp(socket, 200, "OK", projectId);
        } catch (Exception e) {
            log.warn("Invalid device request: {}", e.getMessage());
            try { sendHttp(socket, 400, "Bad Request"); } catch (Exception ignored) {}
        }
    }

    private int contentLength(String headerText) {
        for (String line : headerText.split("\\r?\\n")) {
            String lower = line.toLowerCase(Locale.ROOT);
            if (lower.startsWith("content-length:")) {
                try {
                    return Integer.parseInt(line.substring(line.indexOf(':') + 1).trim());
                } catch (NumberFormatException ignored) {
                    return -1;
                }
            }
        }
        return -1;
    }

    private void sendHttp(Socket socket, int code, String text) throws Exception {
        sendHttp(socket, code, text, -1);
    }

    private void sendHttp(Socket socket, int code, String text, int projectId) throws Exception {
        String body = "{\"status\":\"" + text + "\"}";
        // 设备上报成功时，若有排队等待的命令，随响应 body 下发：{"status":"OK","cmd":"M"}
        // 设备端（GD32/ESP32-C3）在收到上报响应后解析 cmd 字段即可执行，短连接场景同样有效。
        if (code == 200 && projectId >= 0) {
            DeviceConnectionManager.PendingCmd pc = connectionManager.pollPendingCommand(projectId);
            if (pc != null && pc.cmd().length() == 1) {
                body = "{\"status\":\"OK\",\"cmd\":\"" + pc.cmd() + "\"}";
                try { deviceService.onCommandSent(projectId); } catch (Exception ignored) {}
                log.info("[Project {}] Queued command delivered via response: {} (seq={})", projectId, pc.cmd(), pc.seq());
            }
        }
        byte[] bodyBytes = body.getBytes(StandardCharsets.UTF_8);
        String response = "HTTP/1.1 " + code + " " + text + "\r\n" +
                "Content-Type: application/json; charset=utf-8\r\n" +
                "Content-Length: " + bodyBytes.length + "\r\n" +
                "Connection: keep-alive\r\n\r\n";
        OutputStream out = socket.getOutputStream();
        out.write(response.getBytes(StandardCharsets.UTF_8));
        out.write(bodyBytes);
        out.flush();
    }

    private int indexOf(byte[] source, byte[] target, int fromIndex) {
        outer:
        for (int i = fromIndex; i <= source.length - target.length; i++) {
            for (int j = 0; j < target.length; j++) {
                if (source[i + j] != target[j]) continue outer;
            }
            return i;
        }
        return -1;
    }

    private String defaultProjectName(int projectId) {
        return switch (projectId) {
            case 0 -> "基础 IoT 控制";
            case 13 -> "智能宠物屋";
            case 15 -> "智能窗户";
            case 16 -> "智能垃圾桶";
            case 17 -> "智能农场";
            case 20 -> "智能教室";
            case 25 -> "智能无人机";
            default -> "Project " + projectId;
        };
    }

    @PreDestroy
    public void stop() {
        running = false;
        try { if (serverSocket != null) serverSocket.close(); } catch (Exception ignored) {}
        pool.shutdownNow();
    }
}
