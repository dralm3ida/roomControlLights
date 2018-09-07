#include <Wire.h>
#include <stdint.h>
#include <Smartcar.h>

Car car;

const uint8_t SONIC_DISC_I2C_ADDRESS = 0x09;
const uint8_t NUM_OF_SENSORS = 8; // No. of ultrasonic sensors on SonicDisc
// The packet contains NUM_OF_MEASUREMENTS measurements and an error code
const uint8_t I2C_PACKET_SIZE = NUM_OF_SENSORS + 1;
// The number of measurements from each sensor to filter
const uint8_t MEASUREMENTS_TO_FILTER = 5;
const uint8_t INT_PIN = 2;
// The max valid variance in a set of MEASUREMENTS_TO_FILTER measurements
const unsigned int VARIANCE_THRESHOLD = 3;

// Sonic Disc's operational states
enum State {
  STANDBY, // MCU and sensors are on but no measurements are being made
  MEASURING // Sonic Disc is conducting measurements using the sensors
};

// Values to be received via I2C from master
enum I2C_RECEIPT_CODE {
  STATE_TO_STANDBY = 0x0A,
  STATE_TO_MEASURING = 0x0B
};

// Error codes to be transmitted via I2c to the master
enum I2C_ERROR_CODE {
  NO_ERROR,
  IN_STANDBY,
  INCOMPLETE_MEASUREMENT
};

// Flag to indicate the SonicDisc is ready to send a new set of data
volatile bool newData = false;

uint8_t filterIndex = 0;
uint8_t filterBuffer[MEASUREMENTS_TO_FILTER][NUM_OF_SENSORS] = {0};
uint8_t filteredMeasurements[NUM_OF_SENSORS] = {0};
bool newFilteredMeasurements = false;

// The different parking states
enum ParkingState {
  PARKING_START,        // Initial state, no obstacle has been found yet
  FIRST_OBSTACLE,       // First obstacle is detected
  BETWEEN_OBSTACLES,    // First obstacle not detected. Driving through the gap
  SECOND_OBSTACLE,      // Second obstacle is detected
  REVERSE_RIGHT,        // First parking maneuver
  REVERSE_LEFT,         // Second parking maneuver
  FINAL_POSITION_FIX,   // Going forward to be in the middle of the parking spot
  PARKING_END           // Parking has finished, remain idle
};

ParkingState parkingState = PARKING_START; // Holds the current parking state

/**
   Requests an I2C packet from the SonicDisc
   @param  i2cInput         The array that will hold the incoming packet
   @param  transmissionSize The size/length of the incoming packet
   @return                  Error code contained inside the incoming packet
*/
I2C_ERROR_CODE requestPacket(uint8_t i2cInput[], const uint8_t transmissionSize = I2C_PACKET_SIZE) {
  Wire.requestFrom(SONIC_DISC_I2C_ADDRESS, transmissionSize);
  uint8_t packetIndex = 0;
  while (Wire.available() && packetIndex < transmissionSize) {
    i2cInput[packetIndex++] = Wire.read();
  }
  return i2cInput[0]; // Return the packet's error code
}

/**
   Sends the supplied byte to the SonicDisc
   @param byteToSend The byte to be sent
*/
void sendData(uint8_t byteToSend) {
  Wire.beginTransmission(SONIC_DISC_I2C_ADDRESS);
  Wire.write(byteToSend);
  Wire.endTransmission(SONIC_DISC_I2C_ADDRESS);
}

/**
   ISR that raises a flag whenever SonicDisc is ready to transmit new data.
*/
void newSonicDiscData() {
  newData = true;
}

/*
   Adds the specified i2c packet in the buffer to be sorted later.
*/
void addInputToFilterBuffer(uint8_t i2cInput[], const uint8_t bufferIndex) {
  // Copy the whole packet (except error code) in the specified row of the buffer
  for (int i = 0, j = 1; i < NUM_OF_SENSORS; i++, j++) {
    filterBuffer[bufferIndex][i] = i2cInput[j];
  }
}

