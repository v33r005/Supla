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

#ifndef combo_h
#define combo_h
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>


#include "supla/sensor/therm_hygro_press_meter.h"

namespace Supla {
namespace Sensor {
class Combo : public ThermHygroPressMeter {
 public:
  explicit Combo() {
  }

  double getTemp() {
    aht.getEvent(&humi, &temp);
    float temperature = temp.temperature;
    return temperature;
  }

  double getHumi() {
    aht.getEvent(&humi, &temp);
    float humidity = humi.relative_humidity;
    return humidity;
  }

  double getPressure() {
    float value = PRESSURE_NOT_AVAILABLE;
    bool retryDone = false;
    do {
      if (!sensorStatus || isnan(value)) {
        sensorStatus = bmp.begin();
        retryDone = true;
      }
      value = PRESSURE_NOT_AVAILABLE;
      if (sensorStatus) {
        value = bmp.readPressure() / 100.0;
      }
    } while (isnan(value) && !retryDone);
    return value;
  }

  void onInit() {
  sensorStatus = bmp.begin();
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1) delay(10);
  }
    pressureChannel.setNewValue(getPressure());
    channel.setNewValue(getTemp(), getHumi());
  }

 protected:
bool sensorStatus;
  sensors_event_t humi, temp;
  float altitude;
  Adafruit_AHTX0 aht; 
  Adafruit_BMP280 bmp;
};

};  // namespace Sensor
};  // namespace Supla

#endif  // combo_h
