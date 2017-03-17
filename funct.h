/*
 * main.c
 *
 * Copyright (C) 2017 Sunlex @ Harbin Institute of Technology at Weihai,CN
 *
 */
#ifndef FUNCT_H_
#define FUNCT_H_
#define SIZE 100
typedef unsigned int unint;
typedef unsigned char unchar;

typedef struct 
{
    char * name;
    unchar addrH;
    unchar addrL;
    unchar value;    
} reg_info;     //´æ´¢¼Ä´æÆ÷µÄÐÅÏ¢



//-------------send_command--------------//
//input:length of raw command (from TFI to PDn without pre adn post)
//return: success 0 error 1
//func: generate complete command(from preamble to postamble),and send to UART file
//date 2017.02.15
unchar send_command (unint );

//----------------show_payload--------------//
//input : length of payload
//func £ºprint payload
//return: 
//date 2017.02.15
void show_payload (unint );

//-----------test_ack------------------//
//input:
//func: test if PN532 receive command from host successfully;
//return: ACK succeeded:0 | ACK failed(but recieve data):1 | ACK failed(do not receive data):2  
//date 2017.2.15
unchar test_ack (void);

//-------------------test_response---------------------//
//input:
//func: test response, store useful bytes in payload[]
//return: success: the number of useful messege(without pre and post) fail:0;
//date:2017.2.17
unint test_response (void);

//---------------wake_up-----------------------//
//input:
//func: wake up PN532 and check the result
//return: wake up successfully:0  fail: 1
//date:2017.2.15
unchar wake_up(void);

//----------------change_reg----------------------------//
//input: struct reg_info
//func£ºshow initial value , change the value , check the result
//return: success:0 fail:1
//date:2017.2.17
char change_reg (reg_info );

//----------------InListPassiveTarget-----------------//
//input:
//func: set PN532 as PCD mode
//return:
//date:2017.2.20
void InListPassiveTarget ( void );

//------------InDataExchange------------------------//
//input: dataout , the length of dataout,Tg
//funct: PN532 send dataout to card(s)
//return:
//date:
void InDataExchange (unchar * , int ,int );

//--------------Authentication_KeyA-------------//
//input:block number
//func:Authentication command for Mifare PCD mode,get Tg,UID...
//return: 0:succeeded 1:failed
//date:2017.2.20
int Authentication_KeyA(int );

//---------------------Reading-----------------//
//input:block
//func: reading block
//return: 0:succeeded 1:failed
//date:2017.2.20
int reading(int );

//--------------------writing----------------//
//input:block ,writing arrary
//func: writing block with writing arrary
//return 0:succeeded 1: failed
//date: 2017.2.21
int writing(int ,unint * );

//---------------TgListPassiveTarget------------------//
//input:Mode,MifareParams,FelicaParams,NFCID3t,Gt,Tk;
//func:set PN532 as emulation card mode
//return:
//date:2017.2.22
void TgListPassiveTarget(unchar Mode,unchar * MifareParams,unchar * FelicaParams,
                    unchar * NFCID3t,unchar *Gt,unchar *Tk);
                    
//-------------------TgResponseToInitiator---------------/
//input: array of TgResponse,length of array
//funct: send a response to Initiator
//return:
//date:2017.2.23
void TgResponseToInitiator(unchar *, int );

#endif
