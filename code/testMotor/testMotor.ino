// for commutator v0.2 Tested 2024-03-08
// Connections to drv8834, mp6500
const int directionPin = 0; // Direction
const int stepPin = 1; // Step
const int sleepPin = 2; 
const int ledPin = 13;
const int MS2Pin = 3;
const int MS1Pin = 4;

// Motor parameters
const int pulleyScaleFactor = 4; // ratio of large/small gear diameters
const int stepsPerRev = 200; // steps to make full turn with motor. Assuming full-step mode.
const int stepPulseWidthMicroseconds = 2000; // pulse-width to turn the motor one step
const int interstepPauseMicroseconds = 5000; // pause between each step
const int clockwiseDirection = 0; // value you write to the direction pin to turn this direction.
const int counterClockwiseDirection = 1;


//turn parameters
const int numForwardTurns = 2;
const int numReverseTurns = 2;
bool doneSpinning = false;


void setup() {
  // let teensy send data to computer over usb
  Serial.begin(9600);

  // Setup the pins as Outputs
  pinMode(stepPin, OUTPUT); 
  pinMode(directionPin, OUTPUT); 
  pinMode(sleepPin, OUTPUT); 
  pinMode(MS1Pin, OUTPUT); 
  pinMode(MS2Pin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // set driver to full step mode
  digitalWrite(MS1Pin, LOW); 
  digitalWrite(MS2Pin, LOW);

  // enable motor
  digitalWrite(sleepPin, HIGH);
  

}
void loop() {
  if (!doneSpinning) {
    digitalWrite(ledPin, HIGH); 
    digitalWrite(directionPin, clockwiseDirection); delay(50);
    for (int x = 0; x < stepsPerRev * numForwardTurns * pulleyScaleFactor; x++) {   
      // one step per iteration
        digitalWrite(stepPin,HIGH);
        delayMicroseconds(stepPulseWidthMicroseconds);
        digitalWrite(stepPin,LOW);
        delayMicroseconds(interstepPauseMicroseconds);

    }
    digitalWrite(directionPin, counterClockwiseDirection); delay(50);
    for (int x = 0; x < stepsPerRev * numReverseTurns * pulleyScaleFactor; x++) {
        digitalWrite(stepPin,HIGH);
        delayMicroseconds(stepPulseWidthMicroseconds);
        digitalWrite(stepPin,LOW);
        delayMicroseconds(interstepPauseMicroseconds);
    }
    digitalWrite(ledPin, LOW); 
    doneSpinning = true;
    digitalWrite(sleepPin, LOW); // cut current to motor
  }

}
