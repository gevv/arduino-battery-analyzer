/**
Battery Analyzer v1.4.0 — Arduino Nano port

Original code: (c) 2016-2017 F. Štefanec

Arduino Nano port: 2026

This firmware is an Arduino Nano port / modified version of the
original rechargeable battery analyzer project.

Original project page: https://tefatronix.g6.cz/display.php?page=batmeter

Original author: F. Štefanec

Original code license: GNU GPL v2 or later, unless otherwise noted by the original author.

Related article/project page: https://320volt.com/arduino-pil-analiz-cihazi

Hardware note:
  Hardware connections are intended to match the original ATmega328P
  design. Arduino Nano uses the same ATmega328P MCU, so register names
  such as TCCR0B, OCR2A, ADMUX, etc. remain unchanged.

Changes from original:
  - main() -> setup() + loop()
  - _delay_ms/_delay_us -> wait_ms()/delayMicroseconds()
  - avr/io.h / util/delay.h -> Arduino.h
  - avr/eeprom.h functions are still used directly
  - avr/pgmspace.h PROGMEM is still used directly
  - Watchdog: wdt.h included for Arduino-safe macros
  - ISR declarations retained as-is

Pin summary (Arduino Nano digital / analog labels):
  D2  = LCD RS
  D3  = LCD E
  D4-D7 = LCD data

  D8  = BTN OK
  D9  = BTN DOWN
  D10 = BTN UP
  D12 = BTN ESC

  D11 = PWM output (OC2A, PB3)
  D13 = Buzzer (PB5) - shares built-in LED

  A0  = Battery voltage ADC
  A1  = Supply voltage ADC
  A2  = INHIBIT output
  A3  = Temperature (LM35) ADC
  A4  = FAN output
  A5  = Power LED output

Notes: This version has been adapted for Arduino Nano and may include
  software changes from the original project.


*/


// ─── Core AVR headers (available inside Arduino framework) ───────────────────
#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/atomic.h>


// Save reset cause and disable watchdog as early as possible.
// This is important on Arduino Nano/Optiboot-style bootloaders when WDT is used.
uint8_t mcusr_mirror __attribute__((section(".noinit")));
void get_mcusr(void) __attribute__((section(".init3")));
void get_mcusr(void)
{
    mcusr_mirror = MCUSR;
    MCUSR = 0;
    wdt_disable();
}

// ─── Project headers ─────────────────────────────────────────────────────────
#include "config.h"
#include "global_defines.h"
#include "lcd.h"
#include "string_table.h"
#include "lcd_customchars.h"
#include "digits.h"
#include "menu.h"

// ─── Omega character position in CGRAM ───────────────────────────────────────
#define OMEGA_CHAR 5

// ════════════════════════════════════════════════════════════════════════════
//  GLOBAL VARIABLES  (identical to original main.h)
// ════════════════════════════════════════════════════════════════════════════

uint8_t x, y, z;

uint8_t ad_preavg_count;
volatile uint8_t stat, stat2;
volatile uint8_t discharge_stop_request = 0;
volatile uint8_t temp_fault_request = 0;
uint8_t settings;
uint8_t submenu_stat = 0;
uint8_t field = 0;
uint8_t menu_pos = 1, old_menu_pos = 1, menu_pos2, old_menu_pos2;
volatile uint8_t beep_counter, old_beep_counter;
uint8_t rin_current_choice = 0;
const uint8_t rin_current1[10] PROGMEM = {0,  25,   0,  50,   0, 100, 150,   0,  50,   0};
const uint8_t rin_current2[10] PROGMEM = {25, 50,  50, 100, 100, 200, 250, 200, 250, 250};
uint8_t led_state1 = 2;
uint8_t led_state2 = 0;
volatile uint16_t led_blink_time = 0;
uint8_t last_option = 0;
volatile uint8_t fan_control = 0;

volatile uint8_t  seconds;
volatile uint8_t  minutes;
volatile uint8_t  hours;
unsigned long long elapsed_secs = 0;
uint8_t tim0_sw_pre;
uint8_t tim0_even = 0;
uint16_t tim0_blink_counter = 0;

uint8_t buttons_old_pin;
volatile uint8_t buttons;
uint8_t buttons_old;
uint8_t buttons_pressed;
uint8_t button_repeat_counter;
uint8_t btn_debounce = 0;

volatile uint8_t batt_icon = 0;
char buf[20];
char disch_power_str[8];
uint8_t display_mode;

uint16_t end_voltage   = DEFAULT_TERM_VOLTAGE;
uint16_t term_voltage  = DEFAULT_TERM_VOLTAGE;
volatile uint16_t adc_voltage       = 0;
uint16_t disp_voltage  = 0;
volatile uint16_t supp_voltage      = 0;
volatile uint16_t temperature_value = 0;
uint16_t volt_buf[64];
uint8_t  ad_buf_ptr = 0;
uint16_t ad_avg_buf[32];
uint8_t  ad_avg_buf_ptr;
uint8_t  ad_throwaway;
uint8_t  pwm_current = 1;
uint16_t v1, v2, i1, i2 = 25, rin;

unsigned long long mAs    = 0;
unsigned long long mAh100 = 0;
unsigned long long mWsx10 = 0;

int8_t offset_comp;

// ════════════════════════════════════════════════════════════════════════════
//  FORWARD DECLARATIONS
// ════════════════════════════════════════════════════════════════════════════

void terminate_discharge();
void print_discharge_result();
void calculate_mAh();
void calculate_mWsx10();
void mAh_string(char *buf);
void mWh_string(char *buf);
void time_string(char *buf);
void voltage_string(char *buf, uint16_t voltage);
void adjust_term_voltage(int16_t delta);
void current_string(char *buf, uint8_t current);
void rin_string(char *buf, uint16_t rin_centi_ohm);
void watt_string(char *buf, uint16_t voltage, uint8_t current);
void display_prog();
void display_ccload();
void enable_1s_timer();
void fast_1s_timer();
void disable_1s_timer();
void set_current(uint8_t current);
void rin_test();
void lcd_print_pmem(const char *data, uint8_t length);
void lcd_print_pmem_setpos(const char *data, uint8_t len, uint8_t column, uint8_t line);
uint8_t get_buttons();
void clear_time();
void enable_wdt();
void disable_wdt();
void deconf_regs();
void halt_error();
void cap_submenu();
void cc_submenu();
void rin_submenu();
void settings_submenu();
void escape_to_menu();
void print_menu(const char **items, uint8_t position);
void service_menu();
void disp_menu();
void print_settings_submenu(uint8_t cursor_pos);
void calib_menu();
void save_eeprom();
void set_offset_comp();
void load_eeprom();
void print_temp_error();

