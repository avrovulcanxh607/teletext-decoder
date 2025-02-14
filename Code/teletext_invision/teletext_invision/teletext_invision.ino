#include <teletext.h>
#include <Wire.h>
#include <IRremote.h>
#include <EEPROM.h>

IRrecv irrecv(2);
decode_results results;
Teletext teletext;
void(* resetFunc) (void) = 0;

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

unsigned long IR_ZERO = eepromReadlong(4);
unsigned long IR_ONE  = eepromReadlong(8);
unsigned long IR_TWO  = eepromReadlong(12);
unsigned long IR_THREE= eepromReadlong(16);
unsigned long IR_FOUR = eepromReadlong(20);
unsigned long IR_FIVE = eepromReadlong(24);
unsigned long IR_SIX  = eepromReadlong(28);
unsigned long IR_SEVEN= eepromReadlong(32);
unsigned long IR_EIGHT= eepromReadlong(36);
unsigned long IR_NINE = eepromReadlong(40);

unsigned long IR_UP   = eepromReadlong(44);
unsigned long IR_DOWN = eepromReadlong(56);
unsigned long IR_ENTER= eepromReadlong(60);

unsigned long IR_RED   = eepromReadlong(48);
unsigned long IR_GREEN = eepromReadlong(64);
unsigned long IR_YELLOW= eepromReadlong(68);
unsigned long IR_CYAN  = eepromReadlong(52);

void setup() {
  teletext.writeByte(0x09,0x40);  // Clear Memory in case of reset
  delay(1000);  // Give it a chance to clear the memory
  irrecv.enableIRIn();
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
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
  if(digitalRead(6)==LOW)
  {
    setupMenu();
  }
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
  while(!teletext.callInit()) { // wait to get 8/30
    delay(20);  // Check every field
    sec++;
    if(sec>200)  // No packet 8/30 for 4 seconds? (should be one every second(?))
    {
      Serial.println(F("No 8/30, calling P100"));
      teletext.callPage(1,0,0); // By default, call P100 if there's no 8/30 for whatever reason
      break;  // Stop waiting for 8/30
    }
  }
}

