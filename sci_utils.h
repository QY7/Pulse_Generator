/*
 * sci_utils.h
 *
 *  Created on: Jun 8, 2020
 *      Author: Felix
 */

#ifndef SCI_UTILS_H_
#define SCI_UTILS_H_
#include "system_state.h"
void scia_echoback_init(void);
void scib_echoback_init(void);
void sci_xmit(char a,HOST host);
void sci_xmit_2byte(unsigned int a,HOST host);
void process_cmd(char *,unsigned char,HOST host);
char *substring(char *string, int position, int length);
void sci_msg(char val[],unsigned int length,HOST host);

extern void update_touch_int(char screen,const char control_id,unsigned int data);
extern void update_touch_data(char screen, const char control_id,float freq,char*format);
extern void update_PC_data(char screen, const char,float);
extern void update_progress(char val);
extern void update_channel(char channel);
extern void send_two_byte_data(char screen,const char control_id,char val);
extern void icon_on_state();
extern void icon_off_state();
extern void change_icon_state(char icon_id[2],char show,HOST host);

void add_data(unsigned int data);
void add_data_channel(void);

extern unsigned char cmd_len;
extern char cmd_receive[100];
extern unsigned char wait_flag;


extern  float width1_us[];//脉宽1的宽度默认为5us
extern float width2_us[];//脉宽2的宽度默认为1us
extern float phase_us[];//相位默认为5us
extern unsigned int frequency[];//相位默认为5us
extern float timer_minute;
extern float timer_second;
extern char index;
extern char pulse_source_index;



#endif /* SCI_UTILS_H_ */
