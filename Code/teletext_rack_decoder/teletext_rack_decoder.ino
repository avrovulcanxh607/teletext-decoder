#include <PCF8574.h>
#include <teletext.h>
#include <Wire.h>
#include <IRremote.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>

PCF8574 INPUTS(0x38);
PCF8574 LED(0x39);
LiquidCrystal_I2C lcd(0x27,24,2);

decode_results results;
Teletext teletext;
void(* resetFunc) (void) = 0;

int page[4]={1,1,1,1};

int i=1,p=1,Mag=1,Pten=0,Punit=0,command=0;
bool check=true,pblf=false,newCom=false;

unsigned long eepromReadlong(long address)
{
      // Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      // Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

void eepromWritelong(int address, unsigned long input)
{
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (input & 0xFF);
      byte three = ((input >> 8) & 0xFF);
      byte two = ((input >> 16) & 0xFF);
      byte one = ((input >> 24) & 0xFF);

      Serial.println(input,HEX);
      Serial.println(four,HEX);
      Serial.println(three,HEX);
      Serial.println(two,HEX);
      Serial.println(one,HEX);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
}

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print(" Nathan Media Services");
  lcd.setCursor(0,1);
  lcd.print("   In-Vision Decoder");
  
  if (!LED.begin())
  {
    Serial.println("could not initialize...");
  }
  if (!INPUTS.begin())
  {
    Serial.println("could not initialize...");
  }
  
  if (!LED.isConnected())
  {
    Serial.println("=> not connected");
  }
  else
  {
    Serial.println("=> connected!!");
  }

  if (!INPUTS.isConnected())
  {
    Serial.println("=> not connected");
  }
  else
  {
    Serial.println("=> connected!!");
  }
  teletext.writeByte(0x09,0x40);  // Clear Memory in case of reset
  delay(1000);  // Give it a chance to clear the memory
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(5,INPUT_PULLUP);
  pinMode(6,INPUT_PULLUP);
  pinMode(7,INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println(F("Teletext In-Vision Decoder"));
  Wire.beginTransmission (0x11);
  if (Wire.endTransmission () != 0)
  {
    Serial.println(F("SAA5244 absent or faulty!"));
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
  teletext.begin();
  waitmsg();
  textmsg();
  Serial.println(F("Waiting for teletext..."));
  while(!teletext.status()) { // Don't do anything until we get a decent teletext signal
    delay(100);
  }
  waitmsg();
  packetmsg();
  Serial.println(F("Waiting for 8/30..."));
  int sec=0;
  teletext.setCursor(0x00,0x00);
  teletext.writeByte(0x0b,0x20);
  teletext.writeByte(0x0b,0x20);
  teletext.writeByte(0x0b,0x20);
  teletext.writeByte(0x0b,0x20);
  teletext.writeByte(0x0b,0x20);
  teletext.writeByte(0x0b,0x20);
  teletext.writeByte(0x0b,0x20);
  
  teletext.callPage(1,5,0);
}

void loop() {
  bool text=teletext.status();
  int pup = digitalRead(5);
  int ent = digitalRead(6);
  int pdn = digitalRead(7);

  teletext.readBSDP();
  
  delay(100);
}

void waitmsg()  // Surely there's a better way to do this!??
{
  int textLine[29]  = {0x03,0x20,0x20,0x20,0x4e,0x4d,0x53,0x20,0x28,0x32,0x30,0x32,0x30,0x29,0x08,0x20,0x20,0x57,0x61,0x69,0x74,0x69,0x6e,0x67,0x20,0x66,0x6f,0x72,0x20};
  teletext.setCursor(0,0);
  teletext.writeByte(0x00,0x00);
  for (int i=0; i<sizeof textLine/sizeof textLine[0]; i++) {
      int s = textLine[i];
      teletext.writeByte(0x0b,s);
  }
}

void textmsg()
{
  int textLine[11]  = {0x74,0x65,0x6c,0x65,0x74,0x65,0x78,0x74,0x2e,0x2e,0x2e};
  for (int i=0; i<sizeof textLine/sizeof textLine[0]; i++) {
      int s = textLine[i];
      teletext.writeByte(0x0b,s);
  }
}

void packetmsg()
{
  int textLine[11]  = {0x38,0x2f,0x33,0x30,0x2e,0x2e,0x2e,0x20,0x20,0x20,0x20};
  for (int i=0; i<sizeof textLine/sizeof textLine[0]; i++) {
      int s = textLine[i];
      teletext.writeByte(0x0b,s);
  }
}

void fasttextMenu()
{
  int settings = EEPROM.read(0);
  char string[4] = "Off ";
  if(bitRead(settings,0)==1) strcpy(string, "Auto");
  teletext.setCursor(3,30);
  for(int i =0; i < strlen(string); i++ ) {
    char c = string[i];
    teletext.writeByte(0x0b,c);
  }
}

void fillscreen()
{
  teletext.writeByte(0x09,0x40);  // Clear Memory
  delay(200);
  for (int r=1; r<23; r++)
  {
    teletext.setCursor(r,0);
    teletext.writeByte(0x00,0x00);
    teletext.writeByte(0x0b,0x04);
    teletext.writeByte(0x0b,0x1d);
    teletext.writeByte(0x0b,0x03);
  }
  teletext.setCursor(21,2);
  teletext.writeByte(0x0b,0x06);
  char string[]="NMS (c) V1.1 Jan 2021";
  for(int i =0; i < strlen(string); i++ ) {
    char c = string[i];
    teletext.writeByte(0x0b,c);
  }
}

void pairIR()
{
  fillscreen();
  teletext.setCursor(2,3);
  char string[] = "Press";
  for(int i =0; i < strlen(string); i++ ) {
    char c = string[i];
    teletext.writeByte(0x0b,c);
  }

  for(int i=0; i<=16; i++)
  {
    teletext.setCursor(2,9);
    if(i<10) teletext.writeByte(0x0b,(48+i));
    else if(i==10)
    {
      teletext.writeByte(0x0b,117); // up
      teletext.writeByte(0x0b,112);
    }
    else if(i==11)
    {
      teletext.writeByte(0x0b,114); // red
      teletext.writeByte(0x0b,101);
      teletext.writeByte(0x0b,100);
    }
    else if(i==12)
    {
      teletext.writeByte(0x0b,99);  // cyan
      teletext.writeByte(0x0b,121);
      teletext.writeByte(0x0b,97);
      teletext.writeByte(0x0b,110);
    }
    else if(i==13)
    {
      teletext.writeByte(0x0b,100); // down
      teletext.writeByte(0x0b,111);
      teletext.writeByte(0x0b,119);
      teletext.writeByte(0x0b,110);
    }
    else if(i==14)
    {
      teletext.writeByte(0x0b,101); // enter
      teletext.writeByte(0x0b,110);
      teletext.writeByte(0x0b,116);
      teletext.writeByte(0x0b,101);
      teletext.writeByte(0x0b,114);
    }
    else if(i==15)
    {
      teletext.writeByte(0x0b,103); // green
      teletext.writeByte(0x0b,114);
      teletext.writeByte(0x0b,101);
      teletext.writeByte(0x0b,101);
      teletext.writeByte(0x0b,110);
    }
    else if(i==16)
    {
      teletext.writeByte(0x0b,121); // yellow
      teletext.writeByte(0x0b,101);
      teletext.writeByte(0x0b,108);
      teletext.writeByte(0x0b,108);
      teletext.writeByte(0x0b,111);
      teletext.writeByte(0x0b,119);
    }
    if(!sanityCheck(results.value))
    {
      i--;
    }
    else
    {
      Serial.println(results.value,HEX);
      EEPROM.write(((i+1)*4), (results.value & 0xFF));
      EEPROM.write(((i+1)*4) + 1, ((results.value >> 8) & 0xFF));
      EEPROM.write(((i+1)*4) + 2, ((results.value >> 16) & 0xFF));
      EEPROM.write(((i+1)*4) + 3, ((results.value >> 24) & 0xFF));
    }
  }
  teletext.setCursor(4,2);
  teletext.writeByte(0x0b,0x02);
  teletext.writeByte(0x0b,79);
  teletext.writeByte(0x0b,75);
}

bool sanityCheck(unsigned long input)
{
  if(input > 0xFFFFFF) return false;
  if(input < 0x100000) return false;
  if(bitRead(input,0)==bitRead(input, 8)) return false;
  if(bitRead(input,1)==bitRead(input, 9)) return false;
  if(bitRead(input,2)==bitRead(input,10)) return false;
  if(bitRead(input,3)==bitRead(input,11)) return false;
  if(bitRead(input,4)==bitRead(input,12)) return false;
  if(bitRead(input,5)==bitRead(input,13)) return false;
  if(bitRead(input,6)==bitRead(input,14)) return false;
  if(bitRead(input,7)==bitRead(input,15)) return false;
  return true;
}
