#ifndef STRING_TABLE_INCLUDED
#define STRING_TABLE_INCLUDED

#include <avr/pgmspace.h>
#include <string.h>
#include "config.h"

#ifdef LANG_EN
#define L_DEFINED

/// EN
/** string definitions **/

const char str_thermal_error_1[] PROGMEM = "Emergency halt!!!   ";
const char str_thermal_error_2[] PROGMEM = "(Temperature error) ";

const char str_firmware_version[] PROGMEM = "Firmware ver. 1.4.0 ";

const char elapsed[] PROGMEM = "Elapsed: ";
const char str_voltage[] PROGMEM = "Voltage: ";
const char str_current[] PROGMEM = "Current: ";
const char str_power[] PROGMEM = "Power:   ";
const char str_mAh[] PROGMEM = "mAh: ";
const char str_mWh[] PROGMEM = "mWh: ";

const char str_welcome1[] PROGMEM = "  Battery analyzer  ";
const char str_welcome2[] PROGMEM = "  (mAh, mWh meter)  ";

const char str_continue[] PROGMEM = "  Press OK to exit  ";
const char str_esc_exit[] PROGMEM = "  Press ESC to exit ";

const char str_rin_wait[] PROGMEM = " WAIT, measuring R. ";
const char str_rin_resistance[] PROGMEM = "Resistance: ";
const char str_rin_v1[] PROGMEM ="V1=";
const char str_rin_v2[] PROGMEM =" V2=";
const char str_rin_i1[] PROGMEM ="I1=";
const char str_rin_i2[] PROGMEM =" I2=";
const char str_rin_psu_weak[] PROGMEM = "Supply too weak.";

const char str_rin_subm1[] PROGMEM = " R(in) measurement  ";
const char str_rin_subm2[] PROGMEM = "I1 current =  ";
const char str_rin_subm3[] PROGMEM = "I2 current =  ";
const char str_rin_subm4[] PROGMEM = "UP/DN chg, OK run ";

const char str_ccl_subm1[] PROGMEM = "       CC load      ";

const char str_term_v[] PROGMEM = "Term. V: ";

const char str_submenu_disch[] PROGMEM = "Capacity measurement";
const char str_back_next[] PROGMEM = "Back/ESC     Next/OK";

const char str_error[] PROGMEM = "Error";

const char str_setmenu[] PROGMEM = "-- SETTINGS MENU  --";
const char str_terminated[] PROGMEM = "Terminated @ ";
const char str_mAh_fin[] PROGMEM = "mAh @ ";
const char str_mWh_fin[] PROGMEM = "mWh @ ";
const char str_low[] PROGMEM = "(LOW)";
const char str_em_shdn[] PROGMEM = " EMERGENCY SHUTDOWN ";
const char str_em_shdn_wd0[] PROGMEM = "Watchdog reset      ";
const char str_em_shdn_wd1[] PROGMEM = "(CPU locked up)     ";
const char str_em_shdn_therm0[] PROGMEM = "Thermal shutdown    ";
const char str_em_shdn_ov0[] PROGMEM = "Overvoltage         ";

const char str_select0[] PROGMEM = "-- MENU --";
const char str_select1[] PROGMEM = "Measure capacity  ";
const char str_select2[] PROGMEM = "Constant current  ";
const char str_select3[] PROGMEM = "Measure internal R";
const char str_select4[] PROGMEM = "Settings          ";

const char * str_menu_items[] =
{
    str_select1,
    str_select2,
    str_select3,
    str_select4
};

const char str_settings_off[] PROGMEM = "OFF";
const char str_settings_on[] PROGMEM = " ON";

const char * str_settings_onoff[] =
{
    str_settings_off,
    str_settings_on
};

const char str_settings_select0[] PROGMEM = "Back to menu   ";
const char str_settings_select1[] PROGMEM = "Sound          ";
const char str_settings_select2[] PROGMEM = "AD offset comp.";
const char str_settings_select3[] PROGMEM = "Save to EEPROM ";


const char * str_settings_items[] =
{
    str_settings_select0,
    str_settings_select1,
    str_settings_select2,
    str_settings_select3,

};

const char str_cal_mode_prompt0[] PROGMEM = " Hold ESC to enter  ";
const char str_cal_mode_prompt1[] PROGMEM = " calibration mode.  ";

const char str_empty20[] PROGMEM = "                    ";


const char str_service_menu_title[] PROGMEM = " -- SERVICE MENU -- ";

const char str_srv_select0[] PROGMEM = "Calibration   ";
const char str_srv_select1[] PROGMEM = "Exit          ";

