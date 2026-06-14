/**
    Battery analyzer - Arduino Nano port
    Original: (c) 2016-2017 F. Štefanec
**/

#ifndef GLOBAL_DEFINES_H_INCLUDED
#define GLOBAL_DEFINES_H_INCLUDED

// ─── Button masks ─────────────────────────────────────────────────────────────
#define BTN_ESC   128
#define BTN_UP     64
#define BTN_DOWN   32
#define BTN_OK     16

// ─── Status byte (stat) bit flags ────────────────────────────────────────────
#define stat_disch_enabled    128
#define stat_display_prog      64
#define stat_term_wait         32
#define stat_inside_menu       16
#define stat_rin_test           8
#define stat_inside_submenu     4
#define stat_ccload_enabled     2
#define stat_ccload_display     1

// ─── Status byte 2 (stat2) ───────────────────────────────────────────────────
#define stat2_redraw_menu  128

// ─── Settings byte ───────────────────────────────────────────────────────────
#define settings_sound_on  128

// ─── Timer 1 compare values ──────────────────────────────────────────────────
// Arduino Nano hardware clock is 16 MHz.  Use the real hardware clock here,
// not F_CPU, because an accidentally selected 4 MHz board profile makes the
// elapsed-time counter run 4× too fast.
#define HW_CPU_HZ              16000000UL
#if (F_CPU != HW_CPU_HZ)
#warning "Battery Analyzer Nano firmware expects a 16 MHz Arduino Nano board profile"
#endif
#define TIME_PRESCALER_1000ms  ((uint16_t)(HW_CPU_HZ/256UL - 1UL))       // 62499
#define TIME_PRESCALER_250ms   ((uint16_t)(HW_CPU_HZ/256UL/4UL - 1UL))   // 15624

// ─── Voltage limits ───────────────────────────────────────────────────────────
// Values are stored in 0.01 V units.  300 = 3.00 V.
// For a single 18650 Li-ion cell, 3.00 V is a safe default cutoff.
// The menu is clamped to 2.50–4.20 V to prevent accidental deep discharge.
#define MIN_VOLTAGE   80
#define MAX_VOLTAGE 2000
#define DEFAULT_TERM_VOLTAGE 300
#define MIN_TERM_VOLTAGE     250
#define MAX_TERM_VOLTAGE     420

#endif // GLOBAL_DEFINES_H_INCLUDED
