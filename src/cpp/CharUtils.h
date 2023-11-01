#ifndef _CHAR_UTILS_H_
#define _CHAR_CUTILS_H_

#include <stdio.h>
#include <stdbool.h>

/**
 * Converts the given character to its lowercase equivalent.
 * 
 * @param c The character to convert.
 * @return The lowercase equivalent of c; or the unchanged value of c, if c is already lowercase or not alphabetic.
 * 
*/
int toLower(int c);

/**
 * Converts the given character to its uppercase equivalent.
 * 
 * @param c The character to convert.
 * @return The uppercase equivalent of c; or the unchanged value of c, if c is already uppercase of not alphabetic.
 */
int toUpper(int c);

/**
 * Indicates whether a character is categorized as a decimal digit [0..9].
 * 
 * @param c The character to evaluate.
 * @return true if c is a decimal digit, otherwise, false.
*/
bool isDigitDec(int c);

/**
 * Indicates whether a character is categorized as a binary digit [0,1].
 * 
 * @param c The character to evaluate.
 * @return true if c is a binary digit, otherwise, false.
*/
bool isDigitBin(int c);

/**
 * Indicates whether a character is categorized as a hexadecimal digit [0..9 or a..f].
 * 
 * @param c The character to evaluate.
 * @return true if c is a hexadecimal digit, otherwise, false.
*/
bool isDigitHex(int c);

/**
 * Converts the given hexadecimal digit [0..9 or a..f] to it's equivalent decimal value.
 * 
 * @param c The character to convert.
 * @return The integer value of the hexad
 * 
*/
int hexToInt(int c);

#endif /* _CHAR_UTILS_H_ */