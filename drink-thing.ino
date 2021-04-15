#include "HX711.h"

HX711 scale;

const int buttonPin = 5;  
const int pumpPin = 12;

float calibrationFactor = 461;  //load cell calibration factor

int buttonState = 0;     // current state of the button
int lastButtonState = 0; // previous state of the button
int startPressed = 0;    // the moment the button was pressed
int endPressed = 0;      // the moment the button was released
int holdTime = 0;        // how long the button was hold
int pumpState = 0;     // pump state (on/off)
int hasMeasured = 0;

float currWeight = 0;
float minWeight = 0;
float maxWeight = 0;

void setup()
{
  pinMode(buttonPin, INPUT); // initialize the button and pump
  pinMode(pumpPin, OUTPUT);
  
  Serial.begin(9600);        // initialize serial communication

  scale.begin(3, 2);

  scale.set_scale();
  scale.tare();  //Reset the scale to 0

  long zeroFactor = scale.read_average(); //Get a baseline reading
}

void loop()
{
  scale.set_scale(calibrationFactor);
  
  buttonState = digitalRead(buttonPin); // read the button input

  if (buttonState != lastButtonState)
  { 
     updateButton(); // button state changed. See function for more info
  }
  else
  {
    getWeight(currWeight);
    while(currWeight >= minWeight && currWeight != 0 && hasMeasured == 2)
    {
      if(currWeight >= maxWeight)
      {
        digitalWrite(12, LOW);
        pumpState = 0;
        break;
      }
        digitalWrite(12, HIGH);
        pumpState = 1;
    }
  }

  lastButtonState = buttonState;        // save state for next loop
}

void updateButton()
{
  // the button has been just pressed
  if (buttonState == HIGH && lastButtonState == LOW)
  {
      startPressed = millis();
      
      if(pumpState==1 && hasMeasured == 1)
      {
         digitalWrite(12, LOW);
         pumpState = 0;
         setMinMax(maxWeight);
         if(maxWeight > 0) {maxWeight += 5;}
         hasMeasured = 2;
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
            digitalWrite(12, HIGH);
            pumpState = 1;
            setMinMax(minWeight);
            minWeight -= 5;
            if(minWeight < 0) {minWeight = 0;}
            hasMeasured = 1;
        }
      }
      
      if (holdTime > 3000)
      {
          minWeight = 0;
          maxWeight = 0;
          digitalWrite(12, LOW);
          pumpState = 0;
          hasMeasured = 0;
      }

  }
}

float getWeight(float weight)
{
  weight = scale.get_units(), 10;
  if (weight < 0)
  {
    weight = 0.00;
  }
  
  return weight;
}

float setMinMax(float weight)
{
  float temp[10];
  float units;
  
  if(pumpState == 1)
  {
    for(int i = 0; i < 10; i++)
    {
      getWeight(units);
      temp[i] = units;

      if(i > 0)
      {
        if(weight > temp[i])
        {
          weight = temp[i];
        }
      }
      else
      {
        weight = temp[0];
      }
    }
  }

  if(pumpState == 0)
  {
    for(int i = 0; i < 10; i++)
    {
      getWeight(units);
      temp[i] = units;

      if(i > 0)
      {
        if(weight < temp[i])
        {
          weight = temp[i];
        }
      }
      else
      {
        weight = temp[0];
      }
    }
  }

  return weight;
}