// Busy-wait delay that does not depend on Arduino Timer0 interrupts.
// The sketch reconfigures Timer0 for its own 2 kHz ISR, so Arduino delay()/millis()
// must not be used in this project.
static void wait_ms(uint16_t ms)
{
    while (ms--) {
        delayMicroseconds(1000);
    }
}

// ════════════════════════════════════════════════════════════════════════════
//  INTERRUPT SERVICE ROUTINES
// ════════════════════════════════════════════════════════════════════════════

/** Timer 1 Compare ISR — 1 Hz time counter **/
ISR(TIMER1_COMPA_vect)
{
    if (stat & stat_disch_enabled)
    {
        stat |= stat_display_prog;
        seconds++;
        if (seconds >= 60) { seconds = 0; minutes++; }
        if (minutes >= 60) { minutes = 0; hours++;   }
        if (hours == 255)  { stat &= ~stat_disch_enabled; stat |= stat_term_wait; }
    }
    if (stat & stat_ccload_enabled) stat |= stat_ccload_display;
    batt_icon++;
    if (batt_icon > 4) batt_icon = 0;
}

/** ADC conversion complete ISR **/
ISR(ADC_vect)
{
    if (ad_throwaway == 0)
    {
        ad_avg_buf[ad_avg_buf_ptr] = ADC;
        ad_avg_buf_ptr++;

        if (ad_avg_buf_ptr >= ad_preavg_count)
        {
            ad_avg_buf_ptr = 0;
            uint8_t  array_ptr, new_admux;
            uint16_t ADC_VALUE = 0;
            uint8_t  i;

            for (i = 0; i < ad_preavg_count; i++) ADC_VALUE += ad_avg_buf[i];
            ADC_VALUE /= ad_preavg_count;

            array_ptr = ad_buf_ptr;
            new_admux = ADMUX & ~((1<<MUX0)|(1<<MUX1)|(1<<MUX2)|(1<<MUX3));

            if (array_ptr < 32)
            {
                volt_buf[array_ptr] = ADC_VALUE;
                uint8_t  j = 0;
                uint16_t adc_volt_temp = 0;
                for (j = 0; j < 32; j++) adc_volt_temp += volt_buf[j];
                adc_voltage = adc_volt_temp;

                if (offset_comp != 0)
                {
                    uint8_t abs_val = (offset_comp < 0) ? (uint8_t)(-1 * offset_comp) : (uint8_t)(offset_comp);
                    if ((offset_comp != (int8_t)abs_val) && (adc_voltage > abs_val))
                        adc_voltage -= abs_val;
                    else
                        adc_voltage += offset_comp;
                }
                adc_voltage /= 16;
                if (adc_voltage > 2500) adc_voltage = 0;
                if (offset_comp > 0 && adc_voltage <= 1 + offset_comp / 16) adc_voltage = 0;
            }
            if (array_ptr == 32) supp_voltage      = ADC_VALUE;
            if (array_ptr == 33) temperature_value = ADC_VALUE;

            ad_buf_ptr++;
            if (ad_buf_ptr == 34) ad_buf_ptr = 0;

            if (ad_buf_ptr == 0)  { new_admux |= MUX_BATT_VOLTAGE;    ad_throwaway = 5; ADMUX = new_admux; }
            if (ad_buf_ptr == 32) { new_admux |= MUX_SUPPLY_VOLTAGE;  ad_throwaway = 5; ADMUX = new_admux; }
            if (ad_buf_ptr == 33) { new_admux |= MUX_TEMPERATURE;     ad_throwaway = 5; ADMUX = new_admux; }
        }
    }
    else
    {
        ad_throwaway--;
        delayMicroseconds(1);
    }
}

/** Timer 0 Compare ISR — 2 kHz: button scan, buzzer, LED blink, fan, ADC trigger **/
ISR(TIMER0_COMPA_vect)
{
    tim0_sw_pre++;
    if (tim0_sw_pre > 7)
    {
        z  = ~BTN_PIN;
        z &= (1<<BTN_ESC_PIN)|(1<<BTN_OK_PIN)|(1<<BTN_DOWN_PIN)|(1<<BTN_UP_PIN);
        y  = 0;
             if ((z & (1<<BTN_ESC_PIN))  != 0) y |= BTN_ESC;
        else if ((z & (1<<BTN_UP_PIN))   != 0) y |= BTN_UP;
        else if ((z & (1<<BTN_DOWN_PIN)) != 0) y |= BTN_DOWN;
        else if ((z & (1<<BTN_OK_PIN))   != 0) y |= BTN_OK;

        if (btn_debounce == 0) { buttons = y; btn_debounce = DEBOUNCE_VAL; }
        else btn_debounce--;

        if (buttons > 0 && (buttons != buttons_old_pin) && beep_counter == 0)
            beep_counter = BEEP_LEN_BUTTON;
        buttons_old_pin = buttons;

        if (y == 0) { button_repeat_counter = 0; }
        else
        {
            button_repeat_counter++;
            if (button_repeat_counter == REPEAT_DELAY)
            {
                buttons_old = 0;
                button_repeat_counter -= REPEAT_PERIOD;
                beep_counter = BEEP_LEN_REPEAT;
            }
        }

        if ((stat & stat_disch_enabled) > 0 && (adc_voltage <= term_voltage))
        {
            // Do not run LCD/calculation routines inside the ISR.
            // Main loop will stop the discharge safely.
            discharge_stop_request = 1;
        }

        if (beep_counter > 0) beep_counter--;
        tim0_sw_pre = 0;
    }

    // Buzzer
    if      (beep_counter == 0)                         BUZZER_PORT &= ~(1<<BUZZER_PIN);
    else if (settings & settings_sound_on)              BUZZER_PORT ^=  (1<<BUZZER_PIN);

    delayMicroseconds(1);

    // Trigger next ADC conversion
    ADCSRA |= (1<<ADSC);

    // Power LED blink
    tim0_blink_counter++;
    if (tim0_blink_counter >= 1000) tim0_blink_counter = 0;
    tim0_even++;
    if (tim0_even >= 2) tim0_even = 0;

    if (led_blink_time <= tim0_blink_counter)
    {
        if (led_state1 == 0 || (led_state1 == 1 && tim0_even == 1)) LED_PORT &= ~(1<<LED_PIN);
        else                                                          LED_PORT |=  (1<<LED_PIN);
    }
    else
    {
        if (led_state2 == 0 || (led_state2 == 1 && tim0_even == 1)) LED_PORT &= ~(1<<LED_PIN);
        else                                                          LED_PORT |=  (1<<LED_PIN);
    }

    // Temperature-based fan control
#ifdef LM35_INSTALLED
    if (temperature_value < TEMP_AD_FAN_STOP) fan_control &= ~2;
    if (temperature_value > TEMP_AD_FAN_START) fan_control |= 2;
    if (temperature_value > TEMP_AD_FAULT)
    {
        // Keep the ISR short: stop load/fan on immediately, show error in loop().
        set_current(0);
        FAN_PORT |= (1<<FAN_PIN);
        temp_fault_request = 1;
    }
    if ((fan_control & 2) != 0) FAN_PORT |= (1<<FAN_PIN); else FAN_PORT &= ~(1<<FAN_PIN);
#else
    if ((fan_control & 1) != 0) FAN_PORT |= (1<<FAN_PIN); else FAN_PORT &= ~(1<<FAN_PIN);
#endif
}

