#ifndef _MESSAGING_H_
#define _MESSAGING_H_

#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Number of commands that can be stored.
#define CP_CMD_COUNT 16
// Total message length in number of charachters.
#define CP_MSG_SIZE 32
// Preferred size of the feedback message, see hasInfo.
#define CP_FB_SIZE 16

// Returns a character read from a serial stream; or -1 if no data is available.
extern int16_t getChar();

// Message format
const char CP_MSG_SOF = '$';        // message Start of Frame
const char CP_MSG_EOF = '\r';       // message End of Frame
const char CP_MSG_CAS = ' ';        // command/argument separator
const char CP_MSG_AAS = ',';        // argument/argument separator

/// @ref TEST
const char TEST = 0;

#ifndef CP_USE_SUBSTRINGS
#define CP_USE_SUBSTRINGS
const char CP_RSP_ACK[] = "ACK";    // acknowledge
const char CP_RSP_ERR[] = "ERR";    // error
const char CP_RSP_CMD[] = "CMD";    // command   
const char CP_RSP_OVF[] = "OVF";    // overflow
const char CP_RSP_FMT[] = "FMT";    // format
#endif

// Enumeration of the message processing status.
enum class CPStatus
{ 
    // Status: synchronize on start of frame.
    S_SYN = 0,
    
    // Status: message is being processed.
    S_BSY,
    
    // Status: message received and command executed.
    S_ACK,
    
    // Exception: message received but cannot execute command.
    E_ERR,
        
    // Exception: message received with an unknown command.
    E_CMD,
        
    // Exception: buffer overflow, message lost.
    E_OVF,
        
    // Exception: message received with an invalid format.
    E_FMT
};

class CommandParser;

// Callback function when receiving a message.
typedef int8_t (*CPAction)(CommandParser *);

// Callback return values.
const int8_t CP_CB_S_FIN =  1;
const int8_t CP_CB_S_BSY =  0;
const int8_t CP_CB_E_ERR = -1;
const int8_t CP_CB_E_FMT = -2;

// Message data structure.
struct CPMessage
{
    uint8_t offset;
    uint8_t length;
    char data[CP_MSG_SIZE];
};

// Command data structure.
struct CPCommand
{
    const char *name;
    CPAction callback;
};

/**
 * Represents a simple ASCII communication protocol.
 * @note Message format: <SOF><command name> <arg1>,<arg2>,..,<argn><EOF>
*/
class CommandParser 
{
    private:

        CPMessage _message;
        CPCommand _commands[CP_CMD_COUNT];

        uint8_t _commandCount;
        int8_t _selectedCommand;

        CPStatus _status;

        bool _argFormatError;
        bool _cmdArgSepFound;

        enum class CPState { 
            START = 0, FETCH, DECODE, EXECUTE 
        } _fsm;

        int8_t parseCommand();
        void fetchMessage(char input);
        
        const int8_t available() { 
            return _message.length - _message.offset; 
        }

    public:
        
        CommandParser();

        /**
         * Calls the command parser logic and returns the current status.
         * 
         * @return The current status of the command parser.
         * @note Put this function in the main loop of the program.
        */
        CPStatus process();

        /**
         * Registers a new command with the given name and callback function.
         *
         * @param name The command name including a terminating null character.
         * @param callback The callback function associated with this command.
         * @return 0 on success; or -1, if the command cannot be registered.
        */ 
        int8_t registerCommand(const char *name, CPAction callback);

        /**
         * Indicates whether the parser has reached the end of the message in the context of the received command.
         * For example, if the command requires two arguments but the message contains only one, than the message format is invalid.
         * 
         * @return true if the message is valid for the received command; otherwise, false. 
         * @note Call this function from a callback routine and after reading the argument(s), to validate the message format.
        */
        const bool isValid();

        /**
         * Converts the next argument in the received message to a 32-bit signed integer. Prefix the argument with 0x for a 
         * hexadecimal value, or 0b for a binary value. Hexadecimal and binary arguments are case insensitive. 
         * 
         * @return The argument converted to an int32.
        */
        int32_t parseInt();

        /**
         * Converts the next argument in the received message to a boolean. The following case insensitive values 
         * are supported: 0 or 1, on or off, low or high and true or false.
         * 
         * @return The argument converted to a boolean.
        */
        bool parseBool();

        /**
         * If available copies information about the current process status to the destination buffer.
         * 
         * @param buffer The string buffer in which the information is to be written.
         * @param length The maximum number characters that can be written, including the terminating null character.
         * @return true if status information is available; otherwise, false.
        */
        bool hasInfo(char *buffer, uint8_t length);
};

#endif