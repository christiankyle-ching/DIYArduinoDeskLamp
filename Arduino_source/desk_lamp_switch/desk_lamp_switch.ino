unsigned int pirSensor = 7;       //pir sensor pin
int pirValue;                     //pir sensor value - high or low

unsigned int photoresistor = A0;  //photoresistor pin
int lightValue;                   //photoresistor value - analog
int minimumLightValue = 240;       //minimum value before considering the place dark
int averageLight = 0;

int ledTest = LED_BUILTIN;        //led builtin
int output = 12;                  //output pin
int outputState;                  //is led on or off - high or low

int autoMode = 2;     //autoMode pin input
int alwaysOnMode = 3; //always on pin input
int currentMode;
bool resetDone = false;

unsigned long timeRemaining = 0;        //time remaining for light to close automatically
unsigned long additionalTime = 4000;    //time to add every motion detection
unsigned long maximumTimeOut = 16000;  //max limit of time out duration if continuous motion is detected - assuming person is doing something
unsigned long startTime;

int lastState = LOW;

void setup() {
  pinMode(pirSensor, INPUT);      //pir sensor pinMode
  pinMode(photoresistor, INPUT);  //photoresistor pinMode

  pinMode(autoMode,INPUT);        //auto mode pin
  pinMode(alwaysOnMode,INPUT);    //always on pin
  
  pinMode(output, OUTPUT);        //output pinMode
  pinMode(ledTest, OUTPUT);       //TEST ONLY

  //get dark light then set minimum light
  int counter = 0;
  while (millis() <= 3100) {
    counter++;
    averageLight += getAverageLight();
    delay(1000);
  }
  minimumLightValue = (averageLight / counter) + 10;

  Serial.begin(9600);
}

void loop() {
  //test
  if (digitalRead(autoMode) == HIGH) {
    Serial.println("AUTO");
    currentMode = 0;
  } else if (digitalRead(alwaysOnMode) == HIGH) {
    Serial.println("ALWAYS ON");
    timeRemaining = 0;
    currentMode = 1;
  } else {
    Serial.println("ALWAYS OFF");
    timeRemaining = 0;
    currentMode = 2;
  }

  switch (currentMode) {
    case 0:
      doAutoMode();     //run autoMode method
      break;
    case 1:
      switchOutput(1);  //turn on output
      break;
    case 2:
      switchOutput(0);   //turn off 
      break;
  }
  
//  if (digitalRead(autoMode) == HIGH) {
//    Serial.println("AUTO");
//    doAutoMode();     //run autoMode method
//  } else if (digitalRead(alwaysOnMode) == HIGH) {
//    Serial.println("ALWAYS ON");
//    switchOutput(1);  //turn on output
//  } else {
//    Serial.println("ALWAYS OFF");
//    switchOutput(0);   //turn off 
//  }

  Serial.print("Minimum Light: ");
  Serial.println(minimumLightValue);
  
  delay(250);
}

void addTimeRemaining() {
  if (timeRemaining < maximumTimeOut) {     //maximum time delay if continuous motion is detected is set to 100s
    timeRemaining += additionalTime;
    startTime = millis();
  }
}

int getAverageLight() {
  return ( ( (analogRead(photoresistor))
              
              ) / 1 //scalability purposes - in case of additional photoresistors
          );
}

bool isDark() {
  if (getAverageLight() < minimumLightValue) return true;
  
  return false;
}

void doAutoMode() {
  
  pirValue = digitalRead(pirSensor);          //read pir data
  lightValue = analogRead(photoresistor);     //read light detector data

  unsigned long currentTime = millis();
  
  //if motion sensor is triggered, add time to timeRemaining
  if (pirValue == HIGH) {
    if (lastState == LOW && isDark()) {    //after motion detection, this is triggered, only if all conditions are met
      addTimeRemaining();
    }
  }

  if (!isDark()) {
    Serial.println("OFFFFF");
    switchOutput(0);
    timeRemaining = 0;
  }

  Serial.println(!isDark());

  //check timeRemaining to decide if light stays on or off
  unsigned long timeDifference = currentTime - startTime;
  if (timeDifference < timeRemaining) {
    switchOutput(1);    //turn on output if there's still time left
  } else {
    switchOutput(0);    //turn off output if no time left    
    timeRemaining = 0;   //reset timer
  }

  lastState = pirValue;   //stores last pir state

  //debugging
  Serial.print("Motion:");
  Serial.println(pirValue);

  Serial.print("Light:");
  Serial.println(lightValue);

  Serial.print("Time Remaining:");
  Serial.println(timeRemaining);

  Serial.print("On/Off:");
  Serial.println(outputState);

  Serial.println(getAverageLight());
}

void switchOutput(int state) {
  if (state == 0) {
    digitalWrite(output, LOW);    
    digitalWrite(ledTest, LOW);   //TEST ONLY
    outputState = LOW;
  } else if (state == 1) {
    digitalWrite(output, HIGH);    
    digitalWrite(ledTest, HIGH);   //TEST ONLY
    outputState = HIGH;
  }
}
