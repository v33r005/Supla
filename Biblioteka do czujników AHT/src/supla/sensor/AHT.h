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

#ifndef SRC_SUPLA_SENSOR_AHT_H_
#define SRC_SUPLA_SENSOR_AHT_H_

#include <Adafruit_AHTX0.h>

#include "therm_hygro_meter.h"

namespace Supla {
namespace Sensor {
class AHT : public ThermHygroMeter {
 public:
  void onInit() {
    aht.begin();
  }

  double getTemp() {
    double value = TEMPERATURE_NOT_AVAILABLE;
    aht.getEvent(&humidity, &temp);
    value = temp.temperature;
    return value;
  }

  double getHumi() {
    double value = HUMIDITY_NOT_AVAILABLE;
    value = humidity.relative_humidity;
    return value;
  }

  void iterateAlways() {
    if (millis() - lastReadTime > 10000) {
      lastReadTime = millis();
      channel.setNewValue(getTemp(), getHumi());
    }
  }

 protected:
  Adafruit_AHTX0 aht;
  sensors_event_t temp;
  sensors_event_t humidity;
};
};      // namespace Sensor
};      // namespace Supla
#endif  // SRC_SUPLA_SENSOR_AHT_H_
