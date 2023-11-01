#include "Messaging.h"

CommandParser parser;
char CPInfo[CP_FB_SIZE];

void setup() {

  Serial.begin(9600);
  while(!Serial);

  parser.registerCommand("Test", cmdTestCallback);
}

void loop() 
{
  parser.process();
  
  if (parser.hasInfo(CPInfo, CP_FB_SIZE))
    Serial.println(CPInfo);
}

int8_t cmdTestCallback(CommandParser *sender)
{
  // read the command arguments..
  int32_t arg1 = sender->parseInt();
  bool arg2 = sender->parseBool();

  // ..if the message format is valid..
  if (!sender->isValid())
    return CP_CB_E_FMT;

  // ..then use the arguments.
  Serial.print("arg1: ");
  Serial.println(arg1, DEC);
  Serial.print("arg2: ");
  Serial.println(arg2, DEC);
  
  // return status finished
  return CP_CB_S_FIN;
}

int16_t getChar()
{
  if (Serial.available() > 0)
    return Serial.read();
  else
    return -1;
}