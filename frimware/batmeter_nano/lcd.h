/**
    HD44780-compatible LCD library - Arduino Nano port
    Original: (c) 2016 F. Štefanec
    License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported
**/

#ifndef LCD_H_INCLUDED
#define LCD_H_INCLUDED

#include <Arduino.h>      // replaces <avr/io.h> and <util/delay.h>
#include "config.h"

// ─── LCD command definitions ──────────────────────────────────────────────────
#define LCD_INITSEQ             0b00110000
#define LCD_CLEAR               0b00000001
#define LCD_HOME                0b00000010
#define LCD_ENTRYMODE           0b00000100
#define LCD_ENTRYMODE_INCREMENT 0b00000010
#define LCD_ENTRYMODE_SHIFT     0b00000001
#define LCD_ONOFF               0b00001000
#define LCD_ONOFF_DISPLAY       0b00000100
#define LCD_ONOFF_CURSOR        0b00000010
#define LCD_ONOFF_BLINK         0b00000001
#define LCD_SHIFT               0b00010000
#define LCD_SHIFT_DISPSHIFT     0b00001000
#define LCD_SHIFT_RIGHT         0b00000100
#define LCD_FUNCSET             0b00100000
#define LCD_FUNCSET_LEN8B       0b00010000
#define LCD_FUNCSET_2LINES      0b00001000
#define LCD_FUNCSET_5X10DOTS    0b00000100
#define LCD_SETCGADD            0b01000000
#define LCD_SETDDADD            0b10000000

#define lcd_writedata()  lcd_rshigh()
#define lcd_writecmd()   lcd_rslow()

void pulse_e();
void lcd_send(uint8_t command);
void lcd_send4h(uint8_t command);
void lcd_init();
void lcd_rshigh();
void lcd_rslow();
void lcd_sendstring(char *in_string, uint8_t len);
void lcd_setcursor(uint8_t column, uint8_t line);
void lcd_customchar(uint8_t addr, char *customchar);
void lcd_clear();
void lcd_printspaces(uint8_t len);

#endif