// ════════════════════════════════════════════════════════════════════════════
//  ARDUINO setup()  —  replaces original int main()  initialisation block
// ════════════════════════════════════════════════════════════════════════════
void setup()
{
    cli();

    // Port initialisation
    PORTD = 0; PORTC = 0; PORTB = 0;
    DDRD  = 0; DDRC  = 0; DDRB  = 0;

    // Button pull-ups
    BTN_PORT |= (1<<BTN_ESC_PIN)|(1<<BTN_DOWN_PIN)|(1<<BTN_UP_PIN)|(1<<BTN_OK_PIN);

    // Outputs
    FAN_DDR    |= (1<<FAN_PIN);
    BUZZER_DDR |= (1<<BUZZER_PIN);
    LCD_DDDR   |= (1<<LCD_D7)|(1<<LCD_D6)|(1<<LCD_D5)|(1<<LCD_D4);
    LCD_CDDR   |= (1<<LCD_E)|(1<<LCD_RS);
    INHIBIT_PORT |= (1<<INHIBIT_PIN);
    INHIBIT_DDR  |= (1<<INHIBIT_PIN);
    DDRB |= (1<<PB3);   // PWM pin (OC2A)
    LED_DDR  |= (1<<LED_PIN);
    LED_PORT |= (1<<LED_PIN);   // keep power LED in its initial ON state

    load_eeprom();
    ad_preavg_count = CAP_PREAVG_COUNT;

    // LCD init
    lcd_init();
    lcd_send(LCD_FUNCSET | LCD_FUNCSET_2LINES);
    lcd_send(LCD_ONOFF   | LCD_ONOFF_DISPLAY);
    lcd_send(LCD_CLEAR);
    wait_ms(5);
    lcd_send(LCD_ENTRYMODE | LCD_ENTRYMODE_INCREMENT);

    // Watchdog reset check
    if (mcusr_mirror & (1<<WDRF))
    {
        set_current(0);
        disable_1s_timer();
        lcd_clear();
        wdt_disable();
        lcd_print_pmem_setpos(str_em_shdn,    20, 0, 0);
        lcd_print_pmem_setpos(str_em_shdn_wd0, 20, 0, 1);
        lcd_print_pmem_setpos(str_em_shdn_wd1, 20, 0, 2);
        mcusr_mirror &= ~(1<<WDRF);
        halt_error();
    }

    // Custom characters
    lcd_customchar(0, (char*)customchar1);
    lcd_customchar(1, (char*)customchar2);
    lcd_customchar(2, (char*)customchar3);
    lcd_customchar(3, (char*)customchar4);
    lcd_customchar(4, (char*)customchar5);
    lcd_customchar(5, (char*)customchar6);

    // Welcome screen
    lcd_print_pmem_setpos(str_welcome1,        20, 0, 0);
    lcd_print_pmem_setpos(str_welcome2,        20, 0, 1);
    lcd_print_pmem_setpos(str_firmware_version,20, 0, 3);
    for (z = 0; z < 250; z++) wait_ms(8);   // ~2 s

    beep_counter = BEEP_LEN_START;
    lcd_clear();

    // Timer 0 — 2 kHz CTC (button scan, buzzer)
    TCCR0B = (1<<CS01)|(1<<CS00);   // prescaler 1:64
    TCCR0A = (1<<WGM01);            // CTC mode
    OCR0A  = 124;                   // 16M / 64 / 125 = 2000 Hz

    // Timer 1 — elapsed time base. It is configured here but its interrupt
    // is enabled only when a capacity / CC-load test actually starts.
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A  = TIME_PRESCALER_1000ms; // 16M / 256 / 62500 = 1 Hz
    TCCR1B = (1<<WGM12)|(1<<CS12);  // CTC, prescaler 1:256

    TIMSK0 = (1<<OCIE0A);
    TIMSK1 = 0;                     // enabled by enable_1s_timer()

    // Timer 2 — PWM for load current control (OC2A = PB3 = D11)
    OCR2A  = 0;
    TCCR2A = (1<<COM2A1)|(1<<WGM20); // phase-correct PWM, non-inverting
    TCCR2B = (1<<CS20);              // no prescaler, phase-correct PWM → ~31.37 kHz

    // ADC setup — internal 1.1 V reference, interrupt-driven
    ADMUX  = (1<<REFS0)|(1<<REFS1);  // 1.1 V ref, ADC0 input
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADIE);
    ADCSRA |= (1<<ADSC);             // start first conversion

    LED_DDR |= (1<<LED_PIN);
    led_state1     = 2;
    led_state2     = 0;
    led_blink_time = 0;

    sei();
    wait_ms(5);

    lcd_clear();

    // Show main menu
    stat  |= stat_inside_menu;
    stat2 |= stat2_redraw_menu;
    lcd_print_pmem_setpos(str_select0, 10, 5, 0);
    set_current(0);
}

