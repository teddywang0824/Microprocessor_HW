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