/*
    Sorts the measurements of each sensor for every cycle of measurements.
*/
void sortMeasurements() {
  // For each sensor
  for (int s = 0; s < NUM_OF_SENSORS; s++) {
    // Use bubble sort to sort all measurements throughout the cycle
    for (int i = 0; i < MEASUREMENTS_TO_FILTER - 1; i++) {
      for (int j = 0; j < MEASUREMENTS_TO_FILTER - i - 1; j++) {
        if (filterBuffer[j][s] > filterBuffer[j + 1][s]) {
          uint8_t toSwap = filterBuffer[j][s];
          filterBuffer[j][s] = filterBuffer[j + 1][s];
          filterBuffer[j + 1][s] = toSwap;
        }
      }
    }
  }
}

/*
    Filter measurements depending on the variance.
    If variance is too high for a MEASUREMENTS_TO_FILTER measurements of a sensor
    then these measurements are disregarded. Otherwise the mean value is chosen.
*/
void filterMeasurements() {
  // Go through all the measurements taken for each sensor
  for (int i = 0; i < NUM_OF_SENSORS; i++) {
    // Calculate the variance across the different measurements
    // by subtracting the first and the last element
    // of the *sorted* measurement cycle.
    int variance = filterBuffer[0][i] - filterBuffer[MEASUREMENTS_TO_FILTER - 1][i];
    if (abs(variance) > VARIANCE_THRESHOLD) {
      filteredMeasurements[i] = 0;
    } else {
      filteredMeasurements[i] = filterBuffer[MEASUREMENTS_TO_FILTER / 2][i];
    }
  }
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(INT_PIN), newSonicDiscData, RISING);
  Serial.println("Requesting packet from SonicDisc");
  uint8_t dummyInput[I2C_PACKET_SIZE] = {0}; // A throw-away array
  // Do not proceed unless the SonicDisc is in "MEASURING" state
  while (requestPacket(dummyInput, I2C_PACKET_SIZE) == IN_STANDBY) {
    Serial.println("Setting state to MEASURING");
    sendData(STATE_TO_MEASURING);
  }
  Serial.println("Communication is established and SonicDisc is measuring distances");
  car.begin();
  car.setSpeed(0);
  car.setAngle(0);
}

