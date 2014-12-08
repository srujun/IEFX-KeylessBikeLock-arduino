//"services.h/spi.h/boards.h" is needed in every new project
#include <SPI.h>
#include <boards.h>
#include <RBL_nRF8001.h>
#include <Servo.h>
#include <EEPROM.h>
 
#define BUTTON_PIN   5
#define SERVO_PIN    8

int SERVO_CLOSE_ANGLE = 0;
int SERVO_OPEN_ANGLE = 90;

int BUTTON_PRESSED = 0;
int BUTTON_RELEASED = 1;

//-------------------------------

int EEPROM_LOCK_ADDR = 1;
byte EEPROM_LOCK_OPEN = 10;
byte EEPROM_LOCK_CLOSE = 20;

int EEPROM_BUTTON_ADDR = 2;
int EEPROM_BUTTON_PRESSED = 10;
int EEPROM_BUTTON_RELEASED = 20;

Servo servo;

void setup()
{
  // Default pins set to 9 and 8 for REQN and RDYN
  // Set your REQN and RDYN here before ble_begin() if you need
  //ble_set_pins(3, 2);
  
  // Set your BLE Shield name here, max. length 10
  ble_set_name("iEFX Lock");
  
  // Init. and start BLE library.
  ble_begin();
  
  // Enable serial debug
  Serial.begin(9600);
  
  // Set Button pin to input
  pinMode(BUTTON_PIN, INPUT);
  
  // Default to internally pull high, change it if you need
  digitalWrite(BUTTON_PIN, HIGH);
  
  // Attach the servo
  servo.attach(SERVO_PIN);
  
  // Reset EEPROM values
  if(EEPROM.read(EEPROM_LOCK_ADDR) == 255) {
    EEPROM.write(EEPROM_LOCK_ADDR, EEPROM_LOCK_CLOSE);
  }
  if(EEPROM.read(EEPROM_BUTTON_ADDR) == 255) {
    EEPROM.write(EEPROM_BUTTON_ADDR, EEPROM_BUTTON_RELEASED);
  }
}

void loop()
{
  // If data is ready
  while(ble_available())
  {
    Serial.println("BLE Available!");
    // read data
    byte data = ble_read();
    
    if (data == 0x01)  // Command is to control Servo pin
    {
      Serial.println("Got instruction!");
      actuateLock();
    }
  }
  
  // Button operation code
  if(ble_connected()) {
    int buttonIn = digitalRead(BUTTON_PIN);
    byte buttonMem = EEPROM.read(EEPROM_BUTTON_ADDR);
    
    Serial.print("Button input: ");
    Serial.print(buttonIn);
    Serial.print(", Button memory: ");
    Serial.println(buttonMem);
    
    if(buttonIn == BUTTON_PRESSED && buttonMem == EEPROM_BUTTON_RELEASED) {
      Serial.println("Button pressed!");
      // Store in EEPROM that button has been pressed
      EEPROM.write(EEPROM_BUTTON_ADDR, EEPROM_BUTTON_PRESSED);
      actuateLock();
    }
    if(buttonIn == BUTTON_RELEASED) {
      // Store in EEPROM that button has been released
      byte temp = EEPROM.read(EEPROM_BUTTON_ADDR);
      if(temp == EEPROM_BUTTON_PRESSED) {
        Serial.println("Button released!");
        EEPROM.write(EEPROM_BUTTON_ADDR, EEPROM_BUTTON_RELEASED);
      }
    }
  } else { // We have lost connection, lock the lock!
    Serial.println("No connection!");
    servo.write(SERVO_CLOSE_ANGLE);
    EEPROM.write(EEPROM_LOCK_ADDR, EEPROM_LOCK_CLOSE);
  }
  
  // Allow BLE Shield to send/receive data
  ble_do_events();
  
  delay(50);
}

void actuateLock() {
  // Get the stored lock orientation
  byte lockMem = EEPROM.read(EEPROM_LOCK_ADDR);
  
  // If lock is currently closed
  if(lockMem == EEPROM_LOCK_CLOSE) {
    servo.write(SERVO_OPEN_ANGLE);
    EEPROM.write(EEPROM_LOCK_ADDR, EEPROM_LOCK_OPEN);
    Serial.println("Got close instruction.");
  }
  // If lock is currently open
  if(lockMem == EEPROM_LOCK_OPEN) {
    servo.write(SERVO_CLOSE_ANGLE);
    EEPROM.write(EEPROM_LOCK_ADDR, EEPROM_LOCK_CLOSE);
    Serial.println("Got open instruction.");
  }
}



