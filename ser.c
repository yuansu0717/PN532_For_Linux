
//-----------------------------------------------------------------------------
// Copyright (C) David Welch, 2000
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

#include <sys/ioctl.h>
#include <time.h>

int ser_hand;   //文件句柄
unsigned short ser_buffcnt;   //buf计数器
unsigned short ser_maincnt;   //主计数器
unsigned char ser_buffer[(0xFFF+1)<<1]; //buffer

//-----------------------------------------------------------------------------
unsigned char ser_open ( char *port )    //打开指定的串口 并且配置工作模式如波特率
{
  struct termios options;   //termios的结构体 应该在termios的库函数里面

  ser_hand=open(port,O_RDWR|O_NOCTTY|O_NDELAY);   //open 函数 返回文件标识（打开成功） 或者 -1（打开失败）
  if(ser_hand==-1)
  {
    fprintf(stderr,"open: error - %s\n",strerror(errno));
    return(1);
  }
  fcntl(ser_hand,F_SETFL,FNDELAY);   //
  bzero(&options,sizeof(options));   // 先把options中的所有成员置为0
  //normal 8N1:
  options.c_cflag=B115200|CS8|CLOCAL|CREAD;  //配置串口
  options.c_iflag=IGNPAR;   //忽略校验位
//***************** 8E1 **********
  //options.c_cflag=B57600|CS8|CLOCAL|CREAD|PARENB;
  //options.c_iflag=INPCK;
//*******************************
  tcflush(ser_hand,TCIFLUSH);   // 清楚正在收到的数据 且不会读出来
  tcsetattr(ser_hand,TCSANOW,&options);   //立即将 options中的数据载入 ser_hand
  ser_maincnt=ser_buffcnt=0;

  return(0);
}
//----------------------------------------------------------------------------
void ser_close ( void )
{
  close(ser_hand);
}
//-----------------------------------------------------------------------------
void ser_senddata ( unsigned char *s, unsigned short len )
{
  unsigned int ret;
  ret=write(ser_hand,s,len);
  ret=ret; //unused variable ret
  tcdrain(ser_hand);    //等待所有写入fd中的数据输出
  //tcflush(ser_hand,TCIFLUSH);
}
//-----------------------------------------------------------------------------
void ser_sendstring ( char *s )
{
  unsigned int ret;
  ret=write(ser_hand,s,strlen(s));  //LINUX下 串口发送数据就相当于给串口写数据 返回实际的写入字节数
  ret=ret; //unused variable ret
  tcdrain(ser_hand); //等待所有写入fd中的数据输出
}
//-----------------------------------------------------------------------------
void ser_update ( void )   //更新buff数组 检查buff是否满了，满了则要舍去旧值
{
  int r;

  r=read(ser_hand,&ser_buffer[ser_maincnt],4000);  //返回读取的真实字节数(不一定是4000)
                                                  // 把数据读入ser_hand指定的文件里
  if(r>0)  //读入了超过1个数据，检查总位数是不是超过了12
  {
    ser_maincnt+=r;   //buffer count
    if(ser_maincnt>0xFFF)
    {
      ser_maincnt&=0xFFF;  //屏蔽高于 12位的数据，即把旧值舍去
      memcpy(ser_buffer,&ser_buffer[0xFFF+1],ser_maincnt);   //把更新
    }
  }
}
//-----------------------------------------------------------------------------
unsigned short ser_copystring ( unsigned char *d )   //把ser_buffer数组中的数据copy到d指向的空间
{
    unsigned short r;
    unsigned short buffcnt;
    unsigned short maincnt;

    ser_update();
    buffcnt=ser_buffcnt;
    maincnt=ser_maincnt;
    for(r=0;buffcnt!=maincnt;buffcnt=(buffcnt+1)&0xFFF,r++) *d++=ser_buffer[buffcnt]; //
    return(r); //返回总共copy的位数
}
//-----------------------------------------------------------------------------
unsigned short ser_dump ( unsigned short x ) //更新buffercount 返回buffercount新增的个数
{
    unsigned short r;

    for(r=0;r<x;r++)
    {
        if(ser_buffcnt==ser_maincnt) break;
        ser_buffcnt=(ser_buffcnt+1)&0xFFF;
    }
    tcflush(ser_hand,TCIFLUSH);
    return(r);
}
//-----------------------------------------------------------------------------
// Copyright (C) David Welch, 2000
//-----------------------------------------------------------------------------

