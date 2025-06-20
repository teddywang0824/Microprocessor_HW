#include <stm32f10x.h>

// 4.1
void USART1_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    GPIOA->CRH |=  0x000004A0;

    USART1->BRR = 0x1D4C;  // 設定 9600 bps（72MHz 時）
    USART1->CR1 |= USART_CR1_TE;   // 啟用傳送器
    USART1->CR1 |= USART_CR1_UE;   // 啟用 USART
}

// 傳送一個字元
void USART1_SendChar(char c) {
    while (!(USART1->SR & USART_SR_TXE));  // 等待 TX 空
    USART1->DR = c;
}

int main(void) {

    USART1_Init();

    while (1) {
    	//4.1
        USART1_SendChar('A');  // 傳送字元 A
        USART1_SendChar('\r');
        USART1_SendChar('\n');
        for (int i = 0; i < 1000000; i++);  // 簡單延遲
    }
}

