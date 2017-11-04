#include "LIS3DSH.h"
#include <stdio.h>

/*
	This is the code for the accelerometer on the board. Here's how to set up the accelerometer.
	
	main.c (Enable the interupts to occur on pinE0
		HAL_NVIC_EnableIRQ(EXTI0_IRQn);
		HAL_NVIC_SetPriority(EXTI0_IRQn, 4,4);
	gpio.c
		-GPIO_InitStruct.Pin = GPIO_PIN_0;
		-GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		-GPIO_InitStruct.Pull = GPIO_NOPULL;
		-HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	

*/

//IMPLEMENT CALLBACK FUNCTION, gets called from stm32f4xx_it.c > stm32f4xx_hal_gpio.c, 
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	printf("CALLBACK SUCCESS\n");
    float Buffer[3];
    float accX, accY, accZ;
    LIS3DSH_ReadACC(&Buffer[0]);
    accX = (float)Buffer[0];
    accY = (float)Buffer[1];
	accZ = (float)Buffer[2];
    printf("X: %3f   Y: %3f   Z: %3f  absX: %d\n", accX, accY, accZ , (int)(Buffer[0]));
    HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);
}


