/*
 * FLASH EEPROM Programmer
 * Supported and tested with Intel flash memory device.
 * May work with other devices too.
 * 
 * List of similar devices:
 * 28F256 tested
 * 28F512
 * 28F010
 * 28F020 tested
 * 28F040 
 *
 * List of manufacturers:
 * Intel, Atmel, AMD, SGS, TI, Toshiba
 * 
 * Copyright (C) 2021-2024, Juha-Pekka Varjonen
 */

#define BUFFER_LENGTH 32
#define ADDR 2
#define SCLK 3
#define LCLK 4

#define setVPP(s) { bitWrite(PORTC, 0, s); }
#define setWE(s) { bitWrite(PORTC, 1, s); }
#define setOE(s) { bitWrite(PORTC, 2, s); }
#define setCE(s) { bitWrite(PORTC, 3, s); }

// With 16MHz clock
#define delay125ns {asm volatile("nop"); asm volatile("nop");}

// IDLE LED related variable
unsigned long ledBlink = 0;

void setup() {
  pinMode(ADDR, OUTPUT); // ADDR
  pinMode(SCLK, OUTPUT); // SHIFT_CLK
  pinMode(LCLK, OUTPUT); // LATCH_CLK
  pinMode(LED_BUILTIN, OUTPUT); // IDLE LED
  pinMode(A0, OUTPUT); // VPP
  pinMode(A1, OUTPUT); // WE
  pinMode(A2, OUTPUT); // OE
  pinMode(A3, OUTPUT); // CE

  setVPP(LOW);
  setWE(HIGH);
  setOE(HIGH);
  setCE(LOW);

  setDir(INPUT);

  bitWrite(PORTD, LCLK, HIGH); // LATCH_CLK
  setAddress(0x00);
    
  Serial.begin(115200);
  Serial.println("ready");
}

void loop() {
  // process incoming data
  byte serialData[1] = {0};
  while (Serial.readBytes(serialData, 1)) {
    
    static char serialBuffer[BUFFER_LENGTH];
    static unsigned int serialBufferLength = 0;
    
    if (serialData[0] == '\r') {
      serialBuffer[serialBufferLength] = '\0';
      if (serialBufferLength) executeCommand(serialBuffer);
      serialBufferLength = 0;
      
    } else if (serialBufferLength < BUFFER_LENGTH - 1) {
      serialBuffer[serialBufferLength++] = serialData[0];
    }
  }
  // IDLE LED
  unsigned long currentMillis = millis();
  if (currentMillis >= ledBlink) {
    ledBlink = currentMillis + 1000;
    bitWrite(PINB, 5, 1);
  }
}

void setDir(uint8_t dir) {
  bitWrite(DDRB, 0, dir);
  bitWrite(DDRB, 1, dir);
  bitWrite(DDRB, 2, dir);
  bitWrite(DDRB, 3, dir);
  bitWrite(DDRB, 4, dir);
  bitWrite(DDRD, 5, dir);
  bitWrite(DDRD, 6, dir);
  bitWrite(DDRD, 7, dir);
 
  /*if (dir == INPUT) {
    // Disable internal pullups
    bitWrite(PORTB, 0, 0);
    bitWrite(PORTB, 1, 0);
    bitWrite(PORTB, 2, 0);
    bitWrite(PORTB, 3, 0);
    bitWrite(PORTB, 4, 0);
    bitWrite(PORTD, 5, 0);
    bitWrite(PORTD, 6, 0);
    bitWrite(PORTD, 7, 0);
  }*/
}

uint8_t readData(uint32_t a) {
  uint8_t x = 0x00;
  setDir(INPUT);
  setAddress(a);
  setOE(LOW); // Data is readable after falling edge
  delay125ns;
  bitWrite(x, 0, bitRead(PINB, 4));
  bitWrite(x, 1, bitRead(PINB, 3));
  bitWrite(x, 2, bitRead(PINB, 2));
  bitWrite(x, 3, bitRead(PINB, 1));
  bitWrite(x, 4, bitRead(PINB, 0));
  bitWrite(x, 5, bitRead(PIND, 7));
  bitWrite(x, 6, bitRead(PIND, 6));
  bitWrite(x, 7, bitRead(PIND, 5));
  setOE(HIGH);
  return x;
}

