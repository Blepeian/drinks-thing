#include "HX711.h"

HX711 scale;

const int buttonPin = 5;  
const int pumpPin = 12;
const int loadcellDout = 3; 
const int loadcellSck = 2;

float calibrationFactor = 461;  //load cell calibration factor

int buttonState = 0;     // current state of the button
int lastButtonState = 0; // previous state of the button
int startPressed = 0;    // the moment the button was pressed
int endPressed = 0;      // the moment the button was released
int holdTime = 0;        // how long the button was hold
int pumpState = 0;     // pump state (on/off)
int hasMeasured = 0;

float currWeight = 0;
float minWeight;
float maxWeight;

void setup()
{
  pinMode(buttonPin, INPUT); // initialize the button and pump
  pinMode(pumpPin, OUTPUT);
  
  Serial.begin(9600);        // initialize serial communication

  scale.begin(loadcellDout, loadcellSck);

  scale.set_scale();
  scale.tare();  //Reset the scale to 0
}

void loop()
{
  scale.set_scale(calibrationFactor);
  
  buttonState = digitalRead(buttonPin); // read the button input

  if (buttonState != lastButtonState)
  { 
     updateButton(); // button state changed. See function for more info
  }

  lastButtonState = buttonState;        // save state for next loop
  
  if(hasMeasured == 2)
  {
      currWeight = scale.get_units();
      if(currWeight < 0)
      {
        currWeight = 0;
      }
      Serial.print(currWeight);
      Serial.println(" grams");

     if(currWeight >= maxWeight)
     {
       digitalWrite(pumpPin, LOW);
       pumpState = 0;
     }
     else if(currWeight < minWeight)
     {
       digitalWrite(pumpPin, LOW);
       pumpState = 0;
     }
     else if(currWeight >= minWeight && currWeight > 0 && currWeight < maxWeight)
     {
         digitalWrite(pumpPin, HIGH);
         pumpState = 1;
     }
  }
}

void updateButton()
{
  // the button has been just pressed
  if (buttonState == HIGH && lastButtonState == LOW)
  {
      startPressed = millis();
      
      if(pumpState == 1 && hasMeasured == 1)
      {
         digitalWrite(pumpPin, LOW);
         pumpState = 0;
         maxWeight = scale.get_units();
         if(maxWeight > 0) {maxWeight -= 2;}
         hasMeasured = 2;
         Serial.print(maxWeight);
         Serial.println(" max");
      }
  }
  else if(buttonState == LOW)
  {
      endPressed = millis();
      holdTime = endPressed - startPressed;

      if (holdTime >= 200 && holdTime < 3000)
      {
        if(pumpState == 0 && hasMeasured == 0)
        {
            digitalWrite(pumpPin, HIGH);
            pumpState = 1;
            minWeight = scale.get_units();
            minWeight -= 25;
            if(minWeight < 0) {minWeight = 0;}
            hasMeasured = 1;
            Serial.print(minWeight);
            Serial.println(" min");
        }
      }
      
      if (holdTime >= 3000)
      {
          minWeight = 0;
          maxWeight = 0;
          digitalWrite(pumpPin, LOW);
          pumpState = 0;
          hasMeasured = 0;
      }
  }
}
