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

