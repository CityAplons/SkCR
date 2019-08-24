/*
 * USBUserInterface.cpp
 *
 *  Created on: 29.10.2018
 *      Author: Taras.Melnik
 */

#include "stm32f4xx.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_dma.h"

#include "FreeRTOS.h"
#include "task.h"

#include "DriverUsart.h"
#include "defines.h"
#include "terminal.h"
#include "LedRgb.h"
#include "BatteryManager.h"
#include "RangefinderManager.h"
#include "MotorManager.h"
//#include "ImuSensor.h"

Terminal terminal;
static DriverUsart uart_6;

void Terminal::run()
{
	uint8_t answerLength = 0;
	uint8_t errByte = 0;

	uart_6.init_usart(GPIOC, USART6, false);

	while(1) {
	        /* Task is suspended until notification */
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (uart_6.usartRxArr[0] == HighLvlCommand::ECHO) {
			uart_6.usartTxArr[0] = 0x11;
			answerLength = 1;

		} else if (uart_6.usartRxArr[0] ==  HighLvlCommand::SET_COLOR) {
		        ledRgb.set_color((cl) uart_6.usartRxArr[1]);
			uart_6.usartTxArr[0] = 0;
			answerLength = 1;

		} else if (uart_6.usartRxArr[0] ==  HighLvlCommand::GET_BATTERY_CHARGE) {
			uart_6.usartTxArr[0] = bat_manager.get_charge();
			answerLength = 1;

		} else if (uart_6.usartRxArr[0] ==  HighLvlCommand::GET_DISTANCE) {
			rf_manager.get_distance(uart_6.usartTxArr);
			answerLength = RANGEFINDERS_NUMBER;

		} else if (uart_6.usartRxArr[0] ==  HighLvlCommand::SET_ROBOT_SPEED) {
		        mot_manager.set_robot_speed(&(uart_6.usartRxArr[1]));
			answerLength = 1;

			/* Receive from RS485 ----- (x, y, theta) */
		} else if (uart_6.usartRxArr[0] ==  HighLvlCommand::GET_WHEELS_ANGLE) {
		        mot_manager.get_robot_position(uart_6.usartTxArr);
			answerLength = 12;

			/* Receive from IMU ----- (x, y, theta) */
		} else if (uart_6.usartRxArr[0] ==  HighLvlCommand::RECEIVE_IMU) {
//			imuSensor->getOdometry(uart_6.usartTxArr);
			answerLength = 12;

		} else {
			uart_6.usartTxArr[0] = 0xff;
			uart_6.usartTxArr[1] = uart_6.usartTxArr[0];
			answerLength = 1;
		}
//		errByte = 0;
//		if (hyroMotor->getWhPowStatus())
//			errByte |= 1;
//		if (hyroMotor->getWhCurColStatus())
//			errByte |= 1 << 1;
//		if (collisionHandler->getStatus())
//			errByte |= 1 << 2;
//		uart_6.usartTxArr[answerLength] = errByte;
//		uart_6.usartTxArr[++answerLength] = 0;
		uart_6.usartTxArr[answerLength] = 0;
		for (uint8_t i = 0; i < answerLength; i++)
			uart_6.usartTxArr[answerLength] += uart_6.usartTxArr[i];
		uart_6.usart_send(uart_6.usartTxArr, ++answerLength);

		/* Finish the task before next tick */
		taskYIELD();
	}
}

extern "C"
{
// ***********************UART_RECEIVE_INTERRUPT******************//
	void USART6_IRQHandler(void)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;			// Notify task about interrupt
		if (USART_GetITStatus(USART6, USART_IT_IDLE)) {			// Clear IDLE flag step 1
			DMA_Cmd(DMA2_Stream1, DISABLE);						// DMA turn off to clear DMA1 counter
			USART_ReceiveData(USART6);							// Clear IDLE flag step 2
		}
		if (xHigherPriorityTaskWoken)							// Run Higher priority task if exist
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);		// between ticks
	}
//***********************DMA_RECEIVE_INTERRUPT******************//
	void DMA2_Stream1_IRQHandler(void)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;			// Notify task about interrupt
		vTaskNotifyGiveFromISR(terminal.task_handle,	// Notify USART task about receiving
				&xHigherPriorityTaskWoken);
		DMA_ClearITPendingBit(DMA2_Stream1, DMA_IT_TCIF1);		// Clear DMA "transmitting complete" interrupt
		DMA_Cmd(DMA2_Stream1, ENABLE);							// Reset DMA
		if (xHigherPriorityTaskWoken)							// Run Higher priority task if exist
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);		// between ticks
	}
//***********************DMA_TRANSMIT_INTERRUPT*****************//
	void DMA2_Stream6_IRQHandler(void)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;			// Notify task about interrupt
		DMA_ClearITPendingBit(DMA2_Stream6, DMA_IT_TCIF6);		// Clear DMA "transmission complete" interrupt
		DMA_Cmd(DMA2_Stream6, DISABLE);							// Stop DMA transmitting
		if (xHigherPriorityTaskWoken)							// Run Higher priority task if exist
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);		// between ticks
	}
}

//extern "C"
//{
////***********************UART_WITHOUT_DMA**********************//
//	void USART3_IRQHandler(void)
//	{
//		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//		vTaskNotifyGiveFromISR(xTaskGetCurrentTaskHandle(),&xHigherPriorityTaskWoken);
//		if (USART_GetITStatus(USART3, USART_IT_RXNE)) {
//			USART_ClearITPendingBit(USART3, USART_IT_RXNE);
//			usartRxArr[usartRxPoiter++] = (uint8_t) USART_ReceiveData(USART3);
//		}
//		usartActiveFlag = true;
//		if (xHigherPriorityTaskWoken)
//			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//	}
//}