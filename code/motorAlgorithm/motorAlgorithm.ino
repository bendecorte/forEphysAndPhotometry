/*
Main algorithm to use for experiments. Things to adjust:
1. nullPosition: comes from the getNullPosition script where you find the right null value for your setup
If needed:
1. swap the values for 'clockwise' and 'counterClockwise' if your motor turns in the wrong direction when testing.  
*/

// algorithm
float nullPosition = 609.08; // adjust me!

// algorithm params
const float nullDeviationTolerance = .2 * nullPosition; // .2 means current position must deviate by 20% from null before motor engages. Too low, and you'll get weird performance from low, high-frequency movements (e.g., grooming)
const float stepDivisor = 8; // 8 means 8th step mode (smoother rotation)
const float degreesPerStep = 1.8/stepDivisor;
float degreesStepped = 0;
const float pulleyScaleFactor = 4;
const float safetyDegreeCriteria = 5*360*pulleyScaleFactor; // first number is number of continuous turns before auto-shutoff feature engages


float positionDeviation1; // these track deviation of each sensor value from null
float positionDeviation2; 
const int sampleDelayMicroseconds = 500; 
bool magnetOutOfRange = false;

// pins
const int hall1Pin = 18; // perpendicular to frame arms
float position1 = 0;
const int hall2Pin = 19; // parallel to frame arms
float position2 = 0;


// motor pin info (teensy->driver mapping)
const int dirPin = 0; 
const int stepPin = 1; 
const int sleepPin = 2;
const int ledPin = 13;
const int MS2Pin = 3; // these two let you toggle the step-mode (see below)
const int MS1Pin = 4;

// values for setting the direction that you write to the direction pin (flip these if your motor is turning in wrong direction during hand-test)
const int clockwise = 1; 
const int counterClockwise = 0;
int magnetDirection = clockwise; // just an default value


// speed parameters (helps with smooth acceleration)
const int minStepPulseWidth =300;
const int minStepPulsePause = 300;
const int startStepPulseWidth = 1000;
const int startStepPulsePause = 1000;
const int rampUpIncrement = 10;

// initial values
int currStepPulseWidth = startStepPulseWidth;
int currStepPulsePause = startStepPulsePause;

// buttons
const int button1Pin = 16;
const int button2Pin = 17;
bool userReset = false; // for button interrupts
bool doubleButtonPressDetected = false;
bool interuptSaysExit = false;


void setup() {
  // setup usb-communication with computer
  Serial.begin(9600);

  // pin setup
  analogReadAveraging(32); // calls to analog read will now represent average of 32 quick samples

  // Motor related pins
  pinMode(stepPin, OUTPUT); 
  pinMode(dirPin, OUTPUT); 
  pinMode(sleepPin, OUTPUT); 
  pinMode(MS1Pin, OUTPUT); 
  pinMode(MS2Pin, OUTPUT); 
  pinMode(ledPin, OUTPUT);

  // set step mode (need to hard-code if not here already)
  if (stepDivisor==1) {digitalWrite(MS1Pin, LOW); digitalWrite(MS2Pin, LOW);}
  if (stepDivisor==8) {digitalWrite(MS1Pin, HIGH); digitalWrite(MS2Pin, HIGH);}

  // button pins
  pinMode(button1Pin,INPUT_PULLUP); 
  pinMode(button2Pin,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button1Pin), button1Interrupt, LOW); // functions in here get executed whenever button pressed (i.e., goes to 0V/low)
  attachInterrupt(digitalPinToInterrupt(button2Pin), button2Interrupt, LOW);

  // keep motor asleep when not in use
  motorOff();
  digitalWrite(ledPin, LOW);
}


void loop() {
  // get 'position'/hall values
  position1  = analogRead(hall1Pin);
  position2  = analogRead(hall2Pin);

  // get deviation
  positionDeviation1 = position1 - nullPosition;
  positionDeviation2 = position2 - nullPosition;

  // decide if motor needs to be turned 
  magnetOutOfRange = checkIfMagnetOutOfRange(positionDeviation1, positionDeviation2, nullDeviationTolerance);
  if (magnetOutOfRange) {
    motorOn(); // just takes out of sleep-mode. doesn't turn.
    
    // set direction
    magnetDirection = getMagnetDirection(position1,position2,nullPosition); // toggles clockwise/counterclockwise appropriately
    setMotorDirection(magnetDirection); // toggles the direction pin accordingly. For button-interrupts, keep these steps de-coupled.

    magnetOutOfRange = true;
    degreesStepped = 0; // used to determine if auto-shutoff needed.
    while (magnetOutOfRange) {
      // execute step
      currStepPulseWidth = getPulseWidth(currStepPulseWidth);
      currStepPulsePause = getPulseWidth(currStepPulsePause);
      runStep(currStepPulseWidth,currStepPulsePause);     
      

      degreesStepped = degreesStepped+degreesPerStep;
      if (degreesStepped>safetyDegreeCriteria){
        userReset = false;
        motorOff();
        Serial.println("Error detected. Please inspect commutator and restart once resolved.");
        while (!userReset){
          userReset = checkForDoubleButtonPress(); // doesn't always work, so best to just unplug/replugin the teensy.        
        }
        Serial.println("user reset detected. starting again.");
        userReset = false;
        break;
      }
      if(interuptSaysExit){
        Serial.println("checking 4 direction change after interupt.");
        interuptSaysExit = false;
        break;   
      }

      // check position/if exit needed
      position1  = analogRead(hall1Pin);
      position2  = analogRead(hall2Pin);
      positionDeviation1 = position1 - nullPosition;
      positionDeviation2 = position2 - nullPosition;
      magnetOutOfRange = getMagnetDirection(position1,position2,nullPosition);
      if (!magnetOutOfRange) {
        motorOff(); 
        Serial.println("finished turning");
      }
    }
  }
  delayMicroseconds(sampleDelayMicroseconds); // small delay helps avoid overloading/flicker 
}

