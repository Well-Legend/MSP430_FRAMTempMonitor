#include <msp430.h>

#define FRAM_MAGIC      0xA55A
#define TEMP_THRESHOLD  2000

#pragma PERSISTENT(lastTemp)
unsigned int lastTemp = 0;

#pragma PERSISTENT(framValid)
unsigned int framValid = 0;

unsigned int lastTempFromFRAM = 0;

void initTemperatureSensor() {
    WDTCTL = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;
    ADC12CTL1 = ADC12SHP;
    ADC12MCTL0 = ADC12INCH_10;
    ADC12CTL0 |= ADC12ENC;
}

unsigned int readTemperature() {
    ADC12CTL0 |= ADC12SC;
    while (!(ADC12IFG & BIT0));
    return ADC12MEM0;
}

void storeTemperatureToFRAM(unsigned int value) {
    lastTemp = value;
    framValid = FRAM_MAGIC;
}

void recoverDataFromFRAM() {
    if (framValid == FRAM_MAGIC)
        lastTempFromFRAM = lastTemp;
    else
        lastTempFromFRAM = 0;
}

void controlLED(unsigned int temp) {
    if (temp > TEMP_THRESHOLD)
        P1OUT |= BIT0;
    else
        P1OUT &= ~BIT0;
}

int main(void) {
    initTemperatureSensor();

    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    recoverDataFromFRAM();

    while (1) {
        unsigned int currentTemp = readTemperature();
        storeTemperatureToFRAM(currentTemp);
        controlLED(currentTemp);

        __delay_cycles(1000000);
    }
}