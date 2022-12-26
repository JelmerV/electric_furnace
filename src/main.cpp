#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>  // Add a custom font
#include <SPI.h>
#include <Wire.h>

#include "Arduino.h"
#include "max6675.h"

// io
#define PIN_DOOR_IN 4    // Door switches input
#define PIN_COIL_OUT 13  // Coil relay control
#define PIN_ENCODER_A 2  // encoder function only works for port C, pins 0-7
#define PIN_ENCODER_B 3
#define ENCODER_PIN_MASK (1 << PIN_ENCODER_A | 1 << PIN_ENCODER_B)

// screen config
#define SCREEN_ADRESS 0x3C  // I2C address (pins A4, A5)
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// thermometer config
int thermoDO = 12;
int thermoCS = 9;
int thermoCLK = 10;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// 'flame_icon', 29x43px
const unsigned char flame_icon[] PROGMEM = {
    0x00, 0x70, 0x0e, 0x00, 0x00, 0x70, 0x0e, 0x00, 0x00, 0x60, 0x0e, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x03, 0xf0, 0x00, 0x1c, 0x0f, 0xc0, 0x00, 0x1c,
    0x0f, 0xc0, 0x00, 0x1e, 0x0f, 0xc0, 0x00, 0x03, 0x83, 0xc0, 0x00, 0x03, 0x83, 0xc0, 0x00, 0x03, 0x8f, 0xf0, 0x38, 0x03, 0x8f, 0xf0, 0x38, 0x03, 0x8f,
    0xf0, 0xf8, 0x03, 0x8f, 0xf1, 0xf8, 0x03, 0x8f, 0xf1, 0xf8, 0x03, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xc0, 0xe3, 0xfe, 0x7f,
    0xc0, 0xe3, 0xfe, 0x7f, 0xc0, 0xe3, 0xf3, 0xff, 0xc0, 0xe3, 0xf3, 0xff, 0xc0, 0xe3, 0xf3, 0xff, 0xf8, 0xe3, 0xf3, 0xcf, 0xf8, 0xf3, 0xf3, 0xcf, 0xf8,
    0xff, 0xf3, 0xf9, 0xf8, 0xff, 0xf3, 0xf9, 0xf8, 0xff, 0xf3, 0xf9, 0xf8, 0xff, 0x83, 0xf9, 0xf8, 0xff, 0x83, 0xf9, 0xf8, 0xfe, 0x00, 0x79, 0xf8, 0xfe,
    0x00, 0x79, 0xf8, 0xfe, 0x00, 0x79, 0xf8, 0xfe, 0x00, 0x01, 0xf8, 0xfe, 0x00, 0x01, 0xf8, 0xff, 0x8e, 0x0f, 0xf8, 0xff, 0x9e, 0x0f, 0xf8, 0xff, 0xff,
    0xff, 0xf8, 0x1f, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xc0, 0x1f, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0x00, 0x03, 0xff, 0xfe, 0x00};

// 'door_icon', 27x42px
const unsigned char door_icon[] PROGMEM = {
    0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xe0, 0xf0, 0x00, 0x00, 0x60, 0xfc, 0x00, 0x00, 0x60, 0xff, 0x00, 0x00, 0x60, 0xff, 0xc0, 0x00, 0x60,
    0xff, 0xf8, 0x00, 0x60, 0xff, 0xfe, 0x00, 0x60, 0xff, 0xff, 0x80, 0x60, 0xff, 0xff, 0xe0, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60,
    0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60,
    0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0x98, 0x60, 0xff, 0xff, 0x98, 0x60, 0xff, 0xff, 0xc8, 0x60, 0xff, 0xff, 0xc8, 0x60,
    0xff, 0xff, 0xc8, 0x60, 0xff, 0xff, 0xc8, 0x60, 0xff, 0xff, 0x98, 0x60, 0xff, 0xff, 0x98, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60,
    0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0xff, 0xff, 0xf8, 0x60, 0x3f, 0xff, 0xf8, 0x60, 0x0f, 0xff, 0xf8, 0x60,
    0x03, 0xff, 0xf8, 0x00, 0x03, 0xff, 0xf8, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x0f, 0xf8, 0x00, 0x00, 0x01, 0xf8, 0x00};

