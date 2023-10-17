/* 
  constants won't change. They're used here to set pin numbers:
*/
const int powerButtonPin = 2;  // the 4th button on the coffee machine "control the idle of the machine"
const int waterButtonPin = 3; // control the water intake for the hot water..
const int coffeButton1Pin = 10; // coffee button.
const int coffeButton2Pin = 9; // coffee button.

const int inletCoffeePin = 6;  // the hot water for the coffee.
const int dispenseHotWaterPin = 4; // bring in water to the machine.
const int heaterPin = 7; // pin for the heater element.

const int heater_on_temperature  = 73; // the mini temp where the element would turn back on.
const int heater_off_temperature  = 79; // the mix temp where the element would turn off.

const int powerLED = 11;//the power light
const int stateLED = 12;//the state light

/* 
  variables will change:
*/
bool dispenseState = false;
bool heaterElementState = true;
bool machineState = false; //start machine in idle.

bool canUsePowerButton = true;
bool canUseWaterButton = true;

bool isCycle = false;
int numOfCycle = 0; 
int numOfCycleState = 0; 

int LEDState = 0;

/*
  cal the function for the temp  
*/
int Ro = 50, B = 3950; //Nominal resistance 50k, beta constant
int Rseries = 10; //Series Resistor 10k
float To = 298.15; //Nominal Temperature


byte lastButtonState = LOW;//power button
byte lastButtonStateWater = LOW;

/* 
  constants won't change. They're used here to set pin numbers:
*/
unsigned long debounceDuration = 50; // millis
unsigned long lastTimeButtonStateChanged = 0;
unsigned long debounceDurationWater = 50;
unsigned long lastTimeButtonStateChangedWater = 0;
unsigned long hotWaterTimer; 
unsigned long CycleTimer; 
unsigned long startTime4; 
unsigned long startTime5; 

