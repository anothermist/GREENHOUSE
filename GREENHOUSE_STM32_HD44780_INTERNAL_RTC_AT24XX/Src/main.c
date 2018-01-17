/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"

/* USER CODE BEGIN Includes */
#include "string.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "hd44780.h"
#include "at24xx.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
RTC_TimeTypeDef clkTime;
RTC_DateTypeDef clkDate;
char bufstr[10], str[25];
uint8_t set = 0, sec = 0, min = 0, hour = 0, a0min = 0, a0hour = 0, a1min = 0, a1hour = 0, a2min = 0, a2hour = 0;
uint64_t millis = 0, modeSwitch = 0, updateUART = 0;
bool lcd_out = 0, big = 0, triggerClear = 1, alarm = 0, powerLight = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_RTC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

uint8_t decToBcd(uint8_t val)
{
	return ((val / 10 * 16) + (val % 10));
}

uint8_t bcdToDec(uint8_t val)
{
	return ((val / 16 * 10) + (val % 16));
}

uint8_t data[] = "UART OK\n";
uint8_t rx_index = 0;
uint8_t rx_data;
uint8_t rx_buffer[256];

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t i;
	if (huart->Instance == USART1)
	{
		if (rx_index == 0)
		{
			for (i = 0; i < 255; i++)
			{
				rx_buffer[i] = 0;
			}
		}
		rx_buffer[rx_index++] = rx_data;

		HAL_UART_Receive_IT(&huart1, &rx_data, 1);
	}
}

