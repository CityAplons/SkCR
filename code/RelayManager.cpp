/*
 * RelayManager.cpp
 *
 *  Created on: Jun 16, 2020
 *      Author: nikita
 */

#include <RelayManager.h>
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "gpio_map.h"

RelayManager fl_manager;

RelayManager::RelayManager() {
	// TODO Auto-generated constructor stub

}

RelayManager::~RelayManager() {
	// TODO Auto-generated destructor stub
}

void RelayManager::set_config(uint8_t mask)
{

}

void RelayManager::get_config(uint8_t* ret_arr)
{

}

void RelayManager::get_timeouts(uint8_t* ret_arr)
{

}

void RelayManager::run()
{
	init_ports();
	uint8_t arr[4] = {0,0,0,0};
	memcpy(timeout_arr,arr,4);
	memcpy(current_status,arr,4);
    while(1) {

        vTaskDelay((TickType_t)1000);
    }
}

void RelayManager::switch_relay(uint8_t mask)
{
	for (uint8_t i = 0; i < 4; i++)
	{
		if ((mask >> i)  & 0x01) {
			GPIO_SetBits(GPIOE,i);
	    } else {
	    	GPIO_ResetBits(GPIOE,i);
	    }
	}

}

uint8_t RelayManager::translate_status()
{
	return 0x00;
}

void RelayManager::init_ports()
{
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

        GPIO_InitTypeDef fl_pin0;
        GPIO_StructInit(&fl_pin0);
        fl_pin0.GPIO_Pin = GPIO_Pin_0;
        fl_pin0.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_Init(GPIOE,&fl_pin0);

        GPIO_InitTypeDef fl_pin1;
        GPIO_StructInit(&fl_pin1);
        fl_pin1.GPIO_Pin = GPIO_Pin_1;
        fl_pin1.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_Init(GPIOE,&fl_pin1);

        GPIO_InitTypeDef fl_pin2;
        GPIO_StructInit(&fl_pin2);
        fl_pin2.GPIO_Pin = GPIO_Pin_2;
        fl_pin2.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_Init(GPIOE,&fl_pin2);

        GPIO_InitTypeDef fl_pin3;
        GPIO_StructInit(&fl_pin3);
        fl_pin3.GPIO_Pin = GPIO_Pin_3;
        fl_pin3.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_Init(GPIOE,&fl_pin3);

        GPIO_ResetBits(GPIOE,0);
        GPIO_ResetBits(GPIOE,1);
        GPIO_ResetBits(GPIOE,2);
        GPIO_ResetBits(GPIOE,3);

}

