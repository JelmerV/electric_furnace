#include "Arduino.h"
#include "max6675.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBold24pt7b.h> // Add a custom font
#include <Fonts/FreeMonoBold12pt7b.h>

// io
#define DOOR_INPUT 2   // Door switches input
#define COIL_OUTPUT 13 // Coil relay control
#define PIN_ENCODER_A 8
#define PIN_ENCODER_B 9

// screen config
#define SCREEN_ADRESS 0x3C // I2C address
#define SCREEN_WIDTH 128   // OLED display width, in pixels
#define SCREEN_HEIGHT 64   // OLED display height, in pixels
#define OLED_RESET -1      // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// thermometer config
int thermoDO = 12;
int thermoCS = 9;
int thermoCLK = 10;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// 'flame_icon', 20x34px
const unsigned char flame_icon[] PROGMEM = {
    0x03, 0x03, 0x00, 0x03, 0x03, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x3c, 0x00, 0x30, 0xf0, 0x00, 0x30,
    0xf0, 0x00, 0x0c, 0x30, 0x00, 0x0c, 0x30, 0x00, 0x0c, 0xfc, 0x30, 0x0c, 0xfc, 0x30, 0x0c, 0xfc,
    0xf0, 0x0c, 0xfc, 0xf0, 0x0f, 0xff, 0xc0, 0x0f, 0xff, 0xc0, 0xcf, 0xcf, 0xc0, 0xcf, 0xcf, 0xc0,
    0xcf, 0x3f, 0xc0, 0xcf, 0x3f, 0xc0, 0xcf, 0x33, 0xf0, 0xef, 0x33, 0xf0, 0xff, 0x3c, 0xf0, 0xff,
    0x3c, 0xf0, 0xfc, 0x3c, 0xf0, 0xfc, 0x3c, 0xf0, 0xf8, 0x0c, 0xf0, 0xf0, 0x0c, 0xf0, 0xf0, 0x00,
    0xf0, 0xf8, 0x01, 0xf0, 0xfc, 0xc3, 0xf0, 0xfc, 0xc3, 0xf0, 0x3f, 0xff, 0xc0, 0x3f, 0xff, 0xc0,
    0x0f, 0xff, 0x00, 0x0f, 0xff, 0x00};

// 'door_icon', 27x42px
const unsigned char door_icon[] PROGMEM = {
    0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xe0, 0xf0, 0x00, 0x00, 0x60, 0xfc, 0x00, 0x00, 0x60,
    0xff, 0x00, 0x00, 0x60, 0xff, 0xc0, 0x00, 0x60, 0xff, 0xf8, 0x00, 0x60, 0xff, 0xfe, 0x00, 0x60,
    0xff, 0xff, 0x80, 0x60, 0xff, 0xff, 0xe0, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60,
    0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60,
    0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60,
    0xff, 0xff, 0x98, 0x60, 0xff, 0xff, 0x98, 0x60, 0xff, 0xff, 0xc8, 0x60, 0xff, 0xff, 0xc8, 0x60,
    0xff, 0xff, 0xc8, 0x60, 0xff, 0xff, 0xc8, 0x60, 0xff, 0xff, 0x98, 0x60, 0xff, 0xff, 0x98, 0x60,
    0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60,
    0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0x3f, 0xff, 0xf8, 0x60, 0x0f, 0xff, 0xf8, 0x60,
    0x03, 0xff, 0xf8, 0x00, 0x03, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x3f, 0xf8, 0x00,
    0x00, 0x0f, 0xf8, 0x00, 0x00, 0x01, 0xf8, 0x00};

// global variables
float temp_c = 0;
int target_temp = 700;
unsigned long door_closed_millis = (2 ^ 32) - 1;
bool is_safe = false;
bool flash_door = true;

// functions
void door_interrupt()
{                                     // door opened: switch off coil
  if (digitalRead(DOOR_INPUT) == LOW) // door closed: high signal
  {
    is_safe = false;
    digitalWrite(COIL_OUTPUT, LOW);
  }
  else
  {
    door_closed_millis = millis();
  }
}


void pciSetup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

const int EncoderSteps[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
ISR (PCINT0_vect) { // handle pin change interrupt for D8 to D13, interrupt duration 8 micros
  static unsigned long last_time;
  static uint8_t last_state;

   if (micros() - last_time <= 50) { //debounce, min pulse duration should be 120us
     return;
   }
   last_time = micros();

  uint8_t state = ((PINB) & 0b00000011);     // value 0-3 for the 'now value'  but up 2 bits
  uint8_t idx = state | last_state;   // index by orring to the last value so we get one binary value for old and new input state
  target_temp += EncoderSteps[idx]*10;  // add value at index
  last_state = state << 2;          // last value is current value in the lower two bits.
}

void displayTemperature(float temp_c)
{
  char temp_text[5] = "1234";
  dtostrf((int)(temp_c + 0.5), 4, 0, temp_text);

  char set_text_d[5] = "1234";
  char set_text[9] = "Set:";
  dtostrf(target_temp, 3, 0, set_text_d);
  strcat(set_text, set_text_d);

  display.clearDisplay();

  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(30, 16); // (x,y)
  display.print(set_text);   // Text or value to print

  display.setFont(&FreeMonoBold24pt7b);
  display.setCursor(15, 62);
  display.print(temp_text); // Text or value to print

  if (!is_safe)
  {
    if (flash_door)
    {
      static bool last_door = false;
      if (last_door)
      {
        display.drawBitmap(2, 62 - 42, door_icon, 27, 42, 1); // 27x42px
      }
      last_door = !last_door;
    } else{
        display.drawBitmap(2, 62 - 42, door_icon, 27, 42, 1); // 27x42px
    }
  }
  else if (digitalRead(COIL_OUTPUT))
  {
    display.drawBitmap(2, 62 - 34, flame_icon, 20, 34, 1); // 20x34px
  }

  display.display();
}

void setup()
{
  pinMode(DOOR_INPUT, INPUT_PULLUP);
  pinMode(COIL_OUTPUT, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(DOOR_INPUT), door_interrupt, CHANGE);
  pciSetup(PIN_ENCODER_A);
  pciSetup(PIN_ENCODER_B);

  Serial.begin(115200);
  Serial.println("Electric Furnace controller");

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADRESS); // start display with internav 3.3v
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.cp437(true); // Use full 256 char 'Code Page 437' font
}

void loop()
{
  // read new temp
  static unsigned long last_millis = millis();
  if (millis() - last_millis >= 250)
  { // For the MAX6675 minimum 250ms delay
    temp_c = thermocouple.readCelsius();
    last_millis = millis();

    Serial.print("C = ");
    Serial.println(temp_c);
    displayTemperature(temp_c);
  }

  // check is door closed some time ago
  if (digitalRead(DOOR_INPUT) == HIGH)
  {
    if (millis() - door_closed_millis >= 2000)
    {
      // delay after closing door
      is_safe = true;
      flash_door = false;
    }
    else if (!is_safe)
    { // door closed but wait for timer
      flash_door = true;
    }
  } else {
      is_safe = false;
      flash_door = false;
  }

  // control coil, with some hysteresis
  if (temp_c <= (target_temp))
  {
    if (is_safe)
    {
      digitalWrite(COIL_OUTPUT, HIGH);
    }
  }
  else if (temp_c > (target_temp + 20))
  { // too hot
    digitalWrite(COIL_OUTPUT, LOW);
  }
}
