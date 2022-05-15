/*
 * sci_utils.c
 *
 *  Created on: Jun 8, 2020
 *      Author: Felix
 */

// Initalize the SCI FIFO
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "global_defs.h"
#include "sci_utils.h"
#include "system_state.h"
#include "string.h"

#define startWith( _message_,_string_)  (!strncmp((_message_),_string_, sizeof(_string_)-1) )

#define SYS_CLK 90e6
#define SCI_DIV 4
#define SCI_CLK SYS_CLK/SCI_DIV
#define BAUD_RATE 115200
#define SCI_REG SCI_CLK/(8*BAUD_RATE)-1

#define MAX_FREQUENCY 150000
#define MIN_FREQUENCY 1

#define BTN_PRESSED 3
#define TIMER_END 2
#define TEXT_MODIFIED 1


unsigned char cmd_len = 0;
char cmd_receive[100];
unsigned char wait_flag = 0;

unsigned char test_uart_flag = 0;

//PWM related parameters




char send_begin[3]={0xEE,0xB1,0x10};
char send_begin_hide[3] = {0xee,0xb1,0x03};
char screen_id0[2] = {0x00,0x00};
char screen_id1[2] = {0x00,0x01};
char send_end[4]={0xFF,0xFC,0xFF,0xFF};
char start_timer[] = {0xEE,0xB1,0x41,0x00,0x00,0x00,0x0A,0xFF,0xFC,0xFF,0xFF};
char set_timer[] = {0xEE,0xB1,0x40,0x00,0x00,0x00,0x0A};
char stop_timer[] = {0xEE,0xB1,0x42,0x00,0x00,0x00,0x0A};

char start_off_icon[] = {0x00,0x11};
char start_on_icon[] = {0x00,0x13};

char stop_off_icon[] = {0x00,0x0e};
char stop_on_icon[] = {0x00,0x6};

char graph_id[]={0x00,23};
char valid_cmd(char *cmd_s,char *cmd_end);

extern void stop_PWM();

extern enum sys_state state;

