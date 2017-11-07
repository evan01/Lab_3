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
#include <stdlib.h>

static float *in_oldestSample;
static float *in_lastSamples;
static float *out_oldestSample;
static float *out_lastSamples;
static float *sample;
#define M_PI 3.14159265358979323846

//Global Constants
double roll = 0.00;
double pitch = 0.00;

static float calibrationData[4][3] = {
    {0.9492311,-0.0124871,0.0021331},
    {-0.0420631,0.9805961,-0.0221681},
    {0.0011911,0.0137351,0.9881551},
    {-6.2797691,-2.6266551,-26.1443411}
};

static float filter_coef[5] = {0.2,0.2,0.2,0.2,0.2};

LIS3DSH_InitTypeDef Acc_instance;
LIS3DSH_DRYInterruptConfigTypeDef Acc_interruptConfig;


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
    arm_biquad_casd_df1_inst_f32 S1 = {numStages, pState, filter_coef};
    /* process the input */

    arm_biquad_cascade_df1_f32(&S1, InputArray, OutputArray, Length);
    return OutputArray;
}


// ***** CALLBACK FUNCTION *** gets called from stm32f4xx_it.c > stm32f4xx_hal_gpio.c, on INTERRUPT
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

    float Buffer[3];
    float accX, accY, accZ;

    //First get the filtered x,y,z readings from the accelerometer
    LIS3DSH_ReadACC(&Buffer[0]);
    accX = (float)Buffer[0];
    accY = (float)Buffer[1];
    accZ = (float)Buffer[2];
	
	printf("RAW----->\t\tX: %3f   Y: %3f   Z: %3f\n", accX, accY, accZ);

    //Then calibrate these readings,using calibration values
    float readings[3]= {
        calibrationData[0][0]*accX + calibrationData[3][0],
        calibrationData[1][1]*accY + calibrationData[3][1],
        calibrationData[2][2]*accZ + calibrationData[3][2]
    };

    printf("CLBRT----->\t\tX: %3f   Y: %3f   Z: %3f\n", readings[0], readings[1], readings[2]);

    float x_in[3] = {in_oldestSample[0], in_lastSamples[0], readings[0]};
    float y_in[3] = {in_oldestSample[1], in_lastSamples[1], readings[1]};
    float z_in[3] = {in_oldestSample[2], in_lastSamples[2], readings[2]};

    float x_out[3] = {out_oldestSample[0], out_lastSamples[0], 0};
    float y_out[3] = {out_oldestSample[1], out_lastSamples[1], 0};
    float z_out[3] = {out_oldestSample[2], out_lastSamples[2], 0};

    IIR_CMSIS(x_in,x_out,3);
    IIR_CMSIS(y_in,y_out,3);
    IIR_CMSIS(z_in,z_out,3);

    accX = x_out[2];
    accY = y_out[2];
    accZ = z_out[2];

	printf("FILTERED----->\t\tX: %3f   Y: %3f   Z: %3f\n", x_out[2], y_out[2], z_out[2]);
	
    /* creating the pitch roll values */
    pitch = (atan2((- accY) , sqrt(accX* accX + accZ * accZ))*180.0)/M_PI;
    roll = (atan2(accX , accZ)*180.0)/M_PI;

    printf("PITCH: %f\nROLL:%f\n\n",pitch,roll);

    HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);

    //Store values for next time, free old memory
    free(in_oldestSample);
    free(out_oldestSample);
    in_oldestSample = in_lastSamples;
    in_lastSamples = sample;
    out_oldestSample = out_lastSamples;
    float out[3] = {accX,accY,accZ};
    memcpy(out_lastSamples,out, sizeof(float)*3);
}




/**
 * Initilizes the accelerometer with the configuration struct
 */
void initializeAccelerometer(void){

    Acc_instance.Axes_Enable				= LIS3DSH_XYZ_ENABLE;
    Acc_instance.AA_Filter_BW				= LIS3DSH_AA_BW_50;
    Acc_instance.Full_Scale					= LIS3DSH_FULLSCALE_2;
    Acc_instance.Power_Mode_Output_DataRate		= LIS3DSH_DATARATE_25; //todo why is this 25?
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

    //Initialize History variables
   in_oldestSample = malloc(sizeof(float)*3);
   in_lastSamples = malloc(sizeof(float)*3);
   out_oldestSample = malloc(sizeof(float)*3);
   out_lastSamples = malloc(sizeof(float)*3);
   sample = malloc(sizeof(float)*3);

}

