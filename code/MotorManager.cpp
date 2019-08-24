/*
 * MotorManager.cpp
 *
 *  Created on: 11.03.2019
 *      Author: Taras.Melnik
 */

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dma.h"
#include <math.h>
#include "arm_math.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "MotorManager.h"
#include "LedRgb.h"
#include "MotorWheel.h"

MotorManager mot_manager;

static MotorWheel motor_wheel_1(1);
static MotorWheel motor_wheel_2(2);

MotorErrorStatus MotorManager::read_motors_aknowlege(uint8_t &code)
{
        motor_wheel_1.read_response();
        motor_wheel_2.read_response();
        //ToDo Add error if there is no acknowledge
        if (motor_wheel_1.options.last_code != 0 && motor_wheel_2.options.last_code != 0)
        {
                /* ToDo if codes do not matched up */
//                if (motor_wheel_1.options.last_code == motor_wheel_2.options.last_code)
//                        code = motor_wheel_1.options.last_code;
                return MotorErrorStatus::OK;
        } else
                return MotorErrorStatus::MOTORS_DONOT_ANSWER;

}

void MotorManager::switch_to_receive()
{
        // ToDo switch pin from transmitting to receiving
	if (firstTransmitedFlag) {
		GPIO_ResetBits(GPIOB, GPIO_Pin_12);
		firstTransmitedFlag = false;
	} else
	        firstTransmitedFlag = true;
}

void MotorManager::convert_robot_to_wheel_speed(float* robArr, int16_t* wheel_arr)
{
	float32_t buff[2];
	buff[0] = (robArr[0] + robArr[1] * L_CENTER) * CONST_WHEEL_1 / R_WHEEL;
	buff[1] = (robArr[0] - robArr[1] * L_CENTER) * CONST_WHEEL_2 / R_WHEEL;
	for (uint8_t i = 0; i < 2; i ++) {
		wheel_arr[i] = (int16_t) buff[i];

		/* More accurate rounding */
		if (buff[i] - 0.5 >= wheel_arr[i]) {
			wheel_arr[i]++;
		} else if (buff[i] + 0.5 <= wheel_arr[i])
			wheel_arr[i]--;
	}
}

void MotorManager::fetch_angle()
{
        static const float CONST_2_PI = 2 * PI;

        if (robot.theta >= CONST_2_PI) {
                robot.theta -= CONST_2_PI;
        } else if (robot.theta <= -CONST_2_PI)
                robot.theta += CONST_2_PI;
}

void MotorManager::calculate_position()
{
        static int32_t wheel_arr_prev[] = {0, 0};
        int32_t wheel_arr[] = {motor_wheel_1.options.angle,
                        motor_wheel_2.options.angle};

	float d_wheel_angle[2] = {0};
	float d_theta = 0;
	float d_dist = 0;

	for (uint8_t i = 0; i < 2; i++) {
	        /*
	         * Protection against random initial
	         * values on motors after switching on
	         */
//		if (wheel_arr_prev[i] == 0)
//		        wheel_arr_prev[i] = wheel_arr[i];

	        d_wheel_angle[i] = (wheel_arr[i] - wheel_arr_prev[i]) * W_CONST;
		wheel_arr_prev[i] = wheel_arr[i];
	}
	d_theta = (d_wheel_angle[0] - d_wheel_angle[1]) * ROTATION_CONST;

	/* Fix an incorrect answer during shutdown */
	if (d_theta < 10 && d_theta > -10) {
		d_dist = (d_wheel_angle[0] + d_wheel_angle[1]) * R_WHEEL / 2;

		robot.theta += d_theta;

		/* The angle should be in 2pi range (for ARM tables) */
		fetch_angle();

		/* Use arm_math table functions for high speed calculation */
		robot.y += d_dist * arm_sin_f32(robot.theta);
		robot.x += d_dist * arm_cos_f32(robot.theta);
	}
}


void MotorManager::set_robot_speed(uint8_t* byte_arr)
{
	for (uint8_t i = 0; i < 8; i++)
	        speed_byte_arr[i]= byte_arr[i];
	terminalRxFlag = true;
}

void MotorManager::get_robot_position(uint8_t* byte_arr)
{
	for (uint8_t i = 0; i < 12; i++)
	        byte_arr[i] = position_byte_arr[i];
}