//void scia_fifo_init()
//{
//    SciaRegs.SCIFFTX.all=0xE040;
//    SciaRegs.SCIFFRX.all=0x2061;
//    SciaRegs.SCIFFCT.all=0x0;
//}
//开启指令 EE
void process_cmd(char *_cmd,unsigned char _len,HOST host){
    char *cmd_content,*cmd_start,*cmd_end,*cmd_id;
    char *input;
    float tmp_input;
    unsigned int timer_num;
    char update_data_flag = 0;

    cmd_content = substring(_cmd, 3, _len-8);
    cmd_start = substring(_cmd,0,1);
    cmd_end = substring(_cmd,_len-4,4);
    cmd_id = substring(_cmd,1,2);

    if(valid_cmd(cmd_start,cmd_end)==0)
        return;
//
    if(cmd_id[1] == 0x43){
        //Timer is up
        if(state != OFF){
            stop_cmd_flag = 1;
        }
    }
    char screen_id = cmd_content[1];
    char control_id = cmd_content[3];

    if(cmd_id[1] == 0x11){
        input = cmd_content+5;
//        control_id = cmd_content[3];
        if(screen_id == 0x00){
            switch(control_id){
            case 0x01:
                //minute
                sscanf( input, "%f", &tmp_input);//convert to float
                update_data_flag = 1;
                timer_minute = tmp_input;
                break;
            case 0x02:
                //second
                sscanf( input, "%f", &tmp_input);//convert to float
                update_data_flag = 1;
                timer_second = tmp_input;
                break;
            case 0x07:
                //外循环
                sscanf( input, "%f", &tmp_input);//convert to float
                update_data_flag = 1;
                Tw = tmp_input;

                ConfigCpuTimer(&CpuTimer1, 90, Tw*1000000);//Tw s
                CpuTimer1Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

                break;
            case 21:
                //外循环
                sscanf( input, "%f", &tmp_input);//convert to float
                update_data_flag = 1;
                Tc = tmp_input;
                break;
            case 0x08:
                //turn on PWM
                //start timer;
                timer_num = timer_minute*60+timer_second;
                sci_msg(set_timer, 7,host);
                sci_xmit(0x00,host);
                sci_xmit(0x00,host);
//                scia_xmit(0x00);
                sci_xmit(timer_num>>8,host);
                sci_xmit(timer_num&0xFF,host);
                sci_msg(send_end, 4,host);
                sci_msg(start_timer,11,host);
                if(state == OFF){
                    //只在OFF的状态的时候才去启动
                    //调试过程中出现过一个BUG，就是第一个段脉冲总是连着第二段脉冲去输出，猜测原因可能就是点开始的时候输入抖动点了两次启动，导致第一次本来在关闭的状态又给打开了。
                    state = START_BTN;
                    icon_on_state();
                }

                break;
            case 9:
                //turn off PWM
                if(state != OFF){
                    stop_cmd_flag = 1;
                }
                sci_msg(stop_timer,7,host);
                sci_msg(send_end,4,host);
                break;
            case 0x12:
                //phase
                //TODO 这里需要修改
                pulse_source_index = _cmd[8];//2021/3/24/ set pulse source

                break;
            default:break;
            }
        }else if(screen_id == 0x01){
            sscanf( input, "%f", &tmp_input);//convert to float
            update_data_flag = 1;
            switch(control_id){
            case 0x04:
                //pulse 1 width1
                width1_us[0] = tmp_input;
                break;
            case 0x07:
                //pulse 1 width1
                width1_us[1] = tmp_input;
                break;
            case 0x3B:
                //pulse 1 width1
                width1_us[2] = tmp_input;
                break;
            case 0x05:
                //pulse 2 width
                width2_us[0] = tmp_input;
                break;
            case 0x08:
                //pulse 2 width
                width2_us[1] = tmp_input;
                break;
            case 0x0B:
                //pulse 2 width
                width2_us[2] = tmp_input;
                break;
            case 0x06:
                //phase
                phase_us[0] = tmp_input;
                break;
            case 0x09:
                //phase
                phase_us[1] = tmp_input;
                break;
            case 0x0C:
                //phase
                phase_us[2] = tmp_input;
                break;
            case 22:
                //phase
                D1 = tmp_input;
                break;
            case 23:
                //phase
                D2 = tmp_input;
                break;
            case 24:
                //phase
                phase2 = tmp_input;
                break;
            case 30:
                //phase
                M34 = tmp_input;
                break;
            case 0x0D:
                index = _cmd[8];
                break;
            case 26:
                frequency[0] = tmp_input;
                if(frequency[0]>MAX_FREQUENCY){
                    frequency[0] = MAX_FREQUENCY;
                }
                if(frequency[0]<MIN_FREQUENCY){
                    frequency[0] = MIN_FREQUENCY;
                }
                break;
            case 27:
                frequency[1] = tmp_input;
                if(frequency[1]>MAX_FREQUENCY){
                    frequency[1] = MAX_FREQUENCY;
                }
                if(frequency[1]<MIN_FREQUENCY){
                    frequency[1] = MIN_FREQUENCY;
                }
                tmp_input = frequency[1];
                break;
            case 28:
                frequency[2] = tmp_input;
                if(frequency[2]>MAX_FREQUENCY){
                    frequency[2] = MAX_FREQUENCY;
                }
                if(frequency[2]<MIN_FREQUENCY){
                    frequency[2] = MIN_FREQUENCY;
                }
                tmp_input = frequency[2];
                break;
            default:break;
            }
        }
        if(update_data_flag == 1){
            update_data_flag = 0;
            if(host == PC){
                //如果数据从PC来，那么更新触摸屏数据
                if(control_id == 18){
                    unsigned int channel;
                    sscanf( input, "%d", &channel);//convert to unsigned int
                    update_touch_int(screen_id,control_id,channel);
                }else{
                    update_touch_data(screen_id,control_id, tmp_input,"%.1f");
                }
            }else if(host == TOUCHPAD){
                update_PC_data(screen_id,control_id,tmp_input);
            }
        }
    }

    free(cmd_content);
    free(cmd_start);
    free(cmd_end);
    free(cmd_id);

}

