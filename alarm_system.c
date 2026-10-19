#include <reg52.h>

/* =========================================================
   PORT DEFINITIONS
   ========================================================= */

/* ADC0808 data bus */
#define ADC_DATA P0

/* ADC0808 control pins */
sbit ADC_ALE   = P3^0;
sbit ADC_START = P3^1;
sbit ADC_EOC   = P3^2;
sbit ADC_OE    = P3^3;

sbit LCD_RS = P2^0;
sbit LCD_EN = P2^1;

/* LCD data pins - 4 bit mode */
sbit LCD_D4 = P2^4;
sbit LCD_D5 = P2^5;
sbit LCD_D6 = P2^6;
sbit LCD_D7 = P2^7;

/* =========================================================
   SAFETY INPUTS
   ACTIVE LOW

   Sensor/switch released = 1
   Sensor/switch active   = 0
   ========================================================= */

sbit FLAME     = P1^0;
sbit GAS       = P1^1;
sbit VIBRATION = P1^2;
sbit EMERGENCY = P1^3;

/* =========================================================
   INDICATORS
   ========================================================= */

sbit GREEN_LED  = P1^4;
sbit YELLOW_LED = P1^5;
sbit RED_LED    = P1^6;
sbit BUZZER     = P1^7;


/* =========================================================
   DELAY FUNCTION
   Crystal = 11.0592 MHz
   ========================================================= */

void delay_ms(unsigned int ms)
{
    unsigned int i, j;

    for(i = 0; i < ms; i++)
    {
        for(j = 0; j < 123; j++)
        {
            ;
        }
    }
}


/* =========================================================
   LCD FUNCTIONS
   ========================================================= */

/* Generate LCD Enable pulse */
void lcd_pulse(void)
{
    LCD_EN = 1;
    delay_ms(1);

    LCD_EN = 0;
    delay_ms(1);
}


/* Send one 4-bit nibble */
void lcd_nibble(unsigned char nibble)
{
    LCD_D4 = (nibble >> 0) & 1;
    LCD_D5 = (nibble >> 1) & 1;
    LCD_D6 = (nibble >> 2) & 1;
    LCD_D7 = (nibble >> 3) & 1;

    lcd_pulse();
}


/* Send command to LCD */
void lcd_cmd(unsigned char cmd)
{
    LCD_RS = 0;

    lcd_nibble(cmd >> 4);
    lcd_nibble(cmd & 0x0F);

    delay_ms(2);
}


/* Send character to LCD */
void lcd_data(unsigned char dat)
{
    LCD_RS = 1;

    lcd_nibble(dat >> 4);
    lcd_nibble(dat & 0x0F);

    delay_ms(1);
}


/* Send string to LCD */
void lcd_string(char *str)
{
    while(*str)
    {
        lcd_data(*str);
        str++;
    }
}


/* Set LCD cursor position */
void lcd_goto(unsigned char row, unsigned char col)
{
    if(row == 0)
    {
        lcd_cmd(0x80 + col);
    }
    else
    {
        lcd_cmd(0xC0 + col);
    }
}


/* Clear LCD */
void lcd_clear(void)
{
    lcd_cmd(0x01);
    delay_ms(2);
}


/* =========================================================
   LCD INITIALIZATION
   ========================================================= */

void lcd_init(void)
{
    LCD_RS = 0;
    LCD_EN = 0;

    delay_ms(20);

    /* Force LCD into 4-bit mode */
    lcd_nibble(0x03);
    delay_ms(5);

    lcd_nibble(0x03);
    delay_ms(1);

    lcd_nibble(0x03);
    delay_ms(1);

    lcd_nibble(0x02);

    /* 4-bit mode, 2 lines, 5x8 font */
    lcd_cmd(0x28);

    /* Display ON, cursor OFF */
    lcd_cmd(0x0C);

    /* Entry mode */
    lcd_cmd(0x06);

    /* Clear display */
    lcd_cmd(0x01);

    delay_ms(5);
}


/* =========================================================
   ADC0808 FUNCTION
   ========================================================= */

