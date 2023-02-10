/*
  Copyright (C) AC SOFTWARE SP. Z O.O.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* This example shows ESP82xx/ESP32 based device with simple WebInterface
 * used to configure Wi-Fi parameters and Supla server connection.
 * There is one RollerShutter, one Relay and 3 buttons configured.
 * Two buttons are for roller shutter with Action Trigger.
 * Third button is for controlling the relay and for switching module to
 * config mode.
 * After fresh installation, device will be in config mode. It will have its
 * own Wi-Fi AP configured. You should connect to it with your mobile phone
 * and open http://192.168.4.1 where you can configure the device.
 * Status LED is also configured. Please adjust GPIOs to your HW.
 */

#define STATUS_LED_GPIO 5
#define BUTTON_CFG_RELAY_GPIO 0
#define RED_PIN              12
#define GREEN_PIN            13
#define BLUE_PIN             14
#define BRIGHTNESS_PIN       4

#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/control/relay.h>
#include <supla/control/button.h>
#include <supla/control/action_trigger.h>
#include <supla/device/status_led.h>
#include <supla/storage/littlefs_config.h>
#include <supla/network/esp_web_server.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/status_led_parameters.h>
#include <supla/network/html/wifi_parameters.h>
#include <supla/device/supla_ca_cert.h>
#include <supla/events.h>
#include <supla/control/rgbw_leds.h>

// Choose where Supla should store roller shutter data in persistent memory
// We recommend to use external FRAM memory
#include <supla/storage/eeprom.h>
Supla::Eeprom eeprom;
// #include <supla/storage/fram_spi.h>
// Supla::FramSpi fram(STORAGE_OFFSET);

Supla::ESPWifi wifi;
Supla::LittleFsConfig configSupla;

Supla::Device::StatusLed statusLed(STATUS_LED_GPIO, false); // inverted state
Supla::EspWebServer suplaServer;


// HTML www component (they appear in sections according to creation
// sequence)
Supla::Html::DeviceInfo htmlDeviceInfo(&SuplaDevice);
Supla::Html::WifiParameters htmlWifi;
Supla::Html::ProtocolParameters htmlProto;
Supla::Html::StatusLedParameters htmlStatusLed;


void setup() {

  Serial.begin(115200);

  // Channels configuration
   auto rgbw = new Supla::Control::RGBWLeds(
      RED_PIN, GREEN_PIN, BLUE_PIN, BRIGHTNESS_PIN);

  // Buttons configuration
  auto button = new Supla::Control::Button(BUTTON_CFG_RELAY_GPIO, true, true);

  button->setMulticlickTime(200);
  button->setHoldTime(400);
  button->repeatOnHoldEvery(200);

  button->configureAsConfigButton(&SuplaDevice);
  button->addAction(Supla::TOGGLE, rgbw, Supla::ON_CLICK_1);

  SuplaDevice.setSuplaCACert(suplaCACert);
  SuplaDevice.setSupla3rdPartyCACert(supla3rdCACert);
  SuplaDevice.setName("Supla-Gosund_WB4");
  SuplaDevice.begin();
    auto cfg = Supla::Storage::ConfigInstance();
  if (cfg) {
    int8_t resetCounter = 0;
    cfg->getInt8("resetCounter", &resetCounter);
    resetCounter++;
    if (resetCounter >= 3) {
      SuplaDevice.requestCfgMode(Supla::Device::WithTimeout);
      cfg->setInt8("resetCounter", 0);
    } else {
      cfg->setInt8("resetCounter", resetCounter);
    }
    cfg->commit();
  }
}

void loop() {
  SuplaDevice.iterate();
  static bool resetCountTimedOut = false;
  if (!resetCountTimedOut && millis() > 5000) {
    resetCountTimedOut = true;
    auto cfg = Supla::Storage::ConfigInstance();
    if (cfg) {
      cfg->setInt8("resetCounter", 0);
      cfg->commit();   // to można pominąć, bo urządzenie samo wywoła commit, ale może to być po kilku sekundach, więc realnie te 5s by się wydłużyło
    }
  }
}
