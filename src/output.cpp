/*
 * -------------------------------------------------------------------
 * ESP-main Serial to ESP-main-2 gateway
 * -------------------------------------------------------------------
 * Adaptation of Chris Howells OpenEVSE ESP Wifi
 * by Trystan Lea, Glyn Hudson, OpenEnergyMonitor
 * All adaptation GNU General Public License as below.
 *
 * -------------------------------------------------------------------
 *
 * This file is part of ESP-main-web project.
 * ESP-main is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * ESP-main is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with ESP-main; see the file COPYING.  If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "io2better.h"
#include "input.h"
#include "output.h"
//#include "gpio_MCP23S17.h"   // import library

#include "Adafruit_MCP23017.h"

//#define R1_BUILTIN 4

//const uint8_t sclk = 14;
//const uint8_t mosi =13; //Master Output Slave Input ESP8266=Master,MCP23S08=slave
/*
const uint8_t MCP_CS = 15;
gpio_MCP23S17 mcp(MCP_CS,0x20);//instance
*/
Adafruit_MCP23017 mcp_i2c;

Ticker ticker;
/*
unsigned long Timer_1[] = {0,0,0,0,0,0,0,0};
unsigned long Timer_2[] = {0,0,0,0,0,0,0,0};
byte isOFF[] = {0,0,0,0,0,0,0,0};
*/
void wireSetup()
{
  byte i;
  mcp_i2c.begin();      // use default address 0
  DEBUG.println("Start I2C wireSetup: ");

  for ( i = 0; i < numberofOUT_gpio ; i++) {
    mcp_i2c.pinMode(i, OUTPUT);
    mcp_i2c.digitalWrite(i, LOW);
    delay(100);
  }
  delay(3000);
  for ( i = 0; i < numberofOUT_gpio ; i++) {
    mcp_i2c.digitalWrite(i, HIGH);
    delay(100);
  }

  DEBUG.println("End I2C wireSetup: ");

  for ( i = 8; i < (numberofIN_gpio + 8) ; i++) {
    mcp_i2c.pinMode(i, INPUT);
  }
}

/*
 * I2C relayControl
 */
void i2c_relayControl() {
    byte i;

    for ( i = 1; i < (numSensor-1) ; i++) {
      rStatus[i] == 0 ? mcp_i2c.digitalWrite(i+3,HIGH) : mcp_i2c.digitalWrite(i+3,LOW);
    }
/*
	for ( i = 0; i < (numSensor) ; i++) {
    //if((L_Temp[i] <= celsius[i]) && ((millis() - Timer_2[i]) > 60000UL)) { // 1min
		if((L_Temp[i] <= celsius[i]) && ((millis() - Timer_2[i]) > 60000UL) && (isOFF[i] == 0)) { // 1min
      //if(isOFF[i] == 0)
      {
        mcp_i2c.digitalWrite(i,HIGH); //if current celsius Greater than setting --> off
        Timer_1[i] = millis();
        isOFF[i] = 1;
        DEBUG.print(" isOFF: ");
        DEBUG.print(i);
        DEBUG.println(isOFF[i]);
      }
		}



    //if((L_Temp[i] > celsius[i]) || ((millis() - Timer_1[i]) > 60000UL)) {
    if(L_Temp[i] > celsius[i]) {
      DEBUG.println();
      DEBUG.print("L_Temp-");
      DEBUG.print(i);
      DEBUG.print(" : ");
      DEBUG.print(L_Temp[i]);
      DEBUG.print("    celsius-");
      DEBUG.print(i);
      DEBUG.print(" : ");
      DEBUG.println(celsius[i]);
      DEBUG.println();

      mcp_i2c.digitalWrite(i,LOW);
      isOFF[i] = 0;
		}
    else if((millis() - Timer_1[i]) > 180000UL) { //3min
      DEBUG.println();
      DEBUG.print(i);
      DEBUG.print(" : millis-");
      DEBUG.print(millis());
      DEBUG.print("  -   vControlTimer-");
      DEBUG.print(Timer_1[i]);
      DEBUG.print("  =  ");
      DEBUG.println((millis() - Timer_1[i]));

      Timer_1[i] = millis();
      Timer_2[i] = millis();
      mcp_i2c.digitalWrite(i,LOW);
      isOFF[i] = 0;

    }
	} // for
*/
}

void wireLoop()
{
  int state = mcp_i2c.digitalRead(0);  // get the current state of GPIO1 pin
      DEBUG.println(" digitalRead(0): " + state);

  mcp_i2c.digitalWrite(0, !state);     // set pin to the opposite state
  mcp_i2c.digitalWrite(7, state);     // set pin to the opposite state
  delay(9000);

}
/*
 * relayControl
 */
/*
void relayControl() {
    byte i;
	if(L_Temp[0] <= celsius[0]){
		digitalWrite(R1_BUILTIN,LOW);
	} else {
		digitalWrite(R1_BUILTIN,HIGH);
	}
	for ( i = 1; i < (numSensor-1) ; i++) {
		if(L_Temp[i] <= celsius[i]){
			mcp.gpioDigitalWrite(i+3,LOW); //mcp.GPIO 0 ~ 3 for input sense
		} else {
			mcp.gpioDigitalWrite(i+3,HIGH);
		}
	}
}

void mcp_GPIO_setup() {
  Serial.print("Attempting SPI mcp.begin()...");
  Serial.println();
  mcp.begin(0);//x.begin(1) will override automatic SPI initialization

  pinMode(R1_BUILTIN, OUTPUT);
  //mcp.gpioPinMode(0x00);
  mcp.gpioPinMode(OUTPUT);

  digitalWrite(R1_BUILTIN, HIGH);
  mcp.gpioPort(0xFFFF);
  Serial.print("mcp.gpioPort(0xFF)...");
  delay(9000);
  mcp.gpioPort(0x0000);
  digitalWrite(R1_BUILTIN, LOW);
  Serial.print("mcp.gpioPort(0x00)...");
  delay(9000);
}
*/
void tick(){
  //toggle state
  int state = digitalRead(ESP_LED);  // get the current state of GPIO1 pin
  digitalWrite(ESP_LED, !state);     // set pin to the opposite state
}

void LED_setup(float t) {
  //set led pin as output
  pinMode(ESP_LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(t, tick);
 }

void LED_clear() {
 	ticker.detach();
	//keep LED off
	digitalWrite(ESP_LED, HIGH);
}
