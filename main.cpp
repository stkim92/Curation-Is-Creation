#include "mbed.h"
#include "kor_char_code.h"
#include <stdint.h>
#include <string> 

#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 400            //mv
#define        SYS_VOLTAGE                     5000

Serial pc(USBTX,USBRX); // tx, rx
Serial TFT(PC_02,PC_03); // tx, rx
Serial lora(PA_13,PA_14); // tx, rx

Timer timer;
AnalogIn dustVal(A0);
DigitalOut myled(D6);

int flag_first = 0;
int analogVal_u16 =0;
int count;

char buffer[200]={0, };

int calibration(int m)
{
    static int _buff[10], sum;
    const int _buff_max = 10;
    int i;

    if(flag_first == 0) {
        flag_first = 1;

        for(i = 0, sum = 0; i < _buff_max; i++) {
            _buff[i] = m;
            sum += _buff[i];
        }
        return m;
    } else {
        sum -= _buff[0];
        for(i = 0; i < (_buff_max - 1); i++) {
            _buff[i] = _buff[i + 1];
        }
        _buff[9] = m;
        sum += _buff[9];

        i = sum / 10.0;
        return i;
    }
}

void statusPrint(float _dustDensity)
{
    if( (_dustDensity>0) && (_dustDensity<=15) ) {
        TFT.printf("$f,0,STATUS :,1,100,#\r\n");
        TFT.printf("$f,1,0,174,240#\r\n");
        TFT.printf("$f,0,GOOD :> ,140,100,#\r\n");
        TFT.printf("$f,1,0,0,0#\r\n");
        TFT.printf("$i,4,1,280,135,130,130#"); 
    }

    else if( (_dustDensity>=16) && (_dustDensity<=50) ) {
        TFT.printf("$f,0,STATUS :,1,100,#\r\n");
        TFT.printf("$f,1,120,254,98#\r\n");
        TFT.printf("$f,0,NORMAL :) ,140,100,#\r\n");
        TFT.printf("$f,1,0,0,0#\r\n");
        TFT.printf("$i,4,2,280,135,130,130#"); 
    }

    else if( (_dustDensity>=51) && (_dustDensity<=100) ) {
        TFT.printf("$f,0,STATUS :,1,100,#\r\n");
        TFT.printf("$f,1,255,255,75#\r\n");
        TFT.printf("$f,0,BAD :( ,140,100,#\r\n");
        TFT.printf("$f,1,0,0,0#\r\n");
        TFT.printf("$i,4,3,280,135,130,130#"); 
    }

    else if(_dustDensity>=101) {
        TFT.printf("$f,0,STATUS :,1,100,#\r\n");
        TFT.printf("$f,1,255,51,51#\r\n");
        TFT.printf("$f,0,DANGER :<,140,100,#\r\n");
        TFT.printf("$f,1,0,0,0#\r\n");
        TFT.printf("$i,4,4,280,135,130,130#"); 
        
    } else {
        TFT.printf("$f,0,STATUS :,1,100,#\r\n");
        TFT.printf("$f,1,0,0,0#\r\n");
        TFT.printf("$f,0,WAIT UNTIL 20 SEC !,140,100,#\r\n");
        TFT.printf("$f,1,0,0,0#\r\n");
        TFT.printf("$i,4,4,280,135,130,130#"); 
    }
}
float dustAnalSensing()
{
    int samplingTime = 280;
    int deltaTime = 40;
    int sleepTime = 9680;

    myled = 0;
    wait_us(samplingTime);

    analogVal_u16 = dustVal.read_u16();
    wait_us(deltaTime);
    
    myled = 1;
    wait_us(sleepTime);
    
    return analogVal_u16;
}

