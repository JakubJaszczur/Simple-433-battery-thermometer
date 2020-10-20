#include <Arduino.h>
#include "LowPower.h"
#include <ArduinoJson.h>
#include <math.h>
#include <RH_ASK.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define BATTERY_PIN       A3
#define REF_VOLTAGE       1.1
#define DEVICE_ID         150
#define VOLTAGE_DIVIDER   0.256   // R2/(R1+R2)
#define SLEEP_TIME        1       // X * 8 sleep time

#define RADIOHEAD_BAUD    2000    // Transmission Speed
#define RADIOHEAD_TX_PIN  5       // Pin of the 433MHz transmitter
#define RADIOHEAD_RX_PIN  -1      // Pin of the 433MHz receiver (here not used)

#define ONE_WIRE_BUS 2

RH_ASK driver(RADIOHEAD_BAUD, RADIOHEAD_RX_PIN, RADIOHEAD_TX_PIN);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

///////////////////////// FUNCTIONS /////////////////////////

float measureBattery()
{
  float rawVoltage = 0;

  for(int i = 0; i < 5; i ++)
  {
    rawVoltage = analogRead(BATTERY_PIN); //for stable measurements
  }

  float measurement = ((rawVoltage / 1023) * REF_VOLTAGE) / VOLTAGE_DIVIDER;

  return measurement;
}

int calculatePercentage(float voltage)
{
  int result = map(voltage, 3.2, 4.2, 0, 100);
  return result;
}

String ComposeJSONmessage(int id, float temp, float bat, int level, int counter)
{
  String message;

  const size_t capacity = JSON_OBJECT_SIZE(5) + 20;
  DynamicJsonDocument doc(capacity);

  doc["id"] = id;
  doc["temp"] = roundf(temp * 100) / 100.0;
  doc["bat"] = roundf(bat * 100) / 100.0;
  doc["lvl"] = level;
  doc["cnt"] = counter;

  serializeJson(doc, message);

  return message;
}

float readTemperature()
{
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  return tempC;
}

void sendMessage(String toSend)
{
  char message[toSend.length() + 1];
  toSend.toCharArray(message, sizeof(message));
  
  // eliminate null
  char data[toSend.length()];

  for(int i = 0; i < (toSend.length()); i ++)
  {
    data[i] = message[i];
  }
  
  driver.send((uint8_t *)data, sizeof(data));
  driver.waitPacketSent();
}

///////////////////////// SETUP /////////////////////////

void setup() 
{
  driver.init();
  sensors.begin();

  analogReference(INTERNAL);
  pinMode(BATTERY_PIN, INPUT);
}

void loop() 
{
  static int counter;
  float temperature = readTemperature();
  float voltage = measureBattery();
  int level = calculatePercentage(voltage);

  String message = ComposeJSONmessage(DEVICE_ID, temperature, voltage, level, counter++);
  sendMessage(message);

  for(int i = 0; i <= SLEEP_TIME; i++)
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}