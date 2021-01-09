#include <Arduino.h>
#include "LowPower.h"
#include <ArduinoJson.h>
#include <math.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

#define BATTERY_PIN       A3
#define REF_VOLTAGE       1.1
#define DEVICE_ID         150
#define VOLTAGE_DIVIDER   0.2326   // R2/(R1+R2) (100k + 330k)
#define SLEEP_TIME        30        // X * 8 sleep time
#define RX_PIN            3
#define TX_PIN            4
#define SET_PIN           5
#define ONE_WIRE_BUS      2
#define CONFIG_PIN        9         // set maintenance mode
#define A_FACTOR          1.0118    // a = y/x, callibration factor, y = real value, e.g 4.2V, x = value calculated, e.g. 4.15V

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
SoftwareSerial HC12(RX_PIN, TX_PIN); // HC-12 TX Pin, HC-12 RX Pin

int timeToSleep = SLEEP_TIME;

///////////////////////// FUNCTIONS /////////////////////////

float measureBattery(int iterations)
{
  float rawVoltage = 0;

  analogRead(BATTERY_PIN); //for stable measurements, drop first one

  for(int i = 0; i < iterations; i ++)
  {
    rawVoltage = rawVoltage + analogRead(BATTERY_PIN);
  }

  rawVoltage = rawVoltage / iterations;

  float measurement = (((rawVoltage / 1023) * REF_VOLTAGE) / VOLTAGE_DIVIDER) * A_FACTOR;

  return measurement;
}

float calculatePercentage(float voltage, float vmin, float vmax)
{
  float result = (voltage - vmin) * 100 / (vmax - vmin);
  
	return result >= 100 ? 100 : result;
}

String ComposeJSONmessage(int id, float temp, float bat, float level, int counter)
{
  String message;

  const size_t capacity = JSON_OBJECT_SIZE(5) + 20;
  DynamicJsonDocument doc(capacity);

  doc["id"] = id;
  doc["temp"] = roundf(temp * 100) / 100.0;
  doc["bat"] = roundf(bat * 1000) / 1000.0;
  doc["lvl"] = roundf(level * 1) / 1.0;
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

void SendCommand(String command)
{
  digitalWrite(SET_PIN, LOW);
  delay(100);
  HC12.print(command);
  delay(100);
  while (HC12.available()) 
  {           // If HC-12 has data (the AT Command response)
    Serial.write(HC12.read());         // Send the data to Serial monitor
  }
  digitalWrite(SET_PIN, HIGH);
  delay(100);
}

void sendMessage(String toSend)
{
  SendCommand("AT");
  Serial.println(toSend);
  HC12.println(toSend);      // Send that data to HC-12
  delay(50);
  SendCommand("AT+SLEEP");
}

unsigned int CheckMode(unsigned int pin)
{
  unsigned int time;

  if(digitalRead(pin) == HIGH)
    time = SLEEP_TIME;
  else
    time = 1;
  
  return time;
}

///////////////////////// SETUP /////////////////////////

void setup() 
{
  Serial.begin(115200);
  HC12.begin(9600);               // Serial port to HC12
  sensors.begin();

  analogReference(INTERNAL);
  pinMode(BATTERY_PIN, INPUT);
  pinMode(CONFIG_PIN, INPUT_PULLUP);
  pinMode(SET_PIN, OUTPUT);
}

void loop() 
{
  static int counter;
  Serial.print("Counter: ");
  Serial.println(counter);

  float temperature = readTemperature();
  Serial.print("Temperature: ");
  Serial.println(temperature);

  float voltage = measureBattery(5);
  Serial.print("Voltage: ");
  Serial.println(voltage);

  float level = calculatePercentage(voltage, 3.2, 4.2);
  Serial.print("Level: ");
  Serial.println(level);

  String message = ComposeJSONmessage(DEVICE_ID, temperature, voltage, level, counter++);
  Serial.print("Message: ");
  Serial.println(message);

  sendMessage(message);

  int Timecounter = CheckMode(CONFIG_PIN);

  //Serial.print("Sleep time: ");
  //Serial.println(Timecounter * 8);
  //Serial.println("Going to sleep");
  //delay(500);

  for(int i = 0; i < Timecounter; i++)
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}