#include <Arduino.h>
#include <Wire.h>
#include "SparkFun_VL53L5CX_Library.h"

// Commenting out the lines below will change the data collection from polling to interrupt driven.
// An interrupt pin must be connected to the /INT pin on the sensor board.

//#define INT_PIN 4
//void interruptRoutine();            // function prototype for ISR handler


#define STAT_LED  LED_BUILTIN
#define LPN_PIN   3

SparkFun_VL53L5CX vl53l5cx;           // Main sensor object
VL53L5CX_ResultsData measurementData; // Result data class structure
bool dataReady = false;               // flag that indicates that updated data is available

void setup()
{
  bool success = false;

  pinMode(LPN_PIN, OUTPUT);
  pinMode(STAT_LED, OUTPUT);

  digitalWrite(STAT_LED, LOW);

  digitalWrite(LPN_PIN, LOW);
  delay(100);
  digitalWrite(LPN_PIN, HIGH);
  delay(200);

  Serial.begin(115200);
  Serial.print(F("\nInitializing sensor board, please wait... "));
  Wire.setClock(400000);
  Wire.begin();

  // Inititialize the board and upload the sensor's firmware
  success = vl53l5cx.begin();
  if (success)
  {
    Serial.println(F("Sensor initialized."));
  }
  else
  {
    Serial.println(F("Sensor not initialized - check your wiring."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }
  Serial.printf("Current API version: %s\n", VL53L5CX_API_REVISION);

  // Change device address. Any other VL53L5CX boards in the bus must have their corresponding /LPN pin pulled LOW
  // so just the target board will have it's address changed.
  /*
  success = vl53l5cx.setAddress(0x53);
  if (success)
  {
    Serial.printf(F("Address changed to 0x%x.\n"), vl53l5cx.getAddress());
  }
  else
  {
    Serial.println(F("Cannot change sensor address."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }
  */

  // Check if device is alive
  success = vl53l5cx.isConnected();
  if (success)
  {
    Serial.println(F("Sensor is connected."));
  }
  else
  {
    Serial.println(F("Cannot reach vl53l5cx."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }

  // Set the ranging frequency
  // Using 4x4, min frequency is 1Hz and max is 60Hz
  // Using 8x8, min frequency is 1Hz and max is 15Hz

  success = vl53l5cx.setRangingFrequency(1);
  if (success)
  {
    uint8_t frequency = vl53l5cx.getRangingFrequency();
    if (frequency > 0)
    {
      Serial.printf("Ranging frequency set to %i Hz.\n", frequency);
    }
    else
    {
      Serial.println(F("Error recovering ranging frequency."));
    }
  }
  else
  {
    Serial.println(F("Cannot set ranging frequency requested."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }

  // Set the ranging resolution
  success = vl53l5cx.setRangingResolution(SF_VL53L5CX_RANGING_RESOLUTION::RES_8X8);
  if (success)
  {
    SF_VL53L5CX_RANGING_RESOLUTION resolution = vl53l5cx.getResolution();
    switch (resolution)
    {
    case SF_VL53L5CX_RANGING_RESOLUTION::RES_8X8:
      Serial.println(F("Resolution set to 8x8."));
      break;

    case SF_VL53L5CX_RANGING_RESOLUTION::RES_4X4:
      Serial.println(F("Resolution set to 4x4."));
      break;

    default:
      Serial.println(F("Error recovering ranging resolution."));
    }
  }
  else
  {
    Serial.println(F("Cannot set ranging resolution requested."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }

  // Set the ranging mode
  success = vl53l5cx.setRangingMode(SF_VL53L5CX_RANGING_MODE::AUTONOMOUS);
  if (success)
  {
    SF_VL53L5CX_RANGING_MODE mode = vl53l5cx.getRangingMode();
    switch (mode)
    {
    case SF_VL53L5CX_RANGING_MODE::AUTONOMOUS:
      Serial.println(F("Ranging mode set to autonomous."));
      break;

    case SF_VL53L5CX_RANGING_MODE::CONTINUOUS:
      Serial.println(F("Ranging mode set to continuous."));
      break;

    default:
      Serial.println(F("Error recovering ranging mode."));
      break;
    }
  }
  else
  {
    Serial.println(F("Cannot set ranging mode requested."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }

  // Set device to sleep mode
  success = vl53l5cx.setPowerMode(SF_VL53L5CX_POWER_MODE::SLEEP);
  if (success)
  {
    Serial.println(F("Set device to sleep mode."));
  }
  else
  {
    Serial.println(F("Cannot set device to sleep mode."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }

  // Check if device is actually in sleep mode
  SF_VL53L5CX_POWER_MODE currentPowerMode = vl53l5cx.getPowerMode();
  switch (currentPowerMode)
  {
  case SF_VL53L5CX_POWER_MODE::SLEEP:
    Serial.println(F("Shhhh... device is sleeping!"));
    break;

  case SF_VL53L5CX_POWER_MODE::WAKEUP:
    Serial.println(F("Device is awake."));
    break;

  default:
    Serial.println(F("Cannot retrieve device power mode."));
    break;
  }

  // Allow us to check messages before proceeding
  Serial.print(F("Waking up device in 5..."));
  delay(1000);
  Serial.print(F(" 4..."));
  delay(1000);
  Serial.print(F(" 3..."));
  delay(1000);
  Serial.print(F(" 2..."));
  delay(1000);
  Serial.println(F(" 1..."));
  delay(1000);
  
  // Wake up device
  success = vl53l5cx.setPowerMode(SF_VL53L5CX_POWER_MODE::WAKEUP);
  if (success)
  {
    Serial.println(F("Set device to wakeup mode."));
  }
  else
  {
    Serial.println(F("Cannot wakeup device."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }

  // Check if device is awake
  currentPowerMode = vl53l5cx.getPowerMode();
  switch (currentPowerMode)
  {
  case SF_VL53L5CX_POWER_MODE::SLEEP:
    Serial.println(F("Shhhh... device is sleeping!"));
    break;

  case SF_VL53L5CX_POWER_MODE::WAKEUP:
    Serial.println(F("Device is awake."));
    break;

  default:
    Serial.println(F("Cannot retrieve device power mode."));
    break;
  }

  Serial.printf("Current integration time is %u miliseconds.\n", vl53l5cx.getIntegrationTime());

  success = vl53l5cx.setIntegrationTime(8);
  if (success)
  {
    Serial.printf("Integration time changed to %u miliseconds.\n", vl53l5cx.getIntegrationTime());
  }
  else
  {
    Serial.println(F("Cannot set integration time."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }

  Serial.printf("Current sharpener value is %i percent.\n", vl53l5cx.getSharpenerPercent());

  success = vl53l5cx.setSharpenerPercent(19);
  if (success)
  {
    Serial.printf("Sharpener value changed to %i percent.\n", vl53l5cx.getSharpenerPercent());
  }
  else
  {
    Serial.println(F("Cannot set sharpener value."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }

  SF_VL53L5CX_TARGET_ORDER order = vl53l5cx.getTargetOrder();

  switch (order)
  {
  case SF_VL53L5CX_TARGET_ORDER::STRONGEST:
    Serial.println(F("Current target order is strongest."));
    break;

  case SF_VL53L5CX_TARGET_ORDER::CLOSEST:
    Serial.println(F("Current target order is closest."));
    break;

  default:
    Serial.println(F("Cannot get target order."));
    break;
  }

  success = vl53l5cx.setTargetOrder(SF_VL53L5CX_TARGET_ORDER::CLOSEST);
  if (success)
  {
    order = vl53l5cx.getTargetOrder();

    switch (order)
    {
    case SF_VL53L5CX_TARGET_ORDER::STRONGEST:
      Serial.println(F("Target order set to strongest."));
      break;

    case SF_VL53L5CX_TARGET_ORDER::CLOSEST:
      Serial.println(F("Target order set to closest."));
      break;

    default:
      break;
    }
  }
  else
  {
    Serial.println(F("Cannot set target order."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }

  // Wait again so we can read all messages
  delay(5000);

  //Uncomment below for using the sensor with an interrupt
  /*
  // Attach the interrupt
  attachInterrupt(digitalPinToInterrupt(INT_PIN), interruptRoutine, FALLING);
  Serial.println(F("Interrupt pin configured."));
 */ 
 
  // Start ranging per se
  success = vl53l5cx.startRanging();
  if (success)
  {
    Serial.println(F("Ranging has started."));
    Serial.println();
  }
  else
  {
    Serial.println(F("Cannot start ranging."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }

  // Uncomment below for testing the STOP ranging command
  /*  
  delay(2000);

  // Stop ranging
  success = vl53l5cx.stopRanging();
  if (success)
  {
    Serial.println(F("Ranging has stopped."));
  }
  else
  {
    Serial.println(F("Cannot stop ranging."));
    Serial.println(F("System halted."));
    while (1)
    {
    }
  }
  */
}

void loop()
{
  bool success = false;
  uint8_t resolution = 0;

  // Uncomment below for polling the sensor instead of waiting for an interrupt
  dataReady = vl53l5cx.isDataReady();

  if (dataReady)
  {
    digitalWrite(STAT_LED, HIGH);
    resolution = (vl53l5cx.getResolution() == SF_VL53L5CX_RANGING_RESOLUTION::RES_8X8) ? 64 : 16;
    success = vl53l5cx.getRangingData(&measurementData);
    if (success)
    {
      if (resolution == 16)
      {
        for (uint8_t row = 0; row < 4; row++)
        {
          Serial.printf("\t%i\t%i\t%i\t%i\n", measurementData.distance_mm[row * 4 + 0], measurementData.distance_mm[row * 4 + 1],
                        measurementData.distance_mm[row * 4 + 2], measurementData.distance_mm[row * 4 + 3]);
        }
      }
      else
      {
        for (uint8_t row = 0; row < 8; row++)
        {
          Serial.printf("\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\n", measurementData.distance_mm[row * 8 + 0], measurementData.distance_mm[row * 8 + 1],
                        measurementData.distance_mm[row * 8 + 2], measurementData.distance_mm[row * 8 + 3],
                        measurementData.distance_mm[row * 8 + 4], measurementData.distance_mm[row * 8 + 5],
                        measurementData.distance_mm[row * 8 + 6], measurementData.distance_mm[row * 8 + 7]);
        }
      }
      Serial.println();
      digitalWrite(STAT_LED, LOW);
      dataReady = false;
    }
  }
  else
  {
    delay(5);
  }
}

//Uncomment below for using the sensor with an interrupt
/*
void interruptRoutine()
{
  // Just set the flag that we have updated data and return from the ISR
  dataReady = true;
}
*/
