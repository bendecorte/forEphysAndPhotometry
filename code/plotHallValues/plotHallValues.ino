// for basic verification that hall sensors work. Tested 2024-03-08
// pins
const int adcPin1 = 18; // hall 1 pin
float reading1 = 0; // holds value for hall 1
const int adcPin2 = 19; // same but for hall 2
float reading2 = 0; 

void setup() {
  Serial.begin(9600);
  analogReadAveraging(32); // specific to teensy. each analog reading will be mean of 32 quick samples, improving resolution.
  
  reading1  = analogRead(adcPin1);
  reading2  = analogRead(adcPin2);  
  
}
void loop() {
    reading1  = analogRead(adcPin1);
    reading2  = analogRead(adcPin2);
    Serial.print(reading1); delay(1);
    Serial.print(","); 
    Serial.println(reading2); delay(1);
    

}   
