const ICONS = {
  '0-iot': '<svg viewBox="0 0 64 64" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><rect x="16" y="16" width="32" height="32" rx="4"/><rect x="26" y="26" width="12" height="12" rx="1"/><path d="M8 24h8M8 32h8M8 40h8M48 24h8M48 32h8M48 40h8M24 8v8M32 8v8M40 8v8M24 48v8M32 48v8M40 48v8"/></svg>',
  '13-pet': '<svg viewBox="0 0 64 64" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><path d="M8 52V30l24-20 24 20v22z"/><path d="M26 52V38h12v14"/><circle cx="32" cy="45" r="2" fill="currentColor" stroke="none"/></svg>',
  '15-window': '<svg viewBox="0 0 64 64" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><rect x="8" y="10" width="48" height="44" rx="2"/><line x1="32" y1="10" x2="32" y2="54"/><line x1="8" y1="32" x2="56" y2="32"/><path d="M14 16v12M50 16v12" stroke-opacity="0.5"/></svg>',
  '16-trash': '<svg viewBox="0 0 64 64" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><path d="M20 22h24l-2 30a4 4 0 0 1-4 4H26a4 4 0 0 1-4-4z"/><path d="M14 16h36M28 16v-4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v4"/><line x1="28" y1="30" x2="28" y2="44"/><line x1="36" y1="30" x2="36" y2="44"/></svg>',
  '17-farm': '<svg viewBox="0 0 64 64" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><path d="M32 54V28"/><path d="M32 36c-6-4-12-4-12 0s6 0 12 0z"/><path d="M32 28c6-4 12-4 12 0s-6 0-12 0z"/><path d="M32 20c-4-6-10-6-10 0s6 0 10 0z"/><path d="M20 54h24"/><circle cx="44" cy="18" r="2" fill="currentColor" stroke="none"/></svg>',
  '20-classroom': '<svg viewBox="0 0 64 64" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><path d="M32 14a12 12 0 0 0-7 22c1 2 2 4 2 6h10c0-2 1-4 2-6a12 12 0 0 0-7-22z"/><path d="M28 46h8v4h-8z"/><line x1="24" y1="54" x2="40" y2="54"/><path d="M16 22l-3 3M48 22l3 3" stroke-opacity="0.6"/></svg>',
  '25-drone': '<svg viewBox="0 0 64 64" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><rect x="26" y="26" width="12" height="12" rx="2"/><line x1="20" y1="20" x2="28" y2="28"/><line x1="44" y1="20" x2="36" y2="28"/><line x1="20" y1="44" x2="28" y2="36"/><line x1="44" y1="44" x2="36" y2="36"/><circle cx="16" cy="16" r="6"/><circle cx="48" cy="16" r="6"/><circle cx="16" cy="48" r="6"/><circle cx="48" cy="48" r="6"/></svg>'
};

let projects = [
  { id: 0, name: '基础 IoT 控制', icon: '▣', img: '0-iot', accent: '#2f9bff', desc: '基础传感器接入、状态采集与远程控制实验', fields: ['temperature', 'humidity', 'light'], commands: [['A', 'LED 红灯开启'], ['E', 'LED 关闭'], ['B', '风扇 50%'], ['F', '风扇关闭'], ['D', '窗帘 100%'], ['G', '窗帘 0%'], ['C', '显示温度']] },
  { id: 13, name: '智能宠物屋', icon: '⌂', img: '13-pet', accent: '#27d2c3', desc: '环境监测、宠物驻留分析与自动喂食控制', fields: ['temperature', 'humidity', 'distance', 'in_nest', 'stay_seconds', 'feed_reminder'], commands: [['H', '执行喂食'], ['Q', '查询状态']] },
  { id: 15, name: '智能窗户', icon: '▦', img: '15-window', accent: '#9b6dff', desc: '环境感知、窗帘联动与光照温湿度协同控制', fields: ['temperature', 'humidity', 'light', 'pir_detected', 'led_brightness', 'curtain_pos', 'usage_rate', 'curtain_runtime'], commands: [['O', '打开窗户'], ['C', '关闭窗户'], ['A', '自动模式']] },
  { id: 16, name: '智能垃圾桶', icon: '♲', img: '16-trash', accent: '#38aaff', desc: '满溢检测、人员感知、桶盖与压缩装置控制', fields: ['distance', 'capacity', 'person_detected', 'lid_open', 'alarm', 'compressing'], commands: [['O', '打开桶盖'], ['C', '关闭桶盖'], ['R', '恢复自动']] },
  { id: 17, name: '智能农场', icon: '♧', img: '17-farm', accent: '#29dc8e', desc: '光温湿监测、作物生长分析与自动灌溉控制', fields: ['light', 'temperature', 'humidity', 'height', 'irrigating', 'shade_closed'], commands: [['I', '开始灌溉'], ['S', '停止灌溉'], ['R', '恢复自动']] },
  { id: 20, name: '智能教室', icon: '▤', img: '20-classroom', accent: '#ab6cff', desc: '环境监测、设备联动、能耗分析与智能控制', fields: ['temperature', 'humidity', 'light', 'pir_detected', 'mode', 'fan_speed', 'led_brightness', 'curtain_pos', 'usage', 'fan_runtime', 'light_runtime'], commands: [['M', '模式切换'], ['F', '风扇控制'], ['L', '灯光控制'], ['O', '开启自动模式']] },
  { id: 25, name: '智能无人机', icon: '✣', img: '25-drone', accent: '#ffb934', desc: '飞行状态监控、环境采集、动力档位与模式控制', fields: ['altitude', 'temperature', 'humidity', 'power_gear', 'alarm_flag', 'flight_mode', 'flight_time'], commands: [['T', '起飞'], ['L', '降落'], ['H', '悬停'], ['U', '上升'], ['D', '下降']] }
];

async function loadProjects() {
  try {
    const r = await fetch('/api/project');
    if (!r.ok) return;
    const list = await r.json();
    if (!Array.isArray(list)) return;
    projects = list.map(x => ({
      id: Number(x.id),
      name: x.name || `Project ${x.id}`,
      icon: x.icon || '◇',
      img: x.img || ({ 0: '0-iot', 13: '13-pet', 15: '15-window', 16: '16-trash', 17: '17-farm', 20: '20-classroom', 25: '25-drone' }[Number(x.id)] || '0-iot'),
      accent: x.accent || '#2f7ad9',
      desc: x.description || '',
      fields: Array.isArray(x.fields) ? x.fields : [],
      commands: Array.isArray(x.commands) ? x.commands.map(c => Array.isArray(c) ? c : [c.cmd, c.text]) : [],
      status: x.status ?? 1
    }));
  } catch { }
  syncProjectCounts();
}
function syncProjectCounts() {
  document.getElementById('projectTotalTop') && (document.getElementById('projectTotalTop').textContent = projects.length);
  document.getElementById('projectTotalSummary') && (document.getElementById('projectTotalSummary').textContent = projects.length);
}

