#define STATUS_LED_GPIO 2
#define BUTTON_CFG_RELAY_GPIO 13
#define DS1_PIN 12
#define DS2_PIN 13
#define DS3_PIN 14
#define DHT1PIN 5
#define DHT1TYPE DHT22

//#include "Wire.h";
#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/control/button.h>
#include <supla/device/status_led.h>
#include <supla/storage/littlefs_config.h>
#include <supla/network/esp_web_server.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/status_led_parameters.h>
#include <supla/network/html/wifi_parameters.h>
#include <supla/device/supla_ca_cert.h>
#include <supla/events.h>
#include <supla/sensor/AHT.h>
#include <supla/sensor/DS18B20.h>
#include <supla/sensor/DHT.h>
//#include <supla/sensor/Combo.h>
#include <supla/device/notifications.h>
#include <supla/time.h>
#include <supla/network/html/custom_parameter.h>

#include <supla/storage/eeprom.h>
Supla::Eeprom eeprom;
// #include <supla/storage/fram_spi.h>
// Supla::FramSpi fram(STORAGE_OFFSET);
const char PARAM1[] = "param1";
const char PARAM2[] = "param2";
int Usypianie;
int Bateryjka;
unsigned long wifimilis = 0;
int wifimilis1 = 0;
int polaczenie_z_serwerem;
int bateria=0;
unsigned long migotkamillis = 0;
unsigned long czas_bezczynności = 0;
static uint32_t lastTime = 0;
int migotka = 0;
int czas = 0;
Supla::ESPWifi wifi;
Supla::LittleFsConfig configSupla;
bool wyslij_wiadomosc = LOW;

//Supla::Device::StatusLed statusLed(STATUS_LED_GPIO, false); // inverted state
Supla::EspWebServer suplaServer;

Supla::Html::DeviceInfo htmlDeviceInfo(&SuplaDevice);
Supla::Html::WifiParameters htmlWifi;
Supla::Html::ProtocolParameters htmlProto;
//Supla::Html::StatusLedParameters htmlStatusLed;
Supla::Sensor::AHT *tempwilg = nullptr;  

class myElement : public Supla::Element {
 public: myElement(int pin) : pin(pin) {}
  
  void onInit() override {
    //pinMode(pin, INPUT);
  }
  
  double getValue() {
    value = (map(analogRead(pin), 400,800,0,100));
    return value;
  }

  void iterateAlways() override {
    if (millis() - lastReadTime > 2000) {
      lastReadTime = millis();
      tempwilg->getChannel()->setBatteryLevel(getValue());
    }
  }
 protected:
  int pin;
  double value;
  uint64_t lastReadTime = 0;
};
myElement *c_analog = nullptr;
void status_func(int status, const char *msg) {     //    ------------------------ Status polaczednia z serwerem--------------------------
 polaczenie_z_serwerem = status;                                       
}

void setup() { 

  Serial.begin(115200);
  Wire.begin();
  pinMode(STATUS_LED_GPIO, OUTPUT);
  //auto tempwilg = new Supla::Sensor::Combo();
  tempwilg = new Supla::Sensor::AHT();
  c_analog = new myElement(A0);
  Supla::Notification::RegisterNotification(-1);  // notifications for device
  //auto temperatura = new Supla::Sensor::DS18B20(DS2_PIN);
  //new Supla::Sensor::DS18B20(DS2_PIN);
  //new Supla::Sensor::DS18B20(DS3_PIN);
  //new Supla::Sensor::DHT(DHT1PIN, DHT1TYPE);
  //temperatura->getChannel()->setValidityTimeSec(15*60);
  tempwilg->getChannel()->setValidityTimeSec(65*60);
  //tempwilg->getSecondaryChannel()->setValidityTimeSec(15*60);
  //temperatura->getChannel()->setFlag(SUPLA_CHANNEL_FLAG_CHANNELSTATE);
  tempwilg->getChannel()->setFlag(SUPLA_CHANNEL_FLAG_CHANNELSTATE);
  tempwilg->getChannel()->setFlag(SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED);
  tempwilg->getChannel()->setFlag(SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL);
  //tempwilg->getSecondaryChannel()->setFlag(SUPLA_CHANNEL_FLAG_CHANNELSTATE);
  //tempwilg->getSecondaryChannel()->setFlag(SUPLA_CHANNELSTATE_FIELD_BATTERYPOWERED);
  //tempwilg->getSecondaryChannel()->setFlag(SUPLA_CHANNELSTATE_FIELD_BATTERYLEVEL);
  //tempwilg->getSecondaryChannel()->setBatteryLevel(bateria);

  auto buttonCfgRelay = new Supla::Control::Button(BUTTON_CFG_RELAY_GPIO, true, true);
  buttonCfgRelay->configureAsConfigButton(&SuplaDevice);
  SuplaDevice.setStatusFuncImpl(&status_func);
  SuplaDevice.setSuplaCACert(suplaCACert);
  SuplaDevice.setSupla3rdPartyCACert(supla3rdCACert);

 // new Supla::Html::DeviceInfo(&SuplaDevice);
  //new Supla::Html::WifiParameters;
  //new Supla::Html::ProtocolParameters;
  //new Supla::Html::StatusLedParameters;
  new Supla::Html::CustomParameter(PARAM1, "Device Sleep [min]", 30);
  new Supla::Html::CustomParameter(PARAM2, "Battery Level Notification", 20);

  SuplaDevice.setName("Supla Czujniczek");

  SuplaDevice.addFlags(SUPLA_DEVICE_FLAG_SLEEP_MODE_ENABLED);
  SuplaDevice.begin();
}

