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

//ALIENTEK Mini STM32�����巶������21
//������ʵ��  
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾
   	
void Load_Drow_Dialog(void)
{
	LCD_Clear(WHITE);//����   
 	POINT_COLOR=BLUE;//��������Ϊ��ɫ 
	LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//��ʾ��������
  	POINT_COLOR=RED;//���û�����ɫ 
}
////////////////////////////////////////////////////////////////////////////////
//���ݴ�����ר�в���
//��ˮƽ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
	if(len==0)return;
	LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//��ʵ��Բ
//x0,y0:����
//r:�뾶
//color:��ɫ
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
//������֮��ľ���ֵ 
//x1,x2����ȡ��ֵ��������
//����ֵ��|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{			 
	if(x1>x2)return x1-x2;
	else return x2-x1;
}  
//��һ������
//(x1,y1),(x2,y2):��������ʼ����
//size�������Ĵ�ϸ�̶�
//color����������ɫ
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	if(x1<size|| x2<size||y1<size|| y2<size)return; 
	delta_x=x2-x1; //������������ 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //���õ������� 
	else if(delta_x==0)incx=0;//��ֱ�� 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//ˮƽ�� 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //ѡȡ�������������� 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//������� 
	{  
		gui_fill_circle(uRow,uCol,size,color);//���� 
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
//5�����ص����ɫ												 
//���败�������Ժ���
void rtp_test(void)
{
	u8 key;
	u8 i=0;	  
	while(1)
	{
	 	key=KEY_Scan(0);
		tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)			//������������
		{	
		 	if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height)
			{	
				if(tp_dev.x[0]>(lcddev.width-24)&&tp_dev.y[0]<16)Load_Drow_Dialog();//���
				else TP_Draw_Big_Point(tp_dev.x[0],tp_dev.y[0],RED);		//��ͼ	  			   
			}
		}else delay_ms(10);	//û�а������µ�ʱ�� 	    
		if(key==KEY0_PRES)	//KEY0����,��ִ��У׼����
		{
			LCD_Clear(WHITE);//����
		    TP_Adjust();  //��ĻУ׼ 
			TP_Save_Adjdata();	 
			Load_Drow_Dialog();
		}
		i++;
		if(i%20==0)LED0=!LED0;
	}
}
const u16 POINT_COLOR_TBL[OTT_MAX_TOUCH]={RED,GREEN,BLUE,BROWN,GRED};  
//���ݴ��������Ժ���
void ctp_test(void)
{
	u8 t=0;
	u8 i=0;	  	    
 	u16 lastpos[5][2];		//���һ�ε����� 
	while(1)
	{
		tp_dev.scan(0);
		for(t=0;t<OTT_MAX_TOUCH;t++)//���5�㴥��
		{
			if((tp_dev.sta)&(1<<t))//�ж��Ƿ��е㴥����
			{
				if(tp_dev.x[t]<lcddev.width&&tp_dev.y[t]<lcddev.height)//��LCD��Χ��
				{
					if(lastpos[t][0]==0XFFFF)
					{
						lastpos[t][0] = tp_dev.x[t];
						lastpos[t][1] = tp_dev.y[t];
					}
					lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]);//����
					lastpos[t][0]=tp_dev.x[t];
					lastpos[t][1]=tp_dev.y[t];
					if(tp_dev.x[t]>(lcddev.width-24)&&tp_dev.y[t]<16)
					{
						Load_Drow_Dialog();//���
					}
				}
			}else lastpos[t][0]=0XFFFF;
		}
		
		delay_ms(5);i++;
		if(i%20==0)LED0=!LED0;
	}	
}

/*�ڵ�line����ʾlength���ȵ��ַ�����bufferָ��ָ��Ԫ�ؿ�ʼ��ʾ*/
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

/*�ӵ�line�п�ʼ��ȫ����ʾ����Ϊlength���ַ�������bufferָ��ָ��Ԫ�ؿ�ʼ��ʾ*/
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
	delay_init();	    	 //��ʱ������ʼ��	  
	uart_init(9600);	 	//���ڳ�ʼ��Ϊ9600
	LCD_Init();			   	//��ʼ��LCD 			 	
	tp_dev.init();			//��������ʼ��
 	POINT_COLOR=BLACK;//��������Ϊ��ɫ 
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
			len=USART_RX_STA[dspFlg] & 0x3fff;//�õ��˴ν��յ������ݳ���
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