const meta = {
  temperature: { label: '温度', unit: '°C', icon: '♨', color: '#ff6f75' }, humidity: { label: '湿度', unit: '%RH', icon: '◉', color: '#3ca9ff' }, light: { label: '光照', unit: 'lx', icon: '☀', color: '#ffc13b' }, illuminance: { label: '照度', unit: 'lx', icon: '☀', color: '#ffc13b' }, distance: { label: '距离', unit: 'cm', icon: '↔', color: '#2fd5e7' }, in_nest: { label: '宠物在窝', unit: '', icon: '⌂', color: '#2fe0a0' }, stay_seconds: { label: '停留时长', unit: 's', icon: '◷', color: '#9f6bff' }, feed_reminder: { label: '喂食提醒', unit: '', icon: '!', color: '#ffb938' }, capacity: { label: '容量', unit: '%', icon: '◒', color: '#36b8ff' }, person_detected: { label: '人员检测', unit: '', icon: '♙', color: '#2fe0a0' }, lid_open: { label: '桶盖状态', unit: '', icon: '⌁', color: '#3ca9ff' }, alarm: { label: '报警', unit: '', icon: '△', color: '#ff6675' }, compressing: { label: '压缩状态', unit: '', icon: '⇣', color: '#b06cff' }, pir_detected: { label: '人体检测', unit: '', icon: '♙', color: '#2fe0a0' }, led_brightness: { label: 'LED 亮度', unit: '%', icon: '☼', color: '#ffc13b' }, curtain_pos: { label: '窗帘位置', unit: '%', icon: '▥', color: '#b06cff' }, usage_rate: { label: '使用率', unit: '%', icon: '◔', color: '#a46cff' }, curtain_runtime: { label: '窗帘运行时长', unit: 's', icon: '◷', color: '#8d78ff' }, mode: { label: '模式', unit: '', icon: '▦', color: '#a46cff' }, fan_speed: { label: '风扇速度', unit: '%', icon: '✣', color: '#35a8ff' }, usage: { label: '使用率', unit: '%', icon: '◔', color: '#a46cff' }, fan_runtime: { label: '风扇运行时长', unit: 's', icon: '◷', color: '#a46cff' }, light_runtime: { label: '灯光运行时长', unit: 's', icon: '◷', color: '#ffc13b' }, height: { label: '作物高度', unit: 'cm', icon: '↟', color: '#2fe08e' }, irrigating: { label: '灌溉状态', unit: '', icon: '◉', color: '#2faeff' }, shade_closed: { label: '遮阳状态', unit: '', icon: '▰', color: '#6b8cff' }, altitude: { label: '飞行高度', unit: 'cm', icon: '↟', color: '#3cb8ff' }, power_gear: { label: '动力档位', unit: '', icon: '⚡', color: '#ffbf3b' }, alarm_flag: { label: '异常报警', unit: '', icon: '△', color: '#ff6675' }, flight_mode: { label: '飞行模式', unit: '', icon: '✣', color: '#a46cff' }, flight_time: { label: '飞行时长', unit: 's', icon: '◷', color: '#35a8ff' }
};

let currentProject = null, pollTimer = null, currentHistory = [], projectLatest = new Map();
let chartLimit = 20, hiddenSeries = new Set();
// 等待设备取走执行的排队命令回执列表：{cmd, seq, btn, willActive, at}
let pendingCmdAcks = [];
const CHART_COLORS = ['#ff6f75', '#3ca9ff', '#ffb93b', '#9f6bff', '#2fd5a0', '#32cbff'];
const CHART_NUMERIC = ['temperature', 'humidity', 'light', 'illuminance', 'distance', 'capacity', 'height', 'altitude', 'usage', 'usage_rate', 'fan_speed', 'led_brightness', 'curtain_pos'];
const RIGHT_AXIS_KEYS = ['light', 'illuminance']; // 数量级大的字段走右轴（lx）
const grid = document.getElementById('projectGrid');

function rgba(hex, alpha = .2) { const h = hex.replace('#', ''); const r = parseInt(h.slice(0, 2), 16), g = parseInt(h.slice(2, 4), 16), b = parseInt(h.slice(4, 6), 16); return `rgba(${r},${g},${b},${alpha})` }
function esc(s) { return String(s ?? '').replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c])) }
function clock() { document.getElementById('clock').textContent = new Date().toLocaleString('zh-CN', { hour12: false }).replaceAll('/', '-') }
setInterval(clock, 1000); clock();

function sparkBars(seed) { let x = seed + 3; return Array.from({ length: 10 }, (_, i) => { x = (x * 9301 + 49297) % 233280; const h = 5 + Math.round((x / 233280) * 17); return `<i style="height:${h}px"></i>` }).join('') }
function renderProjectCards() { grid.innerHTML = ''; projects.forEach((p, idx) => { const d = projectLatest.get(p.id) || {}; const online = !!d.online; const mainField = p.fields.find(f => d[f] !== undefined); const m = mainField ? meta[mainField] : null; const val = mainField ? displayValue(mainField, d[mainField]) : '等待数据'; const card = document.createElement('article'); card.className = 'project-card'; card.style.setProperty('--accent', p.accent); card.style.setProperty('--accent-soft', rgba(p.accent, .16)); card.innerHTML = `<div class="project-top"><span class="project-id">${String(idx + 1).padStart(2, '0')}</span><span class="status-chip ${online ? '' : 'offline'}">${online ? '运行中' : '待连接'}</span></div><h3>${p.name}</h3><div class="project-art">${ICONS[p.img] || ''}</div><div class="project-desc">${p.desc}</div><div class="project-mini"><div><small>${m ? m.label : '状态'}</small><strong>${esc(val)}</strong></div><div><small>数据流</small><div class="spark">${sparkBars(p.id)}</div></div></div>`; card.onclick = () => openProject(p); grid.appendChild(card) }) }

async function refreshOverview() { document.getElementById('streamStatus').textContent = '同步中'; const tasks = projects.map(async p => { try { const r = await fetch(`/api/sensor/${p.id}`); if (!r.ok) throw 0; const d = await r.json(); projectLatest.set(p.id, d); return true } catch { projectLatest.set(p.id, { online: false }); return false } }); const results = await Promise.all(tasks); renderProjectCards(); const online = projects.filter(p => projectLatest.get(p.id)?.online).length; updateOnlineSummary(online); await loadOverviewActivity(); await refreshSystemHealth(); document.getElementById('streamStatus').textContent = results.some(Boolean) || projects.length === 0 ? '正常' : '接口异常'; document.getElementById('runtimeStatus').textContent = '● 正常' }
function updateOnlineSummary(online) { const total = Math.max(1, projects.length); const pct = Math.round(online / total * 100); document.getElementById('onlineCount').textContent = `${online}/${projects.length}`; const ring = document.getElementById('topOnlineRing'); if (ring) ring.style.background = `conic-gradient(var(--green) 0 ${pct}%,#e6edf4 ${pct}% 100%)`; const rate = document.getElementById('onlineRateText'); if (rate) rate.textContent = `${pct}% 在线率`; document.getElementById('donutOnline').textContent = online; document.getElementById('onlineLegend').textContent = online; document.getElementById('offlineLegend').textContent = Math.max(0, projects.length - online); document.getElementById('summaryOnline').textContent = online; document.getElementById('onlineBar').style.width = pct + '%'; document.getElementById('onlineDonut').style.background = `conic-gradient(var(--green) 0 ${pct}%,#1a2940 ${pct}% 100%)` }
async function refreshSystemHealth() { try { const r = await fetch('/api/system/health'); if (!r.ok) throw new Error(); const d = await r.json(); const ok = d.storageWritable === true; document.getElementById('dbStatus').textContent = ok ? '本地正常' : '不可写'; const detail = document.getElementById('mysqlDetailState'); if (detail) detail.textContent = ok ? '本地正常' : '异常'; } catch { document.getElementById('dbStatus').textContent = '异常'; const detail = document.getElementById('mysqlDetailState'); if (detail) detail.textContent = '异常'; } }

async function loadOverviewActivity() { const rows = []; for (const p of projects) { try { const r = await fetch(`/api/sensor/${p.id}/history?limit=6`); if (!r.ok) continue; const list = await r.json(); list.forEach(x => rows.push({ ...x, _p: p })) } catch { } } rows.sort((a, b) => String(b.timestamp).localeCompare(String(a.timestamp))); const top = rows.slice(0, 10); document.getElementById('summaryRows').textContent = rows.length; const box = document.getElementById('activityList'); if (!top.length) { box.innerHTML = '<div class="empty">暂无本地记录，设备上报后会自动显示</div>' } else { box.innerHTML = top.map(row => { const field = Object.keys(row).find(k => !['project', 'name', 'timestamp', 'online', '_p'].includes(k)); const mm = meta[field] || { label: field || '数据' }; return `<div class="activity-row"><time>${esc((row.timestamp || '--').split(' ')[1] || '--')}</time><strong>${esc(row._p.name)}</strong><span>${esc(mm.label)} · ${esc(field ? displayValue(field, row[field]) : '--')}</span><em>● 写入成功</em></div>` }).join('') } drawOverviewChart(rows) }

function numericSample(row) { for (const k of ['temperature', 'humidity', 'light', 'distance', 'capacity', 'height', 'altitude', 'usage', 'usage_rate']) if (Number.isFinite(Number(row[k]))) return Number(row[k]); return NaN }

