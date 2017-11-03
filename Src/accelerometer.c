#include "LIS3DSH.h"

//IMPLEMENT CALLBACK FUNCTION
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	printf("CALLBACK SUCCESS");
	while (1)
  {
      float Buffer[3];
      float accX, accY, accZ;
      LIS3DSH_ReadACC(&Buffer[0]);
      accX = (float)Buffer[0];
      accY = (float)Buffer[1];
      accZ = (float)Buffer[2];
      printf("X: %3f   Y: %3f   Z: %3f  absX: %d\n", accX, accY, accZ , (int)(Buffer[0]));
//	printf("Interrupt COUNT: %d\n", INTERUPTCOUNT);
      HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);
			break;
  }
}
