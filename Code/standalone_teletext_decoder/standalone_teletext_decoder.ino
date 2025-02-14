// Teletext In-Vision Decoder
// Nathan Media Services, (c) 2023
// V2.0 ALPHA

#include <Wire.h>
#include <EEPROM.h>

int selected_page[3] = {1,0,0};
int thumbwheel[3] = {1,5,2};

const int TextOK = 5;
const int SyncOK = 6;

const byte colourBars[] = {
  0x1d,0x20,0x20,0x20,0x03,0x1d,0x20,0x20,0x20,
  0x20,0x06,0x1d,0x20,0x20,0x20,0x20,0x02,0x1d,
  0x20,0x20,0x20,0x20,0x05,0x1d,0x20,0x20,0x20,
  0x20,0x01,0x1d,0x20,0x20,0x20,0x20,0x04,0x1d,
  0x20,0x20,0x20,0x20
};

const byte splashIdent[] = {
  0x17,0x1c,0x20,0x20,0x7c,0x7c,0x7c,0x7c,0x20,
  0x7c,0x7c,0x7c,0x7c,0x7c,0x7c,0x20,0x7c,0x7c,
  0x7c,0x7c,0x01,0x1d,0x17,0x1c,0x20,0x6a,0x35,
  0x20,0x6a,0x35,0x6a,0x35,0x20,0x7f,0x20,0x6a,
  0x35,0x6a,0x7d,0x7c,0x7c,0x34,0x01,0x1d,0x17,
  0x1c,0x20,0x7f,0x20,0x20,0x7f,0x20,0x7f,0x20,
  0x6a,0x35,0x20,0x7f,0x20,0x7c,0x7c,0x7c,0x3f,
  0x20,0x01,0x1d,0x14,0x1c,0x20,0x2c,0x20,0x20,
  0x2c,0x20,0x2c,0x20,0x28,0x24,0x20,0x2c,0x20,
  0x2c,0x2c,0x2c,0x24,0x20,0x01,0x1d,0x06,0x1c,
  0x20,0x54,0x65,0x6c,0x65,0x74,0x65,0x78,0x74,
  0x20,0x44,0x65,0x63,0x6f,0x64,0x65,0x72,0x20,
  0x01,0x1d  
};

int i2c_registers[9] = {
  0x00, // 0
  0x14, // 1
  0x00, // 2
  0x00, // 3
  0x00, // 4 (ignored)
  0xcf, // 5
  0x55, // 6
  0x07, // 7
};

void readThumbwheel() {
  digitalWrite(8,LOW);
  digitalWrite(9,HIGH);
  digitalWrite(10,HIGH);

  thumbwheel[0] = (15-((digitalRead(11)*1) + (digitalRead(12)*2) + (digitalRead(13)*4) + (digitalRead(PD2)*8)));

  delay(10);

  digitalWrite(8,HIGH);
  digitalWrite(9,LOW);
  digitalWrite(10,HIGH);

  thumbwheel[1] = (15-((digitalRead(11)*1) + (digitalRead(12)*2) + (digitalRead(13)*4) + (digitalRead(PD2)*8)));

  delay(10);
  
  digitalWrite(8,HIGH);
  digitalWrite(9,HIGH);
  digitalWrite(10,LOW);

  thumbwheel[2] = (15-((digitalRead(11)*1) + (digitalRead(12)*2) + (digitalRead(13)*4) + (digitalRead(PD2)*8)));
}

void setup() {
  Serial.begin(9600);
  Serial.println("NMS-INV-2000-V2.0-ALPHA");
  
  pinMode(TextOK, OUTPUT);
  pinMode(SyncOK, OUTPUT);
  pinMode(PD4, OUTPUT);

  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  
  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  pinMode(PD2, INPUT_PULLUP);

  Wire.begin();
  Wire.setClock(100000);
  
  for (int address=0; address<=7; address++) {
    writeByte(address,i2c_registers[address]);
  }

  writeByte(12,0);
  writeByte(13,0);

  splashScreen();

  //bitSet(i2c_registers[0],7);
  //writeByte(0x00,i2c_registers[0]);

  digitalWrite(TextOK,HIGH);
  digitalWrite(SyncOK,HIGH);
  digitalWrite(PD4,HIGH);

  delay(1000);

  readThumbwheel();
  callPage(thumbwheel[0],thumbwheel[1],thumbwheel[2]);
}

void loop() {
  updateStatus();
  readThumbwheel();

  if(thumbwheel[0] != selected_page[0] || thumbwheel[1] != selected_page[1] || thumbwheel[2] != selected_page[2])
  {
    selected_page[0] = thumbwheel[0];
    selected_page[1] = thumbwheel[1];
    selected_page[2] = thumbwheel[2];

    Serial.print(thumbwheel[0]);
    Serial.print(thumbwheel[1]);
    Serial.print(thumbwheel[2]);
    Serial.println();
    
    callPage(thumbwheel[0],thumbwheel[1],thumbwheel[2]);
  }
}

void writeByte(int subAddress, int data)  // Write a Byte to the I2C IC
{
  Wire.beginTransmission(0x11);  // Initialize the Tx buffer
  Wire.write(subAddress);        // Put slave register address in Tx buffer
  Wire.write(data);              // Put data in Tx buffer
  Wire.endTransmission();        // Send the Tx buffer
}

int readByte(int subAddress)  // Read a Byte from the I2C IC
{
  Wire.beginTransmission(0x11);    // Get the slave's attention, tell it we're sending a command byte
  Wire.write(subAddress);          // The command byte, sets pointer to register with address of 0x32
  Wire.requestFrom(0x11,1);        // Tell slave we need to read 1byte from the current register
  int slaveByte2 = Wire.read();    // read that byte into 'slaveByte2' variable
  Wire.endTransmission();
  return slaveByte2;
}

void setCursor(int row, int column)
{
  writeByte(0x09,row); // row
  writeByte(0x0a,column+64); // column
}

void writeChar(char input)  // Write a Character to the Framebuffer
{
  bitClear(i2c_registers[0],0);  // Make sure we're writing to 11A not 11B
  writeByte(0x00,i2c_registers[0]);
  writeByte(0x0b,input);
}

void splashScreen(void)
{
  int i=0;
  setCursor(0,0);
  for (int l=0; l<25; l++)
  {
    for (i=0; i<40; i++)
    {
      writeByte(0x0b,colourBars[i]);
    }
  }

  for (int l=10; l<15; l++)
  {
    setCursor(l,9);

    for (i=(22 * (l - 10)); i<((22 * (l - 10))+22); i++)
    {
      writeByte(0x0b,splashIdent[i]);
    }
  }
}

void updateStatus()
{
  bitSet(i2c_registers[0],0);
  writeByte(0x00,i2c_registers[0]);
  digitalWrite(TextOK,bitRead(readByte(0x0b),1));

  digitalWrite(PD4,!bitRead(readByte(0x0b),0));

  bitClear(i2c_registers[0],0);  // Make sure we're reading 11A
  writeByte(0x00,i2c_registers[0]);
  setCursor(0x19,0x09);
  digitalWrite(SyncOK,!bitRead(readByte(0x0b),5));
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

  bitClear(i2c_registers[0],0);  // Make sure we're writing to 11A not 11B
  writeByte(0x00,i2c_registers[0]);
  
  if (M < 9 && M > 0)
  {
    setCursor(0x00,0x00);
    
    //writeByte(0x0b,0x6a);
    //writeByte(0x0b,0x7a);
    
    writeByte(0x0b,0x07); // (space 0x07) (green 0x02)
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