void loop() {
  teletext.status();
  int pup = digitalRead(5);
  int ent = digitalRead(6);
  int pdn = digitalRead(7);
  
  pblf = teletext.pblf();  // Read PBLF (Page Being Looked For)
  if(pblf && check) // If a P(i)BLF and the check flag is true
    check=false;    // It's wrong. This is a new page and will need to be checked again.
  if(!pblf && !check) // If the page is found and we haven't checked it...
  {
    delay(40); // Give it a sec to read in the page
    teletext.checkX24();  // Check it!
    check=true; // Ok, we've checked it now
  }
  
  // Here is where you can enter commands to be processed by the control loop
  // The higher up the list a command is, the higher it's priority

  if (!newCom && Serial.available())  // Serial Always takes priority
  {
    newCom=true;
    command=Serial.read();
    Serial.write(command);
  }
  
  if (!newCom && irrecv.decode(&results)) // IR takes priority over buttons
  {
    if(results.value==IR_ONE)
    {
      newCom=true;
      command=49;
    }
    if(results.value==IR_TWO)
    {
      newCom=true;
      command=50;
    }
    if(results.value==IR_THREE)
    {
      newCom=true;
      command=51;
    }
    if(results.value==IR_FOUR)
    {
      newCom=true;
      command=52;
    }
    if(results.value==IR_FIVE)
    {
      newCom=true;
      command=53;
    }
    if(results.value==IR_SIX)
    {
      newCom=true;
      command=54;
    }
    if(results.value==IR_SEVEN)
    {
      newCom=true;
      command=55;
    }
    if(results.value==IR_EIGHT)
    {
      newCom=true;
      command=56;
    }
    if(results.value==IR_NINE)
    {
      newCom=true;
      command=57;
    }
    if(results.value==IR_ZERO)
    {
      newCom=true;
      command=48;
    }
    if(results.value==IR_UP)
    {
      newCom=true;
      command=43;
    }
    if(results.value==IR_DOWN)
    {
      newCom=true;
      command=45;
    }
    if(results.value==IR_ENTER)
    {
      newCom=true;
      command=13;
    }
     if(results.value==IR_RED)
    {
      newCom=true;
      command=114;
    }
     if(results.value==IR_GREEN)
    {
      newCom=true;
      command=103;
    }
     if(results.value==IR_YELLOW)
    {
      newCom=true;
      command=121;
    }
     if(results.value==IR_CYAN)
    {
      newCom=true;
      command=99;
    }
    if(results.value==0xFFFFFFFF) // If the button is still being held, repeat the last command
    {
      newCom=true;
    }
    irrecv.resume(); // Receive the next value
  }
  
  if (!newCom && ent == LOW) // Enter button
  {
    newCom=true;
    command=13; // Trigger a command
  }
  if (!newCom && pup == LOW) // Page up button
  {
    newCom=true;
    command=43;
  }
  if (!newCom && pdn == LOW) // Page down button
  {
    newCom=true;
    command=45; 
  }

  // This is the command loop. Don't mess with it if you don't know what you're doing!
  // Changing values here will break serial and remote control.
  // If there's a new command, read it and do whatever needs done
  if (newCom){
    if (isDigit(command))
    {
      command = command-48;
      if (p == 1)
      {
        teletext.setCursor(0x00,0x00);
        teletext.writeByte(0x0b,0x01);
        teletext.writeByte(0x0b,0x50);
        teletext.writeByte(0x0b,0x20);
        teletext.writeByte(0x0b,0x20); // Clear the status line
        teletext.writeByte(0x0b,0x20);
        teletext.setCursor(0x00,0x02);
      }
      if (p == 1)
      {
        teletext.setCursor(0x00,0x02);
        teletext.writeByte(0x0b,command+48);
        Mag = (int)command;
      }
      if (p == 2)
      {
        teletext.setCursor(0x00,0x03);
        teletext.writeByte(0x0b,command+48);
        Pten = (int)command;
      }
      if (p == 3)
      {
        teletext.setCursor(0x00,0x04);
        teletext.writeByte(0x0b,command+48);
        Punit = (int)command;
      }
      p++;
      if (p > 3)
      {
        p = 1;
      }
    }
    if (command == 13) // Carriage Return is Enter
    {
      teletext.callPage(Mag,Pten,Punit);
    }
    if (command == 43)  // Page Up
    {
      Punit++;
      if(Punit>9)
      {
        Punit=0;
        Pten++;
        if(Pten>9)
        {
          Pten=0;
          Mag++;
          if(Mag>8)
          {
            Mag=1;
          }
        }
      }
      teletext.setCursor(0x00,0x00);
      teletext.writeByte(0x0b,0x01); // red
      teletext.writeByte(0x0b,0x50); // P
      teletext.writeByte(0x0b,Mag+48); 
      teletext.writeByte(0x0b,Pten+48); // Write the Page status on row 0
      teletext.writeByte(0x0b,Punit+48);
    }
    if (command == 45) 
    {  // Page Down
      if(Punit==0)
      {
        if(Pten==0)
        {
          if(Mag==1)
          {
            Mag=8;
            Punit=9;
            Pten=9;
          }
          else
          {
            Mag--;
            Punit=9;
            Pten=9;
          }
        }
        else
        {
          Pten--;
          Punit=9;
        }
      }
      else
        Punit--;
      teletext.setCursor(0x00,0x00);
      teletext.writeByte(0x0b,0x01); // red
      teletext.writeByte(0x0b,0x50); // P
      teletext.writeByte(0x0b,Mag+48); 
      teletext.writeByte(0x0b,Pten+48); // Write the Page status on row 0
      teletext.writeByte(0x0b,Punit+48);
    }
    if (command == 114)
    {
      teletext.fastText(0);
    }
    if (command == 103)
    {
      teletext.fastText(1);
    }
    if (command == 121)
    {
      teletext.fastText(2);
    }
    if (command == 99)
    {
      teletext.fastText(3);
    }
    newCom=false;
  }
  delay(100);
}

