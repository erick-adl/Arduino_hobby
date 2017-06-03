#include <Wire.h>
#include <ACROBOTIC_SSD1306.h>

unsigned char brightness = 255;
#define LED1 5
#define LED2 18
#define LED3 19

const int analogInPin = 35;
int sensorValue = 0;
int outputValue = 0;
char buf [20];


void setup()
{
  Wire.begin();
  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  oled.init();
  oled.clearDisplay();
  oled.setBrightness((unsigned char)brightness);

}

void loop()
{

  // read the analog in value:
  sensorValue = analogRead(analogInPin);
  outputValue = map(sensorValue, 0, 2048, 0, 100);
  sprintf(buf, "LUX: %d%%   ", outputValue);

  if (outputValue < 10) {
    digitalWrite(LED1, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED2, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED3, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else if (outputValue >= 10 && outputValue <= 20) {
    digitalWrite(LED1, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED2, LOW);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED3, LOW);   // turn the LED on (HIGH is the voltage level)
  } else if (outputValue > 20 && outputValue <= 40) {
    digitalWrite(LED1, LOW);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED2, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED3, LOW);   // turn the LED on (HIGH is the voltage level)
  }else if (outputValue > 40 && outputValue <= 60) {
    digitalWrite(LED1, LOW);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED2, LOW);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED3, HIGH);   // turn the LED on (HIGH is the voltage level)
  } else if(outputValue > 60) {
    digitalWrite(LED1, LOW);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED2, LOW);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(LED3, LOW);   // turn the LED on (HIGH is the voltage level)
  }
  oled.setTextXY(2, 3);             // Set cursor position, start of line 0
  oled.putString(buf);
  delay(100);



}

