#include "CharUtils.h"

int toLower(int c) { return ((c >= 'A') && (c <= 'Z')) ? c + 32 : c; }
int toUpper(int c) { return ((c >= 'a') && (c <= 'z')) ? c - 32 : c; }

bool isDigitDec(int c) { return ((c >= '0') && (c <= '9')); }
bool isDigitBin(int c) { return ((c == '0') || (c == '1')); }
bool isDigitHex(int c) 
{ 
    return ( ((c >= '0') && (c <= '9')) || 
             ((c >= 'a') && (c <= 'f')) || 
             ((c >= 'A') && (c <= 'F')) );
}

int hexToInt(int c)
{
    if ((c >= '0') && (c <= '9'))
        return c - '0';
    if ((c >= 'a') && (c <= 'f'))
        return c - 'a' + 10;
    if ((c >= 'A') && (c <= 'F'))
        return c - 'A' + 10;
    return -1;
}