// ════════════════════════════════════════════════════════════════════════════
//  ARDUINO loop()  —  replaces original while(1) main loop
// ════════════════════════════════════════════════════════════════════════════
void loop()
{
#ifdef LM35_INSTALLED
    if (temp_fault_request)
    {
        cli();
        temp_fault_request = 0;
        sei();
        wdt_disable();
        deconf_regs();
        FAN_PORT |= (1<<FAN_PIN);
        print_temp_error();
        halt_error();
    }
#endif

    if (discharge_stop_request)
    {
        cli();
        discharge_stop_request = 0;
        sei();
        if (stat & stat_disch_enabled)
        {
            terminate_discharge();
        }
    }

    if (stat2 & stat2_redraw_menu)
    {
        lcd_print_pmem_setpos(str_select0, 10, 5, 0);
        lcd_setcursor(2,1); strcpy_P(buf, str_menu_items[menu_pos2+0]); lcd_sendstring(buf, 18);
        lcd_setcursor(2,2); strcpy_P(buf, str_menu_items[menu_pos2+1]); lcd_sendstring(buf, 18);
        lcd_setcursor(2,3); strcpy_P(buf, str_menu_items[menu_pos2+2]); lcd_sendstring(buf, 18);
        stat2 &= ~stat2_redraw_menu;
    }

    if (stat & stat_inside_menu)
    {
        led_blink_time = 0;
        if (OCR2A != 0) set_current(0);
        disp_menu();
    }

    if (stat & stat_display_prog)
    {
        led_blink_time = 200;
        led_state2 = 0;
        calculate_mAh();
        calculate_mWsx10();
        display_prog();
        stat &= ~(stat_display_prog);
    }

    if (stat & stat_ccload_display)
    {
        led_blink_time = 200;
        led_state2 = 0;
        display_ccload();
        stat &= ~(stat_ccload_display);
    }

    if (stat & stat_term_wait)
    {
        led_blink_time = 500;
        led_state2 = 0;
        buttons_pressed = get_buttons();
        if (buttons_pressed & BTN_ESC)
        {
            disable_wdt();
            lcd_clear();
            print_menu(str_menu_items, menu_pos2);
            stat &= ~(stat_term_wait);
            stat |= stat_inside_menu;
        }
        if ((buttons_pressed & BTN_OK || buttons_pressed & BTN_UP || buttons_pressed & BTN_DOWN) && (last_option == 0))
        {
            display_mode = display_mode == 1 ? 0 : 1;
            print_discharge_result();
        }
    }

    if (stat & (stat_ccload_enabled | stat_disch_enabled))
    {
        watt_string(disch_power_str, disp_voltage, pwm_current);
        buttons_pressed = get_buttons();

        if (buttons_pressed & BTN_ESC)
        {
            set_current(0);
            if (stat & stat_ccload_enabled)
            {
                stat &= ~stat_ccload_enabled;
                disable_wdt();
                escape_to_menu();
            }
            else if (stat & stat_disch_enabled)
            {
                terminate_discharge();
            }
        }

        if ((buttons_pressed & BTN_OK) && (stat & stat_ccload_enabled))
            display_mode = display_mode == 1 ? 0 : 1;

        else if ((buttons_pressed & (BTN_UP|BTN_DOWN)) && (stat & stat_ccload_enabled))
        {
            if ((buttons_pressed & BTN_UP)   && pwm_current < 255) { set_current(0); pwm_current++; set_current(pwm_current); }
            if ((buttons_pressed & BTN_DOWN) && pwm_current > 1)   { set_current(0); pwm_current--; set_current(pwm_current); }
            display_ccload();
        }

        if ((stat & stat_disch_enabled) && (buttons_pressed & BTN_OK || buttons_pressed & BTN_UP || buttons_pressed & BTN_DOWN))
        {
            display_mode = display_mode == 1 ? 0 : 1;
            display_prog();
        }
    }

    wdt_reset();   // replaces asm("WDR")
}

// ════════════════════════════════════════════════════════════════════════════
//  HELPER / UTILITY FUNCTIONS  (identical logic to original)
// ════════════════════════════════════════════════════════════════════════════

void set_current(uint8_t current)
{
    OCR2A = current;
    if (current == 0) { INHIBIT_PORT |= (1<<INHIBIT_PIN); fan_control &= ~1; }
    else              { INHIBIT_PORT &= ~(1<<INHIBIT_PIN); fan_control |= 1; }
}

void halt_error()
{
    cli();
    set_current(0);
    disable_1s_timer();
    disable_wdt();
    while (1);
}

void clear_time() { hours = 0; minutes = 0; seconds = 0; }

void enable_wdt()
{
    wdt_reset();
    wdt_enable(WDTO_500MS);
}

void disable_wdt()
{
    wdt_reset();
    wdt_disable();
}

void enable_1s_timer()
{
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A  = TIME_PRESCALER_1000ms;
    TIFR1  = (1<<OCF1A);            // clear a pending compare flag
    TCCR1B = (1<<WGM12)|(1<<CS12);  // CTC, prescaler 1:256
    TIMSK1 |= (1<<OCIE1A);
    sei();
}

void fast_1s_timer()
{
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A  = TIME_PRESCALER_250ms;
    TIFR1  = (1<<OCF1A);
    TCCR1B = (1<<WGM12)|(1<<CS12);
    TIMSK1 |= (1<<OCIE1A);
    sei();
}

void disable_1s_timer()
{
    cli();
    TIMSK1 &= ~(1<<OCIE1A);
    TCCR1B &= ~((1<<CS12)|(1<<CS11)|(1<<CS10)); // stop Timer1 clock
    TIFR1 = (1<<OCF1A);
    sei();
}

void deconf_regs()
{
    TCCR0B=0; TCCR0A=0; OCR0A=0;
    TCCR1B=0; OCR1A=0;
    TIMSK0=0; TIMSK1=0;
    OCR2A=0;  TCCR2A=0; TCCR2B=0;
    ADMUX=0;  ADCSRA=0;
    PORTB=0;  DDRB=0;
    INHIBIT_DDR  |= (1<<INHIBIT_PIN);
    INHIBIT_PORT |= (1<<INHIBIT_PIN);
    FAN_DDR  |= (1<<FAN_PIN);
    FAN_PORT |= (1<<FAN_PIN);
}

/** Print from PROGMEM **/
void lcd_print_pmem(const char *data, uint8_t len)
{
    for (int n = 0; n < len; n++) buf[n] = pgm_read_byte_near(data + n);
    lcd_sendstring(buf, len);
}

void lcd_print_pmem_setpos(const char *data, uint8_t len, uint8_t column, uint8_t line)
{
    for (int n = 0; n < len; n++) buf[n] = pgm_read_byte_near(data + n);
    lcd_setcursor(column, line);
    lcd_sendstring(buf, len);
}

uint8_t get_buttons()
{
    uint8_t btns = (buttons ^ buttons_old) & buttons;
    buttons_old = buttons;
    return btns;
}

