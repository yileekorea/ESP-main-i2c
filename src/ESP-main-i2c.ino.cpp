# 1 "c:\\users\\yj\\appdata\\local\\temp\\tmpjrla83"
#include <Arduino.h>
# 1 "F:/GitHub/ESP-main-i2c/src/ESP-main-i2c.ino"
#include <Arduino.h>
# 55 "F:/GitHub/ESP-main-i2c/src/ESP-main-i2c.ino"
#include "io2better.h"

#include "config.h"

#include "wifi.h"

#include "web_server.h"

#include "ota.h"

#include "input.h"

#include "output.h"



#include "mqtt.h"







unsigned long tempTry = 0;

int numSensor = 0;

byte s_loop = 0;
# 95 "F:/GitHub/ESP-main-i2c/src/ESP-main-i2c.ino"
void setup();
void loop();
#line 95 "F:/GitHub/ESP-main-i2c/src/ESP-main-i2c.ino"
void setup() {

  pinMode(ESP_RESET_CTL, OUTPUT);

  digitalWrite(ESP_RESET_CTL, HIGH);



  LED_setup(0.2);



  delay(1000);



  Serial.begin(115200);

#ifdef DEBUG_SERIAL1

  Serial1.begin(115200);

#endif



  DEBUG.println();

  DEBUG.print("io2Better ");



  DEBUG.println(WiFi.macAddress());

  DEBUG.println("Firmware: "+ currentfirmware);





  config_load_settings();







  wifi_setup();





  web_server_setup();





  ota_setup();



  if (wifi_mode==WIFI_MODE_STA){

    DEBUG.println("Start finger print verify : ");

    verifyFingerprint();

  }







  wireSetup();



  readOneWireAddr();



  SPIFFS2L_Temp();



  DEBUG.println("Loop start");

}
# 197 "F:/GitHub/ESP-main-i2c/src/ESP-main-i2c.ino"
void loop()

{

    ota_loop();

    web_server_loop();

    wifi_loop();







  if (wifi_mode==WIFI_MODE_STA || wifi_mode==WIFI_MODE_AP_AND_STA)

  {

    if((mqtt_server != 0) && (WiFi.status() == WL_CONNECTED))

    {

    mqtt_loop();



    if ((tempTry == 0 || ((millis() - tempTry) > 6000UL)) && 1)

    {

        DEBUG.println("Firmware: "+ currentfirmware);

        if ((userTempset == 1)){

            readFromOneWire();

            sendTempData();

            userTempset = 0;

        }

        else

        {

            measureTemperature(s_loop);

            readoutTemperature(s_loop);

            if (initSending > 0) {

              sendTempData();

              initSending > 0 ? initSending-- : initSending = 0;

            }

            else {

              send_a_TempData(s_loop);

            }

        }

      i2c_relayControl();

        tempTry = millis();

        s_loop == (numSensor-1) ? s_loop=0 : s_loop++;

    }



    }

  }

}