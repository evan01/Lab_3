#include "stm32f4xx.h"
#include "systick.h"
 
#define MHz 1000000L
#define KHz 1000L
 
/* The LED indicators on the STM32F4Discovery board */
#define LED_PORT        GPIOD
#define LED_PORT_CLOCK  RCC_AHB1Periph_GPIOD
#define GREEN_PIN       GPIO_Pin_12
#define ORANGE_PIN      GPIO_Pin_13
#define RED_PIN         GPIO_Pin_14
#define BLUE_PIN        GPIO_Pin_15
#define ALL_LED_PINS    GREEN_PIN | ORANGE_PIN | RED_PIN | BLUE_PIN
#define GREEN_LED       LED_PORT,GREEN_PIN
#define ORANGE_LED      LED_PORT,ORANGE_PIN
#define RED_LED         LED_PORT,RED_PIN
#define BLUE_LED        LED_PORT,BLUE_PIN
#define ALL_LEDS        LED_PORT,ALL_LED_PINS
 
#define TIMER TIM4
#define TIMER_PERIPHERAL_CLOCK RCC_APB1Periph_TIM4
#define TIMER_AF GPIO_AF_TIM4
 
/* unfortunate globals */
uint32_t PWM_Frequency = 100;
uint32_t PWM_Steps = 100;
 
 
uint32_t get_timer_clock_frequency (void)
{
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq (&RCC_Clocks);
  uint32_t multiplier;
  if (RCC_Clocks.PCLK1_Frequency == RCC_Clocks.SYSCLK_Frequency) {
    multiplier = 1;
  } else {
    multiplier = 2;
  }
  return multiplier * RCC_Clocks.PCLK1_Frequency;
}
 
void timer_clock_init (void)
{
  uint32_t TIMER_Frequency = get_timer_clock_frequency();
  uint32_t COUNTER_Frequency = PWM_Steps * PWM_Frequency;
  uint32_t PSC_Value = (TIMER_Frequency / COUNTER_Frequency) - 1;
  uint16_t ARR_Value = PWM_Steps - 1;
 
  /* make sure the peripheral is clocked */
  RCC_APB1PeriphClockCmd (TIMER_PERIPHERAL_CLOCK, ENABLE);
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  /* set everything back to default values */
  TIM_TimeBaseStructInit (&TIM_TimeBaseStructure);
  /* only changes from the defaults are needed */
  TIM_TimeBaseStructure.TIM_Period = ARR_Value;
  TIM_TimeBaseStructure.TIM_Prescaler = PSC_Value;
  TIM_TimeBaseInit (TIMER, &TIM_TimeBaseStructure);
}
 
void timer_start (void)
{
  TIM_Cmd (TIMER, ENABLE);
}
 
void timer_stop (void)
{
  TIM_Cmd (TIMER, DISABLE);
}
 
 
void timer_pwm_init (void)
{
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  /* always initialise local variables before use */
  TIM_OCStructInit (&TIM_OCInitStructure);
 
  /* Common settings for all channels */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_Pulse = 0;
 
  /* Channel2 - ORANGE LED*/
  TIM_OC2Init (TIMER, &TIM_OCInitStructure);
 
  /* Channel3 - RED LED*/
  TIM_OC3Init (TIMER, &TIM_OCInitStructure);
 
  /* Channel4 - BLUE LED*/
  /* make this the opposite polarity to the other two */
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OC4Init (TIMER, &TIM_OCInitStructure);
}
 
 
// these are the LEDs on the STM32F4Discovery board
void board_leds_init (void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  /* always initialise local variables before use */
  GPIO_StructInit (&GPIO_InitStructure);
 
  RCC_AHB1PeriphClockCmd (LED_PORT_CLOCK, ENABLE);
 
  /* these pins will be controlled by the CCRx registers */
  GPIO_InitStructure.GPIO_Pin = ORANGE_PIN + RED_PIN + BLUE_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_Init (LED_PORT, &GPIO_InitStructure);
 
  /* ensure that the pins all start off in a known state */
  GPIO_ResetBits (LED_PORT, ORANGE_PIN + RED_PIN + BLUE_PIN);
 
  /* this one is used with delay_ms() to act as a timing reference */
  GPIO_InitStructure.GPIO_Pin = GREEN_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init (LED_PORT, &GPIO_InitStructure);
 
  /* The others get connected to the AF function for the timer */
  GPIO_PinAFConfig (GPIOD, GPIO_PinSource13, TIMER_AF);
  GPIO_PinAFConfig (GPIOD, GPIO_PinSource14, TIMER_AF);
  GPIO_PinAFConfig (GPIOD, GPIO_PinSource15, TIMER_AF);
}
 
// return a fairly gaussian random number in the range 0-99 inclusive
int rnd (void)
{
  int intensity = 0;
  int sampleCount = 10;
  for (int i = 0; i < sampleCount; i++) {
    intensity += rand() % 100;
  }
  return intensity / sampleCount;
}
 
 
/* the flashing green LED acts as a visual timing reference */
void flash_green_led_forever (void)
{
  int brightness = 0;
  int increment = 5;
  while (1) {
    for (int i = 0; i < 20; i++) {
      delay_ms (50);
      TIM_SetCompare3 (TIMER, rnd());
      brightness += increment;
      TIM_SetCompare2 (TIMER, brightness);
      TIM_SetCompare4 (TIMER, brightness); // note opposite polarity
    }
    increment = -increment;
    GPIO_ToggleBits (GREEN_LED);
  }
}
 
int main (void)
{
  systickInit (1000);
  board_leds_init();
  timer_clock_init();
  timer_pwm_init();
  timer_start();
  flash_green_led_forever();
  return 0; // there is no going back but keep the compiler happy
}