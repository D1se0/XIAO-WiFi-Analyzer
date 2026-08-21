#ifndef WEB_CONTENT_H
#define WEB_CONTENT_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"HTMLPAGE(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0">
<title>XIAO WiFi Analyzer</title>
<link rel="stylesheet" href="/style.css">
</head>
<body>
  <header class="topbar">
    <h1>XIAO WIFI ANALYZER</h1>
    <div class="status-pill" id="status-pill">STATUS: <span id="status-text">ONLINE</span></div>
  </header>

  <main>
    <section class="summary-grid">
      <div class="card"><div class="card-value" id="stat-total">-</div><div class="card-label">Networks Found</div></div>
      <div class="card"><div class="card-value" id="stat-24">-</div><div class="card-label">2.4 GHz</div></div>
      <div class="card"><div class="card-value" id="stat-5">-</div><div class="card-label">5 GHz</div></div>
      <div class="card"><div class="card-value" id="stat-open">-</div><div class="card-label">Open Networks</div></div>
    </section>

    <section class="panel">
      <button id="scan-btn" onclick="startScan()">SCAN NETWORKS</button>
      <div id="scan-progress-wrap" class="progress-wrap" style="display:none">
        <div class="progress-bg"><div id="scan-progress-fill" class="progress-fill"></div></div>
        <div id="scan-progress-label" class="scan-state">SCANNING... (0%)</div>
      </div>
      <div class="scan-meta-row">
        <span>Last scan: <span id="stat-duration">-</span></span>
        <label class="auto-refresh-label">
          <input type="checkbox" id="auto-refresh-toggle"> Auto-scan every 30s
        </label>
      </div>
    </section>

    <section class="panel">
      <h2>DEVICE STATUS</h2>
      <div class="device-grid">
        <div><span class="device-label">Uptime</span><span class="device-value" id="dev-uptime">-</span></div>
        <div><span class="device-label">Free RAM</span><span class="device-value" id="dev-heap">-</span></div>
        <div><span class="device-label">Clients</span><span class="device-value" id="dev-clients">-</span></div>
      </div>
    </section>

    <section class="panel">
      <h2>SCAN HISTORY</h2>
      <div id="history-chart" class="history-chart"></div>
      <p class="note">Networks found in previous scans. Kept in memory only, resets on reboot.</p>
    </section>

    <section class="panel">
      <h2>SIGNAL STRENGTH DISTRIBUTION</h2>
      <div id="rssi-dist"></div>
    </section>

    <section class="panel">
      <h2>CHANNEL ANALYSIS (2.4 GHz)</h2>
      <div id="channel-chart" class="channel-chart"></div>
      <p id="channel-note" class="note"></p>
    </section>

    <section class="panel">
      <h2>SECURITY ANALYSIS</h2>
      <div id="security-analysis" class="security-list"></div>
    </section>

    <section class="panel">
      <h2>DETECTED NETWORKS</h2>
      <div class="net-controls">
        <input type="text" id="net-search" placeholder="Search SSID...">
        <select id="net-sort">
          <option value="rssi">Sort: Signal</option>
          <option value="channel">Sort: Channel</option>
          <option value="ssid">Sort: Name</option>
        </select>
      </div>
      <div id="networks-container"></div>
    </section>
  </main>

  <footer>
    <p>Passive &amp; defensive Wi-Fi analysis. Use only on your own or authorized networks.</p>
    <p class="note">Distance estimates are rough approximations based on signal strength only; real distance varies with walls, materials and antenna orientation.</p>
  </footer>

<script src="/app.js"></script>
</body>
</html>
)HTMLPAGE";

