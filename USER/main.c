#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "lcd.h"
#include "key.h"  
#include "24cxx.h" 
#include "myiic.h"
#include "touch.h" 
#include "timer.h"

#define POST_ENABLE 0

//ALIENTEK Mini STM32开发板范例代码21
//触摸屏实验  
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司
   	
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);//清屏   
 	POINT_COLOR=BLUE;//设置字体为蓝色 
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//显示清屏区域
  	POINT_COLOR=RED;//设置画笔蓝色 
}
////////////////////////////////////////////////////////////////////////////////
//电容触摸屏专有部分
//画水平线
//x0,y0:坐标
//len:线长度
//color:颜色
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//画实心圆
//x0,y0:坐标
//r:半径
//color:颜色
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{											  
	u32 i;
	u32 imax = ((u32)r*707)/1000+1;
	u32 sqmax = (u32)r*(u32)r+(u32)r/2;
	u32 x=r;
	gui_draw_hline(x0-r,y0,2*r,color);
	for (i=1;i<=imax;i++) 
	{
		if ((i*i+x*x)>sqmax)// draw lines from outside  
		{
 			if (x>imax) 
			{
				gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
				gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
			}
			x--;
		}
		// draw lines from inside (center)  
		gui_draw_hline(x0-x,y0+i,2*x,color);
		gui_draw_hline(x0-x,y0-i,2*x,color);
	}
}  
//两个数之差的绝对值 
//x1,x2：需取差值的两个数
//返回值：|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//画一条粗线
//(x1,y1),(x2,y2):线条的起始坐标
//size：线条的粗细程度
//color：线条的颜色
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		gui_fill_circle(uRow,uCol,size,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}   
////////////////////////////////////////////////////////////////////////////////
//5个触控点的颜色												 
//电阻触摸屏测试函数
void rtp_test(void)
{
	u8 key;
	u8 i=0;	  
	while(1)
	{
	 	key=KEY_Scan(0);
		tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
		{	
		 	if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height)
			{	
				if(tp_dev.x[0]>(lcddev.width-24)&&tp_dev.y[0]<16)Load_Drow_Dialog();//清除
				else TP_Draw_Big_Point(tp_dev.x[0],tp_dev.y[0],RED);		//画图	  			   
			}
		}else delay_ms(10);	//没有按键按下的时候 	    
		if(key==KEY0_PRES)	//KEY0按下,则执行校准程序
		{
			LCD_Clear(WHITE);//清屏
		    TP_Adjust();  //屏幕校准 
			TP_Save_Adjdata();	 
			Load_Drow_Dialog();
		}
		i++;
		if(i%20==0)LED0=!LED0;
	}
}
const u16 POINT_COLOR_TBL[OTT_MAX_TOUCH]={RED,GREEN,BLUE,BROWN,GRED};  
//电容触摸屏测试函数
void ctp_test(void)
{
	u8 t=0;
	u8 i=0;	  	    
 	u16 lastpos[5][2];		//最后一次的数据 
	while(1)
	{
		tp_dev.scan(0);
		for(t=0;t<OTT_MAX_TOUCH;t++)//最多5点触摸
		{
			if((tp_dev.sta)&(1<<t))//判断是否有点触摸？
			{
				if(tp_dev.x[t]<lcddev.width&&tp_dev.y[t]<lcddev.height)//在LCD范围内
				{
					if(lastpos[t][0]==0XFFFF)
					{
						lastpos[t][0] = tp_dev.x[t];
						lastpos[t][1] = tp_dev.y[t];
					}
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]);//画线
					lastpos[t][0]=tp_dev.x[t];
					lastpos[t][1]=tp_dev.y[t];
					if(tp_dev.x[t]>(lcddev.width-24)&&tp_dev.y[t]<16)
					{
						Load_Drow_Dialog();//清除
					}
				}
			}else lastpos[t][0]=0XFFFF;
		}
		
		delay_ms(5);i++;
		if(i%20==0)LED0=!LED0;
	}	
}

/*在第line行显示length长度的字符，从buffer指针指向元素开始显示*/
void dspLine(unsigned char *line, unsigned int lenTmp, unsigned char *buffer)
{
    
    if (0 == (*line))
    {
        LCD_Clear(WHITE);
    }
    
    LCD_ShowString(0, 20*(*line), 8*lenTmp, 16, 16, buffer);
    (*line)++;
    if ((*line) > 15)
    {
        *line = 0;
    }
}