void waitmsg()  // Surely there's a better way to do this!??
{
  int textLine[29]  = {0x03,0x20,0x20,0x20,0x4e,0x4d,0x53,0x20,0x28,0x32,0x30,0x32,0x30,0x29,0x08,0x20,0x20,0x57,0x61,0x69,0x74,0x69,0x6e,0x67,0x20,0x66,0x66,0x72,0x20};
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

void setupMenu()
{
  Serial.println(F("Setup Mode - use the onboard buttons"));
  teletext.writeByte(0x01,36);  // Turn off acquisition
  int settings = EEPROM.read(0);
  fillscreen();
  drawMenu();
  fasttextMenu();
  int menupos=0;
  while(digitalRead(6)==LOW) delay(200);  // Debounce - Don't activate the menu until enter's been released
  while(true)
  {
    if(digitalRead(5)==LOW)
    {
      if(menupos<2) menupos++;
    }
    if(digitalRead(7)==LOW)
    {
      if(menupos>0) menupos--;
    }
    if(digitalRead(6)==LOW)
    {
      if(menupos==0)  // Pair Remote
      {
        Serial.println(F("Pairing new remote"));
        pairIR();
        delay(2000);
        resetFunc();
      }
      if(menupos==1)  // Fasttext
      {
        if(bitRead(settings,0)==1) bitClear(settings,0);
        else bitSet(settings,0);
        EEPROM.update(0, settings);
        fasttextMenu();
      }
      if(menupos==2)
      {
        teletext.colourBars();
        while(true) delay(5000);
      }
    }
    teletext.setCursor((menupos+1),0);
    teletext.writeByte(0x0b,0x04);
    teletext.setCursor((menupos+2),0);
    teletext.writeByte(0x0b,0x01);
    teletext.setCursor((menupos+3),0);
    teletext.writeByte(0x0b,0x04);
    delay(200);
  }
}

void drawMenu()
{
  teletext.setCursor(0,0);
  int text2Line[18]  = {0x07,0x1d,0x00,0x53,0x79,0x73,0x74,0x65,0x6d,0x20,0x73,0x65,0x74,0x74,0x69,0x6e,0x67,0x73};
  for (int i=0; i<sizeof text2Line/sizeof text2Line[0]; i++) {
    int s = text2Line[i];
    teletext.writeByte(0x0b,s);
  }
  teletext.setCursor(2,3);
  char string[] = "Pair IR remote";
  for(int i =0; i < strlen(string); i++ ) {
    char c = string[i];
    teletext.writeByte(0x0b,c);
  }
  teletext.setCursor(3,3);
  strcpy(string, "Fasttext (row 24) display: ");
  for(int i =0; i < strlen(string); i++ ) {
    char c = string[i];
    teletext.writeByte(0x0b,c);
  }
  teletext.setCursor(4,3);
  strcpy(string, "Colour Bars");
  for(int i =0; i < strlen(string); i++ ) {
    char c = string[i];
    teletext.writeByte(0x0b,c);
  }
  strcpy(string, "");
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
  char string[]="NMS (c) V1.1 Jan 2022";
  for(int i =0; i < strlen(string); i++ ) {
    char c = string[i];
    teletext.writeByte(0x0b,c);
  }
}

void pairIR()
{
  fillscreen();
  delay(100);
  teletext.setCursor(2,3);
  char pressMsg[] = "Press";
  for(int i =0; i < strlen(pressMsg); i++ ) {
    char c = pressMsg[i];
    teletext.writeByte(0x0b,c);
    delay(10);
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
    irrecv.resume();
    while(!irrecv.decode(&results))
    {
      delay(100);
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
