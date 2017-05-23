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

#include "OneWire.h"


OneWire  ds(2);  // on pin 2 (a 4.7K resistor is necessary)

String sName[]= {"a","b","c","d","e","f","g","h","i","j"};
float old_celsius[] = {26,26,26,26,26,26,26,26};
float celsius[] = {26,26,26,26,26,26,26,26};
float rStatus[] = {0,0,0,0,0,0,0,0}; //room status all OFF
float L_Temp[] = {26.7,26.7,26.7,26.7,26.7,26.7,26.7,26.7};
byte address[10][8];


String input_string="";
String last_datastr="";

/*
 * Sensor address read
 */
void readOneWireAddr()
{
  byte nSensor = 0;
  byte i;
  byte addr[8];

  //Serial.println("readOneWireAddr");
  while (ds.search(addr)) {
    sName[nSensor] = "";
    Serial.println();
    Serial.print("ROM =");
    for ( i = 0; i < 8; i++) {
      sName[nSensor] += String(addr[i], HEX);
      address[nSensor][i] = addr[i];
      Serial.write(' ');
      //Serial.print(addr[i], HEX);
      Serial.print(address[nSensor][i], HEX);
    }
    //Serial.print(sName[nSensor]);

    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!:" + nSensor);
    }
    Serial.println();
    nSensor++;
    delay(250);
  }
  numSensor = nSensor;
  DEBUG.printf("number of Sensors are '%d'\n", numSensor);

}

/*
 * read out the measured Temperature data
 */
void readoutTemperature(byte Sensor)
{
  byte nSensor = Sensor;
  byte s, i;
  byte present = 0;
  byte type_s = 0; //Chip = DS18B20
  byte data[12];
  byte addr[8];

  //Serial.println("readoutTemperature");
  //while (ds.search(addr)) {
  //for ( s = 0; i < numSensor; i++) {
    for ( i = 0; i < 8; i++) {
      addr[i] = address[nSensor][i];
    }

    // the first ROM byte indicates which chip
    switch (addr[0]) {
          case 0x10:
          //Serial.println("  Chip = DS18S20");  // or old DS1820
          type_s = 1;
          break;
    case 0x28:
          //Serial.println("  Chip = DS18B20");
          type_s = 0;
          break;
    case 0x22:
          //Serial.println("  Chip = DS1822");
          type_s = 0;
          break;
    default:
          Serial.println("Device is not a DS18x20 family device.");
    }

    // we might do a ds.depower() here, but the reset will take care of it.
    present = ds.reset();
    ds.select(addr);
    ds.write(0xBE);     // Read Scratchpad
    /*
    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" ");
    */
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = ds.read();
        //Serial.print(data[i], HEX);
        //Serial.print(" ");
    }
    /*
    Serial.print(" CRC=");
    Serial.print(OneWire::crc8(data, 8), HEX);
    Serial.println();
    */


    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10) {
			// "count remain" gives full 12 bit resolution
			raw = (raw & 0xFFF0) + 12 - data[6];
        }
    } else {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
        // default is 12 bit resolution, 750 ms conversion time
		// but set as 10bits
		raw = raw & ~3; // 10 bit res, 187.5 ms
    }
    celsius[nSensor] = (float)raw / 16.0;
    //fahrenheit[nSensor] = celsius[nSensor] * 1.8 + 32.0;
    Serial.print("  Temps = ");
    Serial.print(celsius[nSensor]);
    Serial.print(" 'C");


	if(L_Temp[nSensor]){
		Serial.print("  L_Temp[");
		Serial.print(nSensor);
		Serial.print("] ====> ");
		Serial.print(L_Temp[nSensor]);
	}

	if(L_Temp[nSensor] <= celsius[nSensor]){
		rStatus[nSensor] = 0;
	}else{
		rStatus[nSensor] = L_Temp[nSensor];
		if(L_Temp[nSensor]){
			Serial.print("  rStatus[] -----> ");
			Serial.println(rStatus[nSensor]);
		}
	}

  //} //numSensor
} //readTemperature

/*
 * ask Sensor to measure Temperature
 */