void scia_echoback_init()
{
   SciaRegs.SCICCR.all =0x0007;   // 1 stop bit,  No loopback
                                  // No parity,8 char bits,
                                  // async mode, idle-line protocol
   SciaRegs.SCICTL1.all =0x0001;  // enable TX, RX, internal SCICLK,
                                  // Disable RX ERR, SLEEP, TXWAKE
   SciaRegs.SCICTL2.bit.TXINTENA =0;
   SciaRegs.SCICTL2.bit.RXBKINTENA =1;
   SciaRegs.SCIHBAUD = 0x0000;
   SciaRegs.SCILBAUD = SCI_REG;
   SciaRegs.SCICCR.bit.LOOPBKENA =0; // Enable loop back
   SciaRegs.SCIFFTX.all=0xC002;
   SciaRegs.SCIFFRX.all=0x0021;
   SciaRegs.SCIFFCT.all=0x00;

   SciaRegs.SCICTL1.all =0x0023;     // Relinquish SCI from Reset
   SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;
   SciaRegs.SCIFFRX.bit.RXFIFORESET=1;
}
void scib_echoback_init()
{
   ScibRegs.SCICCR.all =0x0007;   // 1 stop bit,  No loopback
                                  // No parity,8 char bits,
                                  // async mode, idle-line protocol
   ScibRegs.SCICTL1.all =0x0001;  // enable TX, RX, internal SCICLK,
                                  // Disable RX ERR, SLEEP, TXWAKE
   ScibRegs.SCICTL2.bit.TXINTENA =0;
   ScibRegs.SCICTL2.bit.RXBKINTENA =1;
   ScibRegs.SCIHBAUD = 0x0000;
   ScibRegs.SCILBAUD = SCI_REG;
   ScibRegs.SCICCR.bit.LOOPBKENA =0; // Enable loop back
   ScibRegs.SCIFFTX.all=0xC002;
   ScibRegs.SCIFFRX.all=0x0021;
   ScibRegs.SCIFFCT.all=0x00;

   ScibRegs.SCICTL1.all =0x0023;     // Relinquish SCI from Reset
   ScibRegs.SCIFFTX.bit.TXFIFOXRESET=1;
   ScibRegs.SCIFFRX.bit.RXFIFORESET=1;
}

// Transmit a character from the SCI
void sci_xmit(char a,HOST host)
{
    switch(host){
    case TOUCHPAD:
        while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
        SciaRegs.SCITXBUF=a;
        break;
    case PC:
        while (ScibRegs.SCIFFTX.bit.TXFFST != 0) {}
        ScibRegs.SCITXBUF=a;
        break;
    }
}
void sci_xmit_2byte(unsigned int a,HOST host)
{
    switch(host){
    case TOUCHPAD:
        while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
        SciaRegs.SCITXBUF=(a >> 8 & 0xFF);
        SciaRegs.SCITXBUF=(a & 0xFF);
        break;
    case PC:
        while (ScibRegs.SCIFFTX.bit.TXFFST != 0) {}
        SciaRegs.SCITXBUF=(a >> 8 & 0xFF);
        SciaRegs.SCITXBUF=(a & 0xFF);
        break;
    }
}

void sci_msg(char val[],unsigned int length,HOST host){
    unsigned int i = 0;
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    for(i = 0 ; i < length;i++){
        sci_xmit(val[i],host);
    }
}

char *substring(char *string, int position, int length)
{
    //get substring of a string
   char *pointer;
   int c;

   pointer = malloc(length+1);

   for (c = 0 ; c < length ; c++)
   {
      *(pointer+c) = *(string+position);
      string++;
   }

   *(pointer+c) = '\0';

   return pointer;
}
char valid_cmd(char *cmd_s,char *cmd_end){
    //check if the command is valid
    char check = 1;

    //check if the start of the data frame is 0xEE
    check&=(cmd_s[0]==0xEE);

    //check if the end of the data frame is FF FC FF FF
    check&=(cmd_end[0]==0xFF);
    check&=(cmd_end[1]==0xFC);
    check&=(cmd_end[2]==0xFF);
    check&=(cmd_end[3]==0xFF);

    //check the command
    if(check){
        return 1;
    }else{
        return 0;
    }
}
//char result[40];
void update_touch_data(char screen,const char control_id,float new_data,char* format){
    char result[10];
    sprintf(result,format,new_data);

    sci_msg(send_begin, 3,TOUCHPAD);
    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(screen,TOUCHPAD);
    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(control_id,TOUCHPAD);
    sci_msg(result,3,TOUCHPAD);
    sci_xmit('\0',TOUCHPAD);
    sci_msg(send_end, 4,TOUCHPAD);
//    free(result);
}
void update_touch_int(char screen,const char control_id,unsigned int data){
    sci_msg(send_begin, 3,TOUCHPAD);
    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(screen,TOUCHPAD);
    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(control_id,TOUCHPAD);
    sci_xmit_2byte(data, TOUCHPAD);
    sci_msg(send_end, 4,TOUCHPAD);
}
void update_PC_data(char screen,const char control_id, float new_data){
    //更新PC端的数据，可能PC端要识别bytes数据，以回车结束
    //格式为 [screen] [control_id] [new_data] [\n]
    char send_buf[50];
    sprintf(send_buf,"%.1f",new_data);
    sci_xmit(screen,PC);
    sci_xmit(control_id,PC);
    sci_msg(send_buf,strlen(send_buf),PC);
    sci_xmit('\n',PC);
}

