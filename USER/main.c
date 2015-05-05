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
#include "cJSON.h"
#include "ds18b20.h"

#define POST_ENABLE 0

int main(void)
{ 
    char *prtTst = "AT+RST\r\n";
    u8 jsonvl = 2;
    float tprt = 0;
    char *out;cJSON *json,*root;
    u8 rdFlg = 0;
    u8 dspFlg = 0;
    u8 line = 0;
	u16 len;	
    u8 dspFlgTmp;

    NVIC_Configuration();

	delay_init();	    	 //延时函数初始化	  
	uart_init(9600);	 	//串口初始化为9600
	LCD_Init();			   	//初始化LCD 
    DS18B20_Init(); 
	tp_dev.init();			//触摸屏初始化
 	POINT_COLOR=BLACK;//设置字体为红色 
    TIM3_Int_Init(4999,7199);
    tprt=DS18B20_Get_Temp();
    tprt/=10;
    root=cJSON_CreateObject();
    cJSON_AddNumberToObject(root,"value",15.5);
    out=cJSON_PrintUnformatted(root);

    
    while (1)
    {
        #if POST_ENABLE
        #else
        tprt=DS18B20_Get_Temp();
        LCD_ShowNum(0,0,(int)tprt,3,16);
        #endif
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
            printf("Accept: */*\r\n");
            printf("U-ApiKey: ee73a5df716ec70702b14b96ec26fb17\r\n");
            printf("Content-Length: 14\r\n");
            printf("Content-Type: application/x-www-form-urlencoded\r\n");
            printf("\r\n");
//            printf("{\"value\":13.5}\r\n");
            printf("%s\r\n",out);
            
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
        if (80 == g_timer)
        {
            rdFlg = 1;
            g_timer++;
        }
        if (90 == g_timer)
        {
            printf("+++");
            g_timer++;
        }
        if (100 == g_timer)
        {
            printf("+++");
            g_timer++;
        }
        if (110 == g_timer)
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
            #if POST_ENABLE
            
            #else
            dipAll(&line, USART_REC_LEN, USART_RX_BUF[dspFlgTmp - 3]);
            json=cJSON_Parse(USART_RX_BUF[dspFlgTmp - 3]);
            jsonvl = json -> child -> next-> valueint;
            #endif
//            *out = '0' + jsonvl;
//            dipAll(&line, 1, out);
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

