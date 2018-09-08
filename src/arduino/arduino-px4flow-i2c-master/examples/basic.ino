/*
 * Copyright (c) 2014 by Laurent Eschenauer <laurent@eschenauer.be>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */

#include <Wire.h>
#include "PX4Flow.h"

// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0

// CONFIG HERE
#define LED 13
#define PARAM_FOCAL_LENGTH_MM 16
#define SCALE_FACTOR 1.5
#define MAX_OUTPUT 255
#define MIN_OUTPUT 0
#define ITERATION 10

long last_check = 0;
//float focal_length_px = (PARAM_FOCAL_LENGTH_MM) / (4.0f * 6.0f) * 1000.0f;

int iterations = 0;
int motion_intensity_tmp_x = 0;
int motion_intensity_tmp_y = 0;
int motion_intensity = 0;
int old_motion_intensity = 0;
int number_of_sums = 0;

int flow_x_sum = 0;
int flow_y_sum = 0; 
int flow_comp_x = 0;
int flow_comp_y = 0;
int quality_sum = 0;
int ground_distance_sum = 0;
int frame_count_sum = 0;

int flow_x_integral = 0;   
int flow_y_integral = 0;    
int quality_integral = 0;
int ground_distance_integral = 0;
int frame_count_integral = 0;

int timespan = 0; // microseconds
    
// Initialize PX4Flow library
PX4Flow sensor = PX4Flow(); 

void setup()
{
  // Initialize the digital pin as an output.
  pinMode(LED, OUTPUT);   
  
  // Initialize the I2C bus
  Wire.begin();       
  
  // Initialize the serial connection
  Serial.begin(9600);  
}

void loop()
{
  long loop_start = millis();
    
  if ( (loop_start - last_check) > ITERATION) {
    iterations ++;
    // Fetch I2C data  
    sensor.update_integral();
    sensor.update();

    flow_x_sum = sensor.pixel_flow_x_sum();
    flow_y_sum = sensor.pixel_flow_y_sum(); 
    flow_comp_x = sensor.flow_comp_m_x();
    flow_comp_y = sensor.flow_comp_m_y();
    quality_sum = sensor.qual();
    ground_distance_sum = sensor.ground_distance();
    frame_count_sum = sensor.frame_count();

    flow_x_integral = sensor.pixel_flow_x_integral();     
    flow_y_integral = sensor.pixel_flow_y_integral();     
    quality_integral = sensor.qual();
    ground_distance_integral = sensor.ground_distance_integral();
    frame_count_integral = sensor.ground_distance_integral();

    timespan = sensor.integration_timespan(); // microseconds

    Serial.print(flow_x_sum);
    Serial.print(",");
    Serial.print(flow_y_sum);
    Serial.print(",");
    Serial.print(old_motion_intensity);
    Serial.print(",");
    Serial.print(flow_x_integral);
    Serial.print(",");
    Serial.print(flow_y_integral);
    Serial.println();
    
/*
 * Values group 1
 */
 /*
    Serial.print(flow_x_sum);Serial.print(",");    
    Serial.print(flow_y_sum);Serial.print(",");    

    Serial.print(flow_comp_x);Serial.print(",");    
    Serial.print(flow_comp_y);Serial.print(","); 

    Serial.print(quality_sum);Serial.print(","); 

    Serial.print(ground_distance_sum);Serial.print(",");
*/ 
/*
 * Values group 2
 */
 /*
    Serial.print(flow_x_integral);Serial.print(",");    
    Serial.print(flow_y_integral);Serial.print(",");

    Serial.print(quality_integral);Serial.print(","); 

    Serial.print(ground_distance_integral);Serial.println();
*/
    //Serial.print(motion_intensity);Serial.println();


  if ( (iterations % ITERATION) == 0) 
  {
     motion_intensity += ((motion_intensity_tmp_x + motion_intensity_tmp_y))/(ITERATION);

    if(motion_intensity < MIN_OUTPUT)
    {
      motion_intensity = MIN_OUTPUT;
    }
    else if(motion_intensity > MAX_OUTPUT)
    {
      motion_intensity = MAX_OUTPUT;
    }
   
    /*
     * Reset to avarage varaibles
     */
    motion_intensity_tmp_x = 0;
    motion_intensity_tmp_y = 0;
    number_of_sums = 0;
    iterations = 0;

    //Update 
    old_motion_intensity = motion_intensity;
    motion_intensity = 0;
  }
  else
  {
    /*
     * Sum to avarage
     */
    motion_intensity_tmp_x = motion_intensity_tmp_x + (abs(flow_x_sum)*SCALE_FACTOR); 
    motion_intensity_tmp_y = motion_intensity_tmp_y + (abs(flow_y_sum)*SCALE_FACTOR);
  }
  
    last_check = loop_start;
  }
}

