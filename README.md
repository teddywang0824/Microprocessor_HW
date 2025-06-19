# 微處理機期末專題
##### 4112064230 電資二 王珈源
---
## 1. 下列請以GPIOB完成，七段顯示器型號為DC56-11EWA：
#### i. 將所有pin設定為output mode，並使pin 0~15輸出為0xAAAA。
---
Ans :
* 先透過 ```GPIOB->CRL``` 以及 ```GPIOB->CRH``` 設定 PB0~PB15。因為是輸出，所以都每個PB腳位都設定成 `0011`。
* 使用`GPIOB->ODR`輸出。

程式碼([question_1/q1_1/Src/main.c](question_1/q1_1/Src/main.c)) :
```c
#include <stm32f10x.h>

void GPIOB_Init_Output(void) {

    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    GPIOB->CRL = 0x33333333;  // PB0 ~ PB7 為輸出模式
    GPIOB->CRH = 0x33333333;  // PB8 ~ PB15 為輸出模式

    GPIOB->ODR = 0xAAAA;      // 0xAAAA = 1010101010101010
}

int main(void) {

    GPIOB_Init_Output();

    while (1) {
    }
}
```
結果截圖 : ![q1-1](question_1/q1_1.png)
pcf 檔案 : [question_1/q1_1.pcf](question_1/q1_1.pcf)

#### ii. 根據圖一完成表一：(Low 為輸出低電壓 0，High 為輸出高電位 1)
---
Ans:
![](question_1/q1_2.png)

#### iii. 編寫程式使七段顯示器顯示 20。
---
Ans:
* 由第二小題表格結果找到2以及0對應的腳位輸出。

程式碼([question_1/q1_3/Src/main.c](question_1/q1_3/Src/main.c)) :
```c
#include <stm32f10x.h>

void GPIOB_Init_Output(void) {

    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    GPIOB->CRL = 0x33333333;  // PB0 ~ PB7 為輸出模式
    GPIOB->CRH = 0x33333333;  // PB8 ~ PB15 為輸出模式
}

int main(void) {

    GPIOB_Init_Output();

    const uint16_t SEG_CODE_2 = 0x5B; // 數字 '2' 的編碼 (g=1, f=0, e=1, d=1, c=0, b=1, a=1)
    const uint16_t SEG_CODE_0 = 0x3F; // 數字 '0' 的編碼 (g=0, f=1, e=1, d=1, c=1, b=1, a=1)
    uint16_t output_value = (SEG_CODE_0 << 7) | SEG_CODE_2;
    GPIOB->ODR = output_value;


    while (1) {
    }
}
```
結果截圖 : ![](question_1/q1_3.png)
pcf 檔案 : [question_1/q1_3.pcf](question_1/q1_3.pcf)

## 2. 下列請以 GPIOA 完成，超音波模組型號為 HC-SR04：
#### i. Generate a pulse with a duration of 10us via PA0 using a for loop.
#### ii. 編寫程式讀取超音波模組數據。

## 3. 下列請以 TIM2 (PWM) 完成，伺服馬達型號為 SG90：
#### i. Generate a square wave with a period 20ms.
#### ii. 編寫程式使伺服馬達順時針與逆時針旋轉。

## 4. 下列請以 USART1 (baud rate 為 9600) 完成，藍芽模組型號為 HC-06：
#### i. Send a character ('A') to the IO Virtual Term.
#### ii. 編寫程式使藍芽 (USART1) 每秒發送一個數字給 IO Virtual Term，數字依序為 0 至 20。

## 5. 下列請以 Interrupt 與 Timer 完成系統整合實現停車場系統（有關時間的都要用 Timer 或 SysTick 實現）：
### i. 超音波感測（建議使用 Timer2）
#### A. 入口每 5 秒產生一個 trig 檢測是否有車子要進來。
#### B. 出口先延遲2.5秒再每5秒產生一個trig是否有車子要出去。每五秒 trig。入口跟出口 trig 時間相差2.5秒。
#### C. 使用中斷捕捉 Echo 上升時間和下降時間，然後透過下面公式轉換成公尺。

### ii.	伺服馬達閘門
#### A. 當出入口的Echo 讀到的值小於 100m，相對應的閘門要打開讓車子進出，然後關閉閘門(伺服馬達打開和關閉閘門90度轉動)。
#### B. 車輛進入時剩餘車位減1，車輛出去時剩餘車位加1，並將剩餘車位數顯示於2-digit七段顯示器(車位數初始值20)。
#### C. 當車位停滿時，七段顯示器顯示00並以一秒為週期進行閃爍(連續重複亮0.5 sec與暗0.5 sec)。此時，閘門維持不動。
#### D. 透過藍芽(USART1)傳送目前車輛數目給 IO Virtual Term (轉成兩個數字的ASCII Code後傳送)。