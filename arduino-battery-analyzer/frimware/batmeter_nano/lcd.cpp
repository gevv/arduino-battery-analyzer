/**
    HD44780-compatible LCD library - Arduino Nano port
    Original: (c) 2016 F. Štefanec
    License: Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported
**/

#include "lcd.h"

// Line-address offsets for 16x2, 16x4, 20x4 displays
const uint8_t lineaddrs[] = {
    0x00, 0x40,              // 16x2
    0x00, 0x40, 0x10, 0x50, // 16x4
    0x00, 0x40, 0x14, 0x54  // 20x4
};

void pulse_e()
{
    LCD_CPORT |=  (1 << LCD_E);
    delayMicroseconds(1);
    LCD_CPORT &= ~(1 << LCD_E);
    delayMicroseconds(1);
}

// Send 8-bit command over 4-bit interface (two nibbles)
void lcd_send(uint8_t command)
{
    // High nibble
    if (command & 0b00010000) LCD_DPORT |=  (1 << LCD_D4); else LCD_DPORT &= ~(1 << LCD_D4);
    if (command & 0b00100000) LCD_DPORT |=  (1 << LCD_D5); else LCD_DPORT &= ~(1 << LCD_D5);
    if (command & 0b01000000) LCD_DPORT |=  (1 << LCD_D6); else LCD_DPORT &= ~(1 << LCD_D6);
    if (command & 0b10000000) LCD_DPORT |=  (1 << LCD_D7); else LCD_DPORT &= ~(1 << LCD_D7);
    pulse_e();
    delayMicroseconds(50);

    // Low nibble
    if (command & 0b00000001) LCD_DPORT |=  (1 << LCD_D4); else LCD_DPORT &= ~(1 << LCD_D4);
    if (command & 0b00000010) LCD_DPORT |=  (1 << LCD_D5); else LCD_DPORT &= ~(1 << LCD_D5);
    if (command & 0b00000100) LCD_DPORT |=  (1 << LCD_D6); else LCD_DPORT &= ~(1 << LCD_D6);
    if (command & 0b00001000) LCD_DPORT |=  (1 << LCD_D7); else LCD_DPORT &= ~(1 << LCD_D7);
    pulse_e();
    delayMicroseconds(50);
}

// Send only high nibble (used during initialisation)
void lcd_send4h(uint8_t command)
{
    if (command & 0b00010000) LCD_DPORT |=  (1 << LCD_D4); else LCD_DPORT &= ~(1 << LCD_D4);
    if (command & 0b00100000) LCD_DPORT |=  (1 << LCD_D5); else LCD_DPORT &= ~(1 << LCD_D5);
    if (command & 0b01000000) LCD_DPORT |=  (1 << LCD_D6); else LCD_DPORT &= ~(1 << LCD_D6);
    if (command & 0b10000000) LCD_DPORT |=  (1 << LCD_D7); else LCD_DPORT &= ~(1 << LCD_D7);
    pulse_e();
    delayMicroseconds(50);
}

void lcd_init()
{
    lcd_send4h(LCD_INITSEQ);  delayMicroseconds(5000);
    lcd_send4h(LCD_INITSEQ);  delayMicroseconds(1000);
    lcd_send4h(LCD_INITSEQ);  delayMicroseconds(250);
    lcd_send4h(LCD_FUNCSET);  delayMicroseconds(50);
}

void lcd_rshigh()
{
    LCD_CPORT |= (1 << LCD_RS);
    delayMicroseconds(1);
}

void lcd_rslow()
{
    LCD_CPORT &= ~(1 << LCD_RS);
    delayMicroseconds(1);
}

void lcd_sendstring(char *in_string, uint8_t len)
{
    lcd_rshigh();
    delayMicroseconds(4);
    for (uint8_t i = 0; i < len; i++)
        lcd_send(in_string[i]);
    lcd_rslow();
}

void lcd_setcursor(uint8_t column, uint8_t line)
{
    lcd_rslow();
    lcd_send(LCD_SETDDADD + lineaddrs[line + DISPSIZE] + column);
}

void lcd_customchar(uint8_t addr, char *customchar)
{
    for (uint8_t bitctr = 0; bitctr < 8; bitctr++)
    {
        lcd_rslow();
        lcd_send(LCD_SETCGADD + addr * 8 + bitctr);
        lcd_rshigh();
        lcd_send(customchar[bitctr]);
    }
    lcd_rslow();
}

void lcd_clear()
{
    lcd_writecmd();
    lcd_send(LCD_CLEAR);
    delayMicroseconds(5000);
}

void lcd_printspaces(uint8_t len)
{
    lcd_writedata();
    for (uint8_t i = 0; i < len; i++)
        lcd_send(' ');
}