// ─── String / display helpers ────────────────────────────────────────────────

void mAh_string(char *buf)
{
    buf[0]=dig32_7(mAh100); buf[1]=dig32_6(mAh100); buf[2]=dig32_5(mAh100);
    buf[3]=dig32_4(mAh100); buf[4]=dig32_3(mAh100); buf[5]=dig32_2(mAh100);
    buf[6]='.'; buf[7]=dig32_1(mAh100); buf[8]=dig32_0(mAh100);
}

void mWh_string(char *buf)
{
    unsigned long long mWh = mWsx10 / 36000ULL;
    buf[0]=dig32_7(mWh); buf[1]=dig32_6(mWh); buf[2]=dig32_5(mWh);
    buf[3]=dig32_4(mWh); buf[4]=dig32_3(mWh); buf[5]=dig32_2(mWh);
    buf[6]=dig32_1(mWh); buf[7]=dig32_0(mWh); buf[8]=' ';
}

void watt_string(char *buf, uint16_t voltage, uint8_t current)
{
    unsigned long long wattage = (unsigned long long)voltage * (unsigned long long)current;
    buf[0]=dig32_5(wattage); buf[1]=dig32_4(wattage); buf[2]='.';
    buf[3]=dig32_3(wattage); buf[4]=dig32_2(wattage);
    buf[5]=dig32_1(wattage); buf[6]=dig32_0(wattage); buf[7]='W';
}

void voltage_string(char *buf, uint16_t voltage)
{
    buf[0]=dig16_3(voltage); buf[1]=dig16_2(voltage); buf[2]='.';
    buf[3]=dig16_1(voltage); buf[4]=dig16_0(voltage); buf[5]='V';
}

void adjust_term_voltage(int16_t delta)
{
    int32_t next = (int32_t)term_voltage + (int32_t)delta;

    if (next < MIN_TERM_VOLTAGE) next = MIN_TERM_VOLTAGE;
    if (next > MAX_TERM_VOLTAGE) next = MAX_TERM_VOLTAGE;

    term_voltage = (uint16_t)next;
}

void current_string(char *buf, uint8_t current)
{
    buf[0]=dig8_2(current); buf[1]='.';
    buf[2]=dig8_1(current); buf[3]=dig8_0(current); buf[4]='A';
}

void rin_string(char *buf, uint16_t rin_centi_ohm)
{
    // rin_centi_ohm is in 0.01 ohm units.
    // Example: 17 = 0.17 ohm = 170 milliohm.
    // Show small/internal-resistance values as mΩ for easier battery comparison.
    if (rin_centi_ohm <= 999)
    {
        uint16_t rin_milliohm = rin_centi_ohm * 10U;
        buf[0]=dig16_3(rin_milliohm); buf[1]=dig16_2(rin_milliohm);
        buf[2]=dig16_1(rin_milliohm); buf[3]=dig16_0(rin_milliohm);
        buf[4]='m'; buf[5]=OMEGA_CHAR;
    }
    else
    {
        // Fall back to ohms for unusually high values so the display does not overflow.
        buf[0]=dig16_3(rin_centi_ohm); buf[1]=dig16_2(rin_centi_ohm); buf[2]='.';
        buf[3]=dig16_1(rin_centi_ohm); buf[4]=dig16_0(rin_centi_ohm); buf[5]=OMEGA_CHAR;
    }
}

void time_string(char *buf)
{
    buf[0]=dig8_2(hours);   buf[1]=dig8_1(hours);   buf[2]=dig8_0(hours);   buf[3]=':';
    buf[4]=dig8_1(minutes); buf[5]=dig8_0(minutes);  buf[6]=':';
    buf[7]=dig8_1(seconds); buf[8]=dig8_0(seconds);
}

void calculate_mAh()
{
    elapsed_secs = ((hours * 60U + minutes) * 60UL) + seconds;
    mAs    = pwm_current;
    mAs   *= 10ULL;
    mAs   *= (unsigned long long)elapsed_secs;
    mAh100 = mAs / 36ULL;
}

void calculate_mWsx10()
{
    unsigned long long add_val = (unsigned long long)(adc_voltage) * (unsigned long long)(pwm_current);
    mWsx10 += add_val;
}

// ─── Display routines ─────────────────────────────────────────────────────────

void display_prog()
{
    lcd_print_pmem_setpos(elapsed, 9, 0, 0);
    time_string(buf); lcd_sendstring(buf, 9);
    lcd_print_pmem_setpos(str_voltage, 9, 0, 1);
    voltage_string(buf, adc_voltage); lcd_sendstring(buf, 6);
    if (display_mode == 0)
    {
        lcd_print_pmem_setpos(str_current, 9, 0, 2);
        current_string(buf, pwm_current); lcd_sendstring(buf, 5); lcd_printspaces(3);
        lcd_print_pmem_setpos(str_mAh, 5, 0, 3);
        mAh_string(buf); lcd_sendstring(buf, 9);
    }
    else
    {
        lcd_print_pmem_setpos(str_power, 9, 0, 2);
        watt_string(disch_power_str, adc_voltage, pwm_current); lcd_sendstring(disch_power_str, 8);
        lcd_print_pmem_setpos(str_mWh, 5, 0, 3);
        mWh_string(buf); lcd_sendstring(buf, 9);
    }
    lcd_setcursor(19, 3); lcd_writedata(); lcd_send(batt_icon);
}

void display_ccload()
{
    lcd_print_pmem_setpos(str_voltage, 9, 0, 0);
    disp_voltage = adc_voltage;
    voltage_string(buf, disp_voltage); lcd_sendstring(buf, 6);
    if (disp_voltage < 80) lcd_print_pmem(str_low, 5); else lcd_printspaces(5);
    lcd_print_pmem_setpos(str_current, 9, 0, 1);
    current_string(buf, pwm_current); lcd_sendstring(buf, 5);
    lcd_print_pmem_setpos(str_power, 7, 0, 2);
    lcd_sendstring(disch_power_str, 8);
}

void terminate_discharge()
{
    end_voltage = adc_voltage;
    set_current(0);
    disable_1s_timer();
    disable_wdt();
    calculate_mAh();
    stat &= ~stat_disch_enabled;
    lcd_clear();
    print_discharge_result();
    beep_counter = BEEP_LEN_FINISHED;
    set_current(0);
}