unsigned char adc_read(void)
{
    unsigned char value;

    /*
       Start ADC conversion
    */

    ADC_ALE = 1;
    ADC_START = 1;

    delay_ms(1);

    ADC_ALE = 0;
    ADC_START = 0;

    /*
       Wait for conversion.

       EOC LOW  = conversion in progress
       EOC HIGH = conversion complete
    */

    while(ADC_EOC == 1);
    while(ADC_EOC == 0);

    /* Enable ADC output */
    ADC_OE = 1;

    delay_ms(1);

    value = ADC_DATA;

    /* Disable ADC output */
    ADC_OE = 0;

    return value;
}


/* =========================================================
   TEMPERATURE CALCULATION
   ========================================================= */

unsigned int get_temperature(void)
{
    unsigned char adc_value;
    unsigned int temperature;

    adc_value = adc_read();

    /*
       ADC0808:
       VREF+ = 5V
       VREF- = 0V

       8-bit ADC = 256 levels

       LM35:
       10 mV / degree C

       Temperature = ADC value * 500 / 256
    */

    temperature = ((unsigned int)adc_value * 500) / 256;

    return temperature;
}


/* =========================================================
   TEMPERATURE DISPLAY
   ========================================================= */

void display_temperature(unsigned int temperature)
{
    lcd_string("TEMP: ");

    if(temperature >= 100)
    {
        lcd_data((temperature / 100) + '0');
        lcd_data(((temperature / 10) % 10) + '0');
        lcd_data((temperature % 10) + '0');
    }
    else if(temperature >= 10)
    {
        lcd_data((temperature / 10) + '0');
        lcd_data((temperature % 10) + '0');
    }
    else
    {
        lcd_data(temperature + '0');
    }

    lcd_string(" C");
}


/* =========================================================
   INDICATOR FUNCTIONS
   ========================================================= */

/* Turn all indicators OFF */
void indicators_off(void)
{
    GREEN_LED  = 0;
    YELLOW_LED = 0;
    RED_LED    = 0;
    BUZZER     = 0;
}


/* Normal condition */
void normal_condition(void)
{
    GREEN_LED  = 1;
    YELLOW_LED = 1;
    RED_LED    = 0;
    BUZZER     = 0;
}


/* Warning condition */
void warning_condition(void)
{
    GREEN_LED  = 1;
    YELLOW_LED = 0;
    RED_LED    = 1;
    BUZZER     = 0;
}


/* Alarm condition */
void alarm_condition(void)
{
    GREEN_LED  = 0;
    YELLOW_LED = 1;
    RED_LED    = 1;
    BUZZER     = 1;
}


/* =========================================================
   DISPLAY MULTIPLE ACTIVE SENSORS
   =========================================================

   LCD line 1:
       MULTI SENSOR

   LCD line 2:
       FLAME GAS VIB

   Abbreviations:
       FLAME = FLAME
       GAS   = GAS
       VIB   = VIB
       EMG   = EMG
   ========================================================= */

void display_multiple_sensors(void)
{
    lcd_clear();

    lcd_goto(0,0);
    lcd_string("MULTI SENSOR");

    lcd_goto(1,0);

    if(FLAME == 0)
    {
        lcd_string("FLAME ");
    }

    if(GAS == 0)
    {
        lcd_string("GAS ");
    }

    if(VIBRATION == 0)
    {
        lcd_string("VIB ");
    }

    if(EMERGENCY == 0)
    {
        lcd_string("EMG");
    }
}


/* =========================================================
   DISPLAY SINGLE FLAME ALERT
   ========================================================= */

void display_flame_alert(void)
{
    lcd_clear();

    lcd_goto(0,0);
    lcd_string("FLAME ALERT");

    lcd_goto(1,0);
    lcd_string("FIRE DETECTED");
}


/* =========================================================
   DISPLAY SINGLE GAS ALERT
   ========================================================= */

void display_gas_alert(void)
{
    lcd_clear();

    lcd_goto(0,0);
    lcd_string("GAS ALERT");

    lcd_goto(1,0);
    lcd_string("GAS DETECTED");
}


/* =========================================================
   DISPLAY SINGLE VIBRATION ALERT
   ========================================================= */

