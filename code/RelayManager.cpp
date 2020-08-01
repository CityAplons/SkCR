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
	uint8_t arr[4] = {0,0,0,0};
	last_value = 0;
	memcpy(timeout_arr,arr,4);
	memcpy(current_status,arr,4);
}

RelayManager::~RelayManager() {
	// TODO Auto-generated destructor stub
}

void RelayManager::set_config(uint8_t mask)
{
	uint8_t modified_mask = mask;
	for (uint8_t i = 0; i < 4; i++)
	{
		if ((modified_mask >> i)  & 0x01) {
			current_status[i] = 1;
			if (!(last_value >> i)&0x01) timeout_arr[i] = TURNOFF_TIMEOUT;
		} else {	// Check where bit is 0
			current_status[i] = 0;
			// Setting this bit if timeout is not pass - lamp still turned on
			if (timeout_arr[i] != 0) {
				modified_mask |= 1 << i;
				current_status[i] = 1;
			}
		}
	}
	// Applying mask
	last_value = modified_mask;
	switch_relay(modified_mask);
}

void RelayManager::get_config(uint8_t* ret_arr)
{
	/*
	 * Transforming array data to mask
	 */
	uint8_t res = 0x00;
	for (uint8_t i = 0; i < 4; i++)
	{
		if (current_status[i] == 1)
			res |= 1 << i;
	}
	ret_arr[0] = res;
}

void RelayManager::get_timeouts(uint8_t* ret_arr)
{
	memcpy(ret_arr,timeout_arr,4);
}

void RelayManager::run()
{
	init_ports();

    while(1) {
    	for (uint8_t i = 0; i < 4; i++)
    	{
    		if (timeout_arr[i] > 0) timeout_arr[i]--;
    	}
        vTaskDelay((TickType_t)60000);
    }
}

void RelayManager::switch_relay(uint8_t mask)
{
	for (uint8_t i = 0; i < 4; i++)
	{
		if ((mask >> i)  & 0x01) {
			switch (i){
				case 0:
					GPIO_SetBits(GPIOD,GPIO_Pin_12);
					break;
				case 1:
					GPIO_SetBits(GPIOD,GPIO_Pin_13);
					break;
				case 2:
					GPIO_SetBits(GPIOD,GPIO_Pin_14);
					break;
				case 3:
					GPIO_SetBits(GPIOD,GPIO_Pin_15);
					break;
				default:
					break;
			}
	    } else {
	    	switch (i){
	    		case 0:
	    			GPIO_ResetBits(GPIOD,GPIO_Pin_12);
	    			break;
	    		case 1:
	    			GPIO_ResetBits(GPIOD,GPIO_Pin_13);
	    			break;
	    		case 2:
	    			GPIO_ResetBits(GPIOD,GPIO_Pin_14);
	    			break;
	    		case 3:
	    			GPIO_ResetBits(GPIOD,GPIO_Pin_15);
	    			break;
	    		default:
	    			break;
	    	}
	    }
	}

}

void RelayManager::init_ports()
{
		/*
		 * Configuring port E bits 0...3 for relay switching
		 */
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

        GPIO_InitTypeDef fl_pin0;
        GPIO_StructInit(&fl_pin0);
        fl_pin0.GPIO_Pin = GPIO_Pin_12;
        fl_pin0.GPIO_Mode = GPIO_Mode_OUT;
        fl_pin0.GPIO_OType = GPIO_OType_PP;
        GPIO_Init(GPIOD,&fl_pin0);

        GPIO_InitTypeDef fl_pin1;
        GPIO_StructInit(&fl_pin1);
        fl_pin1.GPIO_Pin = GPIO_Pin_13;
        fl_pin1.GPIO_Mode = GPIO_Mode_OUT;
        fl_pin1.GPIO_OType = GPIO_OType_PP;
        GPIO_Init(GPIOD,&fl_pin1);

        GPIO_InitTypeDef fl_pin2;
        GPIO_StructInit(&fl_pin2);
        fl_pin2.GPIO_Pin = GPIO_Pin_14;
        fl_pin2.GPIO_Mode = GPIO_Mode_OUT;
        fl_pin2.GPIO_OType = GPIO_OType_PP;
        GPIO_Init(GPIOD,&fl_pin2);

        GPIO_InitTypeDef fl_pin3;
        GPIO_StructInit(&fl_pin3);
        fl_pin3.GPIO_Pin = GPIO_Pin_15;
        fl_pin3.GPIO_Mode = GPIO_Mode_OUT;
        fl_pin3.GPIO_OType = GPIO_OType_PP;
        GPIO_Init(GPIOD,&fl_pin3);

        GPIO_ResetBits(GPIOD,GPIO_Pin_12);
        GPIO_ResetBits(GPIOD,GPIO_Pin_13);
        GPIO_ResetBits(GPIOD,GPIO_Pin_14);
        GPIO_ResetBits(GPIOD,GPIO_Pin_15);

}

