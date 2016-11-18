#include "bsp.h"


USBD_HandleTypeDef USBD_Device;

extern void stop();

void bsp_init()
{
  // STM32Cube HAL Init
  HAL_Init();

  // Clock Setup
  // SYSCLK(Hz)    = 216000000
  // HCLK(Hz)      = 216000000
  // HSE(Hz)       = 25000000
  SystemClock_Config();

  SCB_EnableDCache();
  SCB_EnableICache();


  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();


  led_init();
  button_init();

  //HAL_Delay(100);

  /* Init Device Library */
  USBD_Init(&USBD_Device, &VCP_Desc, 0);


  /* Add Supported Class */
  USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);

  /* Add CDC Interface Class */
  USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);

  /* Start Device Process */
  USBD_Start(&USBD_Device);
}


void bsp_deinit()
{

  USBD_DeInit(&USBD_Device);
  HAL_DeInit();
  __disable_irq();

  SCB_InvalidateDCache();
  SCB_InvalidateICache();
  SCB_DisableDCache();
  SCB_DisableICache();

}