void relayLight(void)
{
	if (hour == a1hour && min == a1min) powerLight = 1;
	if (hour == a2hour && min == a2min) powerLight = 0;

	if (powerLight)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	}
}

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  MX_RTC_Init();

  /* USER CODE BEGIN 2 */

	HAL_RTC_Init(&hrtc);
	HAL_RTCEx_SetSecond_IT(&hrtc);
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_UART_Receive_IT(&huart1, &rx_data, 1);

	uint8_t uartTransmit[] = "UART OK\r\n";
	HAL_UART_Transmit(&huart1, uartTransmit, sizeof(uartTransmit), 100);

	HD44780_Init();
	HD44780_Clear();
	HD44780_SetPos(0, 0);
	HD44780_String("*** STRING 0 ***");
	HD44780_SetPos(1, 0);
	HD44780_String("*** STRING 1 ***");
	HAL_Delay(500);
	HD44780_Clear();

	while (AT24XX_IsConnected() == false)
	{
		HAL_Delay(100);
	}

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
		if (rx_index != 0)
		{
			HD44780_Clear();
			HD44780_SetPos(0, 0);
			HD44780_String((char*)rx_buffer);
			rx_index = 0;
			HAL_Delay(1000);
		}

		if (lcd_out)
		{
			lcd_out = false;

			HAL_RTC_GetTime(&hrtc, &clkTime, RTC_FORMAT_BCD);
			sec = bcdToDec(clkTime.Seconds);
			min = bcdToDec(clkTime.Minutes);
			hour = bcdToDec(clkTime.Hours);

			if (!set && !big && millis > modeSwitch + 5000)
			{
			modeSwitch = millis;
			triggerClear = 1;
			big = 1;
			}
			else if (!set && big && millis > modeSwitch + 5000)
			{
			modeSwitch = millis;
			big = 0;
			}
			else if (set && millis > modeSwitch + 10000)
			{
				set = 0;
			}

			if (big)
			{
				drawBigDigits(hour / 10, 0);
				drawBigDigits(hour % 10, 4);
				drawBigDigits(min / 10, 9);
				drawBigDigits(min % 10, 13);

				if (sec % 2 == 0)
				{
					HD44780_SetPos(0, 7);
					HD44780_String("+ ");
					HD44780_SetPos(1, 7);
					HD44780_String(" +");
					HD44780_SetPos(1, 3);
					HD44780_String("+");
					HD44780_SetPos(0, 12);
					HD44780_String("+");
					HD44780_SetPos(0, 3);
					HD44780_String(" ");
					HD44780_SetPos(1, 12);
					HD44780_String(" ");
				}
				else
				{
					HD44780_SetPos(0, 7);
					HD44780_String(" +");
					HD44780_SetPos(1, 7);
					HD44780_String("+ ");
					HD44780_SetPos(0, 3);
					HD44780_String("+");
					HD44780_SetPos(1, 12);
					HD44780_String("+");
					HD44780_SetPos(1, 3);
					HD44780_String(" ");
					HD44780_SetPos(0, 12);
					HD44780_String(" ");
				}
			}
			else
			{
				if (triggerClear)
				{
					HD44780_Clear();
					triggerClear = 0;
				}

				sprintf(str, "%.2x:", clkTime.Hours);
				sprintf(bufstr, "%.2x:", clkTime.Minutes);
				strcat(str, bufstr);
				sprintf(bufstr, "%.2x", clkTime.Seconds);
				strcat(str, bufstr);
				HD44780_SetPos(0, 0);
				HD44780_String(str);
				str[0] = 0;

				a1hour = AT24XX_Read(10);
				HD44780_SetPos(0, 11);
				HD44780_SendChar((char)((a1hour / 10) % 10) + 0x30);
				HD44780_SendChar((char)((a1hour) % 10) + 0x30);
				HD44780_SendChar(':');
				a1min = AT24XX_Read(11);
				HD44780_SendChar((char)((a1min / 10) % 10) + 0x30);
				HD44780_SendChar((char)((a1min) % 10) + 0x30);
				a2hour = AT24XX_Read(20);
				HD44780_SetPos(1, 11);
				HD44780_SendChar((char)((a2hour / 10) % 10) + 0x30);
				HD44780_SendChar((char)((a2hour) % 10) + 0x30);
				HD44780_SendChar(':');
				a2min = AT24XX_Read(21);
				HD44780_SendChar((char)((a2min / 10) % 10) + 0x30);
				HD44780_SendChar((char)((a2min) % 10) + 0x30);

				a0hour = AT24XX_Read(30);
				HD44780_SetPos(1, 0);
				HD44780_SendChar((char)((a0hour / 10) % 10) + 0x30);
				HD44780_SendChar((char)((a0hour) % 10) + 0x30);
				HD44780_SendChar(':');
				a0min = AT24XX_Read(31);
				HD44780_SendChar((char)((a0min / 10) % 10) + 0x30);
				HD44780_SendChar((char)((a0min) % 10) + 0x30);

				alarm = AT24XX_Read(9);

				if (alarm)
				{
					HD44780_SetPos(1, 7); HD44780_String("!");
				}

				if (set && sec % 2 == 0)
				{
					switch (set)
					{
					case 1: HD44780_SetPos(0, 0); HD44780_String("  "); break;
					case 2: HD44780_SetPos(0, 3); HD44780_String("  "); break;
					case 3: HD44780_SetPos(0, 11); HD44780_String("  "); break;
					case 4: HD44780_SetPos(0, 14); HD44780_String("  "); break;
					case 5: HD44780_SetPos(1, 11); HD44780_String("  "); break;
					case 6: HD44780_SetPos(1, 14); HD44780_String("  "); break;
					case 7: HD44780_SetPos(1, 0); HD44780_String("  "); break;
					case 8: HD44780_SetPos(1, 3); HD44780_String("  "); break;
					case 9: HD44780_SetPos(1, 7); HD44780_String(" "); break;
					}
				}

				if (powerLight)
				{
					HD44780_SetPos(0, 10);
					HD44780_SendChar('>');
					HD44780_SetPos(1, 10);
					HD44780_SendChar(' ');
				}
				else
				{
					HD44780_SetPos(0, 10);
					HD44780_SendChar(' ');
					HD44780_SetPos(1, 10);
					HD44780_SendChar('>');
				}
			}

			if (sec % 2 != 0 && hour == a0hour && min == a0min)
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
			}

			if (sec) relayLight();
		}

		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == GPIO_PIN_SET)
		{
			if (powerLight) powerLight = 0; else powerLight = 1;
			relayLight(); HAL_Delay(500);
		}

		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_SET)
		{
			if (set < 9)
			{
			set++;
			}
			else
			{
			set = 0;
			}
			HAL_Delay(500);
			modeSwitch = millis;
			big = 0;
		}

		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_SET)
		{
			if (set)
			{
				switch (set)
				{
				case 1: clkTime.Hours = decToBcd(hour--); HAL_RTC_SetTime(&hrtc, &clkTime, RTC_FORMAT_BCD); break;
				case 2: clkTime.Minutes = decToBcd(min--); clkTime.Seconds = 0x00; HAL_RTC_SetTime(&hrtc, &clkTime, RTC_FORMAT_BCD); break;
				case 3: if (a1hour > 0) { a1hour--; } else { a1hour = 23; } break;
				case 4: if (a1min > 0) { a1min--; } else { a1min = 59; } break;
				case 5: if (a2hour > 0) { a2hour--; } else { a2hour = 23; } break;
				case 6: if (a2min > 0) { a2min--; } else { a2min = 59; } break;
				case 7: if (a0hour > 0) { a0hour--; } else { a0hour = 23; } AT24XX_Write(30, a0hour); break;
				case 8: if (a0min > 0) { a0min--; } else { a0min = 59; } AT24XX_Write(31, a0min); break;
				case 9: if (alarm) alarm = 0; else alarm = 1; AT24XX_Write(9, alarm); break;
				}
				HAL_Delay(500);
			}
			modeSwitch = millis;
			big = 0;
		}

		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET)
		{
			if (set)
			{
				switch (set)
				{
				case 1: clkTime.Hours = decToBcd(hour++); HAL_RTC_SetTime(&hrtc, &clkTime, RTC_FORMAT_BCD); break;
				case 2: clkTime.Minutes = decToBcd(min++); clkTime.Seconds = 0x00; HAL_RTC_SetTime(&hrtc, &clkTime, RTC_FORMAT_BCD); break;
				case 3: if (a1hour < 23) { a1hour++; } else { a1hour = 0; } break;
				case 4: if (a1min < 59) { a1min++; } else { a1min = 0; } break;
				case 5: if (a2hour < 23) { a2hour++; } else { a2hour = 0; } break;
				case 6: if (a2min < 59) { a2min++; } else { a2min = 0; } break;
				case 7: if (a0hour < 23) { a0hour++; } else { a0hour = 0; } AT24XX_Write(30, a0hour); break;
				case 8: if (a0min < 59) { a0min++; } else { a0min = 0; } AT24XX_Write(31, a0min); break;
				case 9: if (alarm) alarm = 0; else alarm = 1; AT24XX_Write(9, alarm); break;
				}
				HAL_Delay(500);
			}
			modeSwitch = millis;
			big = 0;
		}

	}
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_HSE_DIV128;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* RTC init function */
static void MX_RTC_Init(void)
{

  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef DateToUpdate;

    /**Initialize RTC Only 
    */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initialize RTC and set the Time and Date 
    */
  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2){
  sTime.Hours = 0x1;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 0x1;
  DateToUpdate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR1,0x32F2);
  }

}

/* TIM1 init function */
static void MX_TIM1_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 10;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 7200;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_Pin|STATUS_LIGHT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, PULSE_LIGHT_OFF_Pin|PULSE_LIGHT_ON_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SIGNAL_Pin|HD44780_RS_Pin|HD44780_EN_Pin|HD44780_D4_Pin 
                          |HD44780_D5_Pin|HD44780_D6_Pin|HD44780_D7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_Pin STATUS_LIGHT_Pin */
  GPIO_InitStruct.Pin = LED_Pin|STATUS_LIGHT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PULSE_LIGHT_OFF_Pin PULSE_LIGHT_ON_Pin */
  GPIO_InitStruct.Pin = PULSE_LIGHT_OFF_Pin|PULSE_LIGHT_ON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SIGNAL_Pin HD44780_RS_Pin HD44780_EN_Pin HD44780_D4_Pin 
                           HD44780_D5_Pin HD44780_D6_Pin HD44780_D7_Pin */
  GPIO_InitStruct.Pin = SIGNAL_Pin|HD44780_RS_Pin|HD44780_EN_Pin|HD44780_D4_Pin 
                          |HD44780_D5_Pin|HD44780_D6_Pin|HD44780_D7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB3 PB4 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */ 
}

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

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