function drawOverviewChart(rows) { const canvas = document.getElementById('overviewChart'); const ctx = setupCanvas(canvas); const W = canvas.clientWidth, H = canvas.clientHeight; const pad = { l: 38, r: 12, t: 16, b: 28 }; ctx.clearRect(0, 0, W, H); drawGrid(ctx, W, H, pad, 5); const pts = rows.slice(0, 40).reverse().map((r, i) => ({ x: i, y: numericSample(r) })).filter(p => Number.isFinite(p.y)); if (pts.length < 2) { drawEmptyCanvas(ctx, W, H, '等待更多本地数据'); return } const vals = pts.map(p => p.y), min = Math.min(...vals), max = Math.max(...vals), range = max - min || 1; ctx.beginPath(); pts.forEach((p, i) => { const x = pad.l + i * (W - pad.l - pad.r) / (pts.length - 1), y = pad.t + (max - p.y) / range * (H - pad.t - pad.b); i ? ctx.lineTo(x, y) : ctx.moveTo(x, y) }); ctx.strokeStyle = '#35aaff'; ctx.lineWidth = 2; ctx.shadowBlur = 10; ctx.shadowColor = '#35aaff'; ctx.stroke(); ctx.shadowBlur = 0; ctx.lineTo(W - pad.r, H - pad.b); ctx.lineTo(pad.l, H - pad.b); ctx.closePath(); const g = ctx.createLinearGradient(0, pad.t, 0, H - pad.b); g.addColorStop(0, 'rgba(53,170,255,.25)'); g.addColorStop(1, 'rgba(53,170,255,0)'); ctx.fillStyle = g; ctx.fill() }

function openProject(p) { currentProject = p; hiddenSeries.clear(); pendingCmdAcks = []; document.getElementById('overviewPage').classList.add('hidden'); document.getElementById('detailPage').classList.remove('hidden'); document.getElementById('crumbProject').textContent = p.name; document.getElementById('projectTitle').textContent = `${p.name} / Project ${p.id}`; document.getElementById('projectDesc').textContent = p.desc; document.getElementById('heroPid').textContent = p.id; document.getElementById('heroIcon').innerHTML = ICONS[p.img] || ''; document.getElementById('heroVisual').style.setProperty('--hero', p.accent); document.getElementById('heroVisual').style.setProperty('--hero-soft', rgba(p.accent, .25)); renderFieldSkeleton(p); renderCommands(p); fetchLatest(); fetchHistory(); clearInterval(pollTimer); pollTimer = setInterval(fetchLatest, 2000); window.scrollTo({ top: 0, behavior: 'smooth' }) }
function closeProject() { clearInterval(pollTimer); currentProject = null; document.getElementById('detailPage').classList.add('hidden'); document.getElementById('overviewPage').classList.remove('hidden'); refreshOverview(); window.scrollTo({ top: 0, behavior: 'smooth' }) }

function renderFieldSkeleton(p) { const box = document.getElementById('sensorFields'); box.innerHTML = ''; p.fields.forEach((field) => { const m = meta[field] || { label: field, unit: '', icon: '•', color: '#35a8ff' }; const item = document.createElement('div'); item.className = 'sensor-item'; item.style.setProperty('--metric', m.color); item.innerHTML = `<span class="metric-icon">${m.icon}</span><div class="field-body"><div class="label">${m.label}</div><div id="field-${field}" class="value empty-value">--</div><div id="sub-${field}" class="sub waiting">等待采集</div></div>`; box.appendChild(item) }) }
// 字段状态分级：normal 正常 / warn 关注 / danger 异常 / waiting 未采集
function fieldLevel(field, v) { if (v === undefined || v === null || v === '') return 'waiting'; if (['alarm', 'alarm_flag'].includes(field)) return truthy(v) ? 'danger' : 'normal'; if (field === 'feed_reminder') return truthy(v) ? 'warn' : 'normal'; if (field === 'temperature') { const n = Number(v); return n >= 18 && n <= 30 ? 'normal' : 'warn' } if (field === 'humidity') { const n = Number(v); return n >= 30 && n <= 80 ? 'normal' : 'warn' } if (field === 'light') return Number(v) < 100 ? 'warn' : 'normal'; return 'normal' }
// 把一个字段卡片更新为"有值/无值"两种视觉状态
function applyFieldState(field, v) { const el = document.getElementById(`field-${field}`), sub = document.getElementById(`sub-${field}`); if (!el || !sub) return; const has = v !== undefined && v !== null && v !== ''; el.textContent = has ? displayValue(field, v) : '--'; el.classList.toggle('empty-value', !has); const lv = fieldLevel(field, v); sub.className = `sub ${lv}`; sub.textContent = has ? statusText(field, v) : '等待采集'; }
function renderCommands(p) {
  const box = document.getElementById('commandButtons');
  box.innerHTML = '';
  p.commands.forEach(([cmd, text], i) => {
    const btn = document.createElement('button');
    btn.className = 'command-btn';
    btn.dataset.cmd = cmd;
    btn.innerHTML = `<span class="cb-icon">${['◫', '✣', '☼', '▥', '⌁', '⚙'][i % 6]}</span><strong>${text}</strong><small>命令 ${cmd} · 点击切换开关</small><em class="cb-state">○ 已关闭</em>`;
    btn.onclick = () => toggleCommand(btn, cmd);
    box.appendChild(btn);
  });
}
// 带状态反馈的开关：排队后等设备下次上报取走命令，收到序号回执才变绿"已执行"
async function toggleCommand(btn, cmd) {
  if (btn.classList.contains('busy')) return;
  const state = btn.querySelector('.cb-state');
  btn.classList.add('busy');
  btn.classList.remove('failed');
  state.textContent = '… 下发中';
  try {
    const res = await fetch(`/api/command/${currentProject.id}`, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ cmd }) });
    const data = await res.json();
    if (!res.ok) throw new Error(data.message || '发送失败');
    const willActive = !btn.classList.contains('active');
    if (data.delivered === 'queued') {
      // 同一项目只有一个排队槽：新命令会顶掉旧命令。其他按钮的旧等待标记"已取消"，
      // 同按钮的旧等待直接丢弃（由本次点击取代）
      pendingCmdAcks.forEach(a => {
        if (a.btn !== btn && a.btn.isConnected) {
          a.btn.classList.remove('waiting');
          a.btn.querySelector('.cb-state').textContent = '○ 已取消（被新指令取代）';
        }
      });
      pendingCmdAcks = pendingCmdAcks.filter(a => a.btn !== btn);
      // 先不切换开关状态，登记回执等待设备取走（每 2s 轮询 latest 时确认）
      btn.classList.add('waiting');
      state.textContent = '◷ 已排队，待设备执行';
      pendingCmdAcks.push({ cmd, seq: data.seq, btn, willActive, at: Date.now() });
      const statusEl = document.getElementById('commandStatus');
      statusEl.textContent = `◷ ${data.message}`;
      toast(data.message || '命令已排队');
    } else {
      btn.classList.toggle('active', willActive);
      state.textContent = willActive ? '● 已开启' : '○ 已关闭';
      const statusEl = document.getElementById('commandStatus');
      statusEl.textContent = `✓ ${data.message}`;
      toast(data.message || '命令发送成功');
    }
  } catch (e) {
    btn.classList.add('failed');
    btn.classList.remove('active');
    state.textContent = '✕ 发送失败';
    const statusEl = document.getElementById('commandStatus');
    statusEl.textContent = `✕ ${e.message || '发送失败'}`;
    toast(e.message || '命令发送失败', 'error');
    setTimeout(() => btn.classList.remove('failed'), 1600);
  } finally {
    btn.classList.remove('busy');
  }
}

// 用 /api/sensor 返回的 lastCmdSeq 回执，确认排队命令已被设备取走执行
function confirmPendingCommands(data) {
  if (!pendingCmdAcks.length || data.lastCmdSeq === undefined) return;
  const survivors = [];
  pendingCmdAcks.forEach(a => {
    if (data.lastCmdSeq === a.seq && data.lastCmd === a.cmd) {
      // 正好命中：设备已取走本命令
      if (a.btn.isConnected) {
        const btn = a.btn, state = btn.querySelector('.cb-state');
        btn.classList.remove('waiting');
        btn.classList.toggle('active', a.willActive);
        state.textContent = a.willActive ? '● 已执行（开）' : '○ 已执行（关）';
      }
      document.getElementById('commandStatus').textContent = `✓ 设备已执行命令 ${data.lastCmd}`;
      toast(`设备已执行命令 ${data.lastCmd}`, 'success');
    } else if (data.lastCmdSeq > a.seq) {
      // 序号已被跳过：本命令在排队槽里被更新的命令顶掉，设备从未取走
      if (a.btn.isConnected) {
        a.btn.classList.remove('waiting');
        a.btn.querySelector('.cb-state').textContent = '○ 已取消（被新指令取代）';
      }
    } else {
      survivors.push(a); // 还没轮到，继续等
    }
  });
  pendingCmdAcks = survivors;
}