const char * str_srv_menu_items[] =
{
    str_srv_select0,
    str_srv_select1

};

/*
const char str_reset_prompt0[] PROGMEM = "Please restart the  ";
const char str_reset_prompt1[] PROGMEM = "device.             ";

const char str_cal_menu_title0[] PROGMEM  = "  Calibration mode  ";
const char str_cal_menu_title1[] PROGMEM  = "Use <- and -> keys. ";

const char str_cal_adj_zero0[] PROGMEM = "> Adjust null offset";
const char str_cal_adj_zero1[] PROGMEM = " (Current = 0 mA)   ";

const char str_cal_adj_curr[] PROGMEM = "> Adjust current:   ";
const char str_cal_adj_curr001[] PROGMEM = " (Current = 10 mA)  ";
const char str_cal_adj_curr002[] PROGMEM = " (Current = 20 mA)  ";
const char str_cal_adj_curr005[] PROGMEM = " (Current = 50 mA)  ";
const char str_cal_adj_curr010[] PROGMEM = " (Current = 100 mA) ";
const char str_cal_adj_curr025[] PROGMEM = " (Current = 250 mA) ";
const char str_cal_adj_curr050[] PROGMEM = " (Current = 500 mA) ";
const char str_cal_adj_curr100[] PROGMEM = " (Current = 1000 mA)";
const char str_cal_adj_curr150[] PROGMEM = " (Current = 1500 mA)";
const char str_cal_adj_curr200[] PROGMEM = " (Current = 2000 mA)";
const char str_cal_adj_curr255[] PROGMEM = " (Current = 2550 mA)";
*/
const uint8_t str_cal_adj_curr_values [11] = {0, 1, 2, 5, 10, 25, 50, 100, 150, 200, 255};

const char str_cal_adj_adcv0[] PROGMEM = "Adjust V divider  ";


const char str_cal_adj_fin0[] PROGMEM = "Calibration finished";
const char str_cal_adj_fin1[] PROGMEM = "Please restart.     ";


const char str_offset_comp1[] PROGMEM = "Offset compensation ";
const char str_offset_comp2[] PROGMEM = "Add: (x0.000625V)   ";

#endif // LANG_CZ

#ifdef LANG_CZ
#define L_DEFINED

/// CZ
/** definice retezcu **/

const char str_thermal_error_1[] PROGMEM = "Nouzove zastaveni!!!";
const char str_thermal_error_2[] PROGMEM = "(Prehrati)          ";


const char str_firmware_version[] PROGMEM = "Firmware verze 1.4.0";

const char elapsed[] PROGMEM = "Cas:     ";
const char str_voltage[] PROGMEM = "Napeti:  ";
const char str_current[] PROGMEM = "Proud:   ";
const char str_power[] PROGMEM = "Vykon:   ";
const char str_mAh[] PROGMEM = "mAh: ";
const char str_mWh[] PROGMEM = "mWh: ";

const char str_welcome1[] PROGMEM = "   Analyzator aku   ";
const char str_welcome2[] PROGMEM = "  (meric mAh, mWh)  ";

const char str_continue[] PROGMEM = "  OK = ukonceni     ";
const char str_esc_exit[] PROGMEM = "  ESC = ukonceni    ";

const char str_rin_wait[] PROGMEM = "Cekejte, merim odpor ";
const char str_rin_resistance[] PROGMEM = "     Odpor: ";
const char str_rin_v1[] PROGMEM ="V1=";
const char str_rin_v2[] PROGMEM =" V2=";
const char str_rin_i1[] PROGMEM ="I1=";
const char str_rin_i2[] PROGMEM =" I2=";
const char str_rin_psu_weak[] PROGMEM = "Prilis slaby aku";

const char str_rin_subm1[] PROGMEM = "Mereni vnitr. odporu";
const char str_rin_subm2[] PROGMEM = "I1 current =  ";
const char str_rin_subm3[] PROGMEM = "I2 current =  ";
const char str_rin_subm4[] PROGMEM = "UP/DN chg, OK run ";

const char str_ccl_subm1[] PROGMEM = "   Proudova zatez   ";

const char str_term_v[] PROGMEM = "Konc. U: ";

const char str_submenu_disch[] PROGMEM = "Mereni kapacity       ";
const char str_back_next[] PROGMEM = "Zpet/ESC    Dalsi/OK";

const char str_error[] PROGMEM = "Chyba";

