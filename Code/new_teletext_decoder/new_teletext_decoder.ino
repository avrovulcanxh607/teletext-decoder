/*
 * NMS Teletext In-Vision Decoder
 * TDE-1200 Rack-Mountable Model with LCD
*/

// Include various Libraries we'll need later
#include <Wire.h>
#include <EEPROM.h>
#include <PCF8574.h>
#include <LiquidCrystal_I2C.h>

// Define various Objects
PCF8574 INPUTS(0x38);
PCF8574 LED(0x39);
LiquidCrystal_I2C lcd(0x27,24,2);

int saa5244[9]={0,4,0,0,0,254,127,7,16};

int oldmag=99;
int oldpten=99;
int oldpunit=99;

bool overrideSelect=false;

byte thinP[] = {
  B00000,
  B00110,
  B00101,
  B00110,
  B00100,
  B00100,
  B00100,
  B00000
};

byte c4[] = {
  B01110,
  B10001,
  B10001,
  B00100,
  B00000,
  B00111,
  B00100,
  B11111
};

byte c5[] = {
  B01110,
  B10001,
  B10001,
  B00100,
  B00000,
  B10111,
  B10101,
  B11101
};

byte c6[] = {
  B01110,
  B10001,
  B10001,
  B00100,
  B00000,
  B11111,
  B10101,
  B11101
};

byte c7[] = {
  B01110,
  B10001,
  B10001,
  B00100,
  B00000,
  B00001,
  B00001,
  B11111
};

byte c8[] = {
  B01110,
  B10001,
  B10001,
  B00100,
  B00000,
  B11111,
  B10101,
  B11111
};