// 排队超时提示（设备长时间未上报取走）
function checkPendingTimeout() {
  pendingCmdAcks = pendingCmdAcks.filter(a => a.btn.isConnected);
  const now = Date.now();
  pendingCmdAcks = pendingCmdAcks.filter(a => {
    const state = a.btn.querySelector('.cb-state');
    const elapsed = now - a.at;
    if (elapsed > 60000) {
      // 硬超时：设备 60 秒没上报取走，恢复按钮
      a.btn.classList.remove('waiting');
      state.textContent = '✕ 未执行（设备未上报）';
      document.getElementById('commandStatus').textContent = '✕ 设备长时间未上报，命令未执行';
      return false;
    }
    if (elapsed > 15000 && state.textContent.indexOf('暂未响应') < 0) {
      state.textContent = '◷ 设备暂未响应，继续等待';
    }
    return true;
  });
}

async function fetchLatest() { if (!currentProject) return; try { const res = await fetch(`/api/sensor/${currentProject.id}`); if (!res.ok) throw new Error('接口异常'); const data = await res.json(); projectLatest.set(currentProject.id, data); currentProject.fields.forEach(field => applyFieldState(field, data[field])); const online = !!data.online; const tcpConnected = !!data.tcpConnected; setDetailOnline(online, tcpConnected); document.getElementById('updateTime').textContent = `最新采集时间：${data.timestamp || '--'}`; document.getElementById('heroUpdated').textContent = data.timestamp || '--'; buildEvents(data); confirmPendingCommands(data); checkPendingTimeout() } catch (e) { setDetailOnline(false); toast('实时数据读取失败', 'error') } }
function setDetailOnline(online, tcpConnected) { const badge = document.getElementById('onlineBadge'); badge.textContent = online ? '● 在线' : '● 离线'; badge.className = `badge ${online ? 'online' : 'offline'}`; const chip = document.getElementById('controlConnection'); chip.classList.remove('online', 'waiting'); if (tcpConnected) { chip.textContent = 'TCP 已连接'; chip.classList.add('online') } else if (online) { chip.textContent = '周期上报中 · 命令将排队下发'; chip.classList.add('waiting') } else { chip.textContent = 'TCP 未连接'; } document.getElementById('healthValue').textContent = online ? '100%' : '0%'; document.getElementById('deviceStateText').textContent = online ? '在线' : '离线'; document.getElementById('deviceStatePct').textContent = online ? '100%' : '0%'; document.getElementById('deviceOnlineBar').style.width = online ? '100%' : '0%'; document.getElementById('tcpDetailState').textContent = tcpConnected ? '在线' : (online ? '周期上报' : '离线') }

async function fetchHistory() {
  const body = document.getElementById('historyBody');
  body.innerHTML = '<tr><td colspan="4">正在读取本地历史数据...</td></tr>';
  try {
    const res = await fetch(`/api/sensor/${currentProject.id}/history?limit=${chartLimit}`);
    if (!res.ok) throw new Error();
    const list = await res.json();
    currentHistory = list;
    document.getElementById('mysqlDetailState').textContent = '本地正常';
    renderHistoryTable(list);
    renderChartLegend();
    drawDetailChart(list);
  } catch {
    body.innerHTML = '<tr><td colspan="4">读取本地数据失败</td></tr>';
    document.getElementById('mysqlDetailState').textContent = '异常';
    drawDetailChart([]);
  }
}

// 当前项目可绘制的数值序列（按项目字段顺序，最多 5 条，避免线条重叠混乱）
function chartSeriesKeys() {
  if (!currentProject) return [];
  return currentProject.fields.filter(k => CHART_NUMERIC.includes(k)).slice(0, 5);
}

// 可点击图例：点击隐藏/显示对应序列
function renderChartLegend() {
  const box = document.getElementById('chartLegend');
  const keys = chartSeriesKeys();
  box.innerHTML = keys.map((k, i) => {
    const m = meta[k] || { label: k, unit: '' };
    const hidden = hiddenSeries.has(k);
    const axis = RIGHT_AXIS_KEYS.includes(k) ? '右轴' : '左轴';
    return `<button class="legend-chip ${hidden ? 'off' : ''}" data-key="${k}" style="--c:${CHART_COLORS[i % CHART_COLORS.length]}"><i></i>${esc(m.label)}${m.unit ? ` (${m.unit})` : ''}<em>${axis}</em></button>`;
  }).join('');
  box.querySelectorAll('.legend-chip').forEach(chip => {
    chip.onclick = () => {
      const key = chip.dataset.key;
      hiddenSeries.has(key) ? hiddenSeries.delete(key) : hiddenSeries.add(key);
      renderChartLegend();
      drawDetailChart(currentHistory);
    };
  });
}

// 计算轴范围（带 8% 内边距，避免折线贴边）
function axisRange(values) {
  if (!values.length) return { min: 0, max: 1 };
  let min = Math.min(...values), max = Math.max(...values);
  if (min === max) { min -= 1; max += 1; }
  const pad = (max - min) * 0.08;
  return { min: min - pad, max: max + pad };
}
function fmtAxis(v) { return Math.abs(v) >= 1000 ? (v / 1000).toFixed(1) + 'k' : (Math.round(v * 10) / 10).toString(); }

