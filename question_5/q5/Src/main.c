#include "stm32f10x.h"
#include <stdio.h>

uint16_t tseg[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x27,0x7F,0x6F};

// --- Global Variables ---
volatile uint32_t tick_ms = 0;
uint32_t counter_1 = 0;
uint32_t entry_start_tick = 0, exit_start_tick = 0;
float distance_entry_cm = -1.0f, distance_exit_cm = -1.0f;
uint8_t entry_measurement_done = 0, exit_measurement_done = 0;
uint32_t entry_pwidth = 0, exit_pwidth = 0;

uint16_t parking = 20;

uint8_t toggle_flag = 0;

void delay_10us(uint32_t t) {
    uint32_t start_tick = tick_ms;
    while ((tick_ms - start_tick) < t);
}

// --- Initialization Functions ---
void My_GPIO_Init(void) {
	RCC->APB2ENR = 0xFD;
    // PA4 (入口Trig), PA2 (出口Trig) -> 推挽輸出
    GPIOA->CRL |= (0x1 << (4 * 4)) | (0x1 << (4 * 2));

    // PA5 (入口Echo), PA3 (出口Echo) -> 浮空輸入
    GPIOA->CRL |= (0x4 << (4 * 5)) | (0x4 << (4 * 3));

    GPIOA->BRR = (1 << 4) | (1 << 2);

    GPIOB->CRL = 0x33333333;  // PB0 ~ PB7 為輸出模式
    GPIOB->CRH = 0x33333333;  // PB8 ~ PB15 為輸出模式
}

void My_Timer_Init(void) {
    RCC->APB1ENR |= (1<<0);
    RCC->APB2ENR |= (1<<11);
	GPIOA->CRL |= 0xB;
	GPIOA->CRH |= 0xB;

	TIM2->CCER = 0x1;
	TIM2->CCMR1 |= 0x60;
	TIM2->PSC = 72 - 1;
	TIM2->ARR = 20000 - 1;
	TIM2->CCR1 = 500;
	TIM2->CR1 = 1;

	TIM1->CCER = 0x1;
	TIM1->CCMR1 |= 0x60;
	TIM1->PSC = 144 - 1;
	TIM1->ARR = 20000 - 1;
	TIM1->CCR1 = 500;
	TIM1->CR1 = 1;
}

void ServoTIM2_SetAngle(uint8_t angle) {
	uint16_t pulse;
	if(angle == 0) {
		pulse = 1000;
	} else if (angle == 90) {
		pulse = 1600;
	} else {
		pulse = 2200;
	}
    TIM2->CCR1 = pulse;
}

void ServoTIM1_SetAngle(uint8_t angle) {
	uint16_t pulse;
	if(angle == 0) {
		pulse = 1000;
	} else if (angle == 90) {
		pulse = 1500;
	} else {
		pulse = 2000;
	}
    TIM1->CCR1 = pulse;
}

void My_EXTI_Init(void) {

	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	AFIO->EXTICR[0] |= (0x0 << (4 * 3)); // 清除 EXTI3
    AFIO->EXTICR[1] |= (0x0 << (4 * 1)); // 清除 EXTI5

    // 設定 EXTI3 (出口) 和 EXTI5 (入口)
    EXTI->RTSR |= (1 << 3) | (1 << 5);
    EXTI->FTSR |= (1 << 3) | (1 << 5);
    EXTI->IMR |= (1 << 3) | (1 << 5);

    // 啟用對應的中斷向量
    NVIC_SetPriority(EXTI3_IRQn, 1);
    NVIC_EnableIRQ(EXTI3_IRQn);
    NVIC_SetPriority(EXTI9_5_IRQn, 1); // PA5 使用 EXTI9_5 中斷
    NVIC_EnableIRQ(EXTI9_5_IRQn);

}

void My_SysTick_Init(void) {
    SysTick->LOAD = 270 - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = (0 << 2) | (1 << 1) | (1 << 0);
}

void My_USART1_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

    GPIOA->CRH &= ~(0xFF << 4);
    GPIOA->CRH |= (0x0B << 4);
    USART1->BRR = 0x1D4C;
    USART1->CR1 |= (1 << 3) | (1 << 13);
}

void USART1_SendString(char *str) {
    while (*str) {
        while (!(USART1->SR & (1 << 7)));
        USART1->DR = *str;
        str++;
    }
}

void USART1_SendChar(char c) {
    while (!(USART1->SR & USART_SR_TXE));  // 等待 TX 空
    USART1->DR = c;
}