void MotorManager::send_next_request(uint8_t current_code)
{
        static uint8_t previouse_code = REG_READ_ANGLE_LOW;

        /* Clear synchronization flag */
        firstTransmitedFlag = false;

        if (current_code == REG_SET_SPEED)
                current_code = previouse_code;

        if (current_code == REG_READ_ANGLE_LOW) {
                motor_wheel_1.request_current();
                motor_wheel_2.request_current();
        } else if (current_code == REG_READ_CURRENT) {
                motor_wheel_1.request_error();
                motor_wheel_2.request_error();
        } else if (current_code == REG_ERROR_CODE) {
                motor_wheel_1.request_angle();
                motor_wheel_2.request_angle();
        }
        previouse_code = current_code;
}

void MotorManager::set_robot_speed()
{
        float robot_speed[2];

        /* Convert from byte array into linear and angular robot velocity */
        memcpy(robot_speed, speed_byte_arr, sizeof(robot_speed));

        /* If status was not processed */
        if (!robot.current_status)
                set_robot_speed(robot_speed[0], robot_speed[1]);
}

void MotorManager::set_robot_speed(float lin_vel, float ang_vel )
{
        float robot_speed[2];
        int16_t wheel_speed[2];


        /* Safety limitation on the robot speed */
        if (lin_vel > MAX_ROBOT_SPEED)
                robot.linear_velocity = MAX_ROBOT_SPEED;
        convert_robot_to_wheel_speed(robot_speed, wheel_speed);

        /* Clear synchronization flag */
        firstTransmitedFlag = false;
        motor_wheel_1.set_wheel_speed(wheel_speed[0]);
        motor_wheel_2.set_wheel_speed(wheel_speed[1]);
}

void MotorManager::process_angle()
{
        float robot_position[3];

        calculate_position();
        robot_position[0] = robot.x;
        robot_position[1] = robot.y;
        robot_position[2] = robot.theta;

        memcpy(position_byte_arr, robot_position, sizeof(position_byte_arr));
}

void MotorManager::process_current()
{
        if (motor_wheel_1.options.current_value > CURRENT_COLLISION_VALUE ||
                        motor_wheel_2.options.current_value > CURRENT_COLLISION_VALUE)
        {
                robot.current_status = true;
                set_robot_speed(STOP_MOTION, STOP_MOTION);
        }
}

void MotorManager::process_error()
{
//                int16_t errBuff1 = motor1.uint8toInt16(rxRsDataArr[0]);
//                int16_t errBuff2 = motor2.uint8toInt16(rxRsDataArr[1]);
//                if (errBuff1 != 0 || errBuff2 != 0) {
//                        for (i = 0; i < 2; i++) {
//                                motArr[i]->modbusWriteReg(i + 1, REG_SET_SPEED, STOP_MOTION);
//                                ulTaskNotifyTake(pdTRUE, oRTOS.fromMsToTick(4));
//                        }
//                        this->curColFlag = true;
//                        setLEDTask->setColor(RED);
//                        vTaskDelay(oRTOS.fromMsToTick(1000));
//                        motorInit(1);
//                        motorInit(2);
}

void MotorManager::run()
{
        TickType_t pxPreviousWakeTime;
	uint8_t code = 0;
	MotorErrorStatus status;

	/* Peripheral initialization */
	motor_wheel_1.init_usart(GPIOB, USART3, true);
	motor_wheel_2.init_usart(GPIOA, UART4, true);

	/* Switch pin for rs485 driver */
	motor_wheel_1.gpioSwitchInit(GPIOB, GPIO_Pin_12);
        motor_wheel_2.gpioSwitchInit(GPIOB, GPIO_Pin_12);

	/* Should be after peripheral init */
	motor_wheel_1.init_wheel();
	motor_wheel_2.init_wheel();
	set_robot_speed(STOP_MOTION, STOP_MOTION);


	while (1) {
	        /* Function for accurate task sleep time */
	        pxPreviousWakeTime = xTaskGetTickCount();

	        status = read_motors_aknowlege(code);
	        if (status == MotorErrorStatus::OK) {

	                /* If command from high level has come set speed */
	                if (terminalRxFlag) {
	                        set_robot_speed();
	                } else if (code == REG_READ_ANGLE_LOW) {
	                        process_angle();
	                } else if (code == REG_READ_CURRENT) {
	                        process_current();
                        } else if (code == REG_ERROR_CODE)
                                process_error();

	                send_next_request(code);
	        } else {
	                uint8_t c = 10;
	                while (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15) && c) {
	                        robot.button_status = true;

	                        /* LED indication, motors do not answer */
	                        ledRgb.mutex_take(RED);
	                        c--;
	                        vTaskDelay(1);
	                }
	                if (!c) {
	                        //ToDo error processing, motors do not work
	                        ledRgb.mutex_give();
	                }
	                ledRgb.mutex_give();

	                /* LED indication, button pressed */
	                ledRgb.mutex_take(RED_FLASH);
	                do {
	                        robot.button_status = true;
	                        vTaskDelay(1);
	                } while (!GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15));
	                ledRgb.mutex_give();

	                /*
	                 *ToDO is it necessary to reinit?
	                for (uint8_t i = 0; i < 2; i++)
	                        whAngleHistArr[i] = 0;
	                        */

	                /* Init motors after power reset */
	                motor_wheel_1.init_wheel();
	                motor_wheel_2.init_wheel();
	                set_robot_speed(STOP_MOTION, STOP_MOTION);
	        }
	        vTaskDelayUntil(&pxPreviousWakeTime, TIME_FOR_RESPONCE);
	}
}

