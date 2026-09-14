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

/* LCD control pins */
sbit LCD_RS = P2^0;
sbit LCD_EN = P2^1;

/* LCD data pins - 4 bit mode */
sbit LCD_D4 = P2^4;
sbit LCD_D5 = P2^5;
sbit LCD_D6 = P2^6;
sbit LCD_D7 = P2^7;

/* Safety inputs - ACTIVE LOW */
sbit FLAME     = P1^0;
sbit GAS       = P1^1;
sbit VIBRATION = P1^2;
sbit EMERGENCY = P1^3;

/* Indicators */
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

void lcd_pulse(void)
{
    LCD_EN = 1;
    delay_ms(1);
    LCD_EN = 0;
    delay_ms(1);
}


/* Send one 4-bit nibble to LCD */
void lcd_nibble(unsigned char nibble)
{
    LCD_D4 = (nibble >> 0) & 1;
    LCD_D5 = (nibble >> 1) & 1;
    LCD_D6 = (nibble >> 2) & 1;
    LCD_D7 = (nibble >> 3) & 1;

    lcd_pulse();
}


/* Send command */
void lcd_cmd(unsigned char cmd)
{
    LCD_RS = 0;

    lcd_nibble(cmd >> 4);
    lcd_nibble(cmd & 0x0F);

    delay_ms(2);
}


/* Send character */
void lcd_data(unsigned char dat)
{
    LCD_RS = 1;

    lcd_nibble(dat >> 4);
    lcd_nibble(dat & 0x0F);

    delay_ms(1);
}


/* Send string */
void lcd_string(char *str)
{
    while(*str)
    {
        lcd_data(*str);
        str++;
    }
}


/* Set LCD cursor
   row = 0 or 1
   col = 0 to 15
*/
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


/* LCD initialization */
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

    /* 4-bit, 2-line, 5x8 font */
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
       ALE and START are pulsed HIGH
    */

    ADC_ALE = 1;
    ADC_START = 1;

    delay_ms(1);

    ADC_ALE = 0;
    ADC_START = 0;

    /*
       Wait for conversion to complete.

       ADC0808 EOC normally goes LOW while converting
       and HIGH when conversion is complete.
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

       ADC resolution = 256 levels

       LM35:
       10 mV / °C

       Temperature:
       ADC value * 500 / 256
    */

    temperature = ((unsigned int)adc_value * 500) / 256;

    return temperature;
}


/* =========================================================
   INDICATOR FUNCTIONS
   ========================================================= */

/* Turn everything OFF */
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
   MAIN PROGRAM
   ========================================================= */

void main(void)
{
    unsigned int temperature;
    unsigned char alarm_count;

    /* Initial port states */
    P0 = 0xFF;
    P1 = 0x0F;
    P2 = 0x00;
    P3 = 0xFF;

    /* Initialize LCD */
    lcd_init();

    /* Initial display */
    lcd_clear();

    lcd_goto(0, 0);
    lcd_string("INDUSTRIAL SAFE");

    lcd_goto(1, 0);
    lcd_string("SYSTEM STARTING");

    delay_ms(2000);

    lcd_clear();


    /* =====================================================
       MAIN LOOP
       ===================================================== */

    while(1)
    {
        /*
           Read temperature
        */
        temperature = get_temperature();


        /*
           Count active safety inputs.

           IMPORTANT:
           Your current Proteus switches are ACTIVE LOW.

           Switch released = 1
           Switch pressed  = 0
        */

        alarm_count = 0;

        if(FLAME == 0)
        {
            alarm_count++;
        }

        if(GAS == 0)
        {
            alarm_count++;
        }

        if(VIBRATION == 0)
        {
            alarm_count++;
        }

        if(EMERGENCY == 0)
        {
            alarm_count++;
        }


        /* =================================================
           PRIORITY 1:
           HIGH ALERT

           HIGH ALERT occurs when:

           1. Temperature >= 50°C
              OR
           2. Two or more safety sensors are active

           This has the highest priority.
           ================================================= */

        if((temperature >= 50) || (alarm_count >= 2))
        {
            alarm_condition();

            lcd_clear();

            lcd_goto(0, 0);
            lcd_string("HIGH ALERT");

            lcd_goto(1, 0);

            if(temperature >= 50)
            {
                lcd_string("TEMP CRITICAL");
            }
            else
            {
                lcd_string("MULTI SENSOR");
            }
        }


        /* =================================================
           PRIORITY 2:
           SINGLE FLAME ALERT
           ================================================= */

        else if(FLAME == 0)
        {
            alarm_condition();

            lcd_clear();

            lcd_goto(0, 0);
            lcd_string("ALERT: FLAME!");

            lcd_goto(1, 0);
            lcd_string("FIRE DETECTED");
        }


        /* =================================================
           PRIORITY 3:
           SINGLE GAS ALERT
           ================================================= */

        else if(GAS == 0)
        {
            alarm_condition();

            lcd_clear();

            lcd_goto(0, 0);
            lcd_string("ALERT: GAS!");

            lcd_goto(1, 0);
            lcd_string("GAS DETECTED");
        }


        /* =================================================
           PRIORITY 4:
           SINGLE VIBRATION ALERT
           ================================================= */

        else if(VIBRATION == 0)
        {
            alarm_condition();

            lcd_clear();

            lcd_goto(0, 0);
            lcd_string("ALERT: VIBRATION");

            lcd_goto(1, 0);
            lcd_string("VIBRATION HIGH");
        }


        /* =================================================
           PRIORITY 5:
           SINGLE EMERGENCY ALERT
           ================================================= */

        else if(EMERGENCY == 0)
        {
            GREEN_LED = 1;
            YELLOW_LED = 0;
            RED_LED = 1;
            BUZZER = 1;

            lcd_clear();

            lcd_goto(0, 0);
            lcd_string("EMERGENCY");

            lcd_goto(1, 0);
            lcd_string("SWITCH ACTIVE");
        }


        /* =================================================
           PRIORITY 6:
           HIGH TEMPERATURE

           This case is technically covered by HIGH ALERT
           above when temperature >= 50°C.
           ================================================= */

        else if(temperature >= 50)
        {
            alarm_condition();

            lcd_clear();

            lcd_goto(0, 0);
            lcd_string("HIGH TEMPERATURE");

            lcd_goto(1, 0);
            lcd_string("TEMP >= 50C");
        }


        /* =================================================
           PRIORITY 7:
           WARNING TEMPERATURE

           30°C to 49°C
           ================================================= */

        else if(temperature >= 30)
        {
            warning_condition();

            lcd_clear();

            lcd_goto(0, 0);
            lcd_string("TEMP WARNING");

            lcd_goto(1, 0);
            lcd_string("TEMP: ");

            /* Display approximate temperature */
            lcd_data((temperature / 10) + '0');
            lcd_data((temperature % 10) + '0');
            lcd_string(" C");
        }


        /* =================================================
           PRIORITY 8:
           NORMAL CONDITION

           Temperature below 30°C
           and no safety sensor active.
           ================================================= */

        else
        {
            normal_condition();

            lcd_clear();

            lcd_goto(0, 0);
            lcd_string("SYSTEM NORMAL");

            lcd_goto(1, 0);
            lcd_string("TEMP: ");

            /* Display temperature */
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


        /* Small delay before next reading */
        delay_ms(300);
    }
}