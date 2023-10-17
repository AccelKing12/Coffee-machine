// constants won't change. They're used here to set pin numbers:
const int buttonPowerPin = 2;  // the 4th button on the coffee machine "Control power"
const int buttonWaterPin = 3; // is working
const int button10Pin = 10; // 
const int button09Pin = 9; // 
const int inletPin = 6;  // the hot water coffee
const int dispensePin = 4; // The coffee inlet
const int heaterPin = 7; // the number of the LED pin
const int heater_on_temperature  = 73; // the number of the LED pin
const int heater_off_temperature  = 79; // the number of the LED pin
const int heater_Wait_Time = 10;//10min
const int powerLED = 11;//10min
const int stateLED = 12;//10min
/*
  7 heater
  6 hot watere coffe
  4
*/
// variables will change:
bool inletState = false;
bool dispenseState = false;
bool heaterState = true;
bool isMachineState = false;
bool canUsePowerButton = true;
bool canUseWaterButton = true;
bool cycle = false;
int numOfCycle = 0; 
int numOfCycleState = 0; 


int Ro = 50, B = 3950; //Nominal resistance 50k, beta constant
int Rseries = 10; //Series Resistor 10k
float To = 298.15; //Nominal Temperature

//power
byte lastButtonState = LOW;
byte lastButtonStateWater = LOW;

unsigned long debounceDuration = 80; // millis
unsigned long lastTimeButtonStateChanged = 0;
unsigned long debounceDurationWater = 50;
unsigned long lastTimeButtonStateChangedWater = 0;

//
unsigned long startTime1; 
unsigned long startTime2; 
unsigned long startTime3; 

unsigned long startTime4; 
unsigned long startTime5; 
unsigned long gpsRunInterval = 1000UL * 60 * 1; // 1 minute interval
bool gpsDownloadInitial = false;

char input;

void setup() {
   // put your setup code here, to run once:
  Serial.begin(9600);
   // initialize the LED pin as an output:
  pinMode(inletPin, OUTPUT);
  pinMode(dispensePin, OUTPUT);
  pinMode(heaterPin, OUTPUT);
    // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(powerLED, OUTPUT);
  pinMode(stateLED, OUTPUT);

  // initialize the pushbutton pin as an input:
  pinMode(buttonPowerPin, INPUT);
  pinMode(buttonWaterPin, INPUT);
  pinMode(button10Pin, INPUT);
  pinMode(button09Pin, INPUT);
  startTime4 = millis(); // fill water
}
 
