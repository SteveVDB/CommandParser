# CommandParser
A C/C++ library to parse ASCII commands from a serial steam.

**Message format**

\<$>\<command\> \<arg1\>,\<arg2\>,..,\<argn\>\<CR\>

Whitespaces are not allowed except between the command and the first argument.

**Supported arguments**

- integer values (signed 32-bit): prefix with `0x` for hexadecimal or `0b` for binary.
- boolean values: `0/1`, `on/off`, `low/high`, `true/false`.

**Examples:**

- `$setPin 1,HIGH`
- `$setPwm 0b01,0xFE`
- `$getIdn`

**Status response**

- `:ACK` message received and command executed.
- `:ERR` message received but cannot execute command.
- `:ERR:OVF` buffer overflow, message lost.
- `:ERR:CMD` message received with an unknown command.
- `:ERR:FMT:X` message received with an invalid format, `X` denotes the current position in the message

## Usage

Quick example:

```C++
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
```

## License

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)