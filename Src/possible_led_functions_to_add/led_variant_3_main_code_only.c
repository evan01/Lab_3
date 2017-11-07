//https://letanphuc.net/2015/06/stm32f0-timer-tutorial-and-counter-tutorial/

/* USER CODE BEGIN 2 */
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);  
/* USER CODE END 2 */

/* USER CODE BEGIN WHILE */
while (1)  
 {
 for (pwm=0;pwm<=200;pwm++)  //darkest to brightest: 0-100% duty cycle
  {
  __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, pwm); //update pwm value
  HAL_Delay(10);
  }
 HAL_Delay(400);  //hold for 400ms
 for (pwm=200;pwm>=0;pwm--)  //brightest to darkest: 100-0% duty cycle
  {
  __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_3, pwm);
  HAL_Delay(10);
  }
 HAL_Delay(400);   //hold for 400ms

 /* USER CODE END WHILE */
 /* USER CODE BEGIN 3 */
 }