void loop() {
  testMachine();
  /*Read analog output of NTC module, i.e the voltage across the thermistor */
  float Vi = analogRead(A0) * (5.0 / 1023.0); // Convert voltage measured to resistance value
//  Serial.print(Vi);
  //All resistance are in kilo ohms.
  float R = (Vi * Rseries) / (5 - Vi);
 // Serial.print(R);
 // Serial.println(":");
 // Serial.println(":");
  /*Use R value in steinhart and hart equation Calculate temperature value in kelvin*/
  float T = 1 / ((1 / To) + ((log(R / Ro)) / B));
  float Tc = T - 273.15; // Converting kelvin to celsius
  float Tf = Tc * 9.0 / 5.0 + 32.0; // Converting celsius to Fahrenheit

  //test code
  // Serial.println((String)"Temperature in celsius    :" + Tc + "°C");
 // Serial.println((String)"Temperature in Fahrenheit  :" + Tf + "°F");

  if(canUsePowerButton){
    if(millis() - lastTimeButtonStateChanged > debounceDuration){
      byte buttonPowerState = digitalRead(buttonPowerPin);
      if(buttonPowerState != lastButtonState){
        lastTimeButtonStateChanged = millis(); 
        lastButtonState = buttonPowerState;
        if(buttonPowerState == LOW){
          // do an action, for example print on Serial
          Serial.println("Button released");
          isMachineState =! isMachineState;
        }      
      }
    }    
  }
  
//isMachineState
  if(true )
  {
    
    Serial.println("  "); 
    //Serial.println("  "); 
   // Serial.println("Machine is on "); 
    digitalWrite(powerLED,HIGH);     
    digitalWrite(stateLED,HIGH);     
/*User can turn waterb inlet...*/
    // read the state of the pushbutton value:
    /*
    The above code is a basic example of using the Arduino's millis()
    function to create a timer that starts when a button is pressed and 
    turns on an LED for one second before turning it off again. The millis() 
    function returns the number of milliseconds since the Arduino board began 
    running the current program. By subtracting the startTime1 variable from the 
    current time in millis(), the code can determine how much time has passed since 
    the button was pressed.
    */ 
    if(numOfCycle == 0){
      cycle = true;
      canUseWaterButton = false;
      LoadFirstCycle();
    }else if(numOfCycle == 1){
      if(canUseWaterButton)
        HotWaterButton();

      if (digitalRead(button10Pin) == HIGH || input == '3') { // check if button is pressed
      // 2.5min
        if(cycle == false){
          cycle = true;
          canUseWaterButton = false;
          numOfCycleState = 0;
        }
      }
      if (digitalRead(button09Pin) == HIGH || input == '2') { // check if button is pressed
        if(cycle == false){
          cycle = true;
          canUseWaterButton = false;
          numOfCycleState = 1;
        }
      }

      //digitalWrite(dispensePin,digitalRead(button09Pin));

      // put your main code here, to run repeatedly:
      
      // downloadGPS();

      if (cycle == true) { 
        if(numOfCycleState == 0)        
          LoadCycleOne();
        if(numOfCycleState == 1)
          LoadCycleTwo();
      }
    }

    if (Tc <= heater_on_temperature){ //8°C to  73°C
      //Turn ON heater_out
      heaterState = true;
    }else if (Tc >= heater_off_temperature){ // if temp go over 79°C  shut off......
      //Turn ON heater_out
      heaterState = false;
    }
    if(heaterState){
      //test code
      //Serial.println((String)"heater state on");
      digitalWrite( heaterPin,HIGH);  //Turn ON heater_out
    }else{
      //Serial.println((String)"heater state off");
      digitalWrite(heaterPin,LOW);   //Turn ON heater_out
      //test code
    }
  }else{
    //Serial.println("  "); 
    //Serial.println("  "); 
    //Serial.println("Machine is OFF ");
    digitalWrite(dispensePin,LOW);     
    digitalWrite(powerLED,LOW);    
    digitalWrite(stateLED,LOW);     

    digitalWrite(inletPin,LOW);     
    digitalWrite(heaterPin,LOW);  
    startTime2 = millis();
    canUseWaterButton = true;
    heaterState = true;
    cycle = false;
  }
 //delay(1000);
  
}

void downloadGPS() {
 if (millis() - startTime4 >= 30000){ 
      Serial.println((String)"Water Start");
      if(millis() - startTime4 >= 40000){
        Serial.println((String)"Water Stop");
      digitalWrite(dispensePin,LOW);
        startTime4 = millis();
      }
    } 
      Serial.println((String)(millis() - startTime4));
 
  if(gpsDownloadInitial){
  }else {
     // Serial.println((String)"start water from being filled");
  }   
}

void LoadFirstCycle(){
  //startTime2
  //startTime5
  //Load Hot Water.........
  bool isWaterOff = true;
  if(isWaterOff){
    if (millis() - startTime2 >= 300000){  // Wait 5 min to heat water...
      Serial.println((String)"Hot water done heat up now");
      canUsePowerButton = true;
      cycle = false;
      canUseWaterButton = true;
      numOfCycle = 1;
      isWaterOff = false;  
      startTime2 = millis();
    }else if (millis() - startTime2 >= 40000){ // Stop water after 40 sec...
      Serial.println((String)" Hot water start");
      digitalWrite(dispensePin,LOW);
    }else if (millis() - startTime2 >= 0){  // run for 0 sec
      Serial.println((String)"First load cycle start hot water");
      canUsePowerButton = false;
      digitalWrite(dispensePin,HIGH);
    }
    Serial.println((String)(millis() - startTime2));
  }
}