extern "C"
{
//---------------------------WHEEL_1---------------------------------//

//***********************UART_RECEIVE_INTERRUPT******************//
	void USART3_IRQHandler(void)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;			// Notify task about interrupt
		if (USART_GetITStatus(USART3, USART_IT_IDLE)) {			// Clear IDLE flag step 1
			DMA_Cmd(DMA1_Stream1, DISABLE);				// DMA turn off to clear DMA1 counter
			USART_ReceiveData(USART3);				// Clear IDLE flag step 2
		}
		if (USART_GetITStatus(USART3, USART_IT_TC)) {
			USART_ClearITPendingBit(USART3, USART_IT_TC);
			mot_manager.switch_to_receive();
		}
		if (xHigherPriorityTaskWoken)
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);		// Run Higher priority task if exist													// between ticks
	}
//***********************DMA_RECEIVE_INTERRUPT******************//
	void DMA1_Stream1_IRQHandler(void)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;			// Notify task about interrupt
		DMA_ClearITPendingBit(DMA1_Stream1, DMA_IT_TCIF1);		// Clear DMA "transmitting complete" interrupt
		DMA_Cmd(DMA1_Stream1, ENABLE);					// Reset DMA
		if (xHigherPriorityTaskWoken)
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);		// Run Higher priority task if exist													// between ticks
	}

//***********************DMA_TRANSMIT_INTERRUPT*****************//
	void DMA1_Stream3_IRQHandler(void)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;			// Notify task about interrupt
		DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);		// Clear DMA "transmission complete" interrupt
		DMA_Cmd(DMA1_Stream3, DISABLE);					// Stop DMA transmitting
		if (xHigherPriorityTaskWoken)
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);		// Run Higher priority task if exist													// between ticks
	}

//---------------------------WHEEL_2---------------------------------//

	//***********************UART_RECEIVE_INTERRUPT******************//
	void UART4_IRQHandler(void)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;			// Notify task about interrupt
		if (USART_GetITStatus(UART4, USART_IT_IDLE)){			// Clear IDLE flag step 1
			DMA_Cmd(DMA1_Stream2, DISABLE);				// DMA turn off to clear DMA1 counter
			USART_ReceiveData(UART4);				// Clear IDLE flag step 2
		}
		if (USART_GetITStatus(UART4, USART_IT_TC)){
			USART_ClearITPendingBit(UART4, USART_IT_TC);
			mot_manager.switch_to_receive();
		}
		if (xHigherPriorityTaskWoken)					// Run Higher priority task if exist
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);		// between ticks
	}
	//***********************DMA_RECEIVE_INTERRUPT******************//
	void DMA1_Stream2_IRQHandler(void)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;			// Notify task about interrupt
		DMA_ClearITPendingBit(DMA1_Stream2, DMA_IT_TCIF2);		// Clear DMA "transmitting complete" interrupt
		DMA_Cmd(DMA1_Stream2, ENABLE);				        // Reset DMA
		if (xHigherPriorityTaskWoken)				        // Run Higher priority task if exist
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);		// between ticks
	}
	//***********************DMA_TRANSMIT_INTERRUPT*****************//
	void DMA1_Stream4_IRQHandler(void)
	{
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;			// Notify task about interrupt
		DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);		// Clear DMA "transmission complete" interrupt
		DMA_Cmd(DMA1_Stream4, DISABLE);					// Stop DMA transmitting
		if (xHigherPriorityTaskWoken)					// Run Higher priority task if exist
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);		// between ticks
	}
}