void print_discharge_result()
{
    lcd_setcursor(0,0); lcd_print_pmem(str_terminated, 13);
    voltage_string(buf, end_voltage); lcd_sendstring(buf, 6);
    if (display_mode == 0)
    {
        lcd_setcursor(0,1); mAh_string(buf); lcd_sendstring(buf, 9);
        lcd_print_pmem(str_mAh_fin, 6);
        current_string(buf, pwm_current); lcd_sendstring(buf, 5);
    }
    else
    {
        lcd_setcursor(0,1); mWh_string(buf); lcd_sendstring(buf, 9);
        lcd_print_pmem(str_mWh_fin, 6);
        current_string(buf, pwm_current); lcd_sendstring(buf, 5);
    }
    lcd_print_pmem_setpos(elapsed, 9, 0, 2);
    time_string(buf); lcd_sendstring(buf, 9);
    lcd_print_pmem_setpos(str_esc_exit, 20, 0, 3);
    stat &= ~stat_disch_enabled;
    stat |= stat_term_wait;
}

// ─── Internal resistance test ────────────────────────────────────────────────

void rin_test()
{
    led_blink_time = 500;
    led_state2 = 1;
    ad_preavg_count = RIN_PREAVG_COUNT;
    lcd_clear(); lcd_print_pmem(str_rin_wait, 20);

    i1 = pgm_read_byte_near(rin_current1 + rin_current_choice);
    i2 = pgm_read_byte_near(rin_current2 + rin_current_choice);
    uint16_t v1avg = 0, v2avg = 0;

    set_current(i1);
    for (x = 0; x < 200; x++) wait_ms(5);

    uint8_t runs;
    for (runs = 0; runs < 8; runs++)
    {
        for (x = 0; x < 100; x++) wait_ms(5);
        v1 = adc_voltage; v1avg += v1;
        if (v1 < 80 || v1 > 2100) { v1avg = 0; v2avg = 0; break; }
        set_current(i2);
        for (x = 0; x < 100; x++) wait_ms(5);
        v2 = adc_voltage; v2avg += v2;
        if (v2 < 80 || v2 > 2100) { v1avg = 0; v2avg = 0; break; }
        set_current(i1);
    }
    set_current(0);
    v1avg /= 8; v2avg /= 8; v1 = v1avg; v2 = v2avg;

    if (v1avg < 10 || v2avg < 10)
    {
        lcd_clear();
        lcd_print_pmem_setpos(str_error,        5, 0, 0);
        lcd_print_pmem_setpos(str_rin_psu_weak, 16, 0, 1);
        lcd_print_pmem_setpos(str_esc_exit,     20, 0, 3);
    }
    else
    {
        lcd_clear();
        rin = (v1 >= v2) ? v1 - v2 : v2 - v1;
        rin = (((unsigned long long)rin * 100) / (i2 - i1));
        lcd_print_pmem_setpos(str_rin_resistance, 12, 0, 0);
        rin_string(buf, rin);
        if (v2 > v1) lcd_sendstring((char*)"-", 1);
        lcd_sendstring(buf, 6);

        lcd_setcursor(0,1);
        lcd_print_pmem(str_rin_v1, 3); voltage_string(buf, v1); lcd_sendstring(buf, 6);
        lcd_print_pmem(str_rin_v2, 4); voltage_string(buf, v2); lcd_sendstring(buf, 6);
        lcd_print_pmem_setpos(str_rin_i1, 3, 0, 2);
        current_string(buf, i1); buf[5]=' '; lcd_sendstring(buf, 6);
        lcd_print_pmem(str_rin_i2, 4); current_string(buf, i2); lcd_sendstring(buf, 5);
        lcd_print_pmem_setpos(str_esc_exit, 20, 0, 3);
    }

    led_blink_time = 0;
    stat &= ~stat_rin_test;
    stat |= stat_term_wait;
    ad_preavg_count = CAP_PREAVG_COUNT;
}

// ─── EEPROM ───────────────────────────────────────────────────────────────────

void load_eeprom()
{
    uint8_t ee_block[2];
    eeprom_read_block(ee_block, 0, 2);
    if (ee_block[0] != 255) { settings = ee_block[0]; wait_ms(3); offset_comp = 255 & ee_block[1]; wait_ms(3); }
    else { offset_comp = DEFAULT_OFFSET_COMP; settings = DEFAULT_SETTINGS; save_eeprom(); }
}

void save_eeprom()
{
    uint8_t old_ee_block[2], new_ee_block[2];
    new_ee_block[0] = settings;
    new_ee_block[1] = 255 & offset_comp;
    eeprom_read_block(old_ee_block, 0, 2);
    if (new_ee_block[0] != old_ee_block[0] || new_ee_block[1] != old_ee_block[1])
        { eeprom_write_block(new_ee_block, 0, 2); beep_counter = 100; }
    else { beep_counter = 2; }
    wait_ms(10);
}

void print_temp_error()
{
    lcd_clear();
    lcd_print_pmem_setpos(str_thermal_error_1, 20, 0, 0);
    lcd_print_pmem_setpos(str_thermal_error_2, 20, 0, 1);
}

// ─── Menu functions ───────────────────────────────────────────────────────────

void escape_to_menu()
{
    disable_wdt();
    wdt_reset();
    cli();
    set_current(0);
    disable_1s_timer();
    sei();
    lcd_writecmd(); lcd_send(LCD_ONOFF | LCD_ONOFF_DISPLAY);
    stat = 0; menu_pos = 1; menu_pos2 = 0;
    lcd_clear();
    print_menu(str_menu_items, menu_pos2);
    stat |= stat_inside_menu;
}

void print_menu(const char **items, uint8_t position)
{
    lcd_print_pmem_setpos(str_select0,        10, 5, 0);
    lcd_print_pmem_setpos(items[0+position],  18, 2, 1);
    lcd_print_pmem_setpos(items[1+position],  18, 2, 2);
    lcd_print_pmem_setpos(items[2+position],  18, 2, 3);
}

