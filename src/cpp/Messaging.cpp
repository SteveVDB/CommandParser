#include "CharUtils.h"
#include "Messaging.h"

CommandParser::CommandParser()
{
    _commandCount = 0;
    _fsm = CPState::START;
    _status = CPStatus::S_SYN;
}

const bool CommandParser::isValid()
{
    if (_fsm != CPState::EXECUTE)
        return true;

    return !_argFormatError && (available() == 0);
}

int8_t CommandParser::registerCommand(const char *name, CPAction callback)
{
    if (_commandCount >= CP_CMD_COUNT)
        return -1;

    CPCommand cmd = {name, callback};

    _commands[_commandCount] = cmd;
    _commandCount++;

    return 0;
}

CPStatus CommandParser::process()
{
    switch (_fsm)
    {
        case CPState::START:
        {
            _status = CPStatus::S_SYN;

            if ((char)getChar() == CP_MSG_SOF)
            {
                _message.offset = 0;
                _message.length = 0;

                _argFormatError = false;
                _cmdArgSepFound = false;

                _fsm = CPState::FETCH;
                _status = CPStatus::S_BSY;
            }
        }break;

        case CPState::FETCH:
        {
            int16_t rcv = getChar();

            if (rcv >= 0)
                fetchMessage((char)rcv);
        }break;

        case CPState::DECODE:
        {
            _selectedCommand = parseCommand();

            if (_selectedCommand >= 0)
                _fsm = CPState::EXECUTE;
            else
            {
                _fsm = CPState::START;
                _status = CPStatus::E_CMD;
            }
        }break;

        case CPState::EXECUTE:
        {
            CPCommand cmd = _commands[_selectedCommand];
            int8_t result = cmd.callback(this);

            if (result != CP_CB_S_BSY)
            {
                _fsm = CPState::START;

                if (result >= CP_CB_S_FIN)
                    _status = CPStatus::S_ACK;
                else 
                {
                    if (result == CP_CB_E_ERR)
                        _status = CPStatus::E_ERR;
                    else
                        _status = CPStatus::E_FMT;
                }
            }
        }break;

        default:
            _fsm = CPState::START;
            _status = CPStatus::S_SYN;
            // do nothing and restart
    }

    return _status;
}

int32_t CommandParser::parseInt()
{
    // function enters on CP_MSG_CAS or CP_MSG_AAS
    _argFormatError |= available() < 2;
    
    if (_argFormatError)
        return 0;

    int8_t sign = 1;
    int8_t base = 10;
    int32_t value = 0;

    bool isCharValid = false;
    bool onSeparator = false;

    // get first char in argument
    char c = _message.data[++_message.offset];

    // compare char against base10 (default), base2 or base16
    bool (*isBaseChar)(int) = isDigitDec;

    if (c == '-')
    {
        sign = -1;
        _message.offset++;
    }
    else
    {
        if (c == '0' && (available() > 0))
        {
            c = toLower(_message.data[_message.offset + 1]);

            if ((c == 'b') || (c == 'x'))
            {
                _message.offset += 2;

                if (c == 'b')
                {
                    base = 2;
                    isBaseChar = isDigitBin; 
                }
                else
                {
                    base = 16;
                    isBaseChar = isDigitHex;
                }
            }
            else
            {
                _message.offset++;
                isCharValid = true;
            }
        }
    }

    while (available() > 0)
    {
        c = _message.data[_message.offset];

        if (c == CP_MSG_AAS)
            break;

        isCharValid = (*isBaseChar)(c);

        if (!isCharValid)
            break;
            
        value = value * base + hexToInt(c);
        _message.offset++;
    }

    _argFormatError |= !isCharValid;
    return value * sign;
}

bool CommandParser::parseBool()
{
    // function enters on CP_MSG_CAS or CP_MSG_AAS
    _argFormatError |= available() < 2;
    
    if (_argFormatError)
        return false;

    // goto first char in argument
    _message.offset++;

    // 0,1,on,off,true,false => max. 5 characters + '\0'
    char buffer[6];
    uint8_t index = 0;
    bool onSeparator = false;

    while((available() > 0) && !onSeparator)
    {
        if (index == 6)
            break;

        char c = _message.data[_message.offset];

        if (c == CP_MSG_AAS)
            onSeparator = true;
        else
        {
            buffer[index++] = toLower(c);
            _message.offset++;
        }
    }

    _argFormatError = (index == 0) || (index == 6);

    if (_argFormatError)
        return false;

    // terminate string
    buffer[index] = '\0';

    // case 0
    if ((index == 1) && (buffer[0] == '0'))
        return false;

    // case 1
    if ((index == 1) && (buffer[0] == '1'))
        return true;

    // case off
    if (strcmp(buffer, "off") == 0)
        return false;

    // case on
    if (strcmp(buffer, "on") == 0)
        return true;

    // case low
    if (strcmp(buffer, "low") == 0)
        return false;
    
    // case high
    if (strcmp(buffer, "high") == 0)
        return true;

    // case false
    if (strcmp(buffer, "false") == 0)
        return false;

    // case true
    if (strcmp(buffer, "true") == 0)
        return true;

    _argFormatError = true;
    return false;
}

bool CommandParser::hasInfo(char *buffer, uint8_t length)
{
    int8_t written = 0;
    uint8_t size = length - 1;

    switch(_status)
    {
        case CPStatus::S_ACK: 
            written = snprintf(buffer, size, ":%s", 
              CP_RSP_ACK);
            break;
        
        case CPStatus::E_ERR:
            written = snprintf(buffer, size, ":%s", 
              CP_RSP_ERR);
            break;

        case CPStatus::E_CMD:
            written = snprintf(buffer, size, ":%s:%s", 
              CP_RSP_ERR, CP_RSP_CMD);
            break;

        case CPStatus::E_OVF:
            written = snprintf(buffer, size, ":%s:%s", 
              CP_RSP_ERR, CP_RSP_OVF);
            break;

        case CPStatus::E_FMT:
            written = snprintf(buffer, size, ":%s:%s:%d", 
              CP_RSP_ERR, CP_RSP_FMT, _message.offset);
            break;

        default:
            break;
            // no response
    }

    return (written > 0) && (written < length);
}

void CommandParser::fetchMessage(char input)
{
    if (_message.length == CP_MSG_SIZE)
    {
        _fsm = CPState::START;
        _status = CPStatus::E_OVF;
        return;
    }

    if (input == CP_MSG_EOF)
    {
        if (_message.length == 0)
        {
            _fsm = CPState::START;
            _status = CPStatus::E_FMT;
            return;
        }
            
        _fsm = CPState::DECODE;
    }
    else
    {
        _message.data[_message.length] = input;
        _message.length++;

        _cmdArgSepFound |= (input == CP_MSG_CAS);

        if (!_cmdArgSepFound)
            _message.offset = _message.length;
    }
}

int8_t CommandParser::parseCommand()
{
    uint8_t i = 0;

    while (i < _commandCount)
    {
        uint8_t j = 0;
        bool stop = false;
        CPCommand cmd = _commands[i];
        
        while (!stop)
        {
            if (j < _message.offset)
            {
                stop = cmd.name[j] != _message.data[j];
                j++;
            }
            else
            {

                if (cmd.name[j] == '\0')
                    return i;

                stop = true;
            }
        }

        i++;
    }

    return -1;
}