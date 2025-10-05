// commutator v0.2 tested 2024-03-08
// target .7-1Amps equates to .2-.28V between VREF and GND, respectively.
// Connections to drv8834, mp6500
const int dirPin = 0; // Direction
const int stepPin = 1; // Step
const int sleepPin = 2; // Step
const int MS2Pin = 3;
const int MS1Pin = 4;

const int ledPin = 13;
bool sleepPinOn = false;
   
void setup() {
  // let teensy send data to computer over USB
  Serial.begin(9600);
  // Setup the pins as Outputs
  pinMode(stepPin, OUTPUT); 
  pinMode(dirPin, OUTPUT); 
  pinMode(sleepPin, OUTPUT); 
  pinMode(MS1Pin, OUTPUT); 
  pinMode(MS2Pin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  // set driver to full step mode (arbitrary)
  digitalWrite(MS1Pin, LOW); 
  digitalWrite(MS2Pin, LOW);

  // enable motor / let current flow
  digitalWrite(sleepPin, LOW);
  
}

void loop() {
  if (!sleepPinOn){
    digitalWrite(ledPin,HIGH);
    digitalWrite(sleepPin,HIGH);
    sleepPinOn = true;
  }

}