// global variables
float temp_c = 0;
volatile int target_temp = 700;
unsigned long door_closed_millis = (2 ^ 32) - 1;
bool is_safe = false;
bool flash_door = true;

// functions
void doorInterrupt() {                      // door opened: switch off coil
    if (digitalRead(PIN_DOOR_IN) == LOW) {  // door closed: high signal
        is_safe = false;
        digitalWrite(PIN_COIL_OUT, LOW);
    } else {
        door_closed_millis = millis();
    }
}

const int encoderSteps[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
void encoderInterrupt() {
    static uint8_t last_state;
    uint8_t state = ((PIND)&ENCODER_PIN_MASK) >> min(PIN_ENCODER_A, PIN_ENCODER_B);

    uint8_t idx = state | (last_state << 2);  // combine 2 states into single int
    target_temp += encoderSteps[idx] * 5;     // add value at index
    last_state = state;                       // last state is current state in the lower two bits.
}

void displayTemperature(float temp_c) {
    char temp_text[5] = "1234";
    dtostrf((int)(temp_c + 0.5), 4, 0, temp_text);

    char set_text_d[5] = "1234";
    char set_text[9] = "Set:";
    dtostrf(target_temp, 3, 0, set_text_d);
    strcat(set_text, set_text_d);

    display.clearDisplay();

    display.setFont(&FreeMonoBold12pt7b);
    display.setCursor(30, 16);  // (x,y)
    display.print(set_text);    // Text or value to print

    display.setFont(&FreeMonoBold24pt7b);
    display.setCursor(15, 62);
    display.print(temp_text);  // Text or value to print

    if (!is_safe) {
        if (flash_door) {
            static bool last_door = false;
            if (last_door) {
                display.drawBitmap(2, 62 - 42, door_icon, 27, 42, 1);  // 27x42px
            }
            last_door = !last_door;
        } else {
            display.drawBitmap(2, 62 - 42, door_icon, 27, 42, 1);  // 27x42px
        }
    } else if (digitalRead(PIN_COIL_OUT)) {
        display.drawBitmap(2, 63 - 43, flame_icon, 29, 43, 1);  // 29x43
    }

    display.display();
}

void setup() {
    pinMode(PIN_DOOR_IN, INPUT_PULLUP);
    pinMode(PIN_COIL_OUT, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_A), encoderInterrupt, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_B), encoderInterrupt, CHANGE);

    Serial.begin(115200);
    Serial.println("Electric Furnace controller");

    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADRESS);  // start display
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.cp437(true);  // Use full 256 char 'Code Page 437' font
}

void loop() {
    // read new temp
    static unsigned long last_millis = millis();
    if (millis() - last_millis >= 250) {  // For the MAX6675 minimum 250ms delay
        temp_c = thermocouple.readCelsius();
        last_millis = millis();

        Serial.print("C = ");
        Serial.println(temp_c);
        displayTemperature(temp_c);
    }

    // check if door state changed
    static bool last_door_state;
    bool door_state = digitalRead(PIN_DOOR_IN);
    if (door_state != last_door_state) {
        if (door_state == LOW) {  // door closed: high signal
            is_safe = false;
            digitalWrite(PIN_COIL_OUT, LOW);
        } else {
            door_closed_millis = millis();
        }
        last_door_state = door_state;
    }

    // check is door closed some time ago
    if (door_state == HIGH) {
        if (millis() - door_closed_millis >= 2000) {
            // delay after closing door
            is_safe = true;
            flash_door = false;
        } else if (!is_safe) {  // door closed but wait for timer
            flash_door = true;
        }
    } else {
        is_safe = false;
        flash_door = false;
    }

    // control coil, with some hysteresis
    if (temp_c <= (target_temp)) {
        if (is_safe) {
            digitalWrite(PIN_COIL_OUT, HIGH);
        }
    } else if (temp_c > (target_temp + 20)) {  // too hot
        digitalWrite(PIN_COIL_OUT, LOW);
    }
}
