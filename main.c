/*
 * main.c
 *
 * Copyright (C) 2017 Sunlex @ Harbin Institute of Technology at Weihai
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  
#include <sys/stat.h>  
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "funct.h"
#include "ser.h"


//-------------------------------------//

unchar sdata[SIZE];   //ra // complete
unchar cdata[SIZE];   //rb // raw
unchar payload[SIZE];  //rc//show
unchar rdata[SIZE];  //rd// recieve

unint ra,rb,rc,rd,i,j;
unint sum; //TFI+... ...+PDn
unchar state_response;
unint state_ack;
unint error_flag;
unchar Tg;

int main ( int argc, char *argv[] )
{
//---------------------argument--------------
    int mode,changeReg;
    int block,tmp;
    int op; //write or read
    //unchar v1,v2,v3;
    unint wdata[16];
    //unint len;
		
    unchar Mode=0x00;     //|SENS_RES| |UID          |SEL_RES
    unchar MifareParams[6]={0x04,0x00, 0x11,0x22,0x33, 0x60};
    unchar FelicaParams[18]={0x00,0x11};
    unchar NFCID3t[10]={0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77};
    unchar Gt[SIZE]={0x00};
    unchar Tk[SIZE]={0x00};
    //unchar TgResponse[SIZE];

//--------------port open---------------------------//
    printf("\n");
    if(argc<2)
    {
        printf("progstm /dev/ttyXYZ\n");
        return(1);
    }

    if(ser_open(argv[1]))
    {
        printf("ser_open() failed\n");
        return(1);
    }
    printf("port opened\n");

//--------------------wake_up--------------------------------------------//
    printf("Start to wake up PN532... ...\n");
    sleep(1);
    i=0;
    while(1)
    {   
        j=wake_up();
        if(j==0) break;
        if(i==5)
        {
            printf("wake up failed\n");
            break;
        }
        i++;
        sleep(0.1);
    }
    ra=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x14; //SAMConfiguration
    cdata[ra++]=0x01; //Normal mode SAM not used
    send_command(ra);
    usleep(8000);
    rc=test_response();
    sleep(0.1);

//------------------configure REG----//
    printf("Do you want to configure registers? 1:YES 2:NO\n");
    setbuf(stdin,NULL);
    j=scanf("%d",&changeReg);
    setbuf(stdin,NULL);
    if(changeReg==1)
    {   

        reg_info CIU_RFCfg = {
            "CIU_RFCfg",
            0x63,
            0x16,
            0x74
        };
        reg_info CIU_ModWidth={
            "CIU_ModWidth",
            0x63,
            0x14,
            0x28            
        };
        reg_info CIU_TxBitPhase={
            "CIU_TxBitPhase",
            0x63,
            0x15,
            0x8a
        };
        change_reg(CIU_RFCfg);
        sleep(1);
        change_reg(CIU_ModWidth);
        sleep(1);
        change_reg(CIU_TxBitPhase);
    }
    else if(changeReg==2)
    {
        printf("Skip configure_REG, use default value.\n");
    }
    else
    {
        printf("iilegal input,exit programme\n");
        exit(EXIT_FAILURE);
    }

//--------------------mode choose----------------------------//
    printf("\n\nMode choose... ..., 1:PICC or 2:PCD? \n");
    setbuf(stdin,NULL);
    j=scanf("%d",&mode);
    if(mode==2) //pcd
    {
//-------------------------PCD mode-------------------------//        
        printf("you have chosen PCD mode\n");
        while(1)
        {//
            //block=0;
            printf("input block number(0-3) you want to access,!9! to exit\n");
            setbuf(stdin,NULL);
            j=scanf("%d",&tmp);
            setbuf(stdin,NULL);
            if(tmp==9) break;
            block=tmp;
            setbuf(stdout,NULL);
            error_flag=Authentication_KeyA(block);
            if(error_flag==1) exit(EXIT_FAILURE);
            printf("input your op,1:read 2:write,other:no op\n");
            setbuf(stdin,NULL);
            j=scanf("%d",&op);
            setbuf(stdin,NULL);
            switch(op)
            {   
                case 1:  //read
                {   error_flag=reading(block);
                    if(error_flag==1) exit(EXIT_FAILURE);
                    break;
                }
                case 2: //write
                {
                    for(i=0;i<4;i++)
                    {
                        setbuf(stdin,NULL); //清空缓冲区
                        printf("N.O. %d 4-bytes,in HEX like 11223344\n",i+1);
                        j=scanf("%02X%02X%02X%02X",&wdata[4*i],&wdata[4*i+1],&wdata[4*i+2],&wdata[4*i+3]);
                    }
                    error_flag=writing(block,wdata);
                    if(error_flag==1) exit(EXIT_FAILURE);
                    break;
                }
                default: break;
            }
        }
        
    }
    else if(mode==1) //picc
    {
//----------------------------PICC mode--------------------------------//
        printf("you have chosen PICC mode\n");
        while(1)
        {
        	TgListPassiveTarget(Mode,MifareParams,FelicaParams,NFCID3t,Gt,Tk);
        	if(test_ack()==0) break;
        }
        printf("PICC set succeeded, enter 'e' to exit\n");
        setbuf(stdin,NULL);
        while(1)
        {
        	if(getchar()=='e') break;
        	setbuf(stdin,NULL);
        } 
        	
        /******************
        usleep(8000);
        rc=test_response();
        error_flag=0;
        if(payload[rc++]!=0xD5) error_flag++;
        if(payload[rc++]!=0x8D) error_flag++;
        if(error_flag!=0)
        {
            printf("error!\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            switch(payload[rc]&0x70)
            {
                case 0x00: printf("Baudrate:106kbs\n");
                case 0x10: printf("Baudrate:212kbs\n");
                case 0x20: printf("Baudrate:424kbs\n");
            }
            switch(payload[rc]&0x08)
            {
                case 0x08: printf("ISO/IEEE 14443-4 PICC ? YES\n");
                case 0x00: printf("ISO/IEEE 14443-4 PICC ? NO\n");
            }
            switch(payload[rc]&0x04)
            {
                case 0x04: printf("DEP? YES\n");
                case 0x00: printf("DEP? NO\n");
            }
            switch(payload[rc]&0x03)
            {
                case 0x00: printf("Mifare\n");
                case 0x01: printf("Active mode\n");
                case 0x02: printf("Felica\n");
            }
        }

        printf("InitiatorCommand\n");
        for(i=3;i<rc;i++) printf("0x%02X ",payload[i]);
				*****************************/
    }
    
    else
    {
        printf("iilegal input!! exit programme\n");
        exit(EXIT_FAILURE);
    }
    printf("end\n");
    //setbuf(stdin,NULL);
    //getchar();
    ser_close();
    return(0);
}