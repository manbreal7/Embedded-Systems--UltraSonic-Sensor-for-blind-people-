#include <stdio.h>
#include <LPC17xx.h>
#include <math.h>
#include <string.h>

#define PRESCALE 29999999
#define LED 0xff // P0.4-0.11
#define RS_CTRL 0x08000000 // P0.27 
#define EN_CTRL 0x10000000 // P0.28 enable pin
#define DT_CTRL 0x07800000 // P0.23 to P0.26 data lines
#define TRIG (1 << 15) // P0.15
#define ECHO (1 << 16) // P0.16
#define BUZZER (1 << 13) // P2.13

char ans[20] = "";
char ans1[20] = "0xC0";
int temp, temp1, temp2 = 0;
int flag = 0, flag1;
int i, j, k, l, r, echoTime = 5000;
float distance = 0;

void clear_ports(void);
void lcd_write(void);
void port_write(void);
void lcd_display(unsigned char *buf1);
void delay(unsigned int r1);
void clearDisplay(void);
void startTimer0(void);
float stopTimer0();
void initTimer0(void);
void delayUS(unsigned int microseconds);
void delayMS(unsigned int milliseconds);

void delayUS(unsigned int microseconds)
{
    LPC_SC->PCLKSEL0 &= ~(0x3 << 2);
    LPC_TIM0->TCR = 0x02;
    LPC_TIM0->PR = 0;
    LPC_TIM0->MR0 = microseconds - 1;
    LPC_TIM0->MCR = 0x01;
    LPC_TIM0->TCR = 0x01;
    while ((LPC_TIM0->IR & 0x01) == 0);
    LPC_TIM0->TCR = 0x00;
    LPC_TIM0->IR = 0x01;
}

void delayMS(unsigned int milliseconds)
{
    delayUS(milliseconds * 1000);
}

void initTimer0(void)
{
    LPC_TIM0->CTCR = 0x0;
    LPC_TIM0->PR = 11999999;
    LPC_TIM0->TCR = 0x02;
}

void startTimer0(void)
{
    LPC_TIM0->TCR = 0x02;
    LPC_TIM0->TCR = 0x01;
}

float stopTimer0()
{
    LPC_TIM0->TCR = 0x0;
    return LPC_TIM0->TC;
}

void delay(unsigned int r1)
{
    for (r = 0; r < r1; r++);
}

void clear_ports(void)
{
    LPC_GPIO0->FIOCLR = DT_CTRL; // clearing data lines
    LPC_GPIO0->FIOCLR = RS_CTRL; // clearing RS line
    LPC_GPIO0->FIOCLR = EN_CTRL; // clearing enable line
    delay(1000000);
    return;
}

void port_write()
{
    int j;
    LPC_GPIO0->FIOPIN = temp2 << 23;
    if (flag1 == 0)
    {
        LPC_GPIO0->FIOCLR = 1 << 27;
    }
    else
    {
        LPC_GPIO0->FIOSET = 1 << 27;
    }
    LPC_GPIO0->FIOSET = 1 << 28;
    for (j = 0; j < 50; j++);
    LPC_GPIO0->FIOCLR = 1 << 28;
    for (j = 0; j < 10000; j++);
}

void lcd_write()
{
    temp2 = (temp1 >> 4) & 0xF;
    port_write();
    delayMS(100); // delay of 10 ms
    temp2 = temp1 & 0xF;
    port_write();
    delayMS(100); // delay of 10 ms
}

int main()
{
    int ledflag = 0;
    int command[] = {3, 3, 3, 2, 2, 0x01, 0x06, 0x0C, 0x80};
    float rounded_down;
    SystemInit();
    SystemCoreClockUpdate();
    initTimer0();
    LPC_PINCON->PINSEL0 &= 0xfffff00f; //LEDs P0.4-P0.11
    LPC_PINCON->PINSEL0 &= 0x3fffffff; //TRIG P0.15
    LPC_PINCON->PINSEL1 &= 0xfffffff0; //ECHO P0.16
    LPC_GPIO0->FIODIR |= TRIG | 1 << 17;
    LPC_GPIO1->FIODIR |= 0 << 16;
    LPC_GPIO0->FIODIR |= LED << 4;
    LPC_PINCON->PINSEL1 |= 0;
    LPC_GPIO0->FIODIR |= 0XF << 23 | 1 << 27 | 1 << 28;
    flag1 = 0;
    for (i = 0; i < 9; i++)
    {
        temp1 = command[i];
        lcd_write();
        for (j = 0; j < 100000; j++);
    }
    flag1 = 1;
    i = 0;
    flag = 1;
    LPC_GPIO0->FIOCLR |= TRIG;
  LPC_PINCON->PINSEL4 &= 0xFCFFFFFF; // configure P2.13 as gpio
    LPC_GPIO2->FIODIR |= BUZZER; // Set P2.13 as output
    while (1)
    {
        LPC_GPIO0->FIOSET = 0x00000800;
        LPC_GPIO0->FIOMASK = 0xFFFF7FFF;
        LPC_GPIO0->FIOPIN |= TRIG;
        delayUS(10);
        LPC_GPIO0->FIOCLR |= TRIG;
        LPC_GPIO0->FIOMASK = 0x0;
        while (!(LPC_GPIO0->FIOPIN & ECHO));// wait for a high on echo
        startTimer0();
        while (LPC_GPIO0->FIOPIN & ECHO); // wait for a low on echo
        echoTime = stopTimer0();   
        distance = (0.0343 * echoTime) / 40;
        sprintf(ans, " Dist:%.2f", distance);
        flag1 = 1;
        i = 0;
        flag1 = 0;
        temp1 = 0x01;
        lcd_write();
        delayMS(100); 
        flag1 = 1;
        while (ans[i] != '\0')
        {
            temp1 = ans[i];
            lcd_write();
            for (j = 0; j < 100000; j++)
                ;
            i++;
        }
        if (distance < 30 && distance > 20)
        {
            LPC_GPIO0->FIOSET = 0x3 << 10;
            LPC_GPIO0->FIOSET = 1 << 17;
            delay(999999);
        }
        if (distance < 20 && distance > 10)
        {
            LPC_GPIO0->FIOSET = 0xF << 8;
            LPC_GPIO0->FIOSET = 1 << 17;
            delay(999999);
        }
        if (distance < 10) {
            LPC_GPIO0->FIOSET = LED << 4;
            LPC_GPIO0->FIOSET = 1 << 17;
            LPC_GPIO2->FIOSET = BUZZER; 
            delay(555555);
        } else {
            LPC_GPIO2->FIOCLR = BUZZER; 
            LPC_GPIO0->FIOCLR = LED << 4;
            LPC_GPIO0->FIOCLR = 1 << 17;
            delay(999999);
        }
    }
}