/*
 *  ACCELEROMETER CODE
	This is the code for the accelerometer on the board. Here's how to set up the accelerometer.

	gpio.c
		-GPIO_InitStruct.Pin = GPIO_PIN_0;
		-GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
		-GPIO_InitStruct.Pull = GPIO_NOPULL;
		-HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    main.c
        -initializeAccelerometer()
*/


#include "LIS3DSH.h"
#include <stm32f407xx.h>
#include <math.h>
#include "arm_math.h"
#define M_PI 3.14159265358979323846

//Global Constants
LIS3DSH_InitTypeDef Acc_instance;
LIS3DSH_DRYInterruptConfigTypeDef Acc_interruptConfig;
double roll = 0.00;
double pitch = 0.00;

//FILTER SETTINGS
float Coef[5] = {0.2, 0.4, 0.2, 0.3, -0.2};


LIS3DSH_InitTypeDef Acc_instance;
LIS3DSH_DRYInterruptConfigTypeDef Acc_interruptConfig;

// ***** CALLBACK FUNCTION *** gets called from stm32f4xx_it.c > stm32f4xx_hal_gpio.c, on INTERRUPT
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    float Buffer[3];
    float accX, accY, accZ;

    //First get the filtered x,y,z readings from the accelerometer
    LIS3DSH_ReadACC(&Buffer[0]);
    accX = (float)Buffer[0];
    accY = (float)Buffer[1];
    accZ = (float)Buffer[2];
    
//    printf("X: %3f   Y: %3f   Z: %3f  absX: %d\n", accX, accY, accZ , (int)(Buffer[0]));

    /*
    creating the pitch roll values
    */

    pitch = (atan2((- accY) , sqrt(accX* accX + accZ * accZ))*180.0)/M_PI;
    roll = (atan2(accX , accZ)*180.0)/M_PI;
    printf("PITCH: %f\nROLL:%f\n\n",pitch,roll);

    HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);
}

/**
 * This is a filter function, with which we feed in the accelerometer values.
 * @param InputArray
 * @param OutputArray
 * @param coef
 * @param Length
 * @return
 */
float* IIR_CMSIS(float* InputArray, float* OutputArray, int Length){
    float pState[4] = {0.0,0.0,0.0,0.0};
    int numStages = 1;
    /* initialize the biquad filter */
    arm_biquad_casd_df1_inst_f32 S1 = {numStages, pState, coef};
    /* process the input */
    arm_biquad_cascade_df1_f32(&S1, InputArray, OutputArray, Length);
    return OutputArray;
}

/**
 * This function should only be called once, will print callibration values
 */
void calibrateAccelerometer(void){
    for (int i = 0; i < 1000; ++i) {

    }
}

/**
 * Initilizes the accelerometer with the configuration struct
 */
void initializeAccelerometer(void){

    Acc_instance.Axes_Enable				= LIS3DSH_XYZ_ENABLE;
    Acc_instance.AA_Filter_BW				= LIS3DSH_AA_BW_50;
    Acc_instance.Full_Scale					= LIS3DSH_FULLSCALE_2;
    Acc_instance.Power_Mode_Output_DataRate		= LIS3DSH_DATARATE_25;
    Acc_instance.Self_Test					= LIS3DSH_SELFTEST_NORMAL;
    Acc_instance.Continous_Update   = LIS3DSH_ContinousUpdate_Enabled;

    LIS3DSH_Init(&Acc_instance); //there are other parameters configured in this function if needed...

    /* Enabling interrupt conflicts with push button. Be careful when you plan to
    use the interrupt of the accelerometer sensor connceted to PIN A.0
    Acc_interruptConfig.
    */
    Acc_interruptConfig.Dataready_Interrupt = LIS3DSH_DATA_READY_INTERRUPT_ENABLED;
    Acc_interruptConfig.Interrupt_signal = LIS3DSH_ACTIVE_HIGH_INTERRUPT_SIGNAL;
    Acc_interruptConfig.Interrupt_type = LIS3DSH_INTERRUPT_REQUEST_PULSED;
    LIS3DSH_DataReadyInterruptConfig(&Acc_interruptConfig);

    //ENABLE the INTERRUPTS
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_NVIC_SetPriority(EXTI0_IRQn, 4,4);
}
