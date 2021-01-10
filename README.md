## Simple 433 battery thermometer


### 1. Introduction

Simple project with Dallas DS18B20 thermometer and precise battery measurements. Data are formatted in JSON and send by HC12 433MHz module to receiver. Project can be use as small temperature sensor powered by battery, and also for car battery monitoring. Code can be used in most of Arduino boards with internal voltage source 1.1V.


### 2. Features

- Temperature measurements  
- Battery voltage measurements (using divider)  
- Calibration factor, for more precise voltage measurements  
- Discharge equation can be used for percentage calculation  
- Service mode  

### 3. Libraries

- ArduinoJson by Benoit Blanchon
- DallasTemperature by Miles Burton
- OneWire by Paul Stoffregen  

### 4. Getting Started

To run the project, user can change some settings in define section:

Basic settings
```
DEVICE_ID         150
VOLTAGE_DIVIDER   0.2326
SLEEP_TIME        30
A_FACTOR          1.0118
```
DEVICE_ID - used in my system, this is used to recognize many devices, which send data through HC12 to one gateway used to catch data  
  
VOLTAGE_DIVIDER - divider ratio, resistor divider is used to adjust measured voltage to maximum value of 1.1V.  

Value = R2/(R1+R2)  

Use 1% or better resistors. Good practice is to measure real resistance values, and use them to calculate divider (instead theoritical values).

![](Pictures/Divider.png)

SLEEP_TIME - this value x 8s (minimum sleep time) = sleep time / measurements period  

Sleep time is set to 30 x 8s by default. Code includes service mode. If user shorts CONFIG_PIN to GND, device will sleep only 8s. This can be use to maintenance/debug device without uploading code with different sleep time (to don't wait too much time for data).

A_FACTOR - value used to calibrate measurements  

Simple calibration:
1. Connect presise voltage source as measured voltage. Use highest possible value, which can be measured by your device, e.g. 4.2V
2. Set calibration factor to 1, upload code to your board, set service mode
3. Open terminal, check voltage read by device. E.g. you can get 4.156V
4. Calculate factor as RealValue/CalculatedValue. In exaple, we get 4.2/4.156 = 1.0106
5. Use calculatet value as new calibration factor (A_FACTOR)

User can change equation used to calculate battery percentage in <b>calculatePercentage</b> function. By default is used Li-Ion battery equation calculated by author of this code. 3.2V is set as 0%, and 4.2V as 100%.

### 5. Output parameters

Parameters send by device are in JSON format:

```json
{
"id":150,
"temp":23.9,
"bat":4.037,
"lvl":96,
"cnt":101,
}
```

cnt - messages counter