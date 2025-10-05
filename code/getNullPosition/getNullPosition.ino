// for finding null position between hall sensors. Last tested for box 2 2024-03-08:  610.27
/*
Notes on use: start with the magnet's face pointed in between the two hall sensors.
Slowly rotate the magnet 45 degrees to point directly at Hall 1 (or Hall 2, doesn't matter). 
Then slowly rotate in the other direction to where it points directly at Hall 2.

Importantly, don't make a full 360 degree turn. Just keep turning back and forth within that 90 degree
range.
*/

// pins
const int adcPin1 = 18; // pin that connects to Hall 1's output lead
float reading1 = 0; // will hold the value read from the ADC/sensor for Hall 1
const int adcPin2 = 19; // same but for Hall 2
float reading2 = 0; 

// variables for tracking the moving average
float nullPositionCumulativeSum = 0;
float nullPositionCount = 0;
float nullPositionMean = 0;

void setup() {
  Serial.begin(9600);
  analogReadAveraging(32); // only available for teensy. Each reading will now be the average of 32 quick readings, improving resolution.
  reading1  = analogRead(adcPin1); // set initial values
  reading2  = analogRead(adcPin2);
}

void loop() {
    // get current values
    reading1  = analogRead(adcPin1);
    reading2  = analogRead(adcPin2);

    // if the magnet is at the null position (~45 degrees from either sensor), track the equality-value and update average.
    if (reading1==reading2){
      nullPositionCumulativeSum = nullPositionCumulativeSum + reading1;
      nullPositionCount = nullPositionCount + 1;
      nullPositionMean = nullPositionCumulativeSum / nullPositionCount;
      Serial.println("recommended null position: ");
      Serial.println(nullPositionMean);
    }

}   
