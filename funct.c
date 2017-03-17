/*
 * main.c
 *
 * Copyright (C) 2017 Sunlex @ Harbin Institute of Technology at Weihai,CN
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ser.h"
#include "funct.h"

#define DEBUG      //if denfine DEBUG, lots of message will be printed
#undef DEBUG

extern unchar sdata[SIZE];   //ra // complete
extern unchar cdata[SIZE];   //rb // raw
extern unchar payload[SIZE];  //rc//show
extern unchar rdata[SIZE];  //rd// recieve

extern unint ra,rb,rc,rd,i,j;
extern unint sum; //TFI+... ...+PDn
extern unchar state_response;
extern unint state_ack;
extern unint error_flag;
extern unchar Tg;




unchar send_command (unint len)
{  
#ifdef DEBUG
    printf("\n");
    printf("send command... ...");
#endif
    ra=0;
    sum=0;
    len&=0xff;

    sdata[ra++]=0x00;//00
    sdata[ra++]=0x00;//00

    sdata[ra++]=0xff;//ff
    sdata[ra++]=len; //len
    sdata[ra++]=(0x100-len)&0xff; //lcs

    for(rb=0;rb<len;rb++)
    {
        sdata[ra++]=cdata[rb];
        sum=sum+cdata[rb];
    }
    sum&=0xff;
    sdata[ra++]=(0x100-sum)&0xff;
    sdata[ra++]=0x00;
    sdata[ra]  =0x00;
    ser_senddata(sdata,ra);
#ifdef DEBUG
    printf("show command :\n");
    for(i=0;i<ra;i++)
    {
        printf("0x%02X ",sdata[i]);
    }
    printf("\n");
#endif
    return(0);
}

void show_payload (unint rx)
{
    ra=0;
    printf("show payload... ...\n");
    for(i=0;i<rx;i++)
    {
       printf("0x%02X ",payload[i]);
    }
    printf("complete, %dbyte(s)\n",rx);
    printf("\n");
}

unchar test_ack (void)
{      
    state_ack=0;
    unint i=0;
    rc=0;
    while(1)
    {   

        rb=ser_copystring(rdata);
        if(rb)  //recieve data, begin to check;
        {  
#ifdef DEBUG 
          for (i=0;i<rb;i++) printf("0x%02X ",rdata[i]);
          printf("\n");
#endif
          for(rc=0;rc<rb;rc++)
           {
            switch(state_ack)
            {  
                case 0: 
                {       
                    if(rdata[rc]==0x00) state_ack++;
                    else  state_ack=0;
                    break;
                }
                case 1:
                {       
                    if(rdata[rc]==0xff) state_ack++;
                    else if (rdata[rc]!=0x00)state_ack=0;
                    break;
                }
                case 2:
                {       
                    if(rdata[rc]==0x00) state_ack++;  // check ack 
                    else  state_ack=0;
                    break;
                }
                case 3:
                {   
                    if(rdata[rc]==0xff) //ack success
                        {
#ifdef DEBUG
                           printf("ACK success!!\n");
#endif                           
                            ser_dump(rb);
                            return(0);
                        }
                    else  state_ack=0;
                    break;
                }
            }

           }
#ifdef DEBUG
            printf("\n state_ack :%d ",state_ack);
#endif
            ser_dump(rb);
            return(1);
        }
        
        else  // fail to receive ,try again, max:5times
        {
            i++;
            if(i==500)
            {
                printf("fail to recieve data from PN532 and please try again \n");
                ser_dump(rb);
                return(2);
            }
            
        }
    }
}

unint test_response (void)
{
    unchar len;
    len=0;
    state_response=0;
    sum=0;
    rd=0;
    rb=0;

    while(1)
    {
        rb=ser_copystring(rdata);
        if(rb)
        {   
#ifdef DEBUG	
        		printf("rb: %d \n",rb);
          	printf("state_response:%d \n",state_response);
          	printf("receive:\n");
          	for(i=0;i<rb;i++) printf("0x%02X  ",rdata[i]);
#endif
            for(rc=0;rc<rb;rc++)
            {          
                //printf("192 state_response:%d \n",state_response);
                switch(state_response)
                {
                    case 0:
                    {
                        if(rdata[rc]==0x00) state_response++;
                        break;
                    }
                    case 1:
                    {
                        if(rdata[rc]==0xff) state_response++;
                        else 
                        if(rdata[rc]!=0x00) state_response=0;
                        break;
                    }
                    case 2:
                    {   
                        if(rdata[rc]!=0x00)
                        {   
                            len=rdata[rc];
                            //printf("rdata:0x%02X",rdata[rc]);
                            //printf("len:0x%02X",len);
                            state_response++;
                        }
                        else
                        {
                            state_response=0;
                        }
                        break;
                    }
                    case 3:  //lcs
                    {
                        if(((len+rdata[rc])&0xff)==0x00) state_response++;
                        else
                        {
                            printf("LCS error!\n");
                            //ser_dump(rb);
                            return(0);
                        }
                        break;
                    }
                    case 4:
                    {
                        payload[rd++]=rdata[rc];
                        sum+=rdata[rc];
                        if((--len)==0x00) state_response++;
                        break;
                    }
                    case 5:
                    {
                        sum+=rdata[rc];
                        sum&=0xff;
                        if(sum==0) 
                        {   
                            ser_dump(rb);
                            //printf("reponse succeed! \n");
                            return(rd);
                        }
                        else
                        {
                            printf("DCS error!\n");
                            return(0);
                        }
                    }
                }
            }
            ser_dump(rb);
        }

    }
}

unchar wake_up(void)
{
    ra=0;
    sdata[ra++]=0x55;
    sdata[ra++]=0x55;
    sdata[ra++]=0x55;
    sdata[ra++]=0x00;
    sdata[ra++]=0x00;
    sdata[ra++]=0x00;
    sdata[ra++]=0xff;
    sdata[ra++]=0x03;
    sdata[ra++]=0xfd;
    sdata[ra++]=0xd4;
    sdata[ra++]=0x14;
    sdata[ra++]=0x01;
    sdata[ra++]=0x17;
    sdata[ra++]=0x00;
    sdata[ra++]=0x00;
    ser_senddata(sdata,ra);
    error_flag=test_ack();
   // test_response();
    if(error_flag==0)
        {
            printf("ACK succeeded!\n");
            printf("Wake up succeeded!\n");
            return (0);
        }
#ifdef DEBUG
    if(error_flag==1) printf("ACK failed,can not recognize recieved data\n");
    if(error_flag==2) printf("ACK failed,fail to recieve data from PN532 and please try again\n");
#endif
    return(1);
}

char change_reg (reg_info creg)
{
    unint temp;
    
#ifdef DEBUG
    printf("show initial value(read register)\n");
#endif
    usleep(10000);
    rb=0;
    cdata[rb++]=0xD4;
    cdata[rb++]=0x06;
    cdata[rb++]=creg.addrH;
    cdata[rb++]=creg.addrL;
    j=0;
    while(1)
    {
        send_command(rb);
        usleep(8000);
        temp=test_response();
        if((temp!=0)&&(payload[1]==0x07))
        {   
            printf("the initial value of %s is 0x%02X \n",creg.name,payload[2]);
            break;
        }
        else if(j==3)
        {
            printf("response failed \n");
            exit(EXIT_FAILURE);
            break;
        }
        else j++;
    }
#ifdef DEBUG 
    printf("change value(write command)\n");
#endif    
    usleep(10000);
    rb=0;
    cdata[rb++]=0xD4;
    cdata[rb++]=0x08;
    cdata[rb++]=creg.addrH;
    cdata[rb++]=creg.addrL;
    cdata[rb++]=creg.value;
    j=0;
    while(1)
    {
        send_command(rb);
        usleep(8000);
        temp=test_response();
        if((temp!=0)&&(payload[1]==0x09))
        {
            printf("change the value to 0x%02X \n",creg.value);
            break;
        }
        if(j==3)
        {
            printf("response failed\n");
            exit(EXIT_FAILURE);
            break;
        }
        j++;
    }
#ifdef DEBUG    
    printf("check(read and compare)\n");
#endif    
    usleep(10000);
    rb=0;
    cdata[rb++]=0xD4;
    cdata[rb++]=0x06;
    cdata[rb++]=creg.addrH;
    cdata[rb++]=creg.addrL;
    j=0;
    while(1)
    {
        send_command(rb);
        usleep(8000);
        temp=test_response();
        if((temp!=0)&&(payload[1]==0x07)) 
        {   
            printf("the new value of %s is 0x%02X \n",creg.name,payload[2]);
            break;
        }
        if(j==3)
        {
            printf("response failed \n");
            exit(EXIT_FAILURE);
            break;
        }
        j++;
    }
    if(payload[2]==creg.value)
    {
        printf("change succeeded \n \n");
        return(0);
    }
    else
    {
        printf("change failed \n");
        exit(EXIT_FAILURE);
        return(1);
    }
}

void InListPassiveTarget ( void )     
{
    printf("\nInListPassiveTarget\n");
    ra=0;
    cdata[ra++]=0xD4;    
    cdata[ra++]=0x4A; //InListPassiveTarget
    cdata[ra++]=0x01; //number of targets
    cdata[ra++]=0x00; //106 kbps type A
    send_command(ra);
}

void InDataExchange (unchar * dataout, int len,int Tg)
{
    //printf("\nInDataExchange\n");
    ra=0;
    i=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x40;
    cdata[ra++]=Tg;
    for(i=0;i<len;i++)
    {
        cdata[ra++]=dataout[i];
    }
    send_command(ra);
}

int Authentication_KeyA(int block)
{
    printf("Authentication_KeyA begins at N.O %d block\n",block);
    int len;
    len=12;
    
    unchar dataout[len];
    unchar key[6];

    key[0]=0xFF;
    key[1]=0xFF;
    key[2]=0xFF;
    key[3]=0xFF;
    key[4]=0xFF;
    key[5]=0xFF;

    while(1)
    {   
        error_flag=0;
        InListPassiveTarget();
        usleep(8000);
        rc=test_response();
        if(payload[0]!=0xD5) error_flag++;
        if(payload[1]!=0x4B) error_flag++;
        if(payload[2]!=0X01) error_flag++; //NbTg
        if(payload[3]!=0x01) error_flag++; //Tg
        if(payload[4]!=0x00) error_flag++; //SENS_RES MSB
        if(payload[5]!=0x04) error_flag++; //SENS_RES LSB
        if(payload[6]!=0x08) error_flag++; //SEL_RES
        if(payload[7]!=0x04) error_flag++; //length of UID
        if(error_flag!=0)
        {
            printf("This Card is not a Mifare S50 card, the response is\n");
            show_payload(rc);
            return(1);
        }
        printf("UID: 0x%02X 0x%02X 0x%02X 0x%02X\n",payload[8],payload[9],payload[10],payload[11]);
        Tg=payload[3];
        i=0;
        dataout[i++]=0x60;
        dataout[i++]=block;
        for(j=0;j<6;j++)
        {
            dataout[i++]=key[j];
        }
        dataout[i++]=payload[8];  //uid
        dataout[i++]=payload[9];
        dataout[i++]=payload[10];
        dataout[i++]=payload[11];
#ifdef DEBUG
        printf("dataout:\n");
        for(j=0;j<len;j++) printf("0x%02X ",dataout[j]);
        printf("\n");
#endif
        InDataExchange(dataout,len,Tg);
        rc=test_response();
        //show_payload(rc);
        if((payload[0]==0xD5)&&(payload[1]==0x41)&&(payload[2]==0x00))
        {
            printf("The key A is right,Authentication succeeded\n");
            return(0);
        }
        else
        {
            printf("the key A is wrong,Authentication failed \n");
            show_payload(rc);
            return(1);
        }
    }
    return(2);
}

int reading(int block)
{
    printf("Reading begins at N.O %d block\n",block);
    int len;
    len=2;
    unchar dataout[len];
    dataout[0]=0x30;
    dataout[1]=block;
    InDataExchange(dataout,len,Tg);
    usleep(8000);
    rc=test_response();
    if((payload[0]==0xD5)&&(payload[1]==0x41)&&(payload[2]==0x00))
    {
        printf("reding succeeded\n");
        printf("Data get from Mifare card:\n");
        for(i=3;i<rc;i++)
        {
            printf("0x%02X ",payload[i]);
        }
        return(0);
    }
    else
    {
        printf("reading failed\n");
        return(1);
    }
    return(1);
}

int writing(int block,unint * wdata)
{
    printf("Writing begins at N.O %d block\n",block);

    printf("show wdata[]\n");
    for(i=0;i<16;i++) printf("0x%02X ",wdata[i]);
    printf("\n");

    int len=18;
    unchar dataout[len];
    i=0;
    j=0;
    dataout[i++]=0xA0;
    dataout[i++]=0x02;
    for(j=0;j<16;j++)   dataout[i++]=wdata[j];
    InDataExchange(dataout,len,Tg);
    usleep(8000);
    rc=test_response();
    if((payload[0]==0xD5)&&(payload[1]==0x41)&&(payload[2]==0x00))
    {
        printf("Writing succeede!\n");
        return(0);
    }
    else
    {
        printf("Writing failed!\n");
        return(1);
    }
    return(1);
}

void TgListPassiveTarget(unchar Mode,unchar * MifareParams,unchar * FelicaParams,
                    unchar * NFCID3t,unchar *Gt,unchar *Tk)
{
    ra=0;
    i=0;
    cdata[ra++]=0xD4;
    cdata[ra++]=0x8C;
    cdata[ra++]=Mode;
    for(i=0;i<6;i++)
    {
        cdata[ra++]=MifareParams[i]&0xff;
    }

    for(i=0;i<18;i++)
    {
        cdata[ra++]=FelicaParams[i]&0xff;
    }

    for(i=0;i<10;i++)
    {
        cdata[ra++]=NFCID3t[i]&0xff;
    }

    for(i=0;i<Gt[0];i++)
    {
        cdata[ra++]=Gt[i]&0xff;
    }

    for(i=0;i<Tk[0];i++)
    {
        cdata[ra++]=Tk[i]&0xff;
    }
    send_command(ra);
}

void TgResponseToInitiator(unchar *TgResponse, int len)
{   
    rb=0;
    cdata[rb++]=0xD4;
    cdata[rb++]=0x90;
    for(i=0;i<len;i++)
    {
        cdata[rb++]=TgResponse[i];
    }
    send_command(rb);
}