function drawDetailChart(list) {
  const canvas = document.getElementById('detailChart');
  const emptyBox = document.getElementById('chartEmpty');
  const ctx = setupCanvas(canvas);
  const W = canvas.clientWidth, H = canvas.clientHeight;
  const pad = { l: 46, r: 52, t: 14, b: 30 };
  ctx.clearRect(0, 0, W, H);

  const allKeys = chartSeriesKeys();
  const keys = allKeys.filter(k => !hiddenSeries.has(k));
  // 数据按时间正序
  const data = (list || []).slice().reverse();
  // 至少要有一条可见序列、且该序列有 >=2 个有效点才画线
  const series = keys.map((k, idx) => ({
    key: k,
    color: CHART_COLORS[allKeys.indexOf(k) % CHART_COLORS.length],
    points: data.map((r, i) => ({ i, v: Number(r[k]) })).filter(p => Number.isFinite(p.v))
  })).filter(s => s.points.length >= 2);

  if (!series.length || data.length < 2) {
    emptyBox.classList.remove('hidden');
    canvas.style.visibility = 'hidden';
    return;
  }
  emptyBox.classList.add('hidden');
  canvas.style.visibility = 'visible';

  // 分左右轴
  const leftSeries = series.filter(s => !RIGHT_AXIS_KEYS.includes(s.key));
  const rightSeries = series.filter(s => RIGHT_AXIS_KEYS.includes(s.key));
  const leftRange = axisRange(leftSeries.flatMap(s => s.points.map(p => p.v)));
  const rightRange = rightSeries.length ? axisRange(rightSeries.flatMap(s => s.points.map(p => p.v))) : null;
  const N = data.length;
  const xOf = i => pad.l + i * (W - pad.l - pad.r) / Math.max(1, N - 1);
  const yLeft = v => pad.t + (leftRange.max - v) / (leftRange.max - leftRange.min) * (H - pad.t - pad.b);
  const yRight = v => pad.t + (rightRange.max - v) / (rightRange.max - rightRange.min) * (H - pad.t - pad.b);

  // 网格 + 左轴刻度（5 格）
  ctx.strokeStyle = 'rgba(79,123,158,.15)';
  ctx.fillStyle = '#7d92a8';
  ctx.font = '10px Microsoft YaHei';
  ctx.lineWidth = 1;
  ctx.setLineDash([3, 5]);
  const rows = 5;
  for (let i = 0; i <= rows; i++) {
    const y = pad.t + i * (H - pad.t - pad.b) / rows;
    ctx.beginPath(); ctx.moveTo(pad.l, y); ctx.lineTo(W - pad.r, y); ctx.stroke();
    ctx.textAlign = 'right';
    ctx.fillText(fmtAxis(leftRange.max - (leftRange.max - leftRange.min) * i / rows), pad.l - 7, y + 3);
    if (rightRange) {
      ctx.textAlign = 'left';
      ctx.fillStyle = '#d99a2e';
      ctx.fillText(fmtAxis(rightRange.max - (rightRange.max - rightRange.min) * i / rows), W - pad.r + 7, y + 3);
      ctx.fillStyle = '#7d92a8';
    }
  }
  ctx.setLineDash([]);

  // X 轴时间刻度（首/中/尾，取 HH:MM:SS）
  ctx.fillStyle = '#7d92a8';
  ctx.textAlign = 'center';
  const tickIdx = [0, Math.floor((N - 1) / 2), N - 1];
  tickIdx.forEach(i => {
    const t = (data[i]?.timestamp || '').split(' ')[1] || '';
    ctx.fillText(t, xOf(i), H - 9);
  });

  // 轴线标题
  ctx.textAlign = 'left';
  ctx.fillStyle = '#7d92a8';
  ctx.fillText('温度/湿度等', 4, 10);
  if (rightRange) { ctx.fillStyle = '#d99a2e'; ctx.textAlign = 'right'; ctx.fillText('光照 lx', W - 4, 10); }

  // 画每条曲线（带圆点 + 悬浮感）
  series.forEach(s => {
    const yFn = RIGHT_AXIS_KEYS.includes(s.key) ? yRight : yLeft;
    ctx.beginPath();
    s.points.forEach((p, i) => {
      const x = xOf(p.i), y = yFn(p.v);
      i ? ctx.lineTo(x, y) : ctx.moveTo(x, y);
    });
    ctx.strokeStyle = s.color;
    ctx.lineWidth = 2;
    ctx.shadowBlur = 6;
    ctx.shadowColor = s.color;
    ctx.stroke();
    ctx.shadowBlur = 0;
    // 数据点
    ctx.fillStyle = s.color;
    s.points.forEach(p => {
      ctx.beginPath();
      ctx.arc(xOf(p.i), yFn(p.v), 2.2, 0, Math.PI * 2);
      ctx.fill();
    });
  });
}
function renderHistoryTable(list) { const body = document.getElementById('historyBody'); if (!list.length) { body.innerHTML = '<tr><td colspan="4">本地暂无记录</td></tr>'; return } const rows = []; list.slice(0, 14).forEach(row => { const keys = Object.keys(row).filter(k => !['project', 'name', 'timestamp', 'online'].includes(k)); keys.slice(0, 2).forEach(k => rows.push({ time: row.timestamp, key: k, val: row[k] })) }); body.innerHTML = rows.slice(0, 18).map(r => `<tr><td>${esc(r.time || '--')}</td><td>${esc(r.key)}</td><td>${esc(displayValue(r.key, r.val))}</td><td><span class="ok-tag">● 写入成功</span></td></tr>`).join('') }
function setupCanvas(canvas) { const dpr = window.devicePixelRatio || 1; const rect = canvas.getBoundingClientRect(); canvas.width = Math.max(1, Math.floor(rect.width * dpr)); canvas.height = Math.max(1, Math.floor(rect.height * dpr)); const ctx = canvas.getContext('2d'); ctx.setTransform(dpr, 0, 0, dpr, 0, 0); return ctx }
function drawGrid(ctx, W, H, pad, rows) { ctx.strokeStyle = 'rgba(79,123,158,.18)'; ctx.lineWidth = 1; ctx.setLineDash([3, 5]); for (let i = 0; i <= rows; i++) { const y = pad.t + i * (H - pad.t - pad.b) / rows; ctx.beginPath(); ctx.moveTo(pad.l, y); ctx.lineTo(W - pad.r, y); ctx.stroke() } ctx.setLineDash([]) }
function drawEmptyCanvas(ctx, W, H, text) { ctx.fillStyle = '#5f7891'; ctx.font = '12px Microsoft YaHei'; ctx.textAlign = 'center'; ctx.fillText(text, W / 2, H / 2) }

function displayValue(field, v) { if (v === undefined || v === null || v === '') return '--'; if (field === 'mode') return ({ 0: '普通模式', 1: '节能模式', 2: '演示模式' })[Number(v)] ?? String(v); if (field === 'flight_mode') return ({ 0: '待机', 1: '起飞', 2: '悬停', 3: '降落' })[Number(v)] ?? String(v); if (['pir_detected', 'person_detected', 'in_nest'].includes(field)) return truthy(v) ? '有' : '无'; if (field === 'lid_open') return truthy(v) ? '开启' : '关闭'; if (field === 'feed_reminder') return truthy(v) ? '需要喂食' : '正常'; if (field === 'alarm' || field === 'alarm_flag') return truthy(v) ? '异常' : '正常'; if (field === 'compressing') return truthy(v) ? '压缩中' : '待机'; if (field === 'irrigating') return truthy(v) ? '灌溉中' : '停止'; if (field === 'shade_closed') return truthy(v) ? '关闭' : '开启'; const m = meta[field]; return `${v}${m?.unit ? ` ${m.unit}` : ''}` }
function truthy(v) { return v === true || v === 1 || v === '1' || v === 'true' }
function statusText(field, v) { if (v === undefined || v === null || v === '') return '等待采集'; if (field === 'temperature') { const n = Number(v); return n >= 18 && n <= 30 ? '舒适' : '需关注' } if (field === 'humidity') { const n = Number(v); return n >= 30 && n <= 80 ? '舒适' : '需关注' } if (field === 'light') return Number(v) > 0 ? '采集中' : '无光照'; if (['alarm', 'alarm_flag', 'feed_reminder'].includes(field)) return truthy(v) ? '异常' : '正常'; if (['pir_detected', 'person_detected', 'in_nest'].includes(field)) return truthy(v) ? '检测到' : '未检测'; return '实时采集' }

function buildEvents(data) {
  const events = [];
  const now = (data.timestamp || '--').split(' ')[1] || '--';
  // 结果标签：normal=成功（绿）/ warning=关注（橙）/ danger=失败/异常（红）
  const resultMap = { normal: ['成功', 'ok'], warning: ['关注', 'warn'], danger: ['异常', 'fail'] };
  if (data.temperature !== undefined) {
    const n = Number(data.temperature);
    const bad = n > 30;
    events.push({ type: bad ? 'danger' : 'normal', icon: bad ? '!' : '✓', title: bad ? '温度偏高' : '温度状态正常', detail: `当前 ${displayValue('temperature', data.temperature)}`, time: now });
  }
  if (data.humidity !== undefined) {
    const n = Number(data.humidity);
    const bad = n < 30 || n > 80;
    events.push({ type: bad ? 'warning' : 'normal', icon: bad ? '!' : '◉', title: bad ? '湿度超出舒适区间' : '湿度状态正常', detail: `当前 ${displayValue('humidity', data.humidity)}`, time: now });
  }
  if (data.light !== undefined) {
    const n = Number(data.light);
    const bad = n < 100;
    events.push({ type: bad ? 'warning' : 'normal', icon: bad ? '!' : '◉', title: bad ? '光照强度偏低' : '光照数据已更新', detail: `当前 ${displayValue('light', data.light)}`, time: now });
  }
  if (data.pir_detected !== undefined) events.push({ type: truthy(data.pir_detected) ? 'normal' : 'warning', icon: '♙', title: truthy(data.pir_detected) ? '检测到人员' : '当前未检测到人员', detail: '人体红外传感器状态', time: now });
  if (data.alarm || data.alarm_flag) events.unshift({ type: 'danger', icon: '!', title: '设备报警', detail: '设备上报异常标志，请检查现场', time: now });
  const box = document.getElementById('eventList');
  box.innerHTML = events.length ? events.slice(0, 5).map(e => {
    const [tagText, tagCls] = resultMap[e.type] || resultMap.normal;
    return `<div class="event-item ${e.type}"><span class="event-icon">${e.icon}</span><div><strong>${e.title}</strong><small>${e.detail}</small></div><em class="event-result ${tagCls}">${tagText}</em><time>${e.time}</time></div>`;
  }).join('') : '<div class="empty">暂无事件</div>';
}

async function sendCommand(cmd) { if (!currentProject) return; const status = document.getElementById('commandStatus'); status.textContent = `正在发送命令 ${cmd} ...`; try { const res = await fetch(`/api/command/${currentProject.id}`, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ cmd }) }); const data = await res.json(); if (!res.ok) throw new Error(data.message || '发送失败'); status.textContent = data.delivered === 'queued' ? `◷ ${data.message}` : `✓ ${data.message}`; toast(data.message || '命令发送成功') } catch (e) { status.textContent = `✕ ${e.message || '发送失败'}`; toast(e.message || '命令发送失败', 'error') } }
function toast(text, type = '') { const el = document.getElementById('toast'); el.textContent = text; el.className = `toast ${type} show`; clearTimeout(el._t); el._t = setTimeout(() => el.className = 'toast', 2200) }