void LoadCycleOne(){
  //startTime2
  //startTime5
  //Load Hot Water.........
  bool isWaterOff = true;
  if(isWaterOff){
    if (millis() - startTime2 >= 420000){// wait after 7 min for user to do any thing else...
      Serial.println((String)"One cycle 4");
      canUsePowerButton = true;
      cycle = false;
      canUseWaterButton = true;
      numOfCycle = 1;   
      isWaterOff = false;  
      startTime2 = millis();
    }else if (millis() - startTime2 >= 340000){// Stop after 5.6 min...
      Serial.println((String)"One cycle 4");
      digitalWrite(dispensePin,LOW);
    } else if (millis() - startTime2 >= 300000){// turn Off coffee after 5 min and then turn on intake hot water...
      Serial.println((String)"One cycle 3");
      digitalWrite(inletPin, LOW); // turn off LED
      digitalWrite(dispensePin,HIGH);
    }  else if (millis() - startTime2 >= 0){// run from 0 sec turn on coffee...
      Serial.println((String)"One cycle 1");
      canUsePowerButton = false;
      digitalWrite(inletPin, HIGH); // turn off LED
    }
    Serial.println((String)(millis() - startTime2));
  }    
}
void LoadCycleTwo(){
  //startTime2
  //startTime5
  //Load Hot Water.........
  bool isWaterOff = true;
  if(isWaterOff){
    if (millis() - startTime2 >= 420000){// wait after 7 min for user to do any thing else...
      Serial.println((String)"One cycle 4");
      canUsePowerButton = true;
      cycle = false;
      canUseWaterButton = true;
      numOfCycle = 1;   
      isWaterOff = false;  
      startTime2 = millis();
    }else if (millis() - startTime2 >= 340000){// Stop after 5.6 min...
      Serial.println((String)"One cycle 4");
      digitalWrite(dispensePin,LOW);
    } else if (millis() - startTime2 >= 300000){// turn Off coffee after 5 min and then turn on intake hot water...
      Serial.println((String)"One cycle 3");
      digitalWrite(inletPin, LOW); // turn off LED
      digitalWrite(dispensePin,HIGH);
    }  else if (millis() - startTime2 >= 0){// run from 0 sec turn on coffee...
      Serial.println((String)"One cycle 1");
      canUsePowerButton = false;
      digitalWrite(inletPin, HIGH); // turn off LED
    }
    Serial.println((String)(millis() - startTime2));
  }   
}
void HotWaterButton(){
  if(millis() - lastTimeButtonStateChangedWater > debounceDurationWater){
    byte buttonPowerState = digitalRead(buttonWaterPin);
    if(buttonPowerState != lastButtonStateWater){
      lastTimeButtonStateChangedWater = millis(); 
      lastButtonStateWater = buttonPowerState;
      if(buttonPowerState == LOW){
        // do an action, for example print on Serial
        Serial.println("Button released");
        canUsePowerButton = true;
        if(!dispenseState){
          startTime1 = millis(); // start the timer
        }
        dispenseState =! dispenseState;
      }      
    }
  }

  if(dispenseState){
    if (millis() - startTime1 >= 60000 ) {// check if 1 min has passed
        digitalWrite(dispensePin, LOW); // turn off LED
        dispenseState = false;
    }else if (millis() - startTime1 >= 0){  // run for 0 sec
      Serial.println((String)"User had prass hot water button");
      digitalWrite(dispensePin,HIGH);
      cycle = true;
    }
  }else{
    cycle = false;
    startTime1 = millis(); // start the timer
  }
}
void testMachine(){

  if(Serial.available()){
    input = Serial.read();
    Serial.print("You typed: " );
    Serial.println(input);
    input = '0';
  }
}