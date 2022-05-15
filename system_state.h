/*
 * system_state.h
 *
 *  Created on: Mar 3, 2021
 *      Author: Felix
 */

#ifndef SYSTEM_STATE_H_
#define SYSTEM_STATE_H_

#define TIMER_PERIOD_US 10
#define PERIOD_1MS 1000

enum sys_state{
    WAIT,
    START_BTN,
    CMD_ON,
    CMD_WAIT,
    ON,
    CMD_OFF,
    OFF
};
enum direction{
    PON,
    POFF
};
typedef struct OPERATION{
    enum direction dir;
    unsigned int operation_time;
    unsigned char pin_number;
}OPERATION;

typedef enum host{
    TOUCHPAD,
    PC
}HOST;

#endif /* SYSTEM_STATE_H_ */
