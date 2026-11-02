#include "driverlib.h"
#include <msp430.h>

#define FRAM_MAGIC      0xA55A
#define TEMP_THRESHOLD  630

volatile unsigned int currentTemp = 0;
#pragma PERSISTENT(storedTemp)
volatile unsigned int storedTemp = 0;

#pragma PERSISTENT(framValid)
unsigned int framValid = 0;

unsigned int lastTempFromFRAM = 0;

#include <msp430.h>

void initTemperatureSensor() {
    // 配置參考電壓
    // 溫度感測器需要穩定的參考電壓
    PMMCTL0_H = PMMPW_H;          // 解鎖 PMM 暫存器
    PMMCTL2 |= INTREFEN | TSENSOREN; // 啟用內部參考電壓 與 溫度感測器
    __delay_cycles(400);          // 等待參考電壓穩定

    // 配置 ADC 控制暫存器
    // ADCSHT_8: 採樣保持時間.
    //           溫度感測器阻抗較高，需要較長的採樣時間
    // ADCON:    開啟 ADC
    ADCCTL0 = ADCSHT_8 | ADCON;

    // ADCSHP:   採樣脈衝模式 (由 SAMPCON 信號源控制)
    // ADCSSEL_1: 時鐘源選擇 ACLK (通常較慢，適合溫度採樣)
    ADCCTL1 = ADCSHP | ADCSSEL_1; 
    
    // ADCRES_1: 10-bit 解析度 (FR4133 是 10-bit ADC)
    ADCCTL2 = ADCRES_1;

    // 配置記憶體控制暫存器 (Memory Control)
    // ADCINCH_12: 在 MSP430FR4133 上，溫度感測器通常連接到 Channel 12
    // ADCSREF_1:  選擇參考電壓 (Vref = Int, Vref- = AVss)
    ADCMCTL0 = ADCINCH_12 | ADCSREF_1;
    
    // 4. 啟用轉換
    ADCCTL0 |= ADCENC;
}

unsigned int readTemperature() {
    ADCCTL0 |= ADCSC;             // 開始轉換 (Start Conversion)
    while (ADCCTL1 & ADCBUSY);    // 等待轉換完成 (檢查 BUSY 位元)
    return ADCMEM0;               // 返回 10-bit 結果
}

void storeTemperatureToFRAM(unsigned int value) {
    storedTemp = value;
    framValid = FRAM_MAGIC;
}

void recoverDataFromFRAM() {
    if (framValid == FRAM_MAGIC)
        lastTempFromFRAM = storedTemp;
    else
        lastTempFromFRAM = 0;

    if (lastTempFromFRAM > 500) {
        // 檢查重啟後的 FRAM 是否有有效資料，有的話閃爍3次
        int i;
        for(i=0; i<3; i++) {
            P1OUT |= BIT0;  // 亮
            P1OUT &= ~BIT0; // 滅
        }
    }
}

void controlLED(unsigned int temp) {
    if (temp > TEMP_THRESHOLD)
        P1OUT |= BIT0;
    else
        P1OUT &= ~BIT0;
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // 關閉 Watchdog Timer
    SYSCFG0 = 0xA500 | DFWP;    // 把 FRAM Write Protection 關閉
    PM5CTL0 &= ~LOCKLPM5;       // 解除 GPIO 高阻抗狀態
    initTemperatureSensor();

    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    recoverDataFromFRAM();
    while (1) {
        currentTemp = readTemperature();
        storeTemperatureToFRAM(currentTemp);
        controlLED(currentTemp);
    }
}
