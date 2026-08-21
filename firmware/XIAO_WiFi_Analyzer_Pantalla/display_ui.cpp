#include <lvgl.h>
#include <WiFi.h>

#define USE_TFT_ESPI_LIBRARY
#include "lv_xiao_round_screen.h"

#include "display_ui.h"
#include "scanner.h"
#include "wifi_state.h"
#include "config.h"

// ============================================================
// COLORES DEL TEMA
// ============================================================
#define COL_BG        0x0b0f10
#define COL_ACCENT    0x00e5c7
#define COL_OK        0x4ade80
#define COL_WARN      0xffb84d
#define COL_DANGER    0xff5470
#define COL_SUB       0x8aa0a4
#define COL_CARD      0x171f21

// ============================================================
// TACTIL: registramos nuestro propio indev (ver sesiones anteriores:
// lv_xiao_touch_init() no funcionaba de forma fiable en este combo).
// ============================================================
static lv_indev_drv_t indev_drv;
static lv_indev_t *touch_indev = NULL;

static void touchpad_read_cb(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  if (chsc6x_is_pressed()) {
    lv_coord_t x = 0, y = 0;
    chsc6x_get_xy(&x, &y);
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// ============================================================
// NAVEGACION ENTRE PANTALLAS (swipe izquierda/derecha)
// ============================================================
#define NUM_PAGES 4
enum PageId { PAGE_HOME = 0, PAGE_NETWORKS, PAGE_CHANNELS, PAGE_STATUS };

static lv_obj_t *pages[NUM_PAGES];
static int currentPage = PAGE_HOME;
static unsigned long lastScanRenderedMillis = 0; // para saber cuando redibujar listas/canales

static void goToPage(int newPage, bool slideLeft) {
  if (newPage < 0) newPage = NUM_PAGES - 1;
  if (newPage >= NUM_PAGES) newPage = 0;
  currentPage = newPage;
  lv_scr_load_anim(pages[currentPage],
                    slideLeft ? LV_SCR_LOAD_ANIM_MOVE_LEFT : LV_SCR_LOAD_ANIM_MOVE_RIGHT,
                    220, 0, false);
}

static void screen_gesture_cb(lv_event_t *e) {
  lv_indev_t *indev = lv_indev_get_act();
  if (indev == NULL) return;
  lv_dir_t dir = lv_indev_get_gesture_dir(indev);
  if (dir == LV_DIR_LEFT) {
    goToPage(currentPage + 1, true);
  } else if (dir == LV_DIR_RIGHT) {
    goToPage(currentPage - 1, false);
  }
}

// Fila de puntitos indicando en que pantalla estamos, igual en las 4 pantallas
static void create_page_dots(lv_obj_t *parent, int activeIdx) {
  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_remove_style_all(row);
  lv_obj_set_size(row, 70, 12);
  lv_obj_align(row, LV_ALIGN_BOTTOM_MID, 0, -8);
  lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  for (int i = 0; i < NUM_PAGES; i++) {
    lv_obj_t *dot = lv_obj_create(row);
    lv_obj_remove_style_all(dot);
    lv_obj_set_size(dot, 6, 6);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(dot, i == activeIdx ? lv_color_hex(COL_ACCENT) : lv_color_hex(0x2a3a3e), 0);
  }
}

// ============================================================
// PANTALLA 1 - HOME
// ============================================================
static lv_obj_t *status_dot;
static lv_obj_t *home_arc;
static lv_obj_t *home_arc_label;
static lv_obj_t *label_open;
static lv_obj_t *label_lastscan;
static lv_obj_t *scan_btn;
static lv_obj_t *scan_btn_label;
static lv_obj_t *scanning_overlay;
static lv_obj_t *scanning_spinner;

static lv_anim_t status_dot_anim;
static int32_t arc_display_value = 0; // valor actualmente animado en pantalla

static void status_dot_anim_cb(void *obj, int32_t value) {
  lv_obj_set_style_bg_opa((lv_obj_t *)obj, value, 0);
}

static void start_status_dot_breathing(uint32_t colorHex, uint32_t periodMs) {
  lv_obj_set_style_bg_color(status_dot, lv_color_hex(colorHex), 0);
  lv_anim_del(status_dot, status_dot_anim_cb);
  lv_anim_init(&status_dot_anim);
  lv_anim_set_var(&status_dot_anim, status_dot);
  lv_anim_set_exec_cb(&status_dot_anim, status_dot_anim_cb);
  lv_anim_set_values(&status_dot_anim, LV_OPA_30, LV_OPA_COVER);
  lv_anim_set_time(&status_dot_anim, periodMs);
  lv_anim_set_playback_time(&status_dot_anim, periodMs);
  lv_anim_set_repeat_count(&status_dot_anim, LV_ANIM_REPEAT_INFINITE);
  lv_anim_start(&status_dot_anim);
}

static void arc_value_anim_cb(void *obj, int32_t value) {
  arc_display_value = value;
  lv_arc_set_value((lv_obj_t *)obj, value);
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", (int)value);
  lv_label_set_text(home_arc_label, buf);
}

static void animate_arc_to(int32_t target) {
  if (target == arc_display_value) return;
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, home_arc);
  lv_anim_set_exec_cb(&a, arc_value_anim_cb);
  lv_anim_set_values(&a, arc_display_value, target);
  lv_anim_set_time(&a, 500);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
  lv_anim_start(&a);
}

static void scan_btn_event_cb(lv_event_t *e) {
  if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    if (g_scanState != SCAN_STATE_RUNNING) {
      startWifiScan();
    }
  }
}