const char str_setmenu[] PROGMEM = "-- MENU NASTAVENI --";
const char str_terminated[] PROGMEM = " Zakonceno @ ";
const char str_mAh_fin[] PROGMEM = "mAh @ ";
const char str_mWh_fin[] PROGMEM = "mWh @ ";
const char str_low[] PROGMEM = "(LOW)";
const char str_em_shdn[] PROGMEM = "  NOUZOVE VYPNUTI   ";
const char str_em_shdn_wd0[] PROGMEM = "Watchdog reset      ";
const char str_em_shdn_wd1[] PROGMEM = "(Zaseknuti CPU)     ";
const char str_em_shdn_therm0[] PROGMEM = "Tepelna ochrana    ";
const char str_em_shdn_ov0[] PROGMEM = "Prepeti             ";

const char str_select0[] PROGMEM = "-- MENU --";
const char str_select1[] PROGMEM = "Mereni kapacity   ";
const char str_select2[] PROGMEM = "Proudova zatez    ";
const char str_select3[] PROGMEM = "Mereni vnitrniho R";
const char str_select4[] PROGMEM = "Nastaveni         ";

const char * str_menu_items[] =
{
    str_select1,
    str_select2,
    str_select3,
    str_select4
};

const char str_settings_off[] PROGMEM = "VYP";
const char str_settings_on[] PROGMEM = "ZAP";

const char * str_settings_onoff[] =
{
    str_settings_off,
    str_settings_on
};

const char str_settings_select0[] PROGMEM = "Zpet do menu  ";
const char str_settings_select1[] PROGMEM = "Zvuk          ";
const char str_settings_select2[] PROGMEM = "Kompen. offset";
const char str_settings_select3[] PROGMEM = "Uloz do EEPROM";


const char * str_settings_items[] =
{
    str_settings_select0,
    str_settings_select1,
    str_settings_select2,
    str_settings_select3,

};

const char str_cal_mode_prompt0[] PROGMEM = "    Stisk ESC =     ";
const char str_cal_mode_prompt1[] PROGMEM = " kalibracni rezim.  ";

const char str_empty20[] PROGMEM = "                    ";


const char str_service_menu_title[] PROGMEM = "-- SERVISNI MENU  --";

const char str_srv_select0[] PROGMEM = "Kalibrace     ";
const char str_srv_select1[] PROGMEM = "Ukoncit       ";

const char * str_srv_menu_items[] =
{
    str_srv_select0,
    str_srv_select1

};

/*
const char str_reset_prompt0[] PROGMEM = "Please restart the  ";
const char str_reset_prompt1[] PROGMEM = "device.             ";

const char str_cal_menu_title0[] PROGMEM  = "  Calibration mode  ";
const char str_cal_menu_title1[] PROGMEM  = "Use <- and -> keys. ";

const char str_cal_adj_zero0[] PROGMEM = "> Adjust null offset";
const char str_cal_adj_zero1[] PROGMEM = " (Current = 0 mA)   ";

const char str_cal_adj_curr[] PROGMEM = "> Adjust current:   ";
const char str_cal_adj_curr001[] PROGMEM = " (Current = 10 mA)  ";
const char str_cal_adj_curr002[] PROGMEM = " (Current = 20 mA)  ";
const char str_cal_adj_curr005[] PROGMEM = " (Current = 50 mA)  ";
const char str_cal_adj_curr010[] PROGMEM = " (Current = 100 mA) ";
const char str_cal_adj_curr025[] PROGMEM = " (Current = 250 mA) ";
const char str_cal_adj_curr050[] PROGMEM = " (Current = 500 mA) ";
const char str_cal_adj_curr100[] PROGMEM = " (Current = 1000 mA)";
const char str_cal_adj_curr150[] PROGMEM = " (Current = 1500 mA)";
const char str_cal_adj_curr200[] PROGMEM = " (Current = 2000 mA)";
const char str_cal_adj_curr255[] PROGMEM = " (Current = 2550 mA)";
*/
const uint8_t str_cal_adj_curr_values [11] = {0, 1, 2, 5, 10, 25, 50, 100, 150, 200, 255};

const char str_cal_adj_adcv0[] PROGMEM = "Nastavte delic nap. ";


const char str_cal_adj_fin0[] PROGMEM = "Kalibrace dokoncena ";
const char str_cal_adj_fin1[] PROGMEM = "Prosim restartujte. ";


const char str_offset_comp1[] PROGMEM = "Kompenzace offsetu  ";
const char str_offset_comp2[] PROGMEM = "Pridat: (x0.000625V)";

#endif

#ifndef L_DEFINED
#error "Invalid language, please correct config.h!"
#endif


#endif // STRING_TABLE_H_INCLUDED