void writeData(uint32_t a, uint8_t x) {
  setDir(OUTPUT);
  bitWrite(PORTB, 4, bitRead(x, 0));
  bitWrite(PORTB, 3, bitRead(x, 1));
  bitWrite(PORTB, 2, bitRead(x, 2));
  bitWrite(PORTB, 1, bitRead(x, 3));
  bitWrite(PORTB, 0, bitRead(x, 4));
  bitWrite(PORTD, 7, bitRead(x, 5));
  bitWrite(PORTD, 6, bitRead(x, 6));
  bitWrite(PORTD, 5, bitRead(x, 7));
  setAddress(a);
  setWE(LOW); // Address is latched during falling edge
  delay125ns;
  setWE(HIGH); // Data is latched during rising edge
  setDir(INPUT);
}

void setAddress(uint32_t a) {
  bitWrite(PORTD, LCLK, LOW);
  shiftOut(ADDR, SCLK, MSBFIRST, (a >> 16) & 0xFF);
  shiftOut(ADDR, SCLK, MSBFIRST, (a >> 8) & 0xFF);
  shiftOut(ADDR, SCLK, MSBFIRST, a & 0xFF);
  bitWrite(PORTD, LCLK, HIGH); // Address is latched on rising edge
}

void executeCommand(char cmd[BUFFER_LENGTH]) {
  char *command = strsep(&cmd, " ");
  if (strcmp_P(command, PSTR("read")) == 0) {
    uint32_t addr = atol(cmd);
    uint8_t x = readData(addr);
    Serial.write(x);
    Serial.println();
    
  } else if (strcmp_P(command, PSTR("write")) == 0) {
    uint32_t addr = atol(strsep(&cmd, " "));
    uint8_t data = atoi(cmd);
    int8_t PLSCNT = 25;
    uint8_t comp = 0x00;
    setVPP(HIGH);
    while (PLSCNT--) {
      writeData(addr, 0x40);
      writeData(addr, data);
      _delay_us(10);
      writeData(addr, 0xC0);
      _delay_us(6);
      comp = readData(addr);
      if (comp == data) {
        break;
      }
    }
    writeData(0x00, 0x00); // Resets the register for read operations
    setVPP(LOW);
    Serial.write(comp);
    Serial.println();
    
  } else if (strcmp_P(command, PSTR("erase")) == 0) {
    uint32_t memSize = atol(cmd);
    uint16_t PLSCNT = 0;
    uint32_t addr = 0;
    setVPP(HIGH);    
    while (PLSCNT < 1000) {
      writeData(0x00, 0x20); // Erase setup command
      writeData(0x00, 0x20); // Erase command
      _delay_ms(10);
      while (addr < memSize) {
        writeData(addr, 0xA0); // Erase verify command
        _delay_us(6);
        uint8_t comp = readData(addr);
        if (comp == 0xFF) {
          addr++;
          Serial.print(".");
        } else if (PLSCNT++ >= 1000) {
          Serial.write(addr);
          Serial.println();
          break;
        }
      }
      if (addr >= memSize) {
        Serial.println(F("pass"));
        break;
      }
    }
    writeData(0x00, 0x00); // Resets the register for read operations
    setVPP(LOW);

  } else if (strcmp_P(command, PSTR("ident")) == 0) {
    uint8_t addr = atoi(cmd);
    setVPP(HIGH);
    writeData(addr, 0x90);
    uint8_t x = readData(addr);
    writeData(0x00, 0x00); // Resets the register for read operations
    setVPP(LOW);
    Serial.write(x);
    Serial.println();

  } else if (strcmp_P(command, PSTR("reset")) == 0) {
    setVPP(HIGH);
    writeData(0x00, 0xff);
    writeData(0x00, 0xff);
    writeData(0x00, 0x00);
    setVPP(LOW);
    Serial.println();
  }
}