void disp_menu()
{
    buttons_pressed = get_buttons();
    if (menu_pos != old_menu_pos)
    {
        lcd_setcursor(0, old_menu_pos+1-menu_pos2); lcd_writedata(); lcd_send(' ');
        old_menu_pos = menu_pos;
    }
    lcd_setcursor(0, menu_pos+1-menu_pos2); lcd_writedata(); lcd_send('>');

    if ((buttons_pressed & BTN_UP)   && menu_pos > 0)
    {
        menu_pos--;
        if (menu_pos2 > menu_pos) { menu_pos2--; stat2 |= stat2_redraw_menu; }
    }
    if ((buttons_pressed & BTN_DOWN) && menu_pos < 3)
    {
        menu_pos++;
        if (menu_pos > menu_pos2 + 2) { menu_pos2++; stat2 |= stat2_redraw_menu; }
    }
    if (buttons_pressed & BTN_OK)
    {
        switch (menu_pos)
        {
            case 3: stat |= stat_inside_submenu; settings_submenu(); break;
            case 2: stat |= stat_inside_submenu; last_option=2; rin_submenu(); break;
            case 1: stat |= stat_inside_submenu; last_option=1; cc_submenu();  break;
            case 0: stat |= stat_inside_submenu; last_option=0; cap_submenu(); break;
            default: break;
        }
    }
}

void print_settings_submenu(uint8_t cursor_pos)
{
    lcd_clear();
    lcd_print_pmem_setpos(str_settings_items[0], 14, 2, 0);
    lcd_print_pmem_setpos(str_settings_items[1], 14, 2, 1);
    lcd_print_pmem_setpos(str_settings_items[2], 14, 2, 2);
    lcd_print_pmem_setpos(str_settings_items[3], 14, 2, 3);
    uint8_t st = (settings & settings_sound_on) ? 1 : 0;
    lcd_print_pmem_setpos(str_settings_onoff[st], 3, 17, 1);
    lcd_setcursor(0, cursor_pos); lcd_writedata(); lcd_send('>');
}

void settings_submenu()
{
    uint8_t settings_menu_pos=0, settings_menu_pos_old=0, settings_temp;
    print_settings_submenu(settings_menu_pos);
    settings_temp = (settings & settings_sound_on) ? 1 : 0;
    lcd_print_pmem_setpos(str_settings_onoff[settings_temp], 3, 17, 1);

    while (1)
    {
        buttons_pressed = get_buttons();
        if ((buttons_pressed & BTN_UP)   && settings_menu_pos > 0) settings_menu_pos--;
        if ((buttons_pressed & BTN_DOWN) && settings_menu_pos < 3) settings_menu_pos++;
        if (settings_menu_pos != settings_menu_pos_old)
        {
            lcd_setcursor(0, settings_menu_pos_old); lcd_writedata(); lcd_send(' ');
            lcd_setcursor(0, settings_menu_pos);     lcd_writedata(); lcd_send('>');
            settings_menu_pos_old = settings_menu_pos;
        }
        if (buttons_pressed & BTN_OK)
        {
            switch (settings_menu_pos)
            {
                case 0:
                    stat &= ~stat_inside_submenu; stat |= stat_inside_menu;
                    stat2 |= stat2_redraw_menu; lcd_clear(); return;
                case 1:
                    settings ^= settings_sound_on;
                    settings_temp = (settings & settings_sound_on) ? 1 : 0;
                    lcd_print_pmem_setpos(str_settings_onoff[settings_temp], 3, 17, 1);
                    break;
                case 2: set_offset_comp(); print_settings_submenu(settings_menu_pos); break;
                case 3: save_eeprom(); break;
                default: break;
            }
        }
    }
}

void rin_submenu()
{
    i1 = pgm_read_byte_near(rin_current1 + rin_current_choice);
    i2 = pgm_read_byte_near(rin_current2 + rin_current_choice);

    lcd_clear();
    lcd_print_pmem_setpos(str_rin_subm1, 20, 0, 0);
    lcd_print_pmem_setpos(str_rin_subm2, 14, 0, 1);
    current_string(buf, i1); lcd_sendstring(buf, 5);
    lcd_print_pmem_setpos(str_rin_subm3, 14, 0, 2);
    current_string(buf, i2); lcd_sendstring(buf, 5);
    lcd_print_pmem_setpos(str_rin_subm4, 20, 0, 3);

    while (1)
    {
        buttons_pressed = get_buttons();
        if (buttons_pressed & BTN_OK)  break;
        if (buttons_pressed & BTN_ESC) { escape_to_menu(); return; }
        if ((buttons_pressed & BTN_DOWN) || (buttons_pressed & BTN_UP))
        {
            if ((buttons_pressed & BTN_UP)   && rin_current_choice < 9) rin_current_choice++;
            if ((buttons_pressed & BTN_DOWN) && rin_current_choice > 0) rin_current_choice--;
            i1 = pgm_read_byte_near(rin_current1 + rin_current_choice);
            i2 = pgm_read_byte_near(rin_current2 + rin_current_choice);
            lcd_setcursor(14,1); current_string(buf, i1); lcd_sendstring(buf, 5);
            lcd_setcursor(14,2); current_string(buf, i2); lcd_sendstring(buf, 5);
        }
    }
    stat &= ~(stat_inside_submenu | stat_inside_menu);
    rin_test();
}

void cc_submenu()
{
    lcd_clear();
    lcd_print_pmem_setpos(str_ccl_subm1, 20, 0, 0);
    lcd_print_pmem_setpos(str_current, 9, 0, 1);
    current_string(buf, pwm_current); lcd_sendstring(buf, 5);
    submenu_stat = 1;
    uint8_t xpos;
    lcd_writecmd(); lcd_send(LCD_ONOFF | LCD_ONOFF_DISPLAY | LCD_ONOFF_BLINK);
    lcd_setcursor(12,1);

    while (1)
    {
        buttons_pressed = get_buttons();
        if (buttons_pressed & BTN_ESC)
        {
            submenu_stat--;
            if (submenu_stat == 0) { escape_to_menu(); break; }
            if (submenu_stat > 0 && submenu_stat < 4)
            {
                xpos = 13 - submenu_stat;
                if (submenu_stat >= 3) xpos--;
                lcd_setcursor(xpos, 1);
            }
        }
        if (buttons_pressed & BTN_OK)
        {
            submenu_stat++;
            if (submenu_stat < 4)
            {
                xpos = 13 - submenu_stat;
                if (submenu_stat >= 3) xpos--;
                lcd_setcursor(xpos, 1);
            }
            if (submenu_stat >= 4)
            {
                lcd_writecmd(); lcd_send(LCD_ONOFF | LCD_ONOFF_DISPLAY);
                lcd_clear(); lcd_print_pmem_setpos(str_esc_exit, 20, 0, 3);
                stat &= ~(stat_inside_submenu | stat_inside_menu);
                stat |= stat_ccload_enabled;
                enable_wdt(); set_current(pwm_current); enable_1s_timer(); return;
            }
        }
        if ((buttons_pressed & BTN_UP) || (buttons_pressed & BTN_DOWN))
        {
            if (buttons_pressed & BTN_UP)
            {
                if (submenu_stat==3 && pwm_current < 156) pwm_current += 100;
                if (submenu_stat==2 && pwm_current < 246) pwm_current += 10;
                if (submenu_stat==1 && pwm_current < 255) pwm_current += 1;
            }
            if (buttons_pressed & BTN_DOWN)
            {
                if (submenu_stat==3 && pwm_current > 100) pwm_current -= 100;
                if (submenu_stat==2 && pwm_current > 10)  pwm_current -= 10;
                if (submenu_stat==1 && pwm_current > 1)   pwm_current -= 1;
            }
            lcd_setcursor(9,1); current_string(buf, pwm_current); lcd_sendstring(buf, 5);
            xpos = 13 - submenu_stat;
            if (submenu_stat == 3) xpos--;
            lcd_setcursor(xpos, 1);
        }
    }
}

