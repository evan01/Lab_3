/* Includes ------------------------------------------------------------------*/
#include <stm32f407xx.h>
#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "display.h"
#include "accelerometer/accelerometer.h"
#include "state_machine.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "tim.h"
#include "keypad/keypad.h"

/* Private variables ---------------------------------------------------------*/
float displayValue = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);


uint32_t judgeDuty(uint32_t target, float current) {
    float rval;
    rval = 200.0 * ((fabsf((float) target - (float) current)) / 180.0);
    return (uint32_t) rval;

}

int display(int value) {
    //display the digits
    if (value == 1) {
        return displayDigits(pitch);
    } else if (value == 0) {
        return displayDigits(roll);
    } else {
        //In sleep state
        return displayDigits(value);
    }
}

int infiniteLoop() {
    static uint32_t duty = 0;
    //Main program execution ins here.
    if (state == PITCH_MONITOR_STATE) {
        //printf("pitch %f\n", (float)pitch);
        display(1);
        uint32_t intensity = judgeDuty(target_pitch, pitch);
        setLedIntensityPitch(intensity);
    } else if (state == ROLL_MONITOR_STATE) {
        display(0);
        uint32_t intensity = judgeDuty(target_roll, roll);
        setLedIntensityPitch(intensity);
    } else if (state == START_STATE) {
        display(8888);
    } else if (state == SLEEP_STATE) {
        resetDisplay();
//    }
    } else if (state == ENTER_ROLL_STATE) {
        float f;
        sscanf(roll_buf, "%f", &f);
        display((int) f);
    } else if (state == ENTER_PITCH_STATE) {
        float f;
        sscanf(pitch_buf, "%f", &f);
        display((int) f);
    }

//			setLedIntensityPitch(duty);

    setLedIntensityRoll(0);
    //setLedIntensity(duty, 3);
    //setLedIntensity(duty, 4);
    duty += 10;
    if (duty == 100) {
        duty = 0;
    }


}

int main(void) {

    initTimer();
    //Initialize the accelerometer
    initializeAccelerometer();

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
//		MX_TIM2_Init();
//		HAL_TIM_Base_Start_IT(&htim2);

    while (1) {

        infiniteLoop();
        readInput();

    }

}

/** System Clock Configuration
	The clock source is configured as external at 168 MHz HCLK
*/
void SystemClock_Config(void) {

    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

        __PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1
                                  | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    //HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_ACR_LATENCY_5WS);
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}



/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
    printf("ERROR");
}

#endif

