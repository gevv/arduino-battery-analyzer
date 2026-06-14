/**
    Battery analyzer - Arduino Nano port
    Original: (c) 2016-2017 F. Štefanec
    Arduino Nano port: 2024

    ATmega328P pin mapping is IDENTICAL between the original board and Arduino Nano.
    Only the build system changes (AVR-C → Arduino framework).

    Original schematic pin assignments are preserved 1:1.
**/

#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED

/// Language
//#define LANG_CZ
#define LANG_EN

// ─── LCD pin / port definitions ──────────────────────────────────────────────
// LCD is connected to PORTD (data + control), same as original schematic.
#define LCD_DPORT  PORTD
#define LCD_CPORT  PORTD
#define LCD_DDDR   DDRD
#define LCD_CDDR   DDRD
#define LCD_E      PD3    // Arduino D3
#define LCD_RS     PD2    // Arduino D2
#define LCD_D4     PD4    // Arduino D4
#define LCD_D5     PD5    // Arduino D5
#define LCD_D6     PD6    // Arduino D6
#define LCD_D7     PD7    // Arduino D7
#define DISPSIZE   6      // 20x4 display, do not modify

// ─── Keyboard ────────────────────────────────────────────────────────────────
#define DEBOUNCE_VAL   6    // 6 × 4ms ≈ 24 ms
#define REPEAT_PERIOD  30   // 30 × 4ms = 120 ms
#define REPEAT_DELAY   250  // 250 × 4ms = 1000 ms

// ─── ADC / measurement ───────────────────────────────────────────────────────
#define DEFAULT_OFFSET_COMP  110
#define RIN_PREAVG_COUNT     8
#define CAP_PREAVG_COUNT     32
#define DEFAULT_SETTINGS     0

// ─── Temperature / LM35 ──────────────────────────────────────────────────────
/// Uncomment the next line only if the LM35 sensor is installed on TFB / A3.
/// If the sensor is not installed, leaving it enabled can cause false thermal shutdown.
//#define LM35_INSTALLED
#define TEMP_AD_FAN_START  512   // ~55 °C
#define TEMP_AD_FAN_STOP   446   // ~48 °C
#define TEMP_AD_FAULT      725   // ~78 °C  → emergency halt

// ─── Button pins (PORTB) ─────────────────────────────────────────────────────
// Original: PINB / PORTB
#define BTN_PIN   PINB
#define BTN_PORT  PORTB

#define BTN_ESC_PIN   PB4   // Arduino D12
#define BTN_UP_PIN    PB2   // Arduino D10
#define BTN_DOWN_PIN  PB1   // Arduino D9
#define BTN_OK_PIN    PB0   // Arduino D8

// ─── INHIBIT pin (PC2 = Arduino A2) ──────────────────────────────────────────
#define INHIBIT_PIN   PC2
#define INHIBIT_PORT  PORTC
#define INHIBIT_DDR   DDRC

// ─── Buzzer (PB5 = Arduino D13) ──────────────────────────────────────────────
// NOTE: D13 also has the built-in LED. Disconnect or ignore LED flicker.
#define BUZZER_PIN   PB5
#define BUZZER_PORT  PORTB
#define BUZZER_DDR   DDRB

// ─── ADC channel mux values ──────────────────────────────────────────────────
#define MUX_BATT_VOLTAGE    (0)                   // ADC0 = A0
#define MUX_SUPPLY_VOLTAGE  (1<<MUX0)             // ADC1 = A1
#define MUX_TEMPERATURE     (0b011)               // ADC3 = A3

// ─── Fan (PC4 = Arduino A4) ──────────────────────────────────────────────────
#define FAN_PIN   PC4
#define FAN_PORT  PORTC
#define FAN_DDR   DDRC

// ─── Power LED (PC5 = Arduino A5) ────────────────────────────────────────────
#define LED_PIN   PC5
#define LED_DDR   DDRC
#define LED_PORT  PORTC

// ─── Beep lengths (timer 0 ticks @ 2 kHz / 8 = 250 Hz effective) ─────────────
#define BEEP_LEN_BUTTON    10
#define BEEP_LEN_FINISHED  250
#define BEEP_LEN_REPEAT    5
#define BEEP_LEN_START     5

#endif // CONFIG_INCLUDED