void cap_submenu()
{
    lcd_clear();
    lcd_print_pmem_setpos(str_submenu_disch, 20, 0, 0);
    lcd_print_pmem_setpos(str_current, 9, 0, 1);
    current_string(buf, pwm_current); lcd_sendstring(buf, 5);
    lcd_print_pmem_setpos(str_term_v, 9, 0, 2);
    voltage_string(buf, term_voltage); lcd_sendstring(buf, 6);
    lcd_print_pmem_setpos(str_back_next, 20, 0, 3);
    lcd_writecmd(); lcd_send(LCD_ONOFF | LCD_ONOFF_DISPLAY | LCD_ONOFF_BLINK);
    lcd_setcursor(12,1);
    field = 0; submenu_stat = 1;
    uint8_t xpos = 0;

    while (1)
    {
        buttons_pressed = get_buttons();
        if (buttons_pressed & BTN_ESC)
        {
            submenu_stat--;
            if (submenu_stat == 0) { escape_to_menu(); break; }
            if (submenu_stat > 0 && submenu_stat < 4)
            {
                field = 0; xpos = 13 - submenu_stat;
                if (submenu_stat >= 3) xpos--;
                lcd_setcursor(xpos, field+1);
            }
            if (submenu_stat > 3 && submenu_stat < 8)
            {
                field = 1; xpos = 17 - submenu_stat;
                if (submenu_stat >= 6) xpos--;
                lcd_setcursor(xpos, field+1);
            }
        }
        if (buttons_pressed & BTN_OK)
        {
            submenu_stat++;
            if (submenu_stat < 4)
            {
                field = 0; xpos = 13 - submenu_stat;
                if (submenu_stat >= 3) xpos--;
                lcd_setcursor(xpos, field+1);
            }
            if (submenu_stat > 3 && submenu_stat < 8)
            {
                field = 1; xpos = 17 - submenu_stat;
                if (submenu_stat >= 6) xpos--;
                lcd_setcursor(xpos, field+1);
            }
            if (submenu_stat >= 8)
            {
                mAs = 0; mAh100 = 0; mWsx10 = 0; clear_time(); wait_ms(8);
                lcd_writecmd(); lcd_send(LCD_ONOFF | LCD_ONOFF_DISPLAY);
                lcd_clear();
                stat &= ~(stat_inside_submenu | stat_inside_menu);
                stat |= stat_disch_enabled;
                wait_ms(8); set_current(pwm_current);
                enable_1s_timer(); enable_wdt(); return;
            }
        }
        if ((buttons_pressed & BTN_UP) || (buttons_pressed & BTN_DOWN))
        {
            if (buttons_pressed & BTN_UP)
            {
                if (submenu_stat==3 && pwm_current  < 156)  pwm_current  += 100;
                if (submenu_stat==2 && pwm_current  < 246)  pwm_current  += 10;
                if (submenu_stat==1 && pwm_current  < 255)  pwm_current  += 1;
                if (submenu_stat==7) adjust_term_voltage(+1000);
                if (submenu_stat==6) adjust_term_voltage(+100);
                if (submenu_stat==5) adjust_term_voltage(+10);
                if (submenu_stat==4) adjust_term_voltage(+1);
            }
            if (buttons_pressed & BTN_DOWN)
            {
                if (submenu_stat==3 && pwm_current  > 100)  pwm_current  -= 100;
                if (submenu_stat==2 && pwm_current  > 10)   pwm_current  -= 10;
                if (submenu_stat==1 && pwm_current  > 1)    pwm_current  -= 1;
                if (submenu_stat==7) adjust_term_voltage(-1000);
                if (submenu_stat==6) adjust_term_voltage(-100);
                if (submenu_stat==5) adjust_term_voltage(-10);
                if (submenu_stat==4) adjust_term_voltage(-1);
            }
            if (submenu_stat < 4)
            {
                lcd_setcursor(9,1); current_string(buf, pwm_current); lcd_sendstring(buf, 5);
                xpos = 13 - submenu_stat; if (submenu_stat == 3) xpos--;
                lcd_setcursor(xpos, field+1);
            }
            else
            {
                lcd_setcursor(9,2); voltage_string(buf, term_voltage); lcd_sendstring(buf, 6);
                xpos = 17 - submenu_stat; if (submenu_stat >= 6) xpos--;
                lcd_setcursor(xpos, field+1);
            }
        }
    }
}

void set_offset_comp()
{
    lcd_clear();
    lcd_print_pmem(str_offset_comp1, 20);
    lcd_print_pmem_setpos(str_offset_comp2, 20, 0, 1);
    int8_t old_off = offset_comp;
    char off_string[4];

    while (1)
    {
        uint8_t abs_val = (offset_comp < 0) ? (uint8_t)(-1 * offset_comp) : (uint8_t)(offset_comp);
        off_string[0] = (offset_comp < 0) ? '-' : (offset_comp > 0) ? '+' : ' ';
        off_string[1] = dig8_2(abs_val);
        off_string[2] = dig8_1(abs_val);
        off_string[3] = dig8_0(abs_val);
        lcd_setcursor(3,2); lcd_writedata(); lcd_sendstring(off_string, 4);

        uint8_t bp = get_buttons();
        if      ((bp & BTN_DOWN) && offset_comp > -128) offset_comp--;
        else if ((bp & BTN_UP)   && offset_comp < 127)  offset_comp++;
        else if (bp & BTN_OK)  break;
        else if (bp & BTN_ESC) { offset_comp = old_off; break; }
    }
}

void service_menu() { /* intentionally not implemented */ }
void calib_menu()   { /* intentionally not implemented */ }
