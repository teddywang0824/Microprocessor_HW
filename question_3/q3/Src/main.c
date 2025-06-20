#include <stm32f10x.h>

// 3.1 Generate a square wave with a period 20ms.
void generate_wave() {
	RCC->APB2ENR = 0xFC;
	RCC->APB1ENR = (1<<0);
	GPIOA->CRL |= 0xB;
	TIM2->CCER = 0x1;
	TIM2->CCMR1 |= 0x60;
	TIM2->PSC = 72 - 1;
	TIM2->ARR = 20000 - 1;
	TIM2->CCR1 = 500;
	TIM2->CR1 = 1;
}

// 3.2
void Servo_SetAngle(uint8_t angle) {
	uint16_t pulse;
	if(angle == 0) {
		pulse = 1000;
	} else if (angle == 90) {
		pulse = 1500;
	} else {
		pulse = 2000;
	}
    TIM2->CCR1 = pulse;
}

void delay_ms(int us) {
	int temp = us * 9000;
	for(int i = temp;i>=0;i--);
}

int main(void) {

    generate_wave();

    while (1) {

    	// 3.2
    	Servo_SetAngle(0);    // 最左，順時針
		delay_ms(10000);  // 延遲等待伺服轉動

		Servo_SetAngle(180);  // 最右，逆時針
		delay_ms(10000);  // 延遲等待

		Servo_SetAngle(90);   // 回中間
		delay_ms(10000);
    }
}

