#define BLYNK_PRINT Serial
#include <Stepper.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>


// REPLACE THIS VALUES
char auth[] = "BLYNK AUTH TOKEN";
char ssid[] = "SSID"; 
char pass[] = "*******";

// change this to fit the number of steps per revolution
const int stepsPerRevolution = 200;

Stepper myStepper(stepsPerRevolution, 14, 12, 13, 15);

const int button1 = 5; // D1 . PULLUP
const int button2 = 16; // D0 // This pin doesn't have a pull-Up resistor (add external)

const int limitSwitch1 = 4; // D2 . PULLUP
const int limitSwitch2 = 0; // D3 . PULLUP

const int ledPin = 2; // LED_BUILTIN // D4

int xVal = 512;
int ledState = LOW;

signed long currPos = 0;
signed long maxPos = 1;

// Timer: Auxiliary variables
unsigned long currentMillis = 0;
unsigned long previousMillisLed = 0;

ICACHE_RAM_ATTR void detectsSwitch1() {
  Serial.println("Switch 1 trriggered");
  currPos = 0;
}

ICACHE_RAM_ATTR void detectsSwitch2() {
  Serial.println("Switch 2 trriggered");
  maxPos = currPos;
}

void setup() {

  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);

  // Set LED
  pinMode(ledPin, OUTPUT); // 2
  digitalWrite(ledPin, LOW);

  // Priority Move Buttons
  pinMode(button2, INPUT_PULLUP);
  pinMode(button1, INPUT_PULLUP);

  // Limit Switchesr mode INPUT_PULLUP
  pinMode(limitSwitch1, INPUT_PULLUP);
  pinMode(limitSwitch2, INPUT_PULLUP);

  // Set interrupt pins, assign interrupt function and set RISING / FALLING / CHANGE mode
  attachInterrupt(digitalPinToInterrupt(limitSwitch1), detectsSwitch1, RISING);
  attachInterrupt(digitalPinToInterrupt(limitSwitch2), detectsSwitch2, RISING);

}

void loop() {

  Blynk.run();

  int sw1 = digitalRead(limitSwitch1);
  int sw2 = digitalRead(limitSwitch2);

  int btn1 = digitalRead(button1);
  int btn2 = digitalRead(button2);
  
  // Set the speed based on Joystic value from BLYNK
  int motorSpeed = map(512 - xVal, 0, 512, 0, 100);

  // If hardware buttons are pushed, override to max speed
  if(btn1 == LOW) {
    motorSpeed = 100;
  }
  if(btn2 == LOW) {
    motorSpeed = -100;
  }

  // set the motor speed:
  if (motorSpeed != 0) {
    myStepper.setSpeed(abs(motorSpeed));
    // step 1/100 of a revolution:
    int dir = abs(motorSpeed) / motorSpeed;

    if ((dir > 0 && !sw2) || (dir < 0 && !sw1)) {
      currPos = currPos + dir;
      myStepper.step(dir);
    }    
  }

  currentMillis = millis();
  if (currentMillis - previousMillisLed >= 1000) {
    previousMillisLed = currentMillis;
    
    // Blink the on-board LED to show the code is running
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(ledPin, ledState);

    Serial.println();
    Serial.println("BTN1 = " + String(btn1) + ", BTN2 = " + String(btn2));
    if (maxPos != 0) {
      Serial.print("Current position " + String(currPos) + " (" + String(100 * currPos / maxPos) + "%)");
    }
  }
}

// Value from BLYNK APP Joystick
BLYNK_WRITE(V1) {
  xVal = param[0].asInt();
  // int y = param[1].asInt();
}

// Value from BLYNK APP Button
BLYNK_WRITE(V2) {
  if ( param.asInt() == 1) {
    xVal = 0;
  } else {
    xVal = 512;
  }
}

// Value from BLYNK APP Button
BLYNK_WRITE(V3) {
  if ( param.asInt() == 1) {
    xVal = 1000;
  } else {
    xVal = 512;
  }
}