void recv(void){
	//string str;
	lora.gets(buffer,100);
	//str = buffer;

	//lora.printf(buffer);
	
	if(strstr(buffer, "JOINED") != NULL){
	//if(strstr(buffer, "completed") != NULL){
		for(int i=0; i< 3; i++){
			wait_us(10);
        	TFT.printf("$i,4,5,75,135,130,130#"); 
        	lora.printf("recv - JOIN compelted\r\n");

		}
    }
    
    if(strstr(buffer, "1320") != NULL){
        for(int i=0; i< 3; i++){
        	wait_us(10);	
        	TFT.printf("$i,4,6,75,135,130,130#"); 
        }    
    }
    if(strstr(buffer, "2320") != NULL){
        for(int i=0; i< 3; i++){
        	wait_us(10);	
        	TFT.printf("$i,4,7,75,135,130,130#"); 
        }    
    }
    if(strstr(buffer, "3320") != NULL){
        for(int i=0; i< 3; i++){
        	wait_us(10);	
        	TFT.printf("$i,4,8,75,135,130,130#"); 
        }    
    }
	//memset(buffer, '0', 99);
}
int main()
{
    TFT.baud(115200);
    lora.baud(38400);
    //lora.baud(115200);
    pc.baud(115200);
    //pc.baud(38400);
    
    unsigned int curr_Time1;
    unsigned int curr_Time2;
    int caliAnalogVal_u16;
    int digitalVal =0;
    int dustDensity =0;
    
    
    TFT.printf("$i,1,1,good.jpg#"); 
    TFT.printf("$i,1,2,soso.jpg#"); 
    TFT.printf("$i,1,3,not_good.jpg#"); 
    TFT.printf("$i,1,4,bad.jpg#"); 
    TFT.printf("$i,1,5,ready.jpg#"); 
    TFT.printf("$i,1,6,work.jpg#"); 
    TFT.printf("$i,1,7,snow.jpg#"); 
    TFT.printf("$i,1,8,slip.jpg#"); 
    TFT.printf("$l,0,255,255,255#"); // 사각형 색(흰색) 지정
    lora.printf("AT+RST\r\n");
    //lora.printf("LRW 70\r\n");

	timer.start();
	curr_Time1 = timer.read();
	//lora.printf("attach before\r\n");
	lora.attach(&recv,Serial::RxIrq);
	//lora.printf("attach after\r\n");
	while(1) 
	{
		TFT.printf("$f,4,2,#"); // 글자 크기 조절
        curr_Time2 = timer.read();
        if( curr_Time2-curr_Time1 >= 2) 
        {
            //lora.printf("2sec\r\n");
            analogVal_u16 = dustAnalSensing();
            caliAnalogVal_u16 = calibration(analogVal_u16); // 보정
            digitalVal =  (SYS_VOLTAGE / 4096) * caliAnalogVal_u16; //adc변경 (mv)

            if(digitalVal >= NO_DUST_VOLTAGE) {
                digitalVal -= NO_DUST_VOLTAGE;
                dustDensity = digitalVal * COV_RATIO; // Voltage(mv)값을  dust density로 변경 (ug/m3)
                dustDensity -= 50;
                if(dustDensity < 0){
                	dustDensity = 0;
                }
            } else {
                dustDensity = 0;
            }

            TFT.printf("$l,2,1,1,500,130,1#"); //흰 사각형 출력(지우기)
            TFT.printf("$f,0,Fine Dust(PM 2.5),1,10,#\r\n");
            TFT.printf("$f,0,Dust Value : %d (ug/m3),1,40,#\r\n",dustDensity);
            TFT.printf("$f,0,Current Time : %d (s),1,70,#\r\n",(int)timer.read());

            statusPrint(dustDensity);
            curr_Time1 = curr_Time2;
            //lora.printf("AT+SEND 01015445//\r\n");
            count += 1;
        }
        
        if( count == 20){
        	if( dustDensity >= 100 ){
        		lora.printf("AT+SEND 010%d\r\n",dustDensity);
        		//lora.printf("LRW 31 0%d cnf 1\r\n",dustDensity);
        	}
        	else if( dustDensity > 10 && dustDensity < 100 ){
        		lora.printf("AT+SEND 01%d\r\n",dustDensity);
        		//lora.printf("LRW 31 %d cnf 1\r\n",dustDensity);
        	}
        	else if( dustDensity < 10 ){
        		lora.printf("AT+SEND 010%d\r\n",dustDensity);
        		//lora.printf("LRW 31 0%d cnf 1\r\n",dustDensity);
        	}
        	count =0;
        }
    }
}