static lv_obj_t *build_home_page() {
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COL_BG), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(scr, screen_gesture_cb, LV_EVENT_GESTURE, NULL);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "XIAO ANALYZER");
  lv_obj_set_style_text_color(title, lv_color_hex(COL_ACCENT), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

  status_dot = lv_obj_create(scr);
  lv_obj_remove_style_all(status_dot);
  lv_obj_set_size(status_dot, 10, 10);
  lv_obj_set_style_radius(status_dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(status_dot, LV_OPA_COVER, 0);
  lv_obj_align(status_dot, LV_ALIGN_TOP_MID, 0, 42);
  start_status_dot_breathing(COL_OK, 1400);

  // Medidor circular con el numero total de redes
  home_arc = lv_arc_create(scr);
  lv_obj_set_size(home_arc, 130, 130);
  lv_arc_set_rotation(home_arc, 270);
  lv_arc_set_bg_angles(home_arc, 0, 360);
  lv_arc_set_range(home_arc, 0, 30); // 30 redes = anillo lleno, es solo escala visual
  lv_arc_set_value(home_arc, 0);
  lv_obj_align(home_arc, LV_ALIGN_CENTER, 0, -8);
  lv_obj_remove_style(home_arc, NULL, LV_PART_KNOB); // sin "pomo" arrastrable
  lv_obj_clear_flag(home_arc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_color(home_arc, lv_color_hex(0x1c2528), LV_PART_MAIN);
  lv_obj_set_style_arc_color(home_arc, lv_color_hex(COL_ACCENT), LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(home_arc, 10, LV_PART_MAIN);
  lv_obj_set_style_arc_width(home_arc, 10, LV_PART_INDICATOR);

  home_arc_label = lv_label_create(home_arc);
  lv_label_set_text(home_arc_label, "0");
  lv_obj_set_style_text_color(home_arc_label, lv_color_hex(0xffffff), 0);
  lv_obj_center(home_arc_label);

  lv_obj_t *networks_caption = lv_label_create(scr);
  lv_label_set_text(networks_caption, "NETWORKS");
  lv_obj_set_style_text_color(networks_caption, lv_color_hex(COL_SUB), 0);
  lv_obj_align_to(networks_caption, home_arc, LV_ALIGN_OUT_BOTTOM_MID, 0, -66);

  label_open = lv_label_create(scr);
  lv_label_set_text(label_open, "OPEN: -");
  lv_obj_set_style_text_color(label_open, lv_color_hex(COL_SUB), 0);
  lv_obj_align(label_open, LV_ALIGN_CENTER, 0, 58);

  label_lastscan = lv_label_create(scr);
  lv_label_set_text(label_lastscan, "never scanned");
  lv_obj_set_style_text_color(label_lastscan, lv_color_hex(COL_SUB), 0);
  lv_obj_align(label_lastscan, LV_ALIGN_CENTER, 0, 74);

  scan_btn = lv_btn_create(scr);
  lv_obj_set_size(scan_btn, 96, 40);
  lv_obj_align(scan_btn, LV_ALIGN_BOTTOM_MID, 0, -26);
  lv_obj_set_style_bg_color(scan_btn, lv_color_hex(COL_ACCENT), 0);
  lv_obj_set_style_bg_color(scan_btn, lv_color_hex(0x00b8a0), LV_STATE_PRESSED);
  lv_obj_set_style_radius(scan_btn, 20, 0);
  lv_obj_add_event_cb(scan_btn, scan_btn_event_cb, LV_EVENT_CLICKED, NULL);

  scan_btn_label = lv_label_create(scan_btn);
  lv_label_set_text(scan_btn_label, "SCAN");
  lv_obj_set_style_text_color(scan_btn_label, lv_color_hex(0x05201c), 0);
  lv_obj_center(scan_btn_label);

  // Overlay de "escaneando", oculto por defecto, se muestra encima de todo
  scanning_overlay = lv_obj_create(scr);
  lv_obj_remove_style_all(scanning_overlay);
  lv_obj_set_size(scanning_overlay, 240, 240);
  lv_obj_set_pos(scanning_overlay, 0, 0);
  lv_obj_set_style_bg_color(scanning_overlay, lv_color_hex(COL_BG), 0);
  lv_obj_set_style_bg_opa(scanning_overlay, LV_OPA_90, 0);
  lv_obj_clear_flag(scanning_overlay, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(scanning_overlay, LV_OBJ_FLAG_HIDDEN);

  scanning_spinner = lv_spinner_create(scanning_overlay, 900, 70);
  lv_obj_set_size(scanning_spinner, 70, 70);
  lv_obj_align(scanning_spinner, LV_ALIGN_CENTER, 0, -10);
  lv_obj_set_style_arc_color(scanning_spinner, lv_color_hex(COL_ACCENT), LV_PART_INDICATOR);

  lv_obj_t *scanning_label = lv_label_create(scanning_overlay);
  lv_label_set_text(scanning_label, "SCANNING...");
  lv_obj_set_style_text_color(scanning_label, lv_color_hex(COL_ACCENT), 0);
  lv_obj_align_to(scanning_label, scanning_spinner, LV_ALIGN_OUT_BOTTOM_MID, 0, 14);

  create_page_dots(scr, PAGE_HOME);
  return scr;
}

// ============================================================
// PANTALLA 2 - NETWORKS (lista de redes detectadas)
// ============================================================
static lv_obj_t *net_list;

static lv_obj_t *build_networks_page() {
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COL_BG), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(scr, screen_gesture_cb, LV_EVENT_GESTURE, NULL);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "NETWORKS");
  lv_obj_set_style_text_color(title, lv_color_hex(COL_ACCENT), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

  net_list = lv_list_create(scr);
  lv_obj_set_size(net_list, 190, 170);
  lv_obj_align(net_list, LV_ALIGN_CENTER, 0, -6);
  lv_obj_set_style_bg_color(net_list, lv_color_hex(COL_CARD), 0);
  lv_obj_set_style_bg_opa(net_list, LV_OPA_60, 0);
  lv_obj_set_style_radius(net_list, 16, 0);
  lv_obj_set_scroll_dir(net_list, LV_DIR_VER);
  lv_obj_add_flag(net_list, LV_OBJ_FLAG_GESTURE_BUBBLE); // deja pasar el swipe horizontal a la pantalla

  create_page_dots(scr, PAGE_NETWORKS);
  return scr;
}

static void rebuild_networks_list() {
  lv_obj_clean(net_list);

  if (g_lastScan.count == 0) {
    lv_obj_t *empty = lv_list_add_text(net_list, "No scan yet");
    return;
  }

  // Copiamos indices y los ordenamos por RSSI descendente (mejor senal primero)
  int order[MAX_NETWORKS];
  int n = g_lastScan.count;
  for (int i = 0; i < n; i++) order[i] = i;
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (g_lastScan.networks[order[j]].rssi < g_lastScan.networks[order[j + 1]].rssi) {
        int tmp = order[j]; order[j] = order[j + 1]; order[j + 1] = tmp;
      }
    }
  }

  int shown = (n > 12) ? 12 : n; // limite razonable de filas visibles/scrollables
  for (int k = 0; k < shown; k++) {
    NetworkEntry &e = g_lastScan.networks[order[k]];
    char line[48];
    snprintf(line, sizeof(line), "%s", e.ssid);
    lv_obj_t *btn = lv_list_add_btn(net_list, LV_SYMBOL_WIFI, line);

    uint32_t color = COL_SUB;
    if (isOpenNetwork(e.encType)) color = COL_DANGER;
    else if (e.rssi >= RSSI_GOOD) color = COL_OK;
    else if (e.rssi >= RSSI_FAIR) color = COL_WARN;
    else color = COL_DANGER;

    lv_obj_set_style_text_color(btn, lv_color_hex(color), 0);
  }
}

// ============================================================
// PANTALLA 3 - CHANNELS (congestion 2.4GHz)
// ============================================================
static lv_obj_t *channel_bars[13];
static lv_obj_t *channel_best_label;

static lv_obj_t *build_channels_page() {
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COL_BG), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(scr, screen_gesture_cb, LV_EVENT_GESTURE, NULL);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "CHANNELS 2.4G");
  lv_obj_set_style_text_color(title, lv_color_hex(COL_ACCENT), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

  lv_obj_t *chart = lv_obj_create(scr);
  lv_obj_remove_style_all(chart);
  lv_obj_set_size(chart, 190, 110);
  lv_obj_align(chart, LV_ALIGN_CENTER, 0, -18);
  lv_obj_clear_flag(chart, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_flex_flow(chart, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(chart, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
  lv_obj_add_flag(chart, LV_OBJ_FLAG_GESTURE_BUBBLE);

  for (int i = 0; i < 13; i++) {
    lv_obj_t *bar = lv_obj_create(chart);
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, 9, 4); // altura minima visible, se actualiza luego
    lv_obj_set_style_bg_color(bar, lv_color_hex(COL_ACCENT), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(bar, 2, 0);
    channel_bars[i] = bar;
  }

  channel_best_label = lv_label_create(scr);
  lv_label_set_text(channel_best_label, "Scan to analyze");
  lv_obj_set_style_text_color(channel_best_label, lv_color_hex(COL_SUB), 0);
  lv_obj_align(channel_best_label, LV_ALIGN_CENTER, 0, 62);

  create_page_dots(scr, PAGE_CHANNELS);
  return scr;
}

static void rebuild_channels_chart() {
  int counts[14] = {0}; // indices 1..13
  for (int i = 0; i < g_lastScan.count; i++) {
    NetworkEntry &e = g_lastScan.networks[i];
    if (e.band == BAND_2_4GHZ && e.channel >= 1 && e.channel <= 13) {
      counts[e.channel]++;
    }
  }

  int maxCount = 1;
  for (int ch = 1; ch <= 13; ch++) if (counts[ch] > maxCount) maxCount = counts[ch];

  for (int ch = 1; ch <= 13; ch++) {
    int h = 4 + (counts[ch] * 100) / maxCount; // 4..104 px
    lv_obj_set_height(channel_bars[ch - 1], h);
    uint32_t color = (counts[ch] == 0) ? 0x2a3a3e : COL_ACCENT;
    lv_obj_set_style_bg_color(channel_bars[ch - 1], lv_color_hex(color), 0);
  }

  int best = 1;
  int candidates[3] = {1, 6, 11};
  for (int c = 0; c < 3; c++) if (counts[candidates[c]] < counts[best]) best = candidates[c];
  if (counts[1] <= counts[6] && counts[1] <= counts[11]) best = 1;
  else if (counts[6] <= counts[1] && counts[6] <= counts[11]) best = 6;
  else best = 11;

  char buf[24];
  snprintf(buf, sizeof(buf), "Best: CH %d", best);
  lv_label_set_text(channel_best_label, buf);
}

// ============================================================
// PANTALLA 4 - DEVICE STATUS
// ============================================================
static lv_obj_t *status_uptime_label;
static lv_obj_t *status_heap_label;
static lv_obj_t *status_clients_label;
static lv_obj_t *status_ip_label;

static lv_obj_t *build_status_page() {
  lv_obj_t *scr = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COL_BG), 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(scr, screen_gesture_cb, LV_EVENT_GESTURE, NULL);

  lv_obj_t *title = lv_label_create(scr);
  lv_label_set_text(title, "DEVICE STATUS");
  lv_obj_set_style_text_color(title, lv_color_hex(COL_ACCENT), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 18);

  lv_obj_t *card = lv_obj_create(scr);
  lv_obj_set_size(card, 170, 130);
  lv_obj_align(card, LV_ALIGN_CENTER, 0, -8);
  lv_obj_set_style_bg_color(card, lv_color_hex(COL_CARD), 0);
  lv_obj_set_style_bg_opa(card, LV_OPA_60, 0);
  lv_obj_set_style_radius(card, 16, 0);
  lv_obj_set_style_border_width(card, 0, 0);
  lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(card, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER);
  lv_obj_add_flag(card, LV_OBJ_FLAG_GESTURE_BUBBLE);

  status_uptime_label = lv_label_create(card);
  lv_label_set_text(status_uptime_label, "Uptime: -");
  lv_obj_set_style_text_color(status_uptime_label, lv_color_hex(0xe6f1f2), 0);

  status_heap_label = lv_label_create(card);
  lv_label_set_text(status_heap_label, "Free RAM: -");
  lv_obj_set_style_text_color(status_heap_label, lv_color_hex(0xe6f1f2), 0);

  status_clients_label = lv_label_create(card);
  lv_label_set_text(status_clients_label, "Clients: -");
  lv_obj_set_style_text_color(status_clients_label, lv_color_hex(0xe6f1f2), 0);

  status_ip_label = lv_label_create(card);
  lv_label_set_text(status_ip_label, "192.168.4.1");
  lv_obj_set_style_text_color(status_ip_label, lv_color_hex(COL_SUB), 0);

  create_page_dots(scr, PAGE_STATUS);
  return scr;
}

// ============================================================
// REFRESCO PERIODICO (se llama cada 1s desde displayInit)
// ============================================================
static void refresh_timer_cb(lv_timer_t *t) {
  displayRefreshNow();
}

void displayInit() {
  Serial.println("[DISPLAY] Iniciando LVGL...");
  lv_init();
  #if LVGL_VERSION_MAJOR == 9
  lv_tick_set_cb(millis);
  #endif

  // Tactil manual (mas fiable que lv_xiao_touch_init() en este combo, ver sesiones previas)
  pinMode(TOUCH_INT, INPUT_PULLUP);
  Wire.begin();

  lv_xiao_disp_init();

  pinMode(D6, OUTPUT);
  digitalWrite(D6, HIGH); // refuerzo manual del backlight

  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchpad_read_cb;
  touch_indev = lv_indev_drv_register(&indev_drv);

  pages[PAGE_HOME] = build_home_page();
  pages[PAGE_NETWORKS] = build_networks_page();
  pages[PAGE_CHANNELS] = build_channels_page();
  pages[PAGE_STATUS] = build_status_page();

  lv_disp_load_scr(pages[PAGE_HOME]);

  lv_timer_create(refresh_timer_cb, 1000, NULL);

  Serial.println("[DISPLAY] UI inicial construida (4 pantallas, swipe para navegar)");
}

void displayLoop() {
  // LVGL v8 necesita que avancemos el reloj interno manualmente (lv_tick_set_cb
  // es API de v9). Sin esto ningun lv_timer ni animacion se dispara jamas.
  #if LVGL_VERSION_MAJOR == 8
  static unsigned long lastTickMillis = 0;
  unsigned long nowMillis = millis();
  lv_tick_inc(nowMillis - lastTickMillis);
  lastTickMillis = nowMillis;
  #endif

  lv_timer_handler();
}

void displayRefreshNow() {
  char buf[40];

  // --- Home ---
  animate_arc_to(g_state.totalNetworks > 30 ? 30 : g_state.totalNetworks);

  snprintf(buf, sizeof(buf), "OPEN: %d", g_state.openNetworks);
  lv_label_set_text(label_open, buf);

  if (g_state.everScanned) {
    unsigned long secondsAgo = (millis() - g_state.lastScanMillis) / 1000;
    snprintf(buf, sizeof(buf), "%lus ago", secondsAgo);
  } else {
    snprintf(buf, sizeof(buf), "never scanned");
  }
  lv_label_set_text(label_lastscan, buf);

  bool scanning = (g_scanState == SCAN_STATE_RUNNING);
  if (scanning) {
    lv_obj_clear_flag(scanning_overlay, LV_OBJ_FLAG_HIDDEN);
    start_status_dot_breathing(COL_WARN, 500);
  } else {
    lv_obj_add_flag(scanning_overlay, LV_OBJ_FLAG_HIDDEN);
    start_status_dot_breathing(COL_OK, 1400);
  }

  // --- Networks / Channels: solo se reconstruyen si hay un escaneo nuevo ---
  if (g_lastScan.lastScanMillis != lastScanRenderedMillis && !scanning) {
    lastScanRenderedMillis = g_lastScan.lastScanMillis;
    rebuild_networks_list();
    rebuild_channels_chart();
  }

  // --- Status ---
  unsigned long s = millis() / 1000;
  unsigned int hh = s / 3600, mm = (s % 3600) / 60, ss = s % 60;
  snprintf(buf, sizeof(buf), "Uptime: %02u:%02u:%02u", hh, mm, ss);
  lv_label_set_text(status_uptime_label, buf);

  snprintf(buf, sizeof(buf), "Free RAM: %u KB", (unsigned)(ESP.getFreeHeap() / 1024));
  lv_label_set_text(status_heap_label, buf);

  snprintf(buf, sizeof(buf), "Clients: %d", (int)WiFi.softAPgetStationNum());
  lv_label_set_text(status_clients_label, buf);

  String ip = WiFi.softAPIP().toString();
  lv_label_set_text(status_ip_label, ip.c_str());
}