void loop() {
  if (newData) {
    newData = false; // Indicate that we have read the latest data

    uint8_t sonicDiscInput[I2C_PACKET_SIZE] = {0};
    // Get the I2C packet
    I2C_ERROR_CODE ret = requestPacket(sonicDiscInput, I2C_PACKET_SIZE);
    // Now sonicDiscInput contains the latest measurements from the sensors.
    // However we need to make sure that the data is also of good quality
    // Process the packet only if it is a valid one
    if (ret == NO_ERROR) {
      addInputToFilterBuffer(sonicDiscInput, filterIndex);
      // When we have filled up the filter buffer, time to filter the measurements
      if (filterIndex + 1 == MEASUREMENTS_TO_FILTER) {
        // For each measurement
        sortMeasurements();
        filterMeasurements();
        // Indicate the the measurements are filtered
        newFilteredMeasurements = true;
      }
      // Move along the index
      filterIndex = (filterIndex + 1) % MEASUREMENTS_TO_FILTER;
    }
  }

  if (newFilteredMeasurements) {
    newFilteredMeasurements = false;
    // Based on the current orientation of the SonicDisc
    // fetch the measurements we are interested in.
    uint8_t front = filteredMeasurements[6];
    uint8_t frontRight = filteredMeasurements[7];
    uint8_t frontLeft = filteredMeasurements[5];
    uint8_t left = filteredMeasurements[4];
    uint8_t right = filteredMeasurements[0];
    uint8_t backLeft = filteredMeasurements[3];
    uint8_t back = filteredMeasurements[2];

    switch (parkingState) {
      static const uint8_t MIN_SIDE_DIST = 30; // In cm
      static const uint8_t BACK_LEFT_DIST = 40;
      static const uint8_t MIN_BACK_LEFT_DIST = 20;
      static const uint8_t MIN_BACK_DIST = 20;
      static const uint8_t MIN_FRONT_DIST = 15;
      static const uint8_t motorSpeed = 60; // Percentage power to motors
      static unsigned long secondObstacleDetected = 0;

      case PARKING_START:
        {
          car.setSpeed(motorSpeed);
          if (right && right <= MIN_SIDE_DIST) {
            parkingState = FIRST_OBSTACLE;
          }
        }
        break;
      case FIRST_OBSTACLE:
        {
          car.setSpeed(motorSpeed);
          if (right == 0 || right > MIN_SIDE_DIST) {
            parkingState = BETWEEN_OBSTACLES;
          }
        }
        break;
      case BETWEEN_OBSTACLES:
        {
          car.setSpeed(motorSpeed);
          if (right && right <= MIN_SIDE_DIST ) {
            parkingState = SECOND_OBSTACLE;
            // While at the second obstacle, we need a timed "oomph"
            // therefore we need to log when we entered the state.
            secondObstacleDetected = millis();
          }
        }
        break;
      case SECOND_OBSTACLE:
        {
          car.setSpeed(-motorSpeed);
          static const unsigned long OOMPH = 350; // In ms
          unsigned long currentTime = millis();
          // Move for a limited time so to improve our position before the
          // parking maneuvers.
          if (currentTime >= secondObstacleDetected + OOMPH) {
            parkingState = REVERSE_RIGHT;
          }
        }
        break;
      case REVERSE_RIGHT:
        {
          car.setSpeed(-motorSpeed);
          car.setAngle(60);
          // Find the shortest distance among the specified sensors
          // and use that to determine whether we need to change maneuver.
          uint8_t minLeftSideDist = left ? left : 255; // If the distance is 0 then assign a large value instead
          if (backLeft && backLeft < minLeftSideDist) minLeftSideDist = backLeft;
          if (minLeftSideDist && minLeftSideDist <= BACK_LEFT_DIST) {
            // In case we are already too close, skip reversing left
            if (minLeftSideDist <= MIN_BACK_LEFT_DIST) {
              parkingState = FINAL_POSITION_FIX;
            } else {
              parkingState = REVERSE_LEFT;
            }
          }
        }
        break;
      case REVERSE_LEFT:
        {
          car.setSpeed(-motorSpeed);
          car.setAngle(-80);
          // Find the shortest distance among the specified sensors
          // and use that to determine whether we need to change maneuver.
          uint8_t minBackSideDist = back ? back : 255; // If the distance is 0 then assign a large value instead
          if (backLeft && backLeft < minBackSideDist) minBackSideDist = backLeft;
          if (minBackSideDist && minBackSideDist <= MIN_BACK_DIST) {
            parkingState = FINAL_POSITION_FIX;
          }
        }
        break;
      case FINAL_POSITION_FIX:
        {
          car.setSpeed(motorSpeed);
          car.setAngle(45);
          // Find the shortest distance among the specified sensors
          // and use that to determine whether we need to change maneuver.
          uint8_t minFrontSideDist = front ? front : 255; // If the distance is 0 then assign a large value instead
          if (frontRight && frontRight < minFrontSideDist) minFrontSideDist = frontRight;
          if (frontLeft && frontLeft < minFrontSideDist) minFrontSideDist = frontLeft;
          if (minFrontSideDist && minFrontSideDist <= MIN_FRONT_DIST) {
            parkingState = PARKING_END;
            // Do not wait for the new state, stop the car already.
            car.stop();
          }
        }
        break;
      case PARKING_END:
        {
          car.setSpeed(0);
          car.setAngle(0);
        }
        break;
      default:
        break;
    }

  }
}
