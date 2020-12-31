#include <avr/sleep.h>

const int SwitchPin = 3;           // The switch is connect to pin 3
const int piezoPin = 11;           // Buzzer in the pin 11
const int time = 50;               // Time (ms) delay
const long TimeWait = 60000;       // Time (ms) after the door is open to beep the buzzer
const int countBeepCycles = 60;    // Base time for beep duration
const int BeepDuration = 100;      // Base time for beep duration
const int Factor = 5;              // Multiply the BeepDuration by this factor
const int volume = 1;              // from 1 to 255
const int interval = 200;          // interval the LED will blink in ms. When DEBUG_LED = 1
const int ledPin =  LED_BUILTIN;   // the number of the LED pin
volatile bool door_state;          // Door is open (true) or closed (false)
unsigned long previousMillis = 0;  // will store last time LED was updated
int ledState = LOW;                // ledState used to set the LED
int varDelay = BeepDuration * Factor;

#define DEBUG_SERIAL 0 // 1 enable serial debug
#define DEBUG_LED 0 // 1 enable LED debug

void wake() {
    sleep_disable();         // first thing after waking from sleep:
    
    // stop LOW interrupt on "SwitchPin"
    detachInterrupt(digitalPinToInterrupt(SwitchPin)); 
}

void sleepNow() {  //Interrupt subroutine to set arduino in sleep mode
    BipDoorClosed(); //Give a feedback that the door was closed!
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
    noInterrupts();          // make sure we don't get interrupted before we sleep
    sleep_enable();          // enables the sleep bit in the mcucr register
    
    // wake up on LOW level on "SwitchPin" 
    attachInterrupt(digitalPinToInterrupt(SwitchPin), wake, LOW);
    
    interrupts();           // interrupts allowed now, next instruction WILL be executed
    sleep_cpu();            // here the device is put to sleep
    //sleep_mode(); // here the device is actually put to sleep!!
}

void BipDoorClosed() {
    int BipLength = 300; //ms
    int BipGap = 200; //ms
    
    analogWrite(piezoPin, volume);
    delay(BipLength);
    digitalWrite(piezoPin, LOW);
    delay(BipGap);
    analogWrite(piezoPin, volume);
    delay(BipLength);
    digitalWrite(piezoPin, LOW);
}

bool read_door() {
    if(digitalRead(SwitchPin) == HIGH) {
        return false; // Means door is closed
    }
    else {
        return true; // Means door is opened
    }
}

void alarm() {
    // play alarm bip countBeepCycles times
    int count = 0;
    
    while(read_door() && count < countBeepCycles) {
    
    #if DEBUG_SERIAL == 1
      Serial.print("while loop with beep: count: ");
      Serial.println(count);
    #endif
    
        analogWrite(piezoPin, volume);
        delay(varDelay);
        digitalWrite(piezoPin, LOW);
        delay(varDelay);
       
        count++;        
    }
    count = 0;
  
    // If door is still open go to crazy mode 1!
    while(read_door()) {
         soundFX(5.0, 50.0); // star trek - crazy alarm!
    count++;
  }
    door_state = false;
}

void blinkLED() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;

        // if the LED is off turn it on and vice-versa:
        if (ledState == LOW) {
            ledState = HIGH;
        } else {
            ledState = LOW;
        }

        // set the LED with the ledState of the variable:
        digitalWrite(ledPin, ledState); 
    }
}

void soundFX(float amplitude,float period){ 
    // example of sounds:
    // soundFX(3000.0,30); // sonic screwdriver
    //soundFX(100.0,30.0); // ray gun
    //soundFX(3.0,30.0); // star trek
    //soundFX(1.0,30.0); // star trek high
    
    int uDelay = 2 + amplitude + amplitude*sin(millis()/period);
    
    for(int i=0; i<5; i++) {
        digitalWrite(piezoPin, HIGH);
        delayMicroseconds(uDelay);
        digitalWrite(piezoPin, LOW);
        delayMicroseconds(uDelay);
    }
}

void setup() {
    pinMode(SwitchPin, INPUT);
    pinMode(piezoPin, OUTPUT);

    #if DEBUG_SERIAL == 1
        Serial.begin(9600);
        while (!Serial); // while the serial stream is not open, do nothing:
        Serial.println("Initialisation complete.");   
    #endif

    #if DEBUG_LED == 1
        // initialize digital pin LED_BUILTIN as an output.
        pinMode(ledPin, OUTPUT);
    #endif
}

void loop() {
    if(read_door()) { //In case the door is opened this statement will be true
        door_state = true; // the door is open
    
        #if DEBUG_SERIAL == 1
            Serial.print("door_state: ");
            Serial.println(door_state);
        #endif
        
        unsigned long start = millis(); 
        
        while((millis() - start) <= TimeWait) {

            #if DEBUG_SERIAL == 1
                Serial.print("while loop time with door open (ms): ");
                Serial.println(millis() - start);
                Serial.print("Door State: ");
                Serial.println(door_state);
            #endif
            
            #if DEBUG_LED == 1
                blinkLED(); // Blink the built-in LED according to constant "interval"
            #endif
        
            if(millis() - start < 0) break; //break while loop if rollover occures
    
            if(!read_door()) { // If door is closed the program does nothing and starts again
                door_state = false;
                break;
            }
            delay(time); //delay needed
        }
        
        if(door_state) alarm(); // If door is still opened start the beep sound!
    }
    
    #if DEBUG_SERIAL == 1
        Serial.println("Entering in Sleep mode...");
        Serial.end();
    #endif
    
    sleepNow(); // Put the system to sleep if the door is closed
}