void measureTemperature(byte Sensor)
{
  byte nSensor = Sensor;
  byte s, i;
  byte type_s;
  byte addr[8];

  //Serial.println("measureTemperature");
  //while (ds.search(addr)) {
  //for ( s = 0; i < numSensor; i++) {
    for ( i = 0; i < 8; i++) {
      addr[i] = address[nSensor][i];
    }

    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);  // start conversion, with parasite power on at the end

    //delay(1000);     	// maybe 750ms is enough, maybe not
    //delay(200);     	// maybe 187.5ms is enough, maybe not
    delay(20);     	// test purpose
  //} //numSensor
} //measureTemperature

/*
 * Temperature measurement
 */
String readFromOneWire()
{
  String payload="";

    byte nSensor = 0;
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte addr[8];

    while (ds.search(addr)) {
    //    measure ();
    sName[nSensor] = "";
    //Serial.print("ROM =");
    for ( i = 0; i < 8; i++) {
        //Serial.write(' ');
        //Serial.print(addr[i], HEX);
		sName[nSensor] += String(addr[i], HEX);
    }
    //Serial.print(sName[nSensor]);

    if (OneWire::crc8(addr, 7) != addr[7]) {
        Serial.println("CRC is not valid!");
		payload="";
        return payload;
    }
    Serial.println();

    // the first ROM byte indicates which chip
    switch (addr[0]) {
          case 0x10:
          //Serial.println("  Chip = DS18S20");  // or old DS1820
          type_s = 1;
          break;
    case 0x28:
          //Serial.println("  Chip = DS18B20");
          type_s = 0;
          break;
    case 0x22:
          //Serial.println("  Chip = DS1822");
          type_s = 0;
          break;
    default:
          Serial.println("Device is not a DS18x20 family device.");
		  payload="";
          return payload;
    }

    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1);  // start conversion, with parasite power on at the end

    //delay(1000);     	// maybe 750ms is enough, maybe not
    delay(200);     	// maybe 187.5ms is enough, maybe not
    //delay(20);     	// test purpose


    // we might do a ds.depower() here, but the reset will take care of it.
    present = ds.reset();
    ds.select(addr);
    ds.write(0xBE);     // Read Scratchpad
    /*
    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" ");
    */
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
        data[i] = ds.read();
        //Serial.print(data[i], HEX);
        //Serial.print(" ");
    }
    /*
    Serial.print(" CRC=");
    Serial.print(OneWire::crc8(data, 8), HEX);
    Serial.println();
    */


    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10) {
			// "count remain" gives full 12 bit resolution
			raw = (raw & 0xFFF0) + 12 - data[6];
        }
    } else {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
        // default is 12 bit resolution, 750 ms conversion time
		// but set as 10bits
		raw = raw & ~3; // 10 bit res, 187.5 ms
    }
    celsius[nSensor] = (float)raw / 16.0;
    //fahrenheit[nSensor] = celsius[nSensor] * 1.8 + 32.0;
    Serial.print("  Temps = ");
    Serial.print(celsius[nSensor]);
    Serial.print(" 'C");


	if(L_Temp[nSensor]){
		Serial.print("  L_Temp[");
		Serial.print(nSensor);
		Serial.print("] ====> ");
		Serial.print(L_Temp[nSensor]);
	}

	if(L_Temp[nSensor] <= celsius[nSensor]){
		rStatus[nSensor] = 0;
	}else{
		rStatus[nSensor] = L_Temp[nSensor];
		if(L_Temp[nSensor]){
			Serial.print("  rStatus[] -----> ");
			Serial.println(rStatus[nSensor]);
		}
	}

    nSensor += 1;
	payload = "OK";

  }
    Serial.println();
    Serial.println("No more addresses.");
    ds.reset_search();
    delay(1000);

	numSensor = nSensor;
    return payload;
}


boolean input_get(String& data)
{
  boolean gotData = false;

  // If data from test API e.g `http://<IP-ADDRESS>/input?string=CT1:3935,CT2:325,T1:12.5,T2:16.9,T3:11.2,T4:34.7`
  if(input_string.length() > 0) {
    data = input_string;
    input_string = "";
    gotData = true;
  }
  // If data received on serial
  else if (Serial.available()) {
    // Could check for string integrity here
    data = Serial.readStringUntil('\n');
    gotData = true;
  }

  if(gotData)
  {
    // Get rid of any whitespace, newlines etc
    data.trim();

    if(data.length() > 0) {
      DEBUG.printf("Got '%s'\n", data.c_str());
      last_datastr = data;
    } else {
      gotData = false;
    }
  }

  return gotData;
}