bool checkIfMagnetOutOfRange(float positionDeviation1, float positionDeviation2, float nullDeviationTolerance) {
  if ((abs(positionDeviation1) > nullDeviationTolerance) || (abs(positionDeviation2) > nullDeviationTolerance)){
    magnetOutOfRange = true;
  }
  else {
    magnetOutOfRange = false;
  }
  return (magnetOutOfRange);
}

int getMagnetDirection(float position1, float position2, float nullPosition) {
    // Don't skip the else by starting with a default value. Not as 'clean', but else guarantees only 1 execution per call.
    if ((position1  < nullPosition) && (position2 > nullPosition)) {
      magnetDirection = counterClockwise;
      Serial.println("counterClockwise");
    }
    else { // the else, rather than if-conditional, allows for edge cases that could in theory occur from oddly rapid turns.
      magnetDirection = counterClockwise;
      Serial.println("clockwise");
    }
    return (magnetDirection); 
}

void button1Interrupt(){
  doubleButtonPressDetected = checkForDoubleButtonPress();
  if (!doubleButtonPressDetected){
    Serial.println("button1PressedInt");
    motorOn();
    setMotorDirection(clockwise);
    while (!digitalRead(button1Pin)){
        currStepPulseWidth = getPulseWidth(currStepPulseWidth);
        currStepPulsePause = getPulseWidth(currStepPulsePause);
        runStep(currStepPulseWidth,currStepPulsePause); 
      }
    motorOff();
    interuptSaysExit = true;
    userReset = true;
  }
}

void button2Interrupt(){
  doubleButtonPressDetected = checkForDoubleButtonPress();
  if (!doubleButtonPressDetected){
    Serial.println("button2PressedInt");
    motorOn();
    setMotorDirection(counterClockwise);
    while (!digitalRead(button2Pin)){
        currStepPulseWidth = getPulseWidth(currStepPulseWidth);
        currStepPulsePause = getPulseWidth(currStepPulsePause);
        runStep(currStepPulseWidth,currStepPulsePause); 
      }
    motorOff();
    interuptSaysExit = true;
    userReset = true;
  }
}

bool checkForDoubleButtonPress(){
  doubleButtonPressDetected = false;
  if (!digitalRead(button1Pin) && !digitalRead(button2Pin)){
    delay(500);
    if (!digitalRead(button1Pin) && !digitalRead(button2Pin)){
      Serial.println("doublePress");
      doubleButtonPressDetected = true; 
      userReset = true;    
    }
  }
  return (doubleButtonPressDetected); 
}

int getPulseWidth (int currStepPulseWidth){
    if (currStepPulseWidth>minStepPulseWidth){
      currStepPulseWidth = currStepPulseWidth - rampUpIncrement;
    } else{
      currStepPulseWidth = minStepPulseWidth;
    }
    return (currStepPulseWidth);  
}

int getPulsePause (int currStepPulsePause){
    if (currStepPulsePause>minStepPulsePause){
      currStepPulsePause = currStepPulsePause - rampUpIncrement;
    } else{
      currStepPulsePause = minStepPulsePause;
    }
    return (currStepPulsePause);  
}

void runStep(int speedDelay, int stepPulsePause) {
    digitalWriteFast(stepPin,HIGH);
    delayMicroseconds(speedDelay);
    digitalWriteFast(stepPin,LOW);
    delayMicroseconds(stepPulsePause);   
}

void setStepStartConditions(){
   currStepPulseWidth = startStepPulseWidth;
   currStepPulsePause = startStepPulsePause;
}

void motorOff() {
    digitalWriteFast(ledPin, LOW); 
    digitalWriteFast(sleepPin, LOW);
    setStepStartConditions();
}

void motorOn() {
    digitalWriteFast(ledPin, HIGH); 
    digitalWriteFast(sleepPin, HIGH); 
    setStepStartConditions(); 
}

void setMotorDirection(int directionValue){
    digitalWriteFast(dirPin, directionValue); 
    delay(100);
}
