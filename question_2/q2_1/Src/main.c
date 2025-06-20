#include <stm32f10x.h>

void GPIOA_Init_Trig() {
    RCC->APB2ENR |= (1 << 4);  // 開啟 GPIOA 時鐘

    // PA0 PA1
    GPIOA->CRL |=  0x00000043;
}

void delay_us(int us) {
	int temp = us * 9;
	for(int i = temp;i>=0;i--);
}

void send_ultrasonic_trig(int us) {
    // 拉高 PA0
    GPIOA->ODR |= (1 << 0);
    delay_us(us);
    // 拉低 PA0
    GPIOA->ODR &= ~(1 << 0);
}

int main(void) {

    GPIOA_Init_Trig();

    while (1) {
    	send_ultrasonic_trig(10);
    	for (int i = 0; i < 100000; i++);

    }
}

