/*
 * global_variables.c
 *
 *  Created on: Mar 3, 2021
 *      Author: Felix
 */
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "system_state.h"


const int x[5]={1,2,3,4,5};

const char PULSE1_CONTROL_1 = 0x04;
const char PULSE1_CONTROL_2 = 0x07;
const char PULSE1_CONTROL_3 = 0x0A;

const char PULSE2_CONTROL_1 = 0x05;
const char PULSE2_CONTROL_2 = 0x08;
const char PULSE2_CONTROL_3 = 0x0B;

const char D1_CONTROL = 0x16;
const char D2_CONTROL = 0x17;
const char PHASE2_CONTROL = 0x18;
const char M34_CONTROL = 0x1E;

const char PHASE_CONTROL_1 = 0x06;
const char PHASE_CONTROL_2 = 0x09;
const char PHASE_CONTROL_3 = 0x0C;

const char FREQUENCY_CONTROL_1 =  0x1A;
const char FREQUENCY_CONTROL_2 =  0x1B;
const char FREQUENCY_CONTROL_3 =  0x1C;
const char MINIUTE_CONTROL =  0x01;
const char SECOND_CONTROL =  0x02;

const char TC_CONTROL =  0x15;
const char TW_CONTROL =  0x07;

const char INDEX_CONTROL =  0x0D;

char index = 0x00;
char pulse_source_index = 0x00;
float PWM_CLK_DIV = 4.0;

long cnt = 0;
unsigned int cnt_Tx = 0;

//unsigned char cnt = 0;

Uint32 pwm1_prd = 0x1;
Uint32 pwm1_cmp = 0x02;

Uint32 pwm2_pha = 0x00000002;
Uint32 pwm2_cmp = 0;
Uint32 pwm2_prd = 1;

unsigned int period = 2250;
Uint32 low_freq_period = 2250;
unsigned char low_freq_mode = 0;

unsigned int frequency[3]={2000,10000,10000};
float width1_us[3] = {5,1.0,2.0};//锟斤拷锟斤拷1锟侥匡拷锟斤拷默锟斤拷为5us
float width2_us[3] = {5,2.0,3.0};//锟斤拷锟斤拷2锟侥匡拷锟斤拷默锟斤拷为1us

float phase_us[3] = {2.0,2.0,3.0};//锟斤拷位默锟斤拷为5us

float D1 = 5;
float Tw = 1;
float Tc = 100;

float D2 = 10;
float M34 = 20;
float phase2 = 3;

float timer_minute = 0.0;
float timer_second = 20.0;

char change_flag = 0;
unsigned int num1;
Uint32 num2;
enum sys_state state;
enum sys_state pre_state;
char rdataA[50];    // Received data for SCI-A
char rdataB[50];    // Received data for SCI-B

char stop_cmd_flag = 0;//
char one_pulse_flag = 0;//

int sampled_voltage = 0;
float sampled_voltage_conv = 0;
unsigned int voltage_max = 4800;  //设置充电电压上限，超过之后停止充电

unsigned char sample_finished_flag = 0;