document.getElementById('backBtn').onclick = closeProject; document.getElementById('refreshHistoryBtn').onclick = fetchHistory; document.getElementById('refreshAllBtn').onclick = refreshOverview; document.getElementById('activityRefresh').onclick = loadOverviewActivity; document.getElementById('sendCustomBtn').onclick = () => { const input = document.getElementById('customCmd'); const cmd = input.value.trim(); if (cmd.length !== 1) { toast('请输入一个字符命令', 'error'); return } sendCommand(cmd); input.value = '' }; document.getElementById('customCmd').addEventListener('keydown', e => { if (e.key === 'Enter') document.getElementById('sendCustomBtn').click() });
// 立即采集：手动触发一次实时拉取（带按钮转动反馈）
document.getElementById('collectNowBtn').onclick = async (e) => { const btn = e.currentTarget; btn.classList.add('spinning'); btn.disabled = true; await fetchLatest(); setTimeout(() => { btn.classList.remove('spinning'); btn.disabled = false; }, 500); toast('已采集最新数据', 'success') };
// 趋势图时间范围切换
document.querySelectorAll('#rangeSelect .range-btn').forEach(b => b.onclick = () => { document.querySelectorAll('#rangeSelect .range-btn').forEach(x => x.classList.remove('active')); b.classList.add('active'); chartLimit = Number(b.dataset.limit); fetchHistory(); });

// ========= 页面路由 =========
const PAGE_MAP = { home: 'overviewPage', projects: 'projectsPage', devices: 'devicesPage', monitor: 'monitorPage', alerts: 'alertsPage', storage: 'storagePage' };
let currentPage = 'home', severityFilter = 'all', unresolvedOnly = false, deviceFilter = 'all', editingProject = null;
document.querySelectorAll('.nav-item').forEach(btn => btn.addEventListener('click', () => switchPage(btn.dataset.page)));
function switchPage(name) { clearInterval(pollTimer); currentProject = null; document.querySelectorAll('.nav-item').forEach(x => x.classList.remove('active')); document.querySelector(`.nav-item[data-page="${name}"]`)?.classList.add('active'); Object.values(PAGE_MAP).forEach(id => document.getElementById(id).classList.add('hidden')); document.getElementById('detailPage').classList.add('hidden'); document.getElementById(PAGE_MAP[name]).classList.remove('hidden'); currentPage = name; window.scrollTo({ top: 0, behavior: 'smooth' }); if (name === 'projects') renderProjectsPage(); if (name === 'devices') renderDevicesPage(); if (name === 'monitor') renderMonitorPage(); if (name === 'alerts') renderAlertsPage(); if (name === 'storage') renderStoragePage() }

function severityColor(s) { return { danger: '#ff5566', warning: '#ffb23b', info: '#3aa8ff' }[s] || '#95a7bd' }
function fmtBytes(b) { if (!b) return '0 B'; const u = ['B', 'KB', 'MB', 'GB']; let i = 0; while (b >= 1024 && i < u.length - 1) { b /= 1024; i++ } return `${b.toFixed(1)} ${u[i]}` }
function fmtDuration(s) { s = Number(s) || 0; if (s < 60) return `${s} 秒`; if (s < 3600) return `${Math.floor(s / 60)}分${s % 60}秒`; return `${Math.floor(s / 3600)}时${Math.floor(s % 3600 / 60)}分` }

// ========= 1. 项目管理 =========
async function renderProjectsPage() {
  const body = document.getElementById('projectsBody'); body.innerHTML = '<tr><td colspan="8">加载中…</td></tr>'; let list = []; try { const r = await fetch('/api/project'); if (r.ok) list = await r.json() } catch { }
  if (!list.length) {// 没配置时从前端预置兜底
    list = projects.map(p => ({ id: p.id, name: p.name, icon: p.icon, accent: p.accent, description: p.desc, fields: p.fields, commands: p.commands.map(c => ({ cmd: c[0], text: c[1] })), status: 1 }))
  }
  body.innerHTML = list.map(x => `<tr><td><strong>${x.id}</strong></td><td style="color:${x.accent};font-size:18px">${esc(x.icon || '◇')}</td><td><strong>${esc(x.name)}</strong></td><td><span class="color-dot" style="background:${x.accent}"></span>${esc(x.accent || '')}</td><td>${esc(x.description || '--')}</td><td><span class="ok-tag ${x.status === 0 ? 'danger' : ''}">${x.status === 0 ? '禁用' : '启用'}</span></td><td><span class="ok-tag ${x.online ? '' : 'danger'}">${x.online ? '● 在线' : '○ 离线'}</span></td><td><button class="text-btn" data-edit="${x.id}">✎ 编辑</button> <button class="text-btn red" data-del="${x.id}">🗑 删除</button></td></tr>`).join('') || '<tr><td colspan="8">暂无数据</td></tr>';
  document.querySelectorAll('[data-edit]').forEach(b => b.onclick = () => openProjectModal(list.find(x => x.id == b.dataset.edit)));
  document.querySelectorAll('[data-del]').forEach(b => b.onclick = async () => { if (!confirm(`确认删除项目 ${b.dataset.del}？`)) return; const id = Number(b.dataset.del); try { const r = await fetch(`/api/project/${id}`, { method: 'DELETE' }); const d = await r.json(); if (d.status === 'ok') { toast('删除成功', 'success'); await loadProjects(); renderProjectCards(); } else toast('删除失败: ' + d.message, 'error') } catch (e) { toast('删除失败: ' + e.message, 'error') } renderProjectsPage() })
}
document.getElementById('refreshProjectsBtn').onclick = renderProjectsPage;
document.getElementById('addProjectBtn').onclick = () => openProjectModal(null);
function openProjectModal(x) { editingProject = x || null; const isEdit = !!x; document.getElementById('modalTitle').textContent = isEdit ? '编辑项目' : '新增项目'; document.getElementById('m_id').value = x?.id ?? ''; document.getElementById('m_name').value = x?.name ?? ''; document.getElementById('m_icon').value = x?.icon ?? '◇'; document.getElementById('m_accent').value = x?.accent ?? '#2f7ad9'; document.getElementById('m_desc').value = x?.description ?? ''; document.getElementById('m_fields').value = (x?.fields || []).join(','); const cmds = Array.isArray(x?.commands) ? x.commands.map(c => typeof c === 'object' ? `${c.cmd}:${c.text}` : `${c[0]}:${c[1]}`).join(',') : ''; document.getElementById('m_commands').value = cmds; document.getElementById('m_status').value = x?.status === 0 ? '0' : '1'; document.getElementById('projectModal').classList.remove('hidden') }
function closeModal() { document.getElementById('projectModal').classList.add('hidden'); editingProject = null }
document.getElementById('closeModalBtn').onclick = closeModal; document.getElementById('cancelModalBtn').onclick = closeModal;
document.getElementById('saveProjectBtn').onclick = async () => { const rawId = document.getElementById('m_id').value.trim(); const id = Number(rawId); const name = document.getElementById('m_name').value.trim(); if (rawId === '' || !Number.isInteger(id) || id < 0 || !name) { toast('ID 必须是非负整数，项目名必填', 'error'); return } const fields = document.getElementById('m_fields').value.split(',').map(s => s.trim()).filter(Boolean); const commands = document.getElementById('m_commands').value.split(',').map(s => { const p = s.split(':'); if (p.length < 2) return null; return { cmd: p[0].trim(), text: p.slice(1).join(':').trim() } }).filter(Boolean); const body = { id, name, icon: document.getElementById('m_icon').value.trim() || '◇', accent: document.getElementById('m_accent').value || '#2f7ad9', description: document.getElementById('m_desc').value, fields, commands, status: parseInt(document.getElementById('m_status').value || '1') }; try { const r = await fetch('/api/project', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(body) }); const d = await r.json(); if (d.status === 'ok') { toast('保存成功', 'success'); closeModal(); await loadProjects(); renderProjectCards(); renderProjectsPage() } else toast('保存失败: ' + d.message, 'error') } catch (e) { toast('保存失败: ' + e.message, 'error') } };