void display_vibration_alert(void)
{
    lcd_clear();

    lcd_goto(0,0);
    lcd_string("VIBRATION ALERT");

    lcd_goto(1,0);
    lcd_string("VIBRATION HIGH");
}


/* =========================================================
   DISPLAY EMERGENCY ALERT
   ========================================================= */

void display_emergency_alert(void)
{
    lcd_clear();

    lcd_goto(0,0);
    lcd_string("EMERGENCY");

    lcd_goto(1,0);
    lcd_string("SWITCH ACTIVE");
}


/* =========================================================
   DISPLAY CRITICAL TEMPERATURE
   ========================================================= */

void display_critical_temperature(unsigned int temperature)
{
    lcd_clear();

    lcd_goto(0,0);
    lcd_string("TEMP CRITICAL");

    lcd_goto(1,0);

    display_temperature(temperature);
}


/* =========================================================
   ADDED: SAFETY INDEX AND ALARM CLASSIFICATION
   ========================================================= */
#define STATE_SAFE     0
#define STATE_PREWARN  1
#define STATE_WARNING  2
#define STATE_DANGER   3
#define STATE_EMERGENCY 4

volatile unsigned char screen_change = 0;
volatile unsigned char screen_number = 0;
volatile unsigned int timer_ticks = 0;

/* 11.0592 MHz, 12 clock cycles per timer tick.
   Mode 1 preload DC00h gives nominal 10 ms. */
void timer0_init(void)
{
    TMOD = (TMOD & 0xF0) | 0x01;
    TH0 = 0xDC;
    TL0 = 0x00;
    ET0 = 1;
    EA = 1;
    TR0 = 1;
}

void timer0_isr(void) interrupt 1
{
    TH0 = 0xDC;
    TL0 = 0x00;
    timer_ticks++;
    if (timer_ticks >= 200)
    {
        timer_ticks = 0;
        screen_change = 1;
    }
}

unsigned char calculate_index(unsigned int temperature,
                              unsigned char flame,
                              unsigned char gas,
                              unsigned char vibration,
                              unsigned char emergency)
{
    unsigned int risk = 0;
    if (emergency) return 0;
    if (temperature >= 50) risk += 30;
    else if (temperature >= 40) risk += 20;
    else if (temperature >= 30) risk += 10;
    if (flame) risk += 30;
    if (gas) risk += 25;
    if (vibration) risk += 15;
    if (risk >= 100) return 0;
    return (unsigned char)(100 - risk);
}

unsigned char classify_alarm(unsigned int temperature,
                             unsigned char flame,
                             unsigned char gas,
                             unsigned char vibration,
                             unsigned char emergency)
{
    if (emergency) return STATE_EMERGENCY;
    if (temperature >= 50 || flame || gas) return STATE_DANGER;
    if (temperature >= 40 || vibration) return STATE_WARNING;
    if (temperature >= 30) return STATE_PREWARN;
    return STATE_SAFE;
}

void lcd_number3(unsigned char number)
{
    lcd_data((number / 100) + '0');
    lcd_data(((number / 10) % 10) + '0');
    lcd_data((number % 10) + '0');
}

void display_index_status(unsigned char index, unsigned char state)
{
    lcd_clear();
    lcd_goto(0, 0);
    lcd_string("SAFETY IDX: ");
    lcd_number3(index);
    lcd_goto(1, 0);
    if (state == STATE_EMERGENCY) lcd_string("EMERGENCY");
    else if (state == STATE_DANGER) lcd_string("STATUS: DANGER");
    else if (state == STATE_WARNING) lcd_string("STATUS: WARNING");
    else if (state == STATE_PREWARN) lcd_string("STATUS: PREWARN");
    else lcd_string("STATUS: SAFE");
}