/*从第line行开始，全屏显示长度为length的字符串，从buffer指针指向元素开始显示*/
void dipAll(unsigned char *line, unsigned int length, unsigned char *buffer)
{
    unsigned int lenTmp = length;
    unsigned char *bufTmp = buffer;
    for (;lenTmp > 30;lenTmp -= 30)
    {
        dspLine(line, 30, bufTmp);
        bufTmp += 30;
    }
    dspLine(line, lenTmp, bufTmp);
}

void cpyArray(u8 * aryObj, u8 * arySrc, u8 size)
{
    int time;
    for (time = 0; time < size; time++)
    {
        *aryObj = *arySrc;
        aryObj++;
        arySrc++;
    }
}

int main(void)
{ 
    u8 rdFlg = 0;
    u8 dspFlg = 0;
    u8 line = 0;
	u16 len;	
    u8 dspFlgTmp;
    NVIC_Configuration();
	delay_init();	    	 //延时函数初始化	  
	uart_init(9600);	 	//串口初始化为9600
	LCD_Init();			   	//初始化LCD 			 	
	tp_dev.init();			//触摸屏初始化
 	POINT_COLOR=BLACK;//设置字体为红色 
    TIM3_Int_Init(4999,7199);
    while (1)
    {
        if (10 == g_timer)
        {
            printf("AT+RST\r\n");
            g_timer++;
        }
        if (20 == g_timer)
        {
            printf("AT+CIFSR\r\n");
            g_timer++;
        }
        if (30 == g_timer)
        {
            printf("AT+CIPSTART=\"TCP\",\"api.yeelink.net\",80\r\n");
            g_timer++;
        }
        if (40 == g_timer)
        {
            printf("AT+CIPMODE=1\r\n");
            g_timer++;
        }
        if (50 == g_timer)
        {
            printf("AT+CIPSEND\r\n");
            g_timer++;
        }
        if (60 == g_timer)
        {
            #if POST_ENABLE 
            printf("POST /v1.0/device/19465/sensor/34148/datapoints HTTP/1.1\r\n");
            printf("Host: api.yeelink.net\r\n");
            printf("U-ApiKey: ee73a5df716ec70702b14b96ec26fb17\r\n");
            printf("Content-Type: application/x-www-form-urlencoded\r\n");
            printf("Content-Length: 14\r\n");
            printf("Connection: close\r\n");
            printf("\r\n");
            printf("{\"value\":10.3}\r\n");
            printf("\r\n");
            #else
            printf("GET /v1.0/device/19465/sensor/34147/datapoints HTTP/1.1\r\n");
            printf("Host: api.yeelink.net\r\n");
            printf("U-ApiKey: ee73a5df716ec70702b14b96ec26fb17\r\n");
            printf("Connection: close\r\n");//Keep-Alive
            printf("\r\n");
            printf("\r\n"); 
            #endif
            g_timer++;
            
        }
        if (75 == g_timer)
        {
            rdFlg = 1;
            g_timer++;
        }
        if (80 == g_timer)
        {
            printf("+++");
            g_timer++;
        }
        if (90 == g_timer)
        {
            printf("+++");
            g_timer++;
        }
        if (100 == g_timer)
        {
            printf("AT+CIPMODE=0\r\n");
            g_timer++;
        }
        if (1 == rdFlg)
        {
            rdFlg = 0;
            
            if (0 == dspFlgTmp)
            {
                dspFlgTmp = USART_BUF_LEN;
            }
            else
            {
                dspFlgTmp = dspFlg;
            }
            dipAll(&line, USART_REC_LEN, USART_RX_BUF[dspFlgTmp - 3]);
        }
        if ((USART_RX_STA[dspFlg] & 0x8000))
		{					   
			len=USART_RX_STA[dspFlg] & 0x3fff;//得到此次接收到的数据长度
            dipAll(&line, len, USART_RX_BUF[dspFlg]);
			USART_RX_STA[dspFlg]=0;
            dspFlg++;
            if (USART_BUF_LEN == dspFlg)
            {
                dspFlg = 0;
            }
		}
    }
}