// ========= 2. 设备管理 =========
async function renderDevicesPage() {
  try { const r = await fetch('/api/device/summary'); if (r.ok) { const d = await r.json(); document.getElementById('devTotal').textContent = d.total || 0; document.getElementById('devOnline').textContent = d.online || 0; document.getElementById('devPackets').textContent = d.packetsReceived || 0; document.getElementById('devCmds').textContent = d.commandsSent || 0 } } catch { }
  const body = document.getElementById('devicesBody'); body.innerHTML = '<tr><td colspan="10">加载中…</td></tr>'; let list = []; try { const url = deviceFilter === 'online' ? '/api/device?onlineOnly=true&limit=200' : '/api/device?limit=200'; const r = await fetch(url); if (r.ok) list = await r.json() } catch { }
  if (deviceFilter === 'online') list = list.filter(s => s.status === 'online');
  body.innerHTML = list.map(s => `<tr><td>#${s.id}</td><td>${s.projectId} · ${esc(s.projectName || '')}</td><td>${esc(s.remoteIp || '--')}:${s.remotePort ?? ''}</td><td>${esc(s.connectedAt || '--').replace('T', ' ')}</td><td>${s.packetsReceived ?? 0}</td><td>${fmtBytes(s.bytesReceived)}</td><td>${s.commandsSent ?? 0}</td><td>${fmtDuration(s.elapsedSec || s.durationSec)}</td><td><span class="ok-tag ${s.status === 'online' ? '' : 'danger'}">${s.status === 'online' ? '● 在线' : '○ 离线'}</span></td><td>${s.status === 'online' ? `<button class="text-btn red" data-off="${s.id}">下线</button>` : '—'}</td></tr>`).join('') || '<tr><td colspan="10">暂无会话记录</td></tr>';
  document.querySelectorAll('[data-off]').forEach(b => b.onclick = async () => { try { await fetch(`/api/device/${b.dataset.off}/offline`, { method: 'POST' }); toast('已标记离线', 'success') } catch { toast('操作失败', 'error') } renderDevicesPage() })
}
document.querySelectorAll('.device-summary + .panel .filter-row .text-btn[data-filter]').forEach(b => b.addEventListener('click', () => { document.querySelectorAll('.device-summary + .panel .filter-row .text-btn[data-filter]').forEach(x => x.classList.remove('active')); b.classList.add('active'); deviceFilter = b.dataset.filter; renderDevicesPage() }));
document.getElementById('refreshDevicesBtn').onclick = renderDevicesPage;

// ========= 3. 数据监控 =========
async function renderMonitorPage() {
  const rows = [];
  for (const p of projects) {
    try {
      const r = await fetch(`/api/sensor/${p.id}`);
      if (!r.ok) continue;
      const d = await r.json();
      rows.push({ _p: p, ...d });
    } catch { }
  }
  const box = document.getElementById('monitorGrid');
  box.innerHTML = '';
  rows.filter(r => !!r.online || Object.keys(r).length > 4).slice(0, 28).forEach(r => {
    const p = r._p;
    p.fields.forEach(f => {
      if (r[f] === undefined) return;
      const m = meta[f] || { label: f, unit: '', icon: '•', color: '#3aa8ff' };
      const div = document.createElement('div');
      div.className = 'sensor-item';
      div.style.setProperty('--metric', m.color);
      div.innerHTML = `<span class="metric-icon">${m.icon}</span><div><div class="label">${esc(p.name)} · ${esc(m.label)}</div><div class="value">${esc(displayValue(f, r[f]))}</div><div class="sub">${esc(statusText(f, r[f]))}</div></div>`;
      box.appendChild(div);
    });
  });
  if (!box.children.length) box.innerHTML = '<div class="empty" style="padding:28px">暂无设备数据，等待 TCP 上报…</div>';

  const cbody = document.getElementById('monitorCompareBody');
  cbody.innerHTML = rows.map(r => {
    const p = r._p;
    const vals = ['temperature', 'humidity', 'light'].map(k => r[k] !== undefined ? displayValue(k, r[k]) : '—');
    const extra = Object.keys(r).filter(k => !['_p', 'project', 'name', 'online', 'timestamp', 'temperature', 'humidity', 'light', 'id'].includes(k)).slice(0, 4).map(k => `${k}:${displayValue(k, r[k])}`).join(' · ');
    return `<tr><td>${p.id}</td><td><strong>${esc(p.name)}</strong></td><td>${vals[0]}</td><td>${vals[1]}</td><td>${vals[2]}</td><td>${esc(extra || '—')}</td><td><span class="ok-tag ${r.online ? '' : 'danger'}">${r.online ? '● 在线' : '○ 离线'}</span></td><td>${esc(r.timestamp || '--')}</td></tr>`;
  }).join('') || '<tr><td colspan="8">暂无数据</td></tr>';
  await drawMonitorCharts();
}

async function drawMonitorCharts() {
  const histories = await Promise.all(projects.slice(0, 7).map(async p => {
    try {
      const r = await fetch(`/api/sensor/${p.id}/history?limit=20`);
      return { p, list: r.ok ? await r.json() : [] };
    } catch { return { p, list: [] }; }
  }));

  for (const tag of ['Temp', 'Hum']) {
    const key = tag === 'Temp' ? 'temperature' : 'humidity';
    const canvas = document.getElementById('monitorChart' + tag);
    const ctx = setupCanvas(canvas);
    const W = canvas.clientWidth, H = canvas.clientHeight;
    const pad = { l: 38, r: 12, t: 30, b: 30 };
    ctx.clearRect(0, 0, W, H);
    drawGrid(ctx, W, H, pad, 5);

    const series = histories.map(({ p, list }) => ({
      p,
      values: list.slice().reverse().map(x => Number(x[key])).filter(Number.isFinite)
    })).filter(x => x.values.length > 0);
    if (!series.length) { drawEmptyCanvas(ctx, W, H, `暂无${tag === 'Temp' ? '温度' : '湿度'}历史数据`); continue; }

    const all = series.flatMap(x => x.values);
    const min = Math.min(...all), max = Math.max(...all), range = max - min || 1;
    const palette = ['#ff6b7a', '#ffb23b', '#3aa8ff', '#2fd5a0', '#a568ff', '#32cbff', '#ffc857'];
    series.forEach((serie, si) => {
      const vals = serie.values;
      ctx.beginPath();
      vals.forEach((v, i) => {
        const x = pad.l + i * (W - pad.l - pad.r) / Math.max(1, vals.length - 1);
        const y = pad.t + (max - v) / range * (H - pad.t - pad.b);
        i ? ctx.lineTo(x, y) : ctx.moveTo(x, y);
      });
      ctx.strokeStyle = palette[si % palette.length];
      ctx.lineWidth = 2;
      ctx.stroke();
    });
    ctx.font = '11px Microsoft YaHei';
    ctx.textAlign = 'left';
    series.slice(0, 4).forEach((serie, i) => {
      ctx.fillStyle = palette[i % palette.length];
      ctx.fillText(serie.p.name, 12 + i * Math.max(70, W / 4), 16);
    });
  }
}
document.getElementById('monitorRefreshBtn').onclick = renderMonitorPage;

