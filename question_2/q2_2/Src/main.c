#include <stm32f10x.h>

// 1.1
void GPIOB_Init_Output(void) {
    // 1. 啟用 GPIOB 的時鐘
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    // 2. 設定 GPIOB 的 16 個腳位為 General purpose output push-pull
    // CRL 控制 PB0~PB7, CRH 控制 PB8~PB15
    // 每個 pin 占 4 位元, 0b0011 = Output mode, max speed 50 MHz

    GPIOB->CRL = 0x33333333;  // PB0 ~ PB7 為輸出模式
    GPIOB->CRH = 0x33333333;  // PB8 ~ PB15 為輸出模式

    // 3. 輸出 0xAAAA 到 ODR（output data register）
    GPIOB->ODR = 0xAAAA;      // 0xAAAA = 1010101010101010
}

//2.1
void GPIOA_Init_Trig() {
    RCC->APB2ENR |= (1 << 4);  // 開啟 GPIOA 時鐘

    // PA0 PA1
    GPIOA->CRL |=  0x00000043;
}

void delay_us(int us) {
//    SysTick->LOAD = us * 27 - 1;
//    SysTick->CTRL = 0x01;
//    while((SysTick->CTRL&(1<<16))==0);
//    SysTick->CTRL = 0x00;
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

//2.2
// 讀取 Echo 時間（回傳 us）
uint32_t read_echo_time_us(void) {
    uint32_t time = 0;

    // 等待 Echo 高電位開始（上升緣）
    while ((GPIOA->IDR & (1 << 1)) == 0);

    // 開始計時：Echo 為 High 的期間
    while ((GPIOA->IDR & (1 << 1)) != 0) {
        delay_us(1);
        time++;
        if (time > 30000) break;  // 最多等 30ms 避免卡死
    }

    return time;
}

// 將 Echo 時間轉換成距離（cm）
float get_distance_cm(void) {
	send_ultrasonic_trig(1);              // 發出 Trig
    uint32_t echo_time = read_echo_time_us();  // 單位 us
    return echo_time / 58.0f;        // 換算距離（cm）
}

// 3.1 Generate a square wave with a period 20ms.
void generate_wave() {
	RCC->APB2ENR = 0xFC;
	RCC->APB1ENR = (1<<0);
	GPIOA->CRL |= 0xB;
	TIM2->CCER = 0x1;
	TIM2->CCMR1 |= 0x60;
	TIM2->PSC = 576 - 1;
	TIM2->ARR = 2500 - 1;
	TIM2->CCR1 = 500;
	TIM2->CR1 = 1;
}

// 3.2
void Servo_SetAngle(uint8_t angle) {
    uint16_t pulse = 125 + ((angle * 125) / 180);
    TIM2->CCR1 = pulse;
}

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

    GPIOB_Init_Output();
//    GPIOA_Init_Trig();
    generate_wave();
    USART1_Init();

    const uint16_t SEG_CODE_2 = 0x5B; // 數字 '2' 的編碼 (g=1, f=0, e=1, d=1, c=0, b=1, a=1)
	const uint16_t SEG_CODE_0 = 0x3F; // 數字 '0' 的編碼 (g=0, f=1, e=1, d=1, c=1, b=1, a=1)
	uint16_t output_value = (SEG_CODE_0 << 7) | SEG_CODE_2;
	GPIOB->ODR = output_value;

	float distance;

    while (1) {
//    	send_ultrasonic_trig();
//    	for (int i = 0; i < 100000; i++);
//    	distance = get_distance_cm();
//    	delay_us(60000);

    	// 3.2
//    	Servo_SetAngle(0);    // 最左，順時針
//		delay_us(100000000);  // 延遲等待伺服轉動
//
//		Servo_SetAngle(180);  // 最右，逆時針
//		delay_us(100000000);  // 延遲等待
//
//		Servo_SetAngle(90);   // 回中間
//		delay_us(100000000);

    	//4.1
//        USART1_SendChar('A');  // 傳送字元 A
//    	  USART1_SendString("\r\n");
//        for (volatile int i = 0; i < 1000000; i++);  // 簡單延遲

    	//4.2
    	for(int i=0;i<=20;i++){
    		if(i<10) {
				USART1_SendChar(i + '0');
				USART1_SendChar('\r');
				USART1_SendChar('\n');
				delay_us(1000000);
    		} else {
    			USART1_SendChar(i/10 + '0');
    			USART1_SendChar(i%10 + '0');
				USART1_SendChar('\r');
				USART1_SendChar('\n');
				delay_us(1000000);
    		}
    	}
    }
}