void setup() { // put your setup code here, to run once:
  Serial.begin(9600);

   // initialize the pin as an output:
  pinMode(inletCoffeePin, OUTPUT);
  pinMode(dispenseHotWaterPin, OUTPUT);
  pinMode(heaterPin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(powerLED, OUTPUT);
  pinMode(stateLED, OUTPUT);

  // initialize the pin as an input:
  pinMode(powerButtonPin, INPUT);
  pinMode(waterButtonPin, INPUT);
  pinMode(coffeButton1Pin, INPUT);
  pinMode(coffeButton2Pin, INPUT);

  CycleTimer = millis(); // rest timer
  startTime4 = millis(); // rest timer
  startTime5 = millis(); // rest timer
}
 
void loop() {
  /*Read analog output of NTC module, i.e the voltage across the thermistor */
  float Vi = analogRead(A0) * (5.0 / 1023.0); // Convert voltage measured to resistance value
  float R = (Vi * Rseries) / (5 - Vi);//All resistance are in kilo ohms.
  float T = 1 / ((1 / To) + ((log(R / Ro)) / B)); //Use R value in steinhart and hart equation Calculate temperature value in kelvin
  float Tc = T - 273.15; // Converting kelvin to celsius
  float Tf = Tc * 9.0 / 5.0 + 32.0; // Converting celsius to Fahrenheit
  // Serial.println((String)"Temperature in celsius    :" + Tc + "°C");
  // Serial.println((String)"Temperature in Fahrenheit  :" + Tf + "°F");

  if(canUsePowerButton){
    if(millis() - lastTimeButtonStateChanged > debounceDuration){
      byte buttonPowerState = digitalRead(powerButtonPin);
      if(buttonPowerState != lastButtonState){
        lastTimeButtonStateChanged = millis(); 
        lastButtonState = buttonPowerState;
        if(buttonPowerState == LOW){
          machineState =! machineState;
          if(machineState) LEDState = 1;
        }      
      }
    }    
  }
  if(machineState)
  {
    if(numOfCycle == 0){
      isCycle = true;
      canUseWaterButton = false;
      LoadFirstCycle();
    }else if(numOfCycle == 1){
      if(canUseWaterButton) HotWaterButton();
      if (digitalRead(coffeButton1Pin) == HIGH) { // check if button is pressed
        if(isCycle == false){
          CycleTimer = millis();
          isCycle = true;
          canUseWaterButton = false;
          numOfCycleState = 0;
        }
      }
      if (digitalRead(coffeButton2Pin) == HIGH) { // check if button is pressed
        if(isCycle == false){
          CycleTimer = millis();
          isCycle = true;
          canUseWaterButton = false;
          numOfCycleState = 1;
        }
      }
      if (isCycle == true) {
        if(numOfCycleState == 0) LoadCycleOne();
        if(numOfCycleState == 1) LoadCycleTwo();
      }
    }
    if (Tc <= heater_on_temperature) heaterElementState = true;
    else if (Tc >= heater_off_temperature) heaterElementState = false;
    if(heaterElementState)
      digitalWrite( heaterPin,HIGH);  //Turn ON heater_out
    else
      digitalWrite(heaterPin,LOW);   //Turn ON heater_out
  }else{
    LEDState = 0;
    digitalWrite(dispenseHotWaterPin,LOW);      
    digitalWrite(inletCoffeePin,LOW);     
    digitalWrite(heaterPin,LOW);  
    CycleTimer = millis();
    startTime5 = millis(); 
    canUseWaterButton = true;
    heaterElementState = true;
    isCycle = false;
  }
 powerLight();
}

void LoadFirstCycle(){
   /*
    turn on coffee for about 5 min
    after the 5 min had pass turn off coffee
    then for 0.6 min turn on hot water 
    then after 0.4 min hot back up water.     
  */
  bool isWaterOff = true;
  if(isWaterOff){
    if (millis() - CycleTimer >= 300000){  // Wait 5 min to heat water...
      Serial.println((String)"Hot water done heat up now");
      LEDState = 1;
      canUsePowerButton = true;
      isCycle = false;
      canUseWaterButton = true;
      numOfCycle = 1;
      isWaterOff = false; 
      CycleTimer = millis();
    }else if (millis() - CycleTimer >= 40000){ // Stop water after 40 sec...
      Serial.println((String)" heat up water");
      digitalWrite(dispenseHotWaterPin,LOW);
      LEDState = 0;
    }else if (millis() - CycleTimer >= 0){  // run for 0 sec
      Serial.println((String)"First load cycle start hot water");
      canUsePowerButton = false;
      digitalWrite(dispenseHotWaterPin,HIGH);
    }
    Serial.println((String)(millis() - CycleTimer));
  }
}

void LoadCycleOne(){
  /*
    turn on coffee for about 5 min
    after the 5 min had pass turn off coffee
    then for 0.6 min turn on hot water 
    then after 0.4 min hot back up water.     
  */
  bool isWaterOff = true;
  LEDState = 0;
  if(isWaterOff){
    if (millis() - CycleTimer >= 420000){// wait after 7 min for user to do any thing else...
      Serial.println((String)" cycle 4       One");
      LEDState = 1;
      canUsePowerButton = true;
      isCycle = false;
      canUseWaterButton = true;
      numOfCycle = 1;   
      isWaterOff = false;  
      LEDState = 2;
      CycleTimer = millis();
    } else if (millis() - CycleTimer >= 340000){// Stop after 5.6 min...
      Serial.println((String)" cycle 3       One");
      digitalWrite(dispenseHotWaterPin,LOW);
      digitalWrite(inletCoffeePin, LOW);
    } else if (millis() - CycleTimer >= 300000){// turn Off coffee after 5 min and then turn on intake hot water...
      Serial.println((String)" cycle 2       One");
      digitalWrite(inletCoffeePin, LOW); // turn off LED
      digitalWrite(dispenseHotWaterPin,HIGH);
    }  else if (millis() - CycleTimer >= 0){// run from 0 sec turn on coffee...
      Serial.println((String)" cycle 1       One");
      canUsePowerButton = false;
      digitalWrite(inletCoffeePin, HIGH); // turn off LED
    }
    Serial.println((String)(millis() - CycleTimer));
  }    
}
void LoadCycleTwo(){
  //Load Hot Water.........
  bool isWaterOff = true;
  LEDState = 0;
  if(isWaterOff){
    if (millis() - CycleTimer >= 420000){// wait after 7 min for user to do any thing else...
      Serial.println((String)" cycle 4       two");
      LEDState = 1;
      canUsePowerButton = true;
      isCycle = false;
      canUseWaterButton = true;
      numOfCycle = 1;   
      isWaterOff = false;  
      LEDState = 2;
      CycleTimer = millis();
    }else if (millis() - CycleTimer >= 340000){// Stop after 5.6 min...
      Serial.println((String)" cycle 3       two");
      digitalWrite(dispenseHotWaterPin,LOW);
    } else if (millis() - CycleTimer >= 300000){// turn Off coffee after 5 min and then turn on intake hot water...
      Serial.println((String)" cycle 2       two");
      digitalWrite(inletCoffeePin, LOW);
      digitalWrite(dispenseHotWaterPin,HIGH);
    }  else if (millis() - CycleTimer >= 0){// run from 0 sec turn on coffee...
      Serial.println((String)"                     cycle 1       two");
      canUsePowerButton = false;
      digitalWrite(inletCoffeePin, HIGH);
    }
    Serial.println((String)(millis() - CycleTimer));
  }   
}
void HotWaterButton(){
  if (digitalRead(waterButtonPin) == HIGH) {
     Serial.println((String)"waterButtonPin");
     hotWaterTimer = millis();
     dispenseState = !dispenseState;
  }
  if(dispenseState){
    if (millis() - hotWaterTimer >= 40000 ) {// check if 1 min has passed
        digitalWrite(dispenseHotWaterPin, LOW);
      Serial.println((String)"it done");
        dispenseState = false;

    }else if (millis() - hotWaterTimer >= 0){ 
      Serial.println((String)"User had prass hot water button");
      digitalWrite(dispenseHotWaterPin,HIGH);
    }
  }
}
void powerLight(){
  if(LEDState == 2){
    if (millis() - startTime4 >= 180000 ){
      startTime5 = millis();
      startTime4 = millis();
      LEDState = 1;     
    } else if (millis() - startTime4 >= 0 ){
      if (millis() - startTime5 >= 3000 ) {
        digitalWrite(powerLED,LOW);     
        digitalWrite(stateLED,LOW);
        startTime5 = millis(); 
      } else if (millis() - startTime5 >= 2000 ) {
        digitalWrite(powerLED,HIGH);     
        digitalWrite(stateLED,HIGH);  
      } else if (millis() - startTime5 >= 1000 ) {
        digitalWrite(powerLED,LOW);     
        digitalWrite(stateLED,LOW);  
      }else if (millis() - startTime5 >= 0 ){
        digitalWrite(powerLED,HIGH);     
        digitalWrite(stateLED,HIGH);  
      }
    }
  }
  if(LEDState == 1){
    digitalWrite(powerLED,HIGH);     
    digitalWrite(stateLED,HIGH);  
  } 
  if(LEDState == 0){
    digitalWrite(powerLED,LOW);     
    digitalWrite(stateLED,LOW);  
  } 
}