void seg_send(uint16_t number){

    uint16_t first = tseg[number/10];
    uint16_t second = tseg[number%10];

    uint16_t output_value = (second << 7) | first;
	GPIOB->ODR = output_value;
}

// --- Interrupt Handlers ---
void SysTick_Handler(void) {
    tick_ms++;
    if(toggle_flag == 1){
    	counter_1++;
    	if(counter_1 >=50000){
    		GPIOB->ODR = (0x3F << 7) | 0x3F;
    	} else {
    		GPIOB->ODR = 0x0000;
    	}
    	if(counter_1 >=100000){
			counter_1 = 0;
		}
    }
}

void EXTI3_IRQHandler(void) {

	EXTI->PR = (1 << 3); // 立即清除旗標
	if (GPIOA->IDR & (1 << 3)) { // 上升緣
			// 記錄當前 SysTick 的計數
			exit_start_tick = tick_ms;

	} else { // 下降緣

			// 讀取結束時的 SysTick 計數
			uint32_t current_tick = tick_ms;
			if (current_tick >= exit_start_tick) {
				exit_pwidth = current_tick - exit_start_tick;
			} else {
				// 處理溢位的情況 (uint32_t 最大值)
				exit_pwidth = (0xFFFFFFFF - exit_start_tick) + current_tick + 1;
			}
			exit_measurement_done = 1;
	}
}


void EXTI9_5_IRQHandler(void) {

	EXTI->PR = (1 << 5); // 立即清除旗標
	if (GPIOA->IDR & (1 << 5)) { // 上升緣

			// 記錄當前 SysTick 的計數
			entry_start_tick = tick_ms;

	} else { // 下降緣

			// 讀取結束時的 SysTick 計數
			uint32_t current_tick = tick_ms;

			if (current_tick >= entry_start_tick) {
				entry_pwidth = current_tick - entry_start_tick;
			} else {
				// 處理溢位的情況 (uint32_t 最大值)
				entry_pwidth = (0xFFFFFFFF - entry_start_tick) + current_tick + 1;
			}
			entry_measurement_done = 1;

	}
}


