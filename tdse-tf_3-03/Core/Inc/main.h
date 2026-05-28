/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define Med_I_LED_Pin GPIO_PIN_0
#define Med_I_LED_GPIO_Port GPIOC
#define Med_I_Bomba_Pin GPIO_PIN_1
#define Med_I_Bomba_GPIO_Port GPIOC
#define LCD_D7_Pin GPIO_PIN_0
#define LCD_D7_GPIO_Port GPIOA
#define S_Nivel_Agua_Pin GPIO_PIN_1
#define S_Nivel_Agua_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define S_Luz_Pin GPIO_PIN_4
#define S_Luz_GPIO_Port GPIOA
#define PWM_LED_G_Pin GPIO_PIN_5
#define PWM_LED_G_GPIO_Port GPIOA
#define PWM_LED_R_Pin GPIO_PIN_6
#define PWM_LED_R_GPIO_Port GPIOA
#define PWM_LED_B_Pin GPIO_PIN_7
#define PWM_LED_B_GPIO_Port GPIOA
#define LCD_D6_Pin GPIO_PIN_4
#define LCD_D6_GPIO_Port GPIOC
#define S_Hum_Suelo_Pin GPIO_PIN_0
#define S_Hum_Suelo_GPIO_Port GPIOB
#define Btn_Esc_Pin GPIO_PIN_1
#define Btn_Esc_GPIO_Port GPIOB
#define Btn_Next_Pin GPIO_PIN_2
#define Btn_Next_GPIO_Port GPIOB
#define LCD_E_Pin GPIO_PIN_10
#define LCD_E_GPIO_Port GPIOB
#define PWM_Bomba_Pin GPIO_PIN_11
#define PWM_Bomba_GPIO_Port GPIOB
#define PWM_Buzzer_Pin GPIO_PIN_13
#define PWM_Buzzer_GPIO_Port GPIOB
#define Btn_Prev_Pin GPIO_PIN_14
#define Btn_Prev_GPIO_Port GPIOB
#define Btn_Enter_Pin GPIO_PIN_15
#define Btn_Enter_GPIO_Port GPIOB
#define PWM_Tira_LED_Pin GPIO_PIN_6
#define PWM_Tira_LED_GPIO_Port GPIOC
#define D9_Pin GPIO_PIN_7
#define D9_GPIO_Port GPIOC
#define S_Hum_Temp_Amb_Pin GPIO_PIN_8
#define S_Hum_Temp_Amb_GPIO_Port GPIOC
#define S_Hum_Temp_Amb_EXTI_IRQn EXTI9_5_IRQn
#define LCD_RS_Pin GPIO_PIN_8
#define LCD_RS_GPIO_Port GPIOA
#define D8_Pin GPIO_PIN_9
#define D8_GPIO_Port GPIOA
#define LCD_D5_Pin GPIO_PIN_10
#define LCD_D5_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define LCD_D4_Pin GPIO_PIN_4
#define LCD_D4_GPIO_Port GPIOB
#define D10_Pin GPIO_PIN_6
#define D10_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
