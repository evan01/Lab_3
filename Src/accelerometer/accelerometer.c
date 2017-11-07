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

static float filter_coef[5] = {0.2,0.2,0.5,0.2,0.5};

LIS3DSH_InitTypeDef Acc_instance;
LIS3DSH_DRYInterruptConfigTypeDef Acc_interruptConfig;

// ***** CALLBACK FUNCTION *** gets called from stm32f4xx_it.c > stm32f4xx_hal_gpio.c, on INTERRUPT
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

    float Buffer[3];
    float accX, accY, accZ;

    //First get the filtered x,y,z readings from the accelerometer
    LIS3DSH_ReadACC(&Buffer[0]);
    accX = (float)Buffer[0];
    accY = (float)Buffer[1];
    accZ = (float)Buffer[2];
	
	printf("RAW----->X: %3f   Y: %3f   Z: %3f\n", accX, accY, accZ);

    //Then calibrate these readings,using calibration values
    float readings[3]= {
        calibrationData[0][0]*accX + calibrationData[3][0],
        calibrationData[1][1]*accY + calibrationData[3][1],
        calibrationData[2][2]*accZ + calibrationData[3][2]
    };

    //then filter each readings
    float out[3];
    for (int i = 0; i < 3; ++i) {
        out[i] = readings[i]*filter_coef[0] +
        in_lastSamples[i]*filter_coef[1] +
        in_oldestSample[i]*filter_coef[2] +
        out_lastSamples[i]*filter_coef[3] +
        out_oldestSample[i]*filter_coef[4];
    }
	
	printf("FILTERED----->X: %3f   Y: %3f   Z: %3f\n", out[0], out[1], out[2]);
	
    /* creating the pitch roll values */
    pitch = (atan2((- out[1]) , sqrt(out[0]* out[0] + out[2] * out[2]))*180.0)/M_PI;
    roll = (atan2(out[0] , out[2])*180.0)/M_PI;
    printf("PITCH: %f\nROLL:%f\n\n",pitch,roll);

    HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);

    //Store values for next time, free old memory
    free(in_oldestSample);
    free(out_oldestSample);
    in_oldestSample = in_lastSamples;
    in_lastSamples = sample;
    out_oldestSample = out_lastSamples;
    out_lastSamples = out;

}

//float* IIR_C(float* InputArray, float* OutputArray, float* coef, int Length){
//
//    /*Assumes that input array has size>=2 */
//    if(Length > 0){
//        OutputArray[0]= coef[0]*InputArray[0];
//        if(Length > 1){
//            OutputArray[1]= coef[0]*InputArray[1]+ coef[1]*InputArray[0] + coef[3]*OutputArray[0];
//            for (int i=2; i < Length; i++){
//                OutputArray[i] =
//                    coef[0]*InputArray[i] +
//                    coef[1]*InputArray[i-1] +
//                    coef[2]*InputArray[i-2] +
//                    coef[3]*OutputArray[i-1]+
//                    coef[4]*OutputArray[i-2];
//            }
//        }
//    }
//    return OutputArray;
//}

/**
 * This is a filter function, with which we feed in the accelerometer values.
 * @param InputArray
 * @param OutputArray
 * @param coef
 * @param Length
 * @return
 */
//float* IIR_CMSIS(float* InputArray, float* OutputArray, int Length){
//    float pState[4] = {0.0,0.0,0.0,0.0};
//    int numStages = 1;
//    /* initialize the biquad filter */
////    arm_biquad_casd_df1_inst_f32 S1 = {numStages, pState, coef};
//    /* process the input */
//
////    arm_biquad_cascade_df1_f32(&S1, InputArray, OutputArray, Length);
//    return OutputArray;
//}


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