void setup() {
  bool fled=false;
  bool fin=false;
  bool text=false;

  LED.begin();  // Enable the front panel ICs
  INPUTS.begin();
  Serial.begin(9600); // Enable the Serial Port
  lcd.init(); // Enable the LCD & turn on its backlight
  lcd.backlight();
  
  lcd.setCursor(1,0); // Display Startup message
  lcd.print("Nathan Media Services");
  lcd.setCursor(3,1);
  lcd.print("TDE-1200 (c) 2021");
  Serial.println(F("TDE-1200 Teletext Decoder"));

  lcd.createChar(0, thinP);
  lcd.createChar(1, c4);
  lcd.createChar(2, c5);
  lcd.createChar(3, c6);  // Create our custom characters
  lcd.createChar(4, c7);
  lcd.createChar(5, c8);

  pinMode(3, OUTPUT); // Set up the board LEDs
  pinMode(4, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(5,INPUT_PULLUP);  // Set up the board buttons
  pinMode(6,INPUT_PULLUP);
  pinMode(7,INPUT_PULLUP);

  writeByte(0x09,0x40); // Clear the display menu
  
  delay(2000);  // Let us see the wonderful boot screen!
  
  // Begin Self-Test

  if (!LED.isConnected()) // Check the front panel I2C Expanders are working
    fled=true;

  if (!INPUTS.isConnected())
    fin=true;

  Wire.beginTransmission (0x11);    // Check the main IC is working
  if (Wire.endTransmission () != 0)
    text=true;

  selftest(fled,fin,text);  // Get test results

  delay(1000);  // Show them for a second or whatever
  lcd.clear();  // Clear the screen
  saainit();    // Fire up the decoder
}

void loop() {
  status();
  readBSDP();

  if(Serial.available())
  {
    int command=Serial.read();
    Serial.write(command);
    if(command==110)
    {
      fastText(0);
    }
    else if(command==111)
    {
      fastText(1);
    }
    else if(command==112)
    {
      fastText(2);
    }
    else if(command==113)
    {
      fastText(3);
    }
    else if(command==114)
    {
      fastText(4);
    }
    else if(command==109)
    {
      fastText(5);
    }
    else if(command==100)
    {
      doubleSize(1);
    }
    else if(command==105)
    {
      doubleSize(2);
    }
    else if(command==106)
    {
      doubleSize(0);
    }
  }

  if(!overrideSelect) // Only read from the rotary selector if something else hasn't set a page
  {
    LED.write(3,LOW);
    LED.write(2,HIGH);
    LED.write(1,HIGH);
    int newmag=readSelect();  // First Digit

    LED.write(3,HIGH);
    LED.write(2,LOW);
    int newpten=readSelect(); // Second Digit

    LED.write(2,HIGH);
    LED.write(1,LOW);
    int newpunit=readSelect();  // Third Digit

    if(newmag!=oldmag || newpten!=oldpten || newpunit!=oldpunit)  // If the value has changed since we last checked...
    {
      callPage(newmag,newpten,newpunit);  // Call the page
    
      oldmag=newmag;  // Store the values for next time
      oldpten=newpten;
      oldpunit=newpunit;
    }
  }
  delay(100);
}

int readSelect()
{
  if(!INPUTS.read(0))
    return 0;
  else if(!INPUTS.read(1))
    return 1;
  else if(!INPUTS.read(2))
    return 2;
  else if(!INPUTS.read(3))
    return 3;
  else if(!INPUTS.read(4))
    return 4;
  else if(!INPUTS.read(5))
    return 5;
  else if(!INPUTS.read(6))
    return 6;
  else if(!INPUTS.read(7))
    return 7;
  else if(!LED.read(7))
    return 8;
  else if(!LED.read(6))
    return 9;
  else
    return 1;
}

void saainit()
{
  for (int i=0; i<9; i++)
  {
    writeByte(i,saa5244[i]);
  }
}

bool callPage(int M, int T, int U)  // Calls a Page
{
  int pten,punit;
  pten=T+48;
  punit=U+48;
  
  if (T > 9)
    pten=T+55;
  if (U > 9)
    punit=U+55;
  
  if (M < 9 && M > 0)
  {
    setCursor(0x00,0x00);
    writeByte(0x0b,0x02); // 9space 0x07) (green 0x02)
    writeByte(0x0b,0x50); // P
    writeByte(0x0b,48+M); 
    writeByte(0x0b,pten); // Write the Page status on row 0
    writeByte(0x0b,punit);
    writeByte(0x0b,0x20);
    setCursor(0x00,0x02);
    
    if (M == 8)
    {
      M = 0;  // 8 becomes 0 because of binary stuff
    }
    
    writeByte(0x02,0x00);
    writeByte(0x03,24+M);
  
    writeByte(0x02,0x01);
    writeByte(0x03,16+T); // Write the page number to the register
  
    writeByte(0x02,0x02);
    writeByte(0x03,16+U);
    return true;
  }
  else  // If the value isn't valid, change the number red
  {
    setCursor(0x00,0x00);
    writeByte(0x0b,0x01); // red
    writeByte(0x0b,0x50); // P
    writeByte(0x0b,48+M); 
    writeByte(0x0b,pten); // Write the Page status on row 0
    writeByte(0x0b,punit);
    setCursor(0x00,0x02);
    return false;
  }
  
}

bool readBSDP()
{
  bitClear(saa5244[0],0);  // Make sure we're writing to 11A not 11B
  writeByte(0x00,saa5244[0]);

  setCursor(0x22,0x00);
  int Dc = readByte(0x0b);  // Read Designation Code
  int Unit = readByte(0x0b);  // Read Page Unit
  int Ten = readByte(0x0b); // Read Page Ten
  
  setCursor(0x22,0x04);
  int byte4 = readByte(0x0b);
  setCursor(0x22,0x06); // Read page Mag
  int byte6 = readByte(0x0b);

  lcd.setCursor(0,1);
  setCursor(0x22,0x14);  // Read Status Display
  for (int i = 0; i <= 19; i++) { // 20 Characters
    lcd.write(readByte(0x0b));
  }

  lcd.setCursor(16,0);
  setCursor(0,32);  // Read Header
  for (int i = 0; i <= 8; i++) {
    lcd.write(readByte(0x0b));
  }

  int M1 = bitRead(byte4,3);
  int M2 = bitRead(byte6,3);
  int M3 = bitRead(byte6,2);
  int Mag=0;
  if (M1 == 1)
    bitSet(Mag,0);
  if (M2 == 1)
    bitSet(Mag,2);  // Work out actual Mag
  if (M3 == 1)
    bitSet(Mag,1);
  if (Mag == 0)
    Mag=8;
  
  if(Mag > 0 && Mag < 9 && Ten < 10 && Unit < 10) // Make sure it all checks out
  {
    lcd.setCursor(20,1);
    lcd.write(0);
    lcd.print(Mag);
    lcd.print(Ten);
    lcd.print(Unit);
  }
}

void status(void)
{
  bitSet(saa5244[0],0);  // Make sure we're reading 11B
  writeByte(0x00,saa5244[0]);
  int status = readByte(0x0b);
  
  bool text=bitRead(status,1);
  bool sync=bitRead(status,0);
  
  digitalWrite(3, sync);
  digitalWrite(4, text);
  
  //LED.write(4, 0);
  LED.write(5, !text);
  LED.write(0, !sync);

  bitClear(saa5244[0],0);  // Make sure we're reading 11A
  writeByte(0x00,saa5244[0]);

  setCursor(25,0);

  int rcd[10] = { 0 };
  
  for (int i = 0; i <= 9; i++) {
    rcd[i]=readByte(0x0b);
  }

  lcd.setCursor(0,0);
  lcd.print("P");

  if((rcd[8] & B00000111)==0)
    lcd.print(8);
  else
    lcd.print(rcd[8] & B00000111);

  toHex(rcd[1] & B00001111);
  toHex(rcd[0] & B00001111);
  lcd.print(":");
  toHex(rcd[5] & B00000011);
  toHex(rcd[4] & B00001111);
  toHex(rcd[3] & B00000111);
  toHex(rcd[2] & B00001111);

  lcd.setCursor(10,0);
  if(bitRead(rcd[3],3))
    //lcd.print("E");
    lcd.write(1);
  else
    lcd.print("-");

  if(bitRead(rcd[5],2))
    //lcd.print("N");
    lcd.write(2);
  else
    lcd.print("-");

  if(bitRead(rcd[5],3))
    //lcd.print("S");
    lcd.write(3);
  else
    lcd.print("-");

  if(bitRead(rcd[6],0))
    //lcd.print("H");
    lcd.write(4);
  else
    lcd.print("-");

  if(bitRead(rcd[6],1))
    //lcd.print("U");
    lcd.write(5);
  else
    lcd.print("-");
  
  setCursor(0x19,0x09);
  LED.write(4, bitRead(rcd[9],5));  // Read PBLF
}

void writeByte(uint8_t subAddress, uint8_t data)  // Write a Byte to the I2C IC
{
  Wire.beginTransmission(0x11);  // Initialize the Tx buffer
  Wire.write(subAddress);        // Put slave register address in Tx buffer
  Wire.write(data);              // Put data in Tx buffer
  Wire.endTransmission();        // Send the Tx buffer
}

int readByte(uint8_t subAddress)  // Read a Byte from the I2C IC
{
  Wire.beginTransmission(0x11);    // Get the slave's attention, tell it we're sending a command byte
  Wire.write(subAddress);          //  The command byte, sets pointer to register with address of 0x32
  Wire.requestFrom(0x11,1);        // Tell slave we need to read 1byte from the current register
  int slaveByte2 = Wire.read();    // read that byte into 'slaveByte2' variable
  Wire.endTransmission();
  return slaveByte2;
}

void setCursor(int row, int column)
{
  writeByte(0x09,row); // row
  writeByte(0x0a,column); // column
}

void writeChar(char input)  // Write a Character to the Framebuffer
{
  bitClear(saa5244[0],0);  // Make sure we're writing to 11A not 11B
  writeByte(0x00,saa5244[0]);
  writeByte(0x0b,input);
}

void toHex(int input)
{
  if(input == 10)
    lcd.print("A");
  else if(input == 11)
    lcd.print("B");
  else if(input == 12)
    lcd.print("C");
  else if(input == 13)
    lcd.print("D");
  else if(input == 14)
    lcd.print("E");
  else if(input == 15)
    lcd.print("F");
  else if(input == 0)
    lcd.print("0");
  else if(input == 1)
    lcd.print("1");
  else if(input == 2)
    lcd.print("2");
  else if(input == 3)
    lcd.print("3");
  else if(input == 4)
    lcd.print("4");
  else if(input == 5)
    lcd.print("5");
  else if(input == 6)
    lcd.print("6");
  else if(input == 7)
    lcd.print("7");
  else if(input == 8)
    lcd.print("8");
  else if(input == 9)
    lcd.print("9");
  else
    lcd.print("-");
}

void selftest(bool fled, bool fin, bool text) {
  lcd.clear();
  if((!fled) && (!fin) && (!text))  // Self-test result
  {
    lcd.setCursor(1,0);
    lcd.print("Power On Self-Test: OK");
    Serial.println(F("POST OK"));
  }
  else if((fled) && (!fin) && (!text))
  {
    lcd.setCursor(1,0);
    lcd.print("Power On Self-Test:");
    lcd.setCursor(0,1);
    lcd.print("Front Panel LEDs Error");
    Serial.println(F("Front Panel LEDs Error"));
  }
  else if((!fled) && (fin) && (!text))
  {
    lcd.setCursor(1,0);
    lcd.print("Power On Self-Test:");
    lcd.setCursor(0,1);
    lcd.print("Page Thumbwheel Error");
    Serial.println(F("Page Thumbwheel Error"));
  }
  else if((fled) && (fin) && (!text))
  {
    lcd.setCursor(1,0);
    lcd.print("Power On Self-Test:");
    lcd.setCursor(0,1);
    lcd.print("Front Panel ICs Absent");
    Serial.println(F("Front Panel ICs Absent"));
  }
  else if((!fled) && (!fin) && (text))
  {
    lcd.setCursor(1,0);
    lcd.print("Power On Self-Test:");
    lcd.setCursor(0,1);
    lcd.print("Decoder IC Absent");
    Serial.println(F("Decoder IC Absent"));

    while(true)
    {
      digitalWrite(3,HIGH); // Flash LEDs if the chip's broken
      digitalWrite(4,LOW);
      delay(100);
      digitalWrite(4,HIGH);
      digitalWrite(3,LOW);
      delay(100);
    }
  }
  else if((fled) && (fin) && (text))
  {
    lcd.setCursor(1,0);
    lcd.print("Power On Self-Test:");
    lcd.setCursor(0,1);
    lcd.print("I2C Bus Failure");
    Serial.println(F("I2C Bus Failure"));

    while(true)
    {
      digitalWrite(3,HIGH); // Flash LEDs if the bus's broken
      digitalWrite(4,LOW);
      delay(300);
      digitalWrite(4,HIGH);
      digitalWrite(3,LOW);
      delay(100);
    }
  }
}

void fastText(int link)
{
  int offset = (link*6);
  
  bitClear(saa5244[0],0);  // Make sure we're writing to 11A not 11B
  writeByte(0x00,saa5244[0]);
  
  setCursor(0x21,0x01+offset);
  int Unit = readByte(0x0b);  // Read Page Unit
  
  setCursor(0x21,0x02+offset);
  int Ten = readByte(0x0b); // Read Page Ten
  
  setCursor(0x21,0x04+offset);
  int byte4 = readByte(0x0b);
  setCursor(0x21,0x06+offset);  // Read page Mag
  int byte6 = readByte(0x0b);
  
  int M1 = bitRead(byte4,3);
  int M2 = bitRead(byte6,3);
  int M3 = bitRead(byte6,2);
  int mag=0;
  if (M1 == 1)
    bitSet(mag,0);
  if (M2 == 1)
    bitSet(mag,2);  // Work out actual Mag
  if (M3 == 1)
    bitSet(mag,1);
  
  setCursor(0x19,0x08);
  int curmag = readByte(0x0b);  // Read current Mag
  
  int Mag = curmag ^ mag;
  
  callPage(Mag,Ten,Unit);
}
void doubleSize(int half)  // Double Height Page
{
  if(half == 0)
  {
    bitClear(saa5244[7],3);
    bitClear(saa5244[7],4);
    writeByte(0x07,saa5244[7]); // Cancel
  }
  else if(half == 1)
  {
    bitSet(saa5244[7],3);
    bitClear(saa5244[7],4);
    writeByte(0x07,saa5244[7]); // Top Half
  }
  else if(half == 2)
  {
    bitSet(saa5244[7],3);
    bitSet(saa5244[7],4);
    writeByte(0x07,saa5244[7]); // Bottom Half
  }
}