void loop() {
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM1, &Usypianie)) {
    //SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM2, LEDS);
  } else {
    //SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM2);
  }
  if (Supla::Storage::ConfigInstance()->getInt32(PARAM2, &Bateryjka)) {
    //SUPLA_LOG_DEBUG(" **** Param[%s]: %d", PARAM2, LEDS);
  } else {
    //SUPLA_LOG_DEBUG(" **** Param[%s] is not set", PARAM2);
  }
  
  SuplaDevice.iterate();

  if (Usypianie > 60){
    Usypianie = 60;
    Supla::Storage::ConfigInstance()->setInt32(PARAM1, 60);
  }
  if (Bateryjka > 99){
    Bateryjka = 99;
    Supla::Storage::ConfigInstance()->setInt32(PARAM2, 99);

  }
  //if (Bateryjka < 1){
  //  Bateryjka = 1;
  //  Supla::Storage::ConfigInstance()->setInt32(PARAM2, 1);
 // }
  
  if ((polaczenie_z_serwerem != 40)&&(polaczenie_z_serwerem != 17)){
    if (czas == 0){
      czas_bezczynności = (millis()+120000);
      czas=1;
    }
    if (czas == 1){
      if (millis() > czas_bezczynności){
        czas = 2;
      }
    }
    if (czas == 2){
      if ((migotka == 0)&&(millis() > migotkamillis)){
        digitalWrite(STATUS_LED_GPIO, LOW);
        migotkamillis = millis()+200;
        migotka = 1;
      }
      if ((migotka == 1)&&(millis() > migotkamillis)){
        digitalWrite(STATUS_LED_GPIO, HIGH);
        migotkamillis = millis()+200;
        migotka = 2;
      }
      if ((migotka == 2)&&(millis() > migotkamillis)){
        digitalWrite(STATUS_LED_GPIO, LOW);
        migotkamillis = millis()+200;
        migotka = 3;
      }
      if ((migotka == 3)&&(millis() > migotkamillis)){
        digitalWrite(STATUS_LED_GPIO, HIGH);
        migotkamillis = millis()+1000;
        migotka = 0;
      }
    }
  }
  if ((polaczenie_z_serwerem != 40)&&((czas == 0)||(czas == 1))){
    //analogWrite(STATUS_LED_GPIO, 50);
    digitalWrite(STATUS_LED_GPIO, HIGH);
  }
  if (polaczenie_z_serwerem == 40){
    czas = 0;
    if ((migotka == 0)&&(millis() > migotkamillis)){
      digitalWrite(STATUS_LED_GPIO, LOW);
      migotkamillis = millis()+100;
      migotka = 1;
    }
    if ((migotka == 1)&&(millis() > migotkamillis)){
      digitalWrite(STATUS_LED_GPIO, HIGH);
      migotkamillis = millis()+100;
      migotka = 0;
    }
  }
  if (polaczenie_z_serwerem == 17) {
    czas = 0;
    if (wyslij_wiadomosc == LOW) {
        lastTime = millis();
          if (c_analog->getValue() < Bateryjka){
            Supla::Notification::Send(-1, "Battery", "Battery is to low, plaese change, or charge");
            wyslij_wiadomosc = HIGH;
          }
      }
    if (wifimilis1 == 0) {
      if (millis() > wifimilis){
        wifimilis = (millis() + 10000);
        wifimilis1 = 1;
        czas = 0;
      }
    }
    if (wifimilis1 == 1){    
      if ((wifimilis - millis())<1000){
        Serial.println("ide spac");
       // WiFi.forceSleepBegin();
       //wifimilis = (millis() + 30000);
       // wifimilis1 = 2;
        ESP.deepSleep((Usypianie*60)*1e6);
      }
    }
  }   
  if (wifimilis1 == 2){
      if ((wifimilis - millis())<1000){
        //WiFi.forceSleepWake();
        wifimilis1 = 0;
        Serial.println("kukuryku");
      }
  }
 // if (millis() > 4000000000){
 //   ESP.reset();
 // }   
  }
  
 