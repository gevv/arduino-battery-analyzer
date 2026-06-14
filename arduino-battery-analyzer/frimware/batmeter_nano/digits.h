#ifndef DIGITS_H_INCLUDED
#define DIGITS_H_INCLUDED

#include <Arduino.h>
// functions to get N-th digit from a numeric value as a char

char dig8_0(uint8_t num);
char dig8_1(uint8_t num);
char dig8_2(uint8_t num);

char dig16_0(uint16_t num);
char dig16_1(uint16_t num);
char dig16_2(uint16_t num);
char dig16_3(uint16_t num);

char dig32_0(unsigned long long num);
char dig32_1(unsigned long long num);
char dig32_2(unsigned long long num);
char dig32_3(unsigned long long num);
char dig32_4(unsigned long long num);
char dig32_5(unsigned long long num);
char dig32_6(unsigned long long num);
char dig32_7(unsigned long long num);

#endif