int main(void) {
    My_GPIO_Init();
    My_Timer_Init();
    My_EXTI_Init();
    My_SysTick_Init();
    My_USART1_Init();

    ServoTIM1_SetAngle(0);
    ServoTIM2_SetAngle(180);
    uint8_t choose_flag = 0;

    uint32_t loop_start_time;
    uint32_t loop2_start_time;

    uint32_t car = 20 - parking;

    while (1) {
    	car = 20 - parking;
    	if (parking != 0) {
			seg_send(parking);

    		if (choose_flag == 0) {

				if (entry_measurement_done == 0){
					loop_start_time = tick_ms;
					GPIOA->BSRR = (1 << 4);
					delay_10us(1);
					GPIOA->BRR = (1 << 4);
				}

				if (entry_measurement_done) {
					distance_entry_cm = (float)entry_pwidth * 0.1715f;
					entry_measurement_done = 0;
					USART1_SendString("Car : ");
					car = 20 - parking;
					if(car >= 10) {
						USART1_SendChar(car/10 + '0');
						USART1_SendChar(car%10 + '0');
						USART1_SendChar('\r');
						USART1_SendChar('\n');
					} else {
						USART1_SendChar(car + '0');
						USART1_SendChar('\r');
						USART1_SendChar('\n');
					}

					if (parking > 0) {
						if (distance_entry_cm < 100) {
							parking -= 1;
							seg_send(parking);
							ServoTIM2_SetAngle(90);
							delay_10us(100000);
							ServoTIM2_SetAngle(180);
						} else {
							ServoTIM2_SetAngle(180);
							delay_10us(10000);
						}
					}
					car = 20 - parking;
					USART1_SendString("Car : ");
					if(car >= 10) {
						USART1_SendChar(car/10 + '0');
						USART1_SendChar(car%10 + '0');
						USART1_SendChar('\r');
						USART1_SendChar('\n');
					} else {
						USART1_SendChar(car + '0');
						USART1_SendChar('\r');
						USART1_SendChar('\n');
					}
					if(parking == 0) {
						continue;
					}
				}
				else {
					car = 20 - parking;
					USART1_SendString("Car : ");
					if(car >= 10) {
						USART1_SendChar(car/10 + '0');
						USART1_SendChar(car%10 + '0');
						USART1_SendChar('\r');
						USART1_SendChar('\n');
					} else {
						USART1_SendChar(car + '0');
						USART1_SendChar('\r');
						USART1_SendChar('\n');
					}
				}
				choose_flag = 1;
    		} else {
    			if (exit_measurement_done == 0){
    				loop_start_time = tick_ms;
					GPIOA->BSRR = (1 << 2);
					delay_10us(1);
					GPIOA->BRR = (1 << 2);
				}

				if (exit_measurement_done) {
					distance_exit_cm = (float)exit_pwidth * 0.1715f;
					exit_measurement_done = 0;
					USART1_SendString("Car : ");
					car = 20 - parking;
					if(car >= 10) {
						USART1_SendChar(car/10 + '0');
						USART1_SendChar(car%10 + '0');
						USART1_SendChar('\r');
						USART1_SendChar('\n');
					} else {
						USART1_SendChar(car + '0');
						USART1_SendChar('\r');
						USART1_SendChar('\n');
					}
					if (parking < 20) {
						if (distance_exit_cm < 100) {
							parking += 1;
							seg_send(parking);
							ServoTIM1_SetAngle(90);
							delay_10us(100000);
							ServoTIM1_SetAngle(0);
						} else {
							ServoTIM1_SetAngle(0);
							delay_10us(10000);
						}
					}
					USART1_SendString("Car : ");
					car = 20 - parking;
					if(car >= 10) {
						USART1_SendChar(car/10 + '0');
						USART1_SendChar(car%10 + '0');
						USART1_SendChar('\r');
						USART1_SendChar('\n');
					} else {
						USART1_SendChar(car + '0');
						USART1_SendChar('\r');
						USART1_SendChar('\n');
					}
				} else {
					USART1_SendString("Car : ");
					car = 20 - parking;
					if(car >= 10) {
						USART1_SendChar(car/10 + '0');
						USART1_SendChar(car%10 + '0');
						USART1_SendChar('\r');
						USART1_SendChar('\n');
					} else {
						USART1_SendChar(car + '0');
						USART1_SendChar('\r');
						USART1_SendChar('\n');
					}
				}
				choose_flag = 0;
    		}

    		// 補齊 delay時間差
	        uint32_t time_elapsed = tick_ms - loop_start_time;
	        if (time_elapsed < 250000) {
	            delay_10us(250000 - time_elapsed);
	        }

    	} else {
			toggle_flag = 1;
			loop2_start_time = tick_ms;
			GPIOA->BSRR = (1 << 2);
			delay_10us(1);
			GPIOA->BRR = (1 << 2);

			if (exit_measurement_done) {
				distance_exit_cm = (float)exit_pwidth * 0.1715f;
				exit_measurement_done = 0;
				entry_measurement_done = 0;
				USART1_SendString("Car : ");
				car = 20 - parking;
				if(car >= 10) {
					USART1_SendChar(car/10 + '0');
					USART1_SendChar(car%10 + '0');
					USART1_SendChar('\r');
					USART1_SendChar('\n');
				} else {
					USART1_SendChar(car + '0');
					USART1_SendChar('\r');
					USART1_SendChar('\n');
				}
				if (distance_exit_cm < 100) {
					parking += 1;
					toggle_flag = 0;
					seg_send(parking);
					ServoTIM1_SetAngle(90);
					delay_10us(100000);
					ServoTIM1_SetAngle(0);
				} else {
					ServoTIM1_SetAngle(0);
					delay_10us(10000);
				}
				car = 20 - parking;
				USART1_SendString("Car : ");
				if(car >= 10) {
					USART1_SendChar(car/10 + '0');
					USART1_SendChar(car%10 + '0');
					USART1_SendChar('\r');
					USART1_SendChar('\n');
				} else {
					USART1_SendChar(parking + '0');
					USART1_SendChar('\r');
					USART1_SendChar('\n');
				}
			} else {
				USART1_SendString("Car : ");
				car = 20 - parking;
				if(car >= 10) {
					USART1_SendChar(car/10 + '0');
					USART1_SendChar(car%10 + '0');
					USART1_SendChar('\r');
					USART1_SendChar('\n');
				} else {
					USART1_SendChar(car + '0');
					USART1_SendChar('\r');
					USART1_SendChar('\n');
				}
			}
			uint32_t time2_elapsed = tick_ms - loop2_start_time;
			if (time2_elapsed < 500000) {
				delay_10us(500000 - time2_elapsed);
			}
		}
    }

}
