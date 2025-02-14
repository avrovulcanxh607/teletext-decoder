// Teletext In-Vision Decoder
// Nathan Media Services, (c) 2023
// V2.0 ALPHA

#include <Wire.h>
#include <EEPROM.h>

const int TextOK = 5;
const int SyncOK = 6;

const int PageUP = 4;
const int PageDN = 2;
const int Enter = 3;

int i2c_registers[9] = {
  0x00,
  0x04,
  0x00,
  0x00,
  0x00,
  0xcf,
  0x55,
  0x07,
  0x10
};

void setup() {
  pinMode(TextOK, OUTPUT);
  pinMode(SyncOK, OUTPUT);

  pinMode(PageUP, INPUT_PULLUP);
  pinMode(PageDN, INPUT_PULLUP);
  pinMode(Enter, INPUT_PULLUP);

  Wire.begin();
  Wire.setClock(100000);
  
  for (int address=0; address<=8; address++)
  {
    writeByte(address,i2c_registers[address]);
  }

  bitSet(i2c_registers[0],7);
  writeByte(0x00,i2c_registers[0]);

  callPage(1,5,2);
}

void loop() {
  digitalWrite(TextOK,HIGH); // Flash LEDs if the chip's broken
  digitalWrite(SyncOK,LOW);
  delay(1000);
  digitalWrite(SyncOK,HIGH);
  digitalWrite(TextOK,LOW);
  delay(1000);
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
  writeByte(0x0a,column); // column
}

void writeChar(char input)  // Write a Character to the Framebuffer
{
  bitClear(i2c_registers[0],0);  // Make sure we're writing to 11A not 11B
  writeByte(0x00,i2c_registers[0]);
  writeByte(0x0b,input);
}

void colourBars(void)
{
  for (int l=0; l<25; l++)
  {
    setCursor(l,0);
    writeByte(0x0b,29);
    writeByte(0x0b,32);
    writeByte(0x0b,32);
    writeByte(0x0b,32);
    writeByte(0x0b,32);
    setCursor(l,4);
    writeByte(0x0b,3);
    setCursor(l,5);
    writeByte(0x0b,29);
    setCursor(l,10);
    writeByte(0x0b,6);
    setCursor(l,11);
    writeByte(0x0b,29);
    setCursor(l,16);
    writeByte(0x0b,2);
    setCursor(l,17);
    writeByte(0x0b,29);
    setCursor(l,22);
    writeByte(0x0b,5);
    setCursor(l,23);
    writeByte(0x0b,29);
    setCursor(l,28);
    writeByte(0x0b,1);
    setCursor(l,29);
    writeByte(0x0b,29);
    setCursor(l,34);
    writeByte(0x0b,4);
    setCursor(l,35);
    writeByte(0x0b,29);
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
    //setCursor(0x00,0x00);
    //writeByte(0x0b,0x02); // 9space 0x07) (green 0x02)
    //writeByte(0x0b,0x50); // P
    //writeByte(0x0b,48+M); 
    //writeByte(0x0b,pten); // Write the Page status on row 0
    //writeByte(0x0b,punit);
    //writeByte(0x0b,0x20);
    //setCursor(0x00,0x02);
    
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
