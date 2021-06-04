/*
 * FLASH ROM Programmer
 * Supported and tested with Intel 28F256 flash memory device.
 * May work with other devices too.
 * 
 * List of similar devices:
 * 28F256
 * 28F512
 * 28F010
 * 28F020
 * 28F040
 * 
 * List of manufacturers:
 * Intel, Atmel, AMD, SGS, TI, Toshiba
 * 
 * Copyright (C) 2021, Juha-Pekka Varjonen
 */

#define TOTAL_BYTES 32768
#define BUFFER_LENGTH 16
#define ADDR 2
#define SCLK 3
#define LCLK 4
#define PROG A0
#define WE A1
#define OE A2

bool programmingMode = false;
unsigned int writeAddress = 0;

// LED related variables
unsigned long ledBlink = 0;
bool ledStatus = false;

void setup() {
  pinMode(ADDR, OUTPUT); // ADDR
  pinMode(SCLK, OUTPUT); // SHIFT_CLK
  pinMode(LCLK, OUTPUT); // LATCH_CLK
  setDir(OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);   // RUN LED
  pinMode(PROG, OUTPUT); // PROG
  pinMode(WE, OUTPUT);   // WE
  pinMode(OE, OUTPUT);   // OE
  
  digitalWrite(WE, HIGH);
  digitalWrite(OE, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(9600);
  Serial.println("ready");
}

void loop() {
  // process incoming data
  byte serialData[1] = {0};
  while (Serial.readBytes(serialData, 1)) {

    digitalWrite(LED_BUILTIN, LOW);
    
    static char serialBuffer[BUFFER_LENGTH];
    static unsigned int serialBufferLength = 0;

    if (programmingMode) {
      programmingMode = false;
      Serial.write(writeRom(writeAddress, serialData[0]));
      Serial.println();
      serialBufferLength = 0;
      
    } else if (serialData[0] == '\r') {
      serialBuffer[serialBufferLength] = '\0';
      if (serialBufferLength) executeCommand(serialBuffer);
      serialBufferLength = 0;
      
    } else if (serialBufferLength < BUFFER_LENGTH - 1) {
      serialBuffer[serialBufferLength++] = serialData[0];
    }
  }

  unsigned long currentMillis = millis();
  if (currentMillis >= ledBlink) {
    ledBlink = currentMillis + 1000;
    if (ledStatus) {
      digitalWrite(LED_BUILTIN, HIGH);
      ledStatus = false;
    } else {
      digitalWrite(LED_BUILTIN, LOW);
      ledStatus = true;
    }
  }
}

void setDir(int dir) {
  pinMode(5, dir);  // D7
  pinMode(6, dir);  // D6
  pinMode(7, dir);  // D5
  pinMode(8, dir);  // D4
  pinMode(9, dir);  // D3
  pinMode(10, dir); // D2
  pinMode(11, dir); // D1
  pinMode(12, dir); // D0
}

byte readData() {
  writeData(0x00); // disable pullups
  setDir(INPUT);
  byte x = 0;
  bitWrite(x, 0, bitRead(PINB, 4));
  bitWrite(x, 1, bitRead(PINB, 3));
  bitWrite(x, 2, bitRead(PINB, 2));
  bitWrite(x, 3, bitRead(PINB, 1));
  bitWrite(x, 4, bitRead(PINB, 0));
  bitWrite(x, 5, bitRead(PIND, 7));
  bitWrite(x, 6, bitRead(PIND, 6));
  bitWrite(x, 7, bitRead(PIND, 5));
  return x;
}

void writeData(byte x) {
  setDir(OUTPUT);
  bitWrite(PORTB, 0, bitRead(x, 4));
  bitWrite(PORTB, 1, bitRead(x, 3));
  bitWrite(PORTB, 2, bitRead(x, 2));
  bitWrite(PORTB, 3, bitRead(x, 1));
  bitWrite(PORTB, 4, bitRead(x, 0));
  bitWrite(PORTD, 5, bitRead(x, 7));
  bitWrite(PORTD, 6, bitRead(x, 6));
  bitWrite(PORTD, 7, bitRead(x, 5));
}

void executeCommand(char cmd[BUFFER_LENGTH]) {
  char *command = strsep(&cmd, " ");
  if (strcmp_P(command, PSTR("read")) == 0) {
    unsigned int readAddress = atoi(cmd);
    Serial.write(readRom(readAddress));
    Serial.println();
    
  } else if (strcmp_P(command, PSTR("write")) == 0) {
    writeAddress = atoi(cmd);
    programmingMode = true;
    Serial.println();
    
  } else if (strcmp_P(command, PSTR("erase")) == 0) {
    Serial.println(eraseRom());
  }
}

byte readRom(unsigned int addr) {
  setAddress(addr);
  digitalWrite(OE, LOW);
  byte x = readData();
  digitalWrite(OE, HIGH);
  return x;
}

byte writeRom(unsigned int addr, byte x) {
  digitalWrite(PROG, HIGH);
  delayMicroseconds(1000);
  
  byte retval = 0;
  for (int i = 0; i < 25; i++) {
  
    setAddress(addr);
    bitWrite(PORTC, 1, 0);
    delayMicroseconds(12);
    writeData(0x40);
    bitWrite(PORTC, 1, 1);
    delayMicroseconds(12);
    
    bitWrite(PORTC, 1, 0);
    delayMicroseconds(12);
    writeData(x);
    bitWrite(PORTC, 1, 1);
    delayMicroseconds(12);
    
    bitWrite(PORTC, 1, 0);
    delayMicroseconds(12);
    writeData(0xC0);
    bitWrite(PORTC, 1, 1);
    delayMicroseconds(12);

    retval = readRom(addr);

    if (retval == x) break;
  }
  
  setAddress(0);
  digitalWrite(WE, LOW);
  writeData(0);
  digitalWrite(WE, HIGH);

  digitalWrite(PROG, LOW);
  return retval;
}

void setAddress(unsigned int a) {
  shiftOut(ADDR, SCLK, MSBFIRST, (a >> 8));
  shiftOut(ADDR, SCLK, MSBFIRST, a);
  digitalWrite(LCLK, LOW);
  digitalWrite(LCLK, HIGH);
  digitalWrite(LCLK, LOW);
}

char* eraseRom() {
  digitalWrite(PROG, HIGH);
  delayMicroseconds(1000);
  bitWrite(PORTC, 1, 0);
  delayMicroseconds(12);
  writeData(0x20);
  bitWrite(PORTC, 1, 1);
  delayMicroseconds(12);
  bitWrite(PORTC, 1, 0);
  delayMicroseconds(12);
  writeData(0x20);
  bitWrite(PORTC, 1, 1);
  delay(2000);

  for (unsigned int i = 0; i < TOTAL_BYTES; i++) {
    setAddress(i);
    bitWrite(PORTC, 1, 0);
    writeData(0xA0);
    bitWrite(PORTC, 1, 1);
    delayMicroseconds(6);
    if (readRom(i) != 0xff) return "fail\0";
  }

  setAddress(0);
  digitalWrite(WE, LOW);
  writeData(0);
  digitalWrite(WE, HIGH);
  
  digitalWrite(PROG, LOW);
  return "pass\0";
}