// ========= 4. 告警中心 =========
async function renderAlertsPage() {
  try { const r = await fetch('/api/alert/summary'); if (r.ok) { const d = await r.json(); document.getElementById('alertDanger').textContent = d.danger || 0; document.getElementById('alertWarning').textContent = d.warning || 0; document.getElementById('alertInfo').textContent = d.info || 0; document.getElementById('alertUnresolved').textContent = d.unresolved || 0 } } catch { }
  const body = document.getElementById('alertsBody'); body.innerHTML = '<tr><td colspan="11">加载中…</td></tr>'; let list = []; try { const params = new URLSearchParams(); if (severityFilter !== 'all') params.set('severity', severityFilter); if (unresolvedOnly) params.set('unresolvedOnly', '1'); params.set('limit', '200'); const r = await fetch('/api/alert?' + params.toString()); if (r.ok) list = await r.json() } catch { }
  body.innerHTML = list.map(a => `<tr><td>#${a.id}</td><td>${esc(a.createdAt || '--').replace('T', ' ')}</td><td>${a.projectId} · ${esc(a.projectName || '')}</td><td><span class="severity-dot" style="background:${severityColor(a.severity)}"></span>${a.severity || 'info'}</td><td>${esc(a.alertType || '')}</td><td>${esc(a.fieldName || '')}</td><td>${esc(a.fieldValue || '')}</td><td>${esc(a.threshold || '')}</td><td>${esc(a.message || '')}</td><td><span class="ok-tag ${a.resolved ? '' : 'danger'}">${a.resolved ? '✓ 已处理' : '⏳ 待处理'}</span></td><td>${!a.resolved ? `<button class="text-btn" data-rslv="${a.id}">✓ 处理</button>` : ''}<button class="text-btn red" data-ad="${a.id}">🗑</button></td></tr>`).join('') || '<tr><td colspan="11">暂无告警记录</td></tr>';
  document.querySelectorAll('[data-rslv]').forEach(b => b.onclick = async () => { try { const r = await fetch(`/api/alert/${b.dataset.rslv}/resolve`, { method: 'POST' }); const d = await r.json(); toast(d.status === 'ok' ? '已标记处理' : '操作失败', d.status === 'ok' ? 'success' : 'error') } catch { toast('操作失败', 'error') } renderAlertsPage() });
  document.querySelectorAll('[data-ad]').forEach(b => b.onclick = async () => { if (!confirm('确认删除这条告警？')) return; try { await fetch(`/api/alert/${b.dataset.ad}`, { method: 'DELETE' }); toast('已删除', 'success') } catch { toast('删除失败', 'error') } renderAlertsPage() })
}
document.querySelectorAll('.section-heading .filter-row .text-btn[data-sev]').forEach(b => b.onclick = () => { document.querySelectorAll('.section-heading .filter-row .text-btn[data-sev]').forEach(x => x.classList.remove('active')); b.classList.add('active'); severityFilter = b.dataset.sev; renderAlertsPage() });
document.getElementById('alertsOnlyUnresolved').onclick = e => { e.target.classList.toggle('active'); unresolvedOnly = e.target.classList.contains('active'); renderAlertsPage() };
document.getElementById('resolveAllBtn').onclick = async () => { if (!confirm('批量标记所有告警为已处理？')) return; try { const r = await fetch('/api/alert/resolve-all', { method: 'POST' }); const d = await r.json(); toast(d.status === 'ok' ? `已处理 ${d.affected || 0} 条告警` : '处理失败', d.status === 'ok' ? 'success' : 'error') } catch { toast('处理失败', 'error') } renderAlertsPage() };
document.getElementById('refreshAlertsBtn').onclick = renderAlertsPage;

// ========= 5. 数据存储 =========
let storageCache = [];
async function renderStoragePage() {
  const sel = document.getElementById('storagePidFilter'); if (sel.options.length <= 1) sel.innerHTML = '<option value="">全部项目</option>' + projects.map(p => `<option value="${p.id}">${p.id} - ${p.name}</option>`).join(''); const body = document.getElementById('storageBody'); body.innerHTML = '<tr><td colspan="8">加载中…</td></tr>'; const rows = []; for (const p of projects) { try { const r = await fetch(`/api/sensor/${p.id}/history?limit=80`); if (!r.ok) continue; const list = await r.json(); list.forEach(x => rows.push({ ...x, _pid: p.id, _name: p.name })) } catch { } }
  rows.sort((a, b) => String(b.timestamp).localeCompare(String(a.timestamp))); storageCache = rows; filterStorage()
}
function filterStorage() { const pid = document.getElementById('storagePidFilter').value; const kw = document.getElementById('storageKeyword').value.trim().toLowerCase(); const body = document.getElementById('storageBody'); let list = storageCache.slice(); if (pid) list = list.filter(r => r._pid == pid); if (kw) list = list.filter(r => Object.entries(r).some(([k, v]) => k !== '_p' && String(v).toLowerCase().includes(kw))); body.innerHTML = list.slice(0, 200).map(r => { const extras = Object.keys(r).filter(k => !['_p', 'project', 'name', 'timestamp', 'online', 'temperature', 'humidity', 'light', '_pid', '_name', 'id'].includes(k)).slice(0, 4).map(k => `${k}=${displayValue(k, r[k])}`).join(' · '); return `<tr><td>${r.id || '—'}</td><td>${esc(r.timestamp || '--')}</td><td>${r._pid}</td><td>${esc(r._name || r.name || '')}</td><td>${r.temperature !== undefined ? displayValue('temperature', r.temperature) : '—'}</td><td>${r.humidity !== undefined ? displayValue('humidity', r.humidity) : '—'}</td><td>${r.light !== undefined ? displayValue('light', r.light) : '—'}</td><td>${esc(extras || '—')}</td></tr>` }).join('') || '<tr><td colspan="8">没有匹配的记录</td></tr>' }
document.getElementById('storagePidFilter').onchange = filterStorage;
document.getElementById('storageKeyword').addEventListener('input', filterStorage);
document.getElementById('refreshStorageBtn').onclick = renderStoragePage;
document.getElementById('exportCsvBtn').onclick = () => { if (!storageCache.length) { toast('暂无数据可导出', 'warning'); return } const header = ['时间', '项目ID', '项目名', '温度', '湿度', '光照', '其他字段']; const lines = [header.join(',')]; storageCache.slice(0, 500).forEach(r => { const extras = Object.keys(r).filter(k => !['_p', 'project', 'name', 'timestamp', 'online', 'temperature', 'humidity', 'light', '_pid', '_name', 'id'].includes(k)).map(k => `${k}=${r[k] ?? ''}`).join(' | '); lines.push([r.timestamp, r._pid, r._name || r.name, r.temperature ?? '', r.humidity ?? '', r.light ?? '', extras].map(v => `"${String(v ?? '').replace(/"/g, '""')}"`).join(',')) }); const blob = new Blob(['\ufeff' + lines.join('\n')], { type: 'text/csv' }); const url = URL.createObjectURL(blob); const a = document.createElement('a'); a.href = url; a.download = `sensor_export_${new Date().toISOString().slice(0, 10)}.csv`; a.click(); URL.revokeObjectURL(url); toast('CSV 已导出', 'success') };

window.addEventListener('resize', () => { if (currentProject) drawDetailChart(currentHistory); else if (currentPage === 'monitor') {/* re-render optional */ } else loadOverviewActivity() });

/* ===== [增强] TCP 通信日志实时轮询 ===== */
let commLogPaused = false;
let commLogHtmlCache = '';

async function refreshCommLogs() {
  const list = document.getElementById('commLogList');
  if (!list || commLogPaused) return;
  try {
    const r = await fetch('/api/comm-logs?limit=50');
    if (!r.ok) return;
    const logs = await r.json();
    if (!Array.isArray(logs) || logs.length === 0) {
      list.innerHTML = '<div class="empty">等待 TCP 通信…</div>';
      return;
    }
    const html = logs.map(l => {
      const cls = l.direction === 'RECEIVED' ? 'received' : 'sent';
      const dirLabel = l.direction === 'RECEIVED' ? '←' : '→';
      return `<div class="comm-log-row ${cls}">
        <span class="cl-time">${l.time}</span>
        <span class="cl-dir">${dirLabel}</span>
        <span class="cl-pid">#${l.projectId}</span>
        <span class="cl-addr">${l.remoteAddr}</span>
        <span class="cl-msg">${escapeHtml(l.content)}</span>
      </div>`;
    }).join('');
    // 只有内容变化才重绘，避免闪烁
    if (html !== commLogHtmlCache) {
      commLogHtmlCache = html;
      list.innerHTML = html;
      list.scrollTop = 0; // 最新在顶
    }
  } catch { }
}
function escapeHtml(s) {
  return String(s).replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
}
// 事件绑定
document.getElementById('commLogPauseBtn')?.addEventListener('click', (e) => {
  commLogPaused = !commLogPaused;
  e.target.textContent = commLogPaused ? '▶ 继续' : '⏸ 暂停';
  const dot = document.getElementById('commLogLiveDot');
  if (dot) dot.classList.toggle('paused', commLogPaused);
  if (!commLogPaused) refreshCommLogs();
});
document.getElementById('commLogClearBtn')?.addEventListener('click', async () => {
  await fetch('/api/comm-logs/clear');
  document.getElementById('commLogList').innerHTML = '<div class="empty">已清空，等待新通信…</div>';
  commLogHtmlCache = '';
});
// 每 2 秒轮询一次通信日志
setInterval(refreshCommLogs, 2000);

(async function initApp() { await loadProjects(); renderProjectCards(); await refreshOverview(); refreshCommLogs(); })(); setInterval(() => { if (currentPage === 'home' && !currentProject) refreshOverview() }, 10000);
