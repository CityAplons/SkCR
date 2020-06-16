/*
 * RelayManager.h
 *
 *  Created on: Jun 16, 2020
 *      Author: nikita
 */

#ifndef CODE_RELAYMANAGER_H_
#define CODE_RELAYMANAGER_H_

#include "stm32f4xx.h"
#include "TaskWrapper.h"

class RelayManager: public TaskWrapper {
public:
	RelayManager();
	virtual ~RelayManager();
	void set_config(uint8_t mask);
	void get_config(uint8_t* ret_arr);
	void get_timeouts(uint8_t* ret_arr);
	void run();

private:
	uint8_t timeout_arr[4];
	bool current_status[4];

	uint8_t translate_status();
	void switch_relay(uint8_t mask);
};

extern RelayManager fl_manager;
#endif /* CODE_RELAYMANAGER_H_ */
