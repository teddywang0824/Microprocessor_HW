#include <stm32f10x.h>

void USART1_Init(void) {
    RCC->APB2ENR |= (1<<14) | (1<<2);

    GPIOA->CRH |=  0x000004A0;

    USART1->BRR = 7500;  // 設定 9600 bps（72MHz 時）
    USART1->CR1 = 0x200C;   // 啟用傳送器
}

// 傳送一個字元
void USART1_SendChar(char c) {
    while (!(USART1->SR & USART_SR_TXE));  // 等待 TX 空
    USART1->DR = c;
}

void delay_ms(int us) {
	int temp = us * 9000;
	for(int i = temp;i>=0;i--);
}

int main(void) {

    USART1_Init();

    while (1) {

    	//4.2
    	for(int i=0;i<=20;i++){
    		if(i<10) {
				USART1_SendChar(i + '0');
				USART1_SendChar('\r');
				USART1_SendChar('\n');
				delay_ms(1000);
    		} else {
    			USART1_SendChar(i/10 + '0');
    			USART1_SendChar(i%10 + '0');
				USART1_SendChar('\r');
				USART1_SendChar('\n');
				delay_ms(1000);
    		}
    	}
    }
}

