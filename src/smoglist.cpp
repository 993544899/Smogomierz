#ifdef ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#elif defined ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <WiFiClient.h>
#endif

#include <ArduinoJson.h>
#include "config.h"

const char *SmoglistServerName = "api.smoglist.pl"; // api.smoglist.pl:8090/postjson
const uint16_t SmoglistPort = 8090;

void sendSmoglistJson(JsonObject& json) {
    if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
    WiFiClient client;
	client.setTimeout(12000);
    Serial.print(F("\nconnecting to "));
    Serial.println(SmoglistServerName);

    if (!client.connect(SmoglistServerName, SmoglistPort)) {
        Serial.println(F("connection failed"));
        //Serial.println("wait 3 sec...\n");
        //delay(3000);
        return;
    }
    delay(100); 

    client.println("POST /postjson HTTP/1.1");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
	client.println(measureJson(json));
    client.println();
	serializeJson(json, client);
    if (DEBUG) {
		serializeJsonPretty(json, Serial);
	}
		unsigned long timeout = millis();
		while (client.available() == 0) {
			if (millis() - timeout > 5000) {
	            Serial.println(F("\n\t>>> Client Timeout!\n"));
	            client.stop();
	            return;
	        }
	    }
		if(client.available()) {
			String line = client.readStringUntil('\r');
			line.trim();
			if (DEBUG) {
				Serial.println("\n"+ line);
			}
		}
	client.stop();
	}
}

void sendSmoglistData(float currentTemperature, float currentPressure, float currentHumidity, int averagePM1, int averagePM25, int averagePM4, int averagePM10) {
	StaticJsonDocument<1024> jsonBuffer;
	JsonObject json = jsonBuffer.to<JsonObject>();
	
#ifdef ARDUINO_ARCH_ESP8266
	json["CHIPID"] = "Smogly-" + String(ESP.getChipId());
#elif defined ARDUINO_ARCH_ESP32
	json["CHIPID"] = "Smogly-" + String((uint32_t)(ESP.getEfuseMac()));
#endif
	json["SOFTWAREVERSION"] = String(SOFTWAREVERSION);
	json["HARDWAREVERSION"] = String(HARDWAREVERSION); // "1.0 - ESP8266" or "2.0 - ESP32"
	json["PMSENSORVERSION"] = String(PMSENSORVERSION); // PMS, SDS, HPMA115S0 ora SPS30
	
	json["FREQUENTMEASUREMENT"] = int(FREQUENTMEASUREMENT); // frequent measurements - True or False
	json["DISPLAY_PM1"] = bool(DISPLAY_PM1); // True or False
	
	json["DUST_TIME"] = int(DUST_TIME); // frequency of PM measurements;  default - 1
	json["NUMBEROFMEASUREMENTS"] = int(NUMBEROFMEASUREMENTS); //  default - 10
	
	json["SENDING_FREQUENCY"] = int(SENDING_FREQUENCY); // default - 2
	json["SENDING_DB_FREQUENCY"] = int(SENDING_DB_FREQUENCY); //  default - 2
	
	json["LUFTDATEN_ON"] = bool(LUFTDATEN_ON); // True or False
	
	json["AIRMONITOR_ON"] = bool(AIRMONITOR_ON); // True or False
	json["AIRMONITOR_GRAPH_ON"] = bool(AIRMONITOR_GRAPH_ON); // True or False
	
	json["THINGSPEAK_ON"] = bool(THINGSPEAK_ON); // True or False
	json["THINGSPEAK_GRAPH_ON"] = bool(THINGSPEAK_GRAPH_ON); // True or False
	
	json["INFLUXDB_ON"] = bool(INFLUXDB_ON); // True or False
	json["MQTT_ON"] = bool(MQTT_ON); // True or False
	
	json["DEEPSLEEP_ON"] = bool(DEEPSLEEP_ON); // True or False
	json["AUTOUPDATE_ON"] = bool(AUTOUPDATE_ON); // True or False
	json["MODEL"] = String(MODEL); // default "white" - automatic calibration, "red" - without calibration
	
	json["LATITUDE"] = String(LATITUDE); // default - 50.2639
    json["LONGITUDE"] = String(LONGITUDE); //  default - 18.9957

	json["MYALTITUDE"] = int(MYALTITUDE); // int;  default - 271.00
	
	//PM data
	if (!strcmp(DUST_MODEL, "PMS7003")) {
		json["DUST_MODEL"] = "PMS5003/7003";
	}
	if (!strcmp(DUST_MODEL, "HPMA115S0")) {
		json["DUST_MODEL"] = "HPMA115S0";
	}
	if (!strcmp(DUST_MODEL, "SDS011/21")) {
		json["DUST_MODEL"] = "SDS011/21";
	}
	if (!strcmp(DUST_MODEL, "SPS30")) {
		json["DUST_MODEL"] = "SPS30";
	}
	if (!strcmp(DUST_MODEL, "Non")) {
		json["DUST_MODEL"] = "Non";
	}
	json["PM1"] = int(averagePM1);
	json["PM25"] = int(averagePM25);
	json["PM4"] = int(averagePM4);
    json["PM10"] = int(averagePM10);
	
	// Temp/Humi/Pressure data
	if (strcmp(THP_MODEL, "Non")) {
		if (!strcmp(THP_MODEL, "BME280")) {
			json["THP_MODEL"] = "BME280";
		} else if (!strcmp(THP_MODEL, "BMP280")) {
			json["THP_MODEL"] = "BMP280";
  		} else if (!strcmp(THP_MODEL, "HTU21")) {
			json["THP_MODEL"] = "HTU21";
  		} else if (!strcmp(THP_MODEL, "DHT22")) {
			json["THP_MODEL"] = "DHT22";
  		} else if (!strcmp(THP_MODEL, "SHT1x")) {
  			json["THP_MODEL"] = "SHT1x";
  		} else if (!strcmp(THP_MODEL, "DS18B20")) {
  			json["THP_MODEL"] = "DS18B20";
  		}
		json["Temperature"] = float(currentTemperature);
		json["Humidity"] = float(currentHumidity);
		json["Pressure"] = float(currentPressure);
	} else {
		json["THP_MODEL"] = "Non";
		json["Temperature"] = 0;
		json["Humidity"] = 0;
		json["Pressure"] = 0;
	}
    sendSmoglistJson(json);
}

void sendDataToSmoglist(float currentTemperature, float currentPressure, float currentHumidity, int averagePM1, int averagePM25, int averagePM4, int averagePM10) {
    if (!(SMOGLIST_ON)) {
        return;
    }
    sendSmoglistData(currentTemperature, currentPressure, currentHumidity, averagePM1, averagePM25, averagePM4, averagePM10);
}
