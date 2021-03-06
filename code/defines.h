/*
 * defines.h
 *
 *  Created on: 01.11.2018
 *      Author: Taras.Melnik
 */

#ifndef DEFINES_H_
#define DEFINES_H_

//#define CMD_DEF(cmd_num, cmd_name) \
//					cmd_name = (cmd_num),

//enum
//{
//	FLAG_NULL,
////#include "cmd_defs"
//	FLAG_END
//};


enum HighLvlCommand {
//	FLAG_NULL,
	ECHO 				= 0x01,

	SET_COLOR       	= 0x07,
	GET_BATTERY_CHARGE	= 0x08,
	SET_ROBOT_SPEED		= 0x09,
	GET_DISTANCE		= 0x0A,
	GET_ROBOT_POSITION  = 0x0B,
	GET_ACCELERATION	= 0x0C,
	GET_ANG_VEL         = 0x0D,
	SET_FL_CONFIG		= 0x1A,		// Configure fluorescent lamps: single byte - mask (0b00000000 - all lamps disabled)
	GET_FL_CONFIG		= 0x1B,		// Return lamps configuration: single byte - mask (0b1111111 - all lamps enabled)
	GET_FL_TIMEOUTS		= 0x1C		// Return time till lamp could be disabled (lamp safety): 4 uint8_t (time in minutes)
//	FLAG
};

#endif /* DEFINES_H_ */