//这个函数仅仅用于触摸屏选择channel用的，应该以后要删掉
void send_two_byte_data(char screen,const char control_id,char val){
    sci_msg(send_begin, 3,TOUCHPAD);
    sci_xmit(screen, TOUCHPAD);
    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(control_id,TOUCHPAD);
    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(val,TOUCHPAD);
    sci_msg(send_end, 4,TOUCHPAD);
}
void update_progress(char val){
//    char *result;
//    result = malloc(40);
//    sprintf(result,format,new_data);

    sci_msg(send_begin, 3,TOUCHPAD);
    //screen id是Byte的数据
    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(0x00,TOUCHPAD);

    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(24,TOUCHPAD);

//    新的进度条值占据4个Byte
    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(val,TOUCHPAD);
//    发送停止
    sci_msg(send_end, 4,TOUCHPAD);
//    free(result);
}
void update_channel(char channel){
    sci_msg(send_begin, 3,TOUCHPAD);
    //screen id是Byte的数据
    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(0x00,TOUCHPAD);

    sci_xmit(0x00,TOUCHPAD);
    sci_xmit(18,TOUCHPAD);
    sci_xmit(channel,TOUCHPAD);
//    发送停止
    sci_msg(send_end, 4,TOUCHPAD);
//    free(result);
}
void change_icon_state(char icon_id[2],char show,HOST host){
    sci_msg(send_begin_hide,3,host);
    sci_msg(screen_id0,2,host);
    sci_msg(icon_id,2,host);
    if(show){
        sci_xmit(0x01,host);
    }else{
        sci_xmit(0x00,host);
    }
    sci_msg(send_end,4,host);
}
void add_data_channel(void){
    char tmp_start[] = {0xEE,0xB1,0x30};
    sci_msg(tmp_start,3,TOUCHPAD);
//    发送屏幕编号
    sci_msg(screen_id0,2,TOUCHPAD);
//    发送控件编号
    sci_msg(graph_id,2,TOUCHPAD);
//    发送颜色
    char tmp_color[] = {0xFF,0xFF};
    sci_msg(tmp_color,2,TOUCHPAD);

    sci_msg(send_end,4,TOUCHPAD);
}

//void add_data(unsigned int data){
//    char tmp_start[] = {0xEE,0xB1,0x32,0x00,0x00,0x00,23,0x00,0x00,0x02};
//    char tmp_data_len[] = {0x00,0x02};
//    char h_data = (data>>8)&0xFF;
//    char l_data = data&0xFF;
//    sci_msg(tmp_start,9,TOUCHPAD);
////发送数据
//    sci_xmit(h_data,TOUCHPAD);
//    sci_xmit(l_data,TOUCHPAD);
////    sci_xmit((data>>8)&0xFF,TOUCHPAD);
////    sci_msg(data,4,TOUCHPAD);
//    sci_msg(send_end,4,TOUCHPAD);
//}
void add_data(unsigned int data){
    char tmp_start[] = {0xEE,0xB1,0x32};
    char tmp_data_len[] = {0x00,0x02};
    sci_msg(tmp_start,3,TOUCHPAD);
//    ·¢ËÍÆÁÄ»±àºÅ
    sci_msg(screen_id0,2,TOUCHPAD);
//    ·¢ËÍ¿Ø¼þ±àºÅ
    sci_msg(graph_id,2,TOUCHPAD);
//    ·¢ËÍÍ¨µÀ
    sci_xmit(0x00,TOUCHPAD);
//·¢ËÍÊý¾Ý³¤¶È
    sci_msg(tmp_data_len,2,TOUCHPAD);
//·¢ËÍÊý¾Ý
    sci_xmit((data>>8)&0xFF,TOUCHPAD);
    sci_xmit(data&0xFF,TOUCHPAD);
//    sci_xmit((data>>8)&0xFF,TOUCHPAD);
//    sci_msg(data,4,TOUCHPAD);
    sci_msg(send_end,4,TOUCHPAD);
}
void icon_on_state(){
    change_icon_state(start_on_icon,1,TOUCHPAD);
    change_icon_state(start_off_icon,0,TOUCHPAD);
    change_icon_state(stop_on_icon,1,TOUCHPAD);
    change_icon_state(stop_off_icon,0,TOUCHPAD);
}
void icon_off_state(){
    change_icon_state(start_on_icon,0,TOUCHPAD);
    change_icon_state(start_off_icon,1,TOUCHPAD);
    change_icon_state(stop_on_icon,0,TOUCHPAD);
    change_icon_state(stop_off_icon,1,TOUCHPAD);
}