const char STYLE_CSS[] PROGMEM = R"CSSPAGE(
:root {
  --bg: #0b0f10;
  --panel: #12181a;
  --card: #171f21;
  --text: #e6f1f2;
  --sub: #8aa0a4;
  --accent: #00e5c7;
  --danger: #ff5470;
  --warn: #ffb84d;
  --ok: #4ade80;
  --border: #223034;
}
* { box-sizing: border-box; }
body {
  margin: 0;
  background: var(--bg);
  color: var(--text);
  font-family: system-ui, -apple-system, "Segoe UI", Roboto, sans-serif;
  padding-bottom: 40px;
}
.topbar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px;
  border-bottom: 1px solid var(--border);
  position: sticky;
  top: 0;
  background: var(--bg);
  z-index: 10;
}
.topbar h1 { font-size: 1.1rem; letter-spacing: 1px; margin: 0; color: var(--accent); }
.status-pill { font-size: 0.75rem; padding: 4px 10px; border: 1px solid var(--accent); border-radius: 20px; color: var(--accent); }
main { padding: 12px; max-width: 720px; margin: 0 auto; }
.summary-grid { display: grid; grid-template-columns: repeat(2, 1fr); gap: 10px; margin-bottom: 16px; }
@media (min-width: 600px) { .summary-grid { grid-template-columns: repeat(4, 1fr); } }
.card { background: var(--card); border: 1px solid var(--border); border-radius: 10px; padding: 14px; text-align: center; }
.card-value { font-size: 1.6rem; font-weight: 700; color: var(--accent); }
.card-label { font-size: 0.7rem; color: var(--sub); margin-top: 4px; text-transform: uppercase; letter-spacing: 0.5px; }
.panel { background: var(--panel); border: 1px solid var(--border); border-radius: 12px; padding: 16px; margin-bottom: 14px; }
.panel h2 { font-size: 0.85rem; letter-spacing: 1px; color: var(--sub); margin: 0 0 12px 0; text-transform: uppercase; }
#scan-btn {
  width: 100%; padding: 14px; background: var(--accent); color: #05201c; border: none;
  border-radius: 10px; font-weight: 700; letter-spacing: 1px; font-size: 0.95rem; cursor: pointer;
}
#scan-btn:active { opacity: 0.85; }
#scan-btn:disabled { opacity: 0.5; }
.progress-wrap { margin-top: 12px; }
.progress-bg { background: #0c1213; border: 1px solid var(--border); border-radius: 8px; height: 10px; overflow: hidden; }
.progress-fill { height: 100%; width: 0%; background: var(--accent); transition: width 0.3s ease; }
.scan-state { text-align: center; margin-top: 8px; font-size: 0.75rem; color: var(--sub); }
.scan-meta-row { display: flex; justify-content: space-between; align-items: center; margin-top: 10px; font-size: 0.72rem; color: var(--sub); flex-wrap: wrap; gap: 8px; }
.auto-refresh-label { display: flex; align-items: center; gap: 4px; cursor: pointer; }
.device-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; }
.device-grid > div { display: flex; flex-direction: column; gap: 4px; }
.device-label { font-size: 0.65rem; color: var(--sub); text-transform: uppercase; letter-spacing: 0.5px; }
.device-value { font-size: 1rem; font-weight: 700; color: var(--accent); }
.history-chart { display: flex; align-items: flex-end; gap: 6px; height: 70px; }
.history-col { flex: 1; display: flex; flex-direction: column; align-items: center; justify-content: flex-end; height: 100%; }
.history-bar { width: 100%; background: var(--accent); border-radius: 3px 3px 0 0; min-height: 2px; }
.history-label { font-size: 0.6rem; color: var(--sub); margin-top: 4px; }
.channel-row { display: flex; align-items: center; gap: 8px; margin-bottom: 6px; font-size: 0.75rem; }
.channel-label { width: 60px; color: var(--sub); }
.channel-bar-bg { flex: 1; background: #0c1213; border: 1px solid var(--border); border-radius: 6px; overflow: hidden; height: 14px; }
.channel-bar-fill { height: 100%; background: var(--accent); min-width: 2px; }
.channel-bar-fill.warn { background: var(--warn); }
.channel-bar-fill.danger { background: var(--danger); }
.channel-count { width: 20px; text-align: right; color: var(--sub); }
.note { font-size: 0.72rem; color: var(--sub); margin-top: 8px; }
.security-list .sec-item { display: flex; align-items: center; gap: 8px; padding: 6px 0; font-size: 0.85rem; border-bottom: 1px solid var(--border); }
.security-list .sec-item:last-child { border-bottom: none; }
.icon-ok::before { content: "\2713"; color: var(--ok); margin-right: 4px; }
.icon-warn::before { content: "\26A0"; color: var(--warn); margin-right: 4px; }
.net-controls { display: flex; gap: 8px; margin-bottom: 10px; }
.net-controls input[type=text] { flex: 1; background: #0c1213; border: 1px solid var(--border); color: var(--text); border-radius: 8px; padding: 8px 10px; font-size: 0.8rem; }
.net-controls select { background: #0c1213; border: 1px solid var(--border); color: var(--text); border-radius: 8px; padding: 8px; font-size: 0.75rem; }
.net-card { background: var(--card); border: 1px solid var(--border); border-radius: 10px; padding: 12px; margin-bottom: 8px; }
.net-top-row { display: flex; justify-content: space-between; align-items: center; gap: 8px; }
.net-card .net-ssid { font-weight: 700; font-size: 0.9rem; }
.net-card .net-meta { display: flex; flex-wrap: wrap; gap: 10px; font-size: 0.72rem; color: var(--sub); margin-top: 6px; align-items: center; }
.badge { display: inline-block; padding: 2px 8px; border-radius: 20px; font-size: 0.68rem; border: 1px solid var(--border); }
.badge-open { color: var(--danger); border-color: var(--danger); }
.badge-secure { color: var(--ok); border-color: var(--ok); }
.sig-meter { display: flex; align-items: flex-end; gap: 2px; }
.sig-bar { width: 4px; background: #2a3a3e; border-radius: 1px; display: inline-block; }
.sig-bar.filled { background: var(--accent); }
.grade { display: inline-block; width: 20px; height: 20px; line-height: 20px; text-align: center; border-radius: 50%; font-weight: 700; font-size: 0.7rem; }
.grade-A { background: rgba(74,222,128,0.15); color: var(--ok); border: 1px solid var(--ok); }
.grade-B { background: rgba(0,229,199,0.15); color: var(--accent); border: 1px solid var(--accent); }
.grade-C { background: rgba(255,184,77,0.15); color: var(--warn); border: 1px solid var(--warn); }
.grade-D { background: rgba(255,84,112,0.12); color: var(--danger); border: 1px solid var(--danger); }
.grade-F { background: rgba(255,84,112,0.25); color: var(--danger); border: 1px solid var(--danger); }
footer { text-align: center; color: var(--sub); font-size: 0.68rem; padding: 20px 12px 0; }
)CSSPAGE";

const char APP_JS[] PROGMEM = R"JSPAGE(
function el(id) { return document.getElementById(id); }

var lastNetworksData = null;
var scanPollTimer = null;
var scanStartClientTime = 0;
var autoRefreshInterval = null;

function qualityToBars(q) {
  switch (q) {
    case 'Excellent': return 5;
    case 'Good': return 4;
    case 'Fair': return 3;
    case 'Weak': return 2;
    default: return 1;
  }
}

function securityGrade(sec, open) {
  if (open) return 'F';
  if (sec.indexOf('WPA3') !== -1) return 'A';
  if (sec.indexOf('WPA2') !== -1) return 'B';
  if (sec === 'WEP') return 'D';
  if (sec.indexOf('WPA') !== -1) return 'C';
  return 'C';
}

function escapeHtml(str) {
  var d = document.createElement('div');
  d.textContent = str;
  return d.innerHTML;
}

function renderSummary(data) {
  el('stat-total').textContent = data.summary.total;
  el('stat-24').textContent = data.summary.band24;
  el('stat-5').textContent = data.summary.band5;
  el('stat-open').textContent = data.summary.open;
  el('stat-duration').textContent = data.lastScanDurationMillis
    ? (data.lastScanDurationMillis / 1000).toFixed(1) + 's'
    : '-';
}

function renderChannels(data) {
  var counts = data.channels24;
  var total24 = data.summary.band24;
  var container = el('channel-chart');
  container.innerHTML = '';

  if (total24 === 0) {
    container.innerHTML = '<p class="note">No 2.4GHz networks in the last scan to analyze.</p>';
    el('channel-note').textContent = '';
    return;
  }

  var maxCount = 1;
  counts.forEach(function (c) { if (c > maxCount) maxCount = c; });

  for (var i = 0; i < counts.length; i++) {
    var ch = i + 1;
    var count = counts[i];
    var pct = Math.round((count / maxCount) * 100);
    var row = document.createElement('div');
    row.className = 'channel-row';
    row.innerHTML =
      '<div class="channel-label">CH ' + ch + '</div>' +
      '<div class="channel-bar-bg"><div class="channel-bar-fill" style="width:' + pct + '%"></div></div>' +
      '<div class="channel-count">' + count + '</div>';
    container.appendChild(row);
  }

  var candidates = [1, 6, 11];
  var best = candidates[0];
  candidates.forEach(function (ch) {
    if (counts[ch - 1] < counts[best - 1]) best = ch;
  });
  el('channel-note').textContent =
    'Channel ' + best + ' appears less congested among non-overlapping channels (1, 6, 11), ' +
    'based only on the networks detected in this scan. This is not a performance guarantee. ' +
    'Note: 5GHz networks (' + data.summary.band5 + ' detected) use different channels, not shown here.';
}

function renderRssiDist(summary) {
  var container = el('rssi-dist');
  container.innerHTML = '';
  var buckets = [
    { label: 'Excellent', val: summary.rssi.excellent, cls: '' },
    { label: 'Good', val: summary.rssi.good, cls: '' },
    { label: 'Fair', val: summary.rssi.fair, cls: 'warn' },
    { label: 'Weak', val: summary.rssi.weak, cls: 'warn' },
    { label: 'Very Weak', val: summary.rssi.veryWeak, cls: 'danger' }
  ];
  var maxVal = 1;
  buckets.forEach(function (b) { if (b.val > maxVal) maxVal = b.val; });
  buckets.forEach(function (b) {
    var pct = Math.round((b.val / maxVal) * 100);
    var row = document.createElement('div');
    row.className = 'channel-row';
    row.innerHTML =
      '<div class="channel-label">' + b.label + '</div>' +
      '<div class="channel-bar-bg"><div class="channel-bar-fill ' + b.cls + '" style="width:' + pct + '%"></div></div>' +
      '<div class="channel-count">' + b.val + '</div>';
    container.appendChild(row);
  });
}

function renderSecurity(data) {
  var s = data.summary;
  var lines = [];
  if (s.wpa3 > 0) lines.push({ ok: true, text: s.wpa3 + ' network(s) using WPA3' });
  if (s.wpa2 > 0) lines.push({ ok: true, text: s.wpa2 + ' network(s) using WPA2' });
  if (s.wpa > 0) lines.push({ ok: false, text: s.wpa + ' network(s) using legacy WPA only' });
  if (s.wep > 0) lines.push({ ok: false, text: s.wep + ' network(s) using WEP (considered insecure)' });
  if (s.open > 0) lines.push({ ok: false, text: s.open + ' open network(s) detected (no encryption)' });
  if (lines.length === 0) lines.push({ ok: true, text: 'No networks scanned yet' });

  var container = el('security-analysis');
  container.innerHTML = '';
  lines.forEach(function (l) {
    var div = document.createElement('div');
    div.className = 'sec-item ' + (l.ok ? 'icon-ok' : 'icon-warn');
    div.textContent = l.text;
    container.appendChild(div);
  });
}

function renderNetworks(data) {
  lastNetworksData = data.networks;
  applyFiltersAndRender();
}

function applyFiltersAndRender() {
  if (!lastNetworksData) return;
  var query = el('net-search').value.trim().toLowerCase();
  var sortBy = el('net-sort').value;

  var filtered = lastNetworksData.filter(function (n) {
    return n.ssid.toLowerCase().indexOf(query) !== -1;
  });

  filtered.sort(function (a, b) {
    if (sortBy === 'rssi') return b.rssi - a.rssi;
    if (sortBy === 'channel') return a.channel - b.channel;
    if (sortBy === 'ssid') return a.ssid.localeCompare(b.ssid);
    return 0;
  });

  var container = el('networks-container');
  container.innerHTML = '';
  if (filtered.length === 0) {
    container.innerHTML = '<p class="note">No networks match your search.</p>';
    return;
  }

  filtered.forEach(function (n) {
    var card = document.createElement('div');
    card.className = 'net-card';
    var badge = n.open
      ? '<span class="badge badge-open">OPEN</span>'
      : '<span class="badge badge-secure">' + n.security + '</span>';

    var bars = qualityToBars(n.quality);
    var barsHtml = '';
    for (var b = 1; b <= 5; b++) {
      barsHtml += '<span class="sig-bar' + (b <= bars ? ' filled' : '') + '" style="height:' + (b * 3 + 2) + 'px"></span>';
    }
    var grade = securityGrade(n.security, n.open);

    card.innerHTML =
      '<div class="net-top-row">' +
        '<div class="net-ssid">' + escapeHtml(n.ssid) + '</div>' +
        '<div class="sig-meter">' + barsHtml + '</div>' +
      '</div>' +
      '<div class="net-meta">' +
        '<span>RSSI: ' + n.rssi + ' dBm (' + n.quality + ')</span>' +
        '<span>CH ' + n.channel + '</span>' +
        '<span>' + n.band + '</span>' +
        '<span>~' + n.estDistanceM + ' m (est.)</span>' +
        badge +
        '<span class="grade grade-' + grade + '">' + grade + '</span>' +
      '</div>';
    container.appendChild(card);
  });
}

function renderAll(data) {
  renderSummary(data);
  renderChannels(data);
  renderSecurity(data);
  renderRssiDist(data.summary);
  renderNetworks(data);
}

function formatUptime(ms) {
  var s = Math.floor(ms / 1000);
  var h = Math.floor(s / 3600);
  var m = Math.floor((s % 3600) / 60);
  var sec = s % 60;
  return (h < 10 ? '0' : '') + h + ':' + (m < 10 ? '0' : '') + m + ':' + (sec < 10 ? '0' : '') + sec;
}

function renderStatus(s) {
  el('dev-uptime').textContent = formatUptime(s.uptimeMillis);
  el('dev-heap').textContent = (s.freeHeapBytes / 1024).toFixed(1) + ' KB';
  el('dev-clients').textContent = s.clients;
}

function loadStatus() {
  fetch('/api/status').then(function (r) { return r.json(); }).then(renderStatus).catch(function () {});
}

function renderHistory(h) {
  var container = el('history-chart');
  container.innerHTML = '';
  if (!h.entries || h.entries.length === 0) {
    container.innerHTML = '<p class="note">No scan history yet.</p>';
    return;
  }
  var maxVal = 1;
  h.entries.forEach(function (e) { if (e.total > maxVal) maxVal = e.total; });
  h.entries.forEach(function (e) {
    var pct = Math.round((e.total / maxVal) * 100);
    var col = document.createElement('div');
    col.className = 'history-col';
    col.innerHTML = '<div class="history-bar" style="height:' + pct + '%"></div><div class="history-label">' + e.total + '</div>';
    container.appendChild(col);
  });
}

function loadHistory() {
  fetch('/api/history').then(function (r) { return r.json(); }).then(renderHistory).catch(function () {});
}

function showProgressBar() {
  el('scan-progress-wrap').style.display = 'block';
  setProgressBar(2, 'SCANNING...');
}
function hideProgressBar() {
  el('scan-progress-wrap').style.display = 'none';
}
function setProgressBar(pct, label) {
  el('scan-progress-fill').style.width = pct + '%';
  el('scan-progress-label').textContent = label + ' (' + pct + '%)';
}

function startScan() {
  var btn = el('scan-btn');
  btn.disabled = true;
  showProgressBar();
  scanStartClientTime = Date.now();

  fetch('/api/scan', { method: 'POST' })
    .then(function (r) { return r.json(); })
    .then(function () { pollScanStatus(); })
    .catch(function () {
      hideProgressBar();
      btn.disabled = false;
    });
}

function pollScanStatus() {
  fetch('/api/scan/status')
    .then(function (r) { return r.json(); })
    .then(function (status) {
      var elapsed = Date.now() - scanStartClientTime;
      var estimate = status.estimateMs || 6000;
      var pct = Math.min(92, Math.round((elapsed / estimate) * 100));

      if (status.scanning) {
        setProgressBar(pct, 'SCANNING...');
        scanPollTimer = setTimeout(pollScanStatus, 400);
      } else {
        setProgressBar(100, 'Done');
        fetch('/api/networks')
          .then(function (r) { return r.json(); })
          .then(function (data) {
            renderAll(data);
            setTimeout(hideProgressBar, 600);
            el('scan-btn').disabled = false;
          });
        loadHistory();
        loadStatus();
      }
    })
    .catch(function () {
      hideProgressBar();
      el('scan-btn').disabled = false;
    });
}

function toggleAutoRefresh() {
  var checked = el('auto-refresh-toggle').checked;
  if (checked) {
    autoRefreshInterval = setInterval(function () {
      if (el('scan-btn').disabled) return;
      startScan();
    }, 30000);
  } else {
    clearInterval(autoRefreshInterval);
  }
}

window.addEventListener('load', function () {
  loadStatus();
  loadHistory();
  fetch('/api/networks').then(function (r) { return r.json(); }).then(renderAll).catch(function () {});
  setInterval(loadStatus, 5000);
  el('net-search').addEventListener('input', applyFiltersAndRender);
  el('net-sort').addEventListener('change', applyFiltersAndRender);
  el('auto-refresh-toggle').addEventListener('change', toggleAutoRefresh);
});
)JSPAGE";

#endif