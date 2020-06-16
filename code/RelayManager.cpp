/*
 * RelayManager.cpp
 *
 *  Created on: Jun 16, 2020
 *      Author: nikita
 */

#include <RelayManager.h>

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
    while(1) {

        vTaskDelay(10);
    }
}

void RelayManager::switch_relay(uint8_t mask)
{

}

uint8_t RelayManager::translate_status()
{
	return 0x00;
}