/* Keep the original hazard display functions and priority messages. */
void display_sensor_details(unsigned int temperature,
                            unsigned char alarm_count)
{
    if (EMERGENCY == 0) display_emergency_alert();
    else if (temperature >= 50) display_critical_temperature(temperature);
    else if (alarm_count >= 2) display_multiple_sensors();
    else if (FLAME == 0) display_flame_alert();
    else if (GAS == 0) display_gas_alert();
    else if (VIBRATION == 0) display_vibration_alert();
    else if (temperature >= 40)
    {
        lcd_clear();
        lcd_goto(0, 0);
        lcd_string("TEMP WARNING");
        lcd_goto(1, 0);
        display_temperature(temperature);
    }
    else if (temperature >= 30)
    {
        lcd_clear();
        lcd_goto(0, 0);
        lcd_string("TEMP PRE-WARN");
        lcd_goto(1, 0);
        display_temperature(temperature);
    }
    else
    {
        lcd_clear();
        lcd_goto(0, 0);
        lcd_string("SYSTEM NORMAL");
        lcd_goto(1, 0);
        display_temperature(temperature);
    }
}

/* =========================================================
   MAIN PROGRAM (existing pin assignments and LCD/ADC retained)
   ========================================================= */
void main(void)
{
    unsigned int temperature;
    unsigned char alarm_count;
    unsigned char flame_active, gas_active, vibration_active;
    unsigned char emergency_active, safety_index, alarm_state;
    unsigned char previous_state = 255;
    unsigned char previous_screen = 255;
    unsigned char previous_index = 255;
    unsigned int previous_temperature = 65535;
    unsigned char previous_count = 255;
    unsigned char previous_flags = 255;
    unsigned char flags;
    unsigned char refresh;

    /* Original initial port states */
    P0 = 0xFF;
    P1 = 0x0F;
    P2 = 0x00;
    P3 = 0xFF;

    lcd_init();
    lcd_clear();
    lcd_goto(0,0);
    lcd_string("INDUSTRIAL SAFE");
    lcd_goto(1,0);
    lcd_string("SYSTEM STARTING");
    delay_ms(2000);
    lcd_clear();
    timer0_init();

    while (1)
    {
        temperature = get_temperature();
        flame_active = (FLAME == 0);
        gas_active = (GAS == 0);
        vibration_active = (VIBRATION == 0);
        emergency_active = (EMERGENCY == 0);

        alarm_count = flame_active + gas_active +
                      vibration_active + emergency_active;
        flags = flame_active | (gas_active << 1) |
                (vibration_active << 2) | (emergency_active << 3);

        safety_index = calculate_index(temperature, flame_active,
                         gas_active, vibration_active, emergency_active);
        alarm_state = classify_alarm(temperature, flame_active,
                         gas_active, vibration_active, emergency_active);

        /* Original indicator functions; pre-warning uses warning LEDs. */
        if (alarm_state == STATE_DANGER || alarm_state == STATE_EMERGENCY)
            alarm_condition();
        else if (alarm_state == STATE_WARNING || alarm_state == STATE_PREWARN)
            warning_condition();
        else
            normal_condition();

        /* Timer interrupt only requests a screen change. */
        refresh = 0;
        if (screen_change)
        {
            EA = 0;
            screen_change = 0;
            EA = 1;
            screen_number = !screen_number;
            refresh = 1;
        }

        /* A newly detected danger overrides the normal screen rotation. */
        if ((alarm_state >= STATE_DANGER) &&
            ((previous_state < STATE_DANGER) || (flags != previous_flags)))
        {
            screen_number = 1;
            refresh = 1;
            EA = 0;
            timer_ticks = 0;
            screen_change = 0;
            EA = 1;
        }

        if (screen_number != previous_screen ||
            alarm_state != previous_state ||
            safety_index != previous_index ||
            (screen_number == 1 &&
             (temperature != previous_temperature ||
              alarm_count != previous_count || flags != previous_flags)))
            refresh = 1;

        if (refresh)
        {
            if (screen_number == 0)
                display_index_status(safety_index, alarm_state);
            else
                display_sensor_details(temperature, alarm_count);
        }

        previous_screen = screen_number;
        previous_state = alarm_state;
        previous_index = safety_index;
        previous_temperature = temperature;
        previous_count = alarm_count;
        previous_flags = flags;
        /* No 800 ms blocking delay: sensors are polled repeatedly. */
    }
}
