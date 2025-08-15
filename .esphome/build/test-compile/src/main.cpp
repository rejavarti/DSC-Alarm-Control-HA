// Auto generated code by esphome
// ========== AUTO GENERATED INCLUDE BLOCK BEGIN ===========
#include "esphome.h"
using namespace esphome;
using std::isnan;
using std::min;
using std::max;
using namespace alarm_control_panel;
logger::Logger *logger_logger_id;
wifi::WiFiComponent *wifi_wificomponent_id;
mdns::MDNSComponent *mdns_mdnscomponent_id;
esphome::ESPHomeOTAComponent *esphome_esphomeotacomponent_id;
safe_mode::SafeModeComponent *safe_mode_safemodecomponent_id;
api::APIServer *api_apiserver_id;
using namespace api;
preferences::IntervalSyncer *preferences_intervalsyncer_id;
dsc_keybus::DSCKeybusComponent *dsc_interface;
dsc_keybus::DSCAlarmControlPanel *test_panel;
// ========== AUTO GENERATED INCLUDE BLOCK END ==========="

void setup() {
  // ========== AUTO GENERATED CODE BEGIN ===========
  App.reserve_alarm_control_panel(1);
  // network:
  //   enable_ipv6: false
  //   min_ipv6_addr_count: 0
  // esphome:
  //   name: test-compile
  //   platformio_options:
  //     build_flags:
  //       - -DdscClassicSeries
  //   min_version: 2025.7.5
  //   build_path: build/test-compile
  //   friendly_name: ''
  //   includes: []
  //   libraries: []
  //   name_add_mac_suffix: false
  //   debug_scheduler: false
  //   areas: []
  //   devices: []
  App.pre_setup("test-compile", "", "", __DATE__ ", " __TIME__, false);
  App.reserve_components(9);
  // alarm_control_panel:
  // logger:
  //   id: logger_logger_id
  //   baud_rate: 115200
  //   tx_buffer_size: 512
  //   deassert_rts_dtr: false
  //   task_log_buffer_size: 768
  //   hardware_uart: UART0
  //   level: DEBUG
  //   logs: {}
  logger_logger_id = new logger::Logger(115200, 512);
  logger_logger_id->create_pthread_key();
  logger_logger_id->init_log_buffer(768);
  logger_logger_id->set_log_level(ESPHOME_LOG_LEVEL_DEBUG);
  logger_logger_id->set_uart_selection(logger::UART_SELECTION_UART0);
  logger_logger_id->pre_setup();
  logger_logger_id->set_component_source("logger");
  App.register_component(logger_logger_id);
  // wifi:
  //   ap:
  //     ssid: test-ap
  //     password: test12345
  //     id: wifi_wifiap_id
  //     ap_timeout: 1min
  //   id: wifi_wificomponent_id
  //   domain: .local
  //   reboot_timeout: 15min
  //   power_save_mode: LIGHT
  //   fast_connect: false
  //   enable_btm: false
  //   enable_rrm: false
  //   passive_scan: false
  //   enable_on_boot: true
  //   networks:
  //     - ssid: test
  //       password: test12345
  //       id: wifi_wifiap_id_2
  //       priority: 0.0
  //   use_address: test-compile.local
  wifi_wificomponent_id = new wifi::WiFiComponent();
  wifi_wificomponent_id->set_use_address("test-compile.local");
  {
  wifi::WiFiAP wifi_wifiap_id_2 = wifi::WiFiAP();
  wifi_wifiap_id_2.set_ssid("test");
  wifi_wifiap_id_2.set_password("test12345");
  wifi_wifiap_id_2.set_priority(0.0f);
  wifi_wificomponent_id->add_sta(wifi_wifiap_id_2);
  }
  {
  wifi::WiFiAP wifi_wifiap_id = wifi::WiFiAP();
  wifi_wifiap_id.set_ssid("test-ap");
  wifi_wifiap_id.set_password("test12345");
  wifi_wificomponent_id->set_ap(wifi_wifiap_id);
  }
  wifi_wificomponent_id->set_ap_timeout(60000);
  wifi_wificomponent_id->set_reboot_timeout(900000);
  wifi_wificomponent_id->set_power_save_mode(wifi::WIFI_POWER_SAVE_LIGHT);
  wifi_wificomponent_id->set_fast_connect(false);
  wifi_wificomponent_id->set_passive_scan(false);
  wifi_wificomponent_id->set_enable_on_boot(true);
  wifi_wificomponent_id->set_component_source("wifi");
  App.register_component(wifi_wificomponent_id);
  // mdns:
  //   id: mdns_mdnscomponent_id
  //   disabled: false
  //   services: []
  mdns_mdnscomponent_id = new mdns::MDNSComponent();
  mdns_mdnscomponent_id->set_component_source("mdns");
  App.register_component(mdns_mdnscomponent_id);
  // ota:
  // ota.esphome:
  //   platform: esphome
  //   password: test12345
  //   id: esphome_esphomeotacomponent_id
  //   version: 2
  //   port: 3232
  esphome_esphomeotacomponent_id = new esphome::ESPHomeOTAComponent();
  esphome_esphomeotacomponent_id->set_port(3232);
  esphome_esphomeotacomponent_id->set_auth_password("test12345");
  esphome_esphomeotacomponent_id->set_component_source("esphome.ota");
  App.register_component(esphome_esphomeotacomponent_id);
  // safe_mode:
  //   id: safe_mode_safemodecomponent_id
  //   boot_is_good_after: 1min
  //   disabled: false
  //   num_attempts: 10
  //   reboot_timeout: 5min
  safe_mode_safemodecomponent_id = new safe_mode::SafeModeComponent();
  safe_mode_safemodecomponent_id->set_component_source("safe_mode");
  App.register_component(safe_mode_safemodecomponent_id);
  if (safe_mode_safemodecomponent_id->should_enter_safe_mode(10, 300000, 60000)) return;
  // api:
  //   encryption:
  //     key: SXVhb/sYwJRj1eQn6FEt4JXVmXOBzMOwkDdDrNjyOpo=
  //   id: api_apiserver_id
  //   port: 6053
  //   password: ''
  //   reboot_timeout: 15min
  //   batch_delay: 100ms
  //   custom_services: false
  api_apiserver_id = new api::APIServer();
  api_apiserver_id->set_component_source("api");
  App.register_component(api_apiserver_id);
  api_apiserver_id->set_port(6053);
  api_apiserver_id->set_reboot_timeout(900000);
  api_apiserver_id->set_batch_delay(100);
  api_apiserver_id->set_noise_psk({73, 117, 97, 111, 251, 24, 192, 148, 99, 213, 228, 39, 232, 81, 45, 224, 149, 213, 153, 115, 129, 204, 195, 176, 144, 55, 67, 172, 216, 242, 58, 154});
  // substitutions:
  //   accessCode: '1234'
  // esp32:
  //   board: esp32dev
  //   framework:
  //     version: 5.3.2
  //     sdkconfig_options: {}
  //     advanced:
  //       compiler_optimization: SIZE
  //       enable_lwip_assert: true
  //       ignore_efuse_custom_mac: false
  //       enable_lwip_mdns_queries: true
  //       enable_lwip_bridge_interface: false
  //     components: []
  //     platform_version: https:github.com/pioarduino/platform-espressif32/releases/download/53.03.13/platform-espressif32.zip
  //     source: pioarduino/framework-espidf@https:github.com/pioarduino/esp-idf/releases/download/v5.3.2/esp-idf-v5.3.2.zip
  //     type: esp-idf
  //   flash_size: 4MB
  //   variant: ESP32
  //   cpu_frequency: 160MHZ
  // preferences:
  //   id: preferences_intervalsyncer_id
  //   flash_write_interval: 60s
  preferences_intervalsyncer_id = new preferences::IntervalSyncer();
  preferences_intervalsyncer_id->set_write_interval(60000);
  preferences_intervalsyncer_id->set_component_source("preferences");
  App.register_component(preferences_intervalsyncer_id);
  // external_components:
  //   - source:
  //       path: extras/ESPHome/components
  //       type: local
  //     components:
  //       - dsc_keybus
  //     refresh: 1d
  // dsc_keybus:
  //   id: dsc_interface
  //   access_code: '1234'
  //   series_type: Classic
  //   standalone_mode: true
  //   debug: 1
  //   enable_05_messages: true
  dsc_interface = new dsc_keybus::DSCKeybusComponent();
  dsc_interface->set_component_source("dsc_keybus");
  App.register_component(dsc_interface);
  dsc_interface->set_access_code("1234");
  dsc_interface->set_debug_level(1);
  dsc_interface->set_enable_05_messages(true);
  dsc_interface->set_standalone_mode(true);
  // alarm_control_panel.dsc_keybus:
  //   platform: dsc_keybus
  //   partition: 1
  //   name: DSC Test Panel
  //   id: test_panel
  //   disabled_by_default: false
  //   dsc_keybus_id: dsc_interface
  test_panel = new dsc_keybus::DSCAlarmControlPanel();
  test_panel->set_component_source("dsc_keybus.alarm_control_panel");
  App.register_component(test_panel);
  App.register_alarm_control_panel(test_panel);
  test_panel->set_name("DSC Test Panel");
  test_panel->set_object_id("dsc_test_panel");
  test_panel->set_disabled_by_default(false);
  test_panel->set_dsc_keybus_component(dsc_interface);
  test_panel->set_partition(1);
  // socket:
  //   implementation: bsd_sockets
  // md5:
  // =========== AUTO GENERATED CODE END ============
  App.setup();
}

void loop() {
  App.loop();
}
