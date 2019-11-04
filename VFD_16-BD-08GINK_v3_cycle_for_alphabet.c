#include "sdcc_reg420.h"
#include "8051.h"
#include <reg51.h>
#include "string.h"
#include <stdlib.h>
#include "stdio.h"
#include <stdbool.h>
#include <math.h>
#include "stdint.h"
#include "pt6302.h"

//#define SWAP_NIBBLES(x) asm volatile("swap %0" : "=r" (x) : "0" (x))

#define VFD_port P1
#define VFD_clk P1_0
#define VFD_in P1_1
//#define VFD_stb P1_2 // 16-BD-08GINK make use of CS inverted
#define VFD_blk P1_3
#define VFD_cs P1_4
#define VFD_reset P1_5	// use of reset inverted on futaba
#define VFD_bit8 P1_7

#define delay_tcsh _delay_us(15)

typedef int boolean;
#define true 1
#define false 0
#define L ((1<<14)-1)
#define R ((1<<12)-1)

#define BIN(x) \
 ( ((0x##x##L & 0x00000001L) ? 0x01 : 0) \
 | ((0x##x##L & 0x00000010L) ? 0x02 : 0) \
 | ((0x##x##L & 0x00000100L) ? 0x04 : 0) \
 | ((0x##x##L & 0x00001000L) ? 0x08 : 0) \
 | ((0x##x##L & 0x00010000L) ? 0x10 : 0) \
 | ((0x##x##L & 0x00100000L) ? 0x20 : 0) \
 | ((0x##x##L & 0x01000000L) ? 0x40 : 0) \
 | ((0x##x##L & 0x10000000L) ? 0x80 : 0))
//how use BIN: LCD_Port=BIN(00000011); 

// Accumulator (signal phase) for Direct Digital Synthesis (DDS)
volatile uint16_t accumulator;

// Increment value for DDS. This defines the signal frequency
volatile uint16_t increment;

void DelayMs(int x);
void DelayUs(int x);
char _sdcc_external_startup (void);
void pt6312_user_char(unsigned char n, unsigned char *font);

void time()
{
int i;
	for (i=0; i< 30;i++)
	{
	DelayMs(50);
	}
}
void delay()
{
int i,j;
for (i=0;i<=500;i++);
for (j=0;j<=500;j++);
}
void DelayMs(int x)
{
            int y = (x*1000)/12;
            while(--y != 0)
            continue;
}

void DelayUs(int x)
{
    int y = x/12; // Note: x must be bigger at least 12
    while(--y != 0)
    continue;
}

void _delay_ms(int x)
{
            int y = (x*1000)/12;
            while(--y != 0)
            continue;
}

void _delay_us(int x)
{
    int y = x/12; // Note: x must be bigger at least 12
    while(--y != 0)
    continue;
}


//###################################################
void pt6312_send_cmd(unsigned char a)
{
int i,j;
VFD_blk=0;
VFD_clk=1;
VFD_in=0;
delay_tcsh; 
VFD_cs=0;
delay_tcsh;
	for (i=0; i<8;i++) // 8 bit 0-7 // aqui inverti para ficar com 0x88 correccto
	{
	VFD_clk=0;
	delay_tcsh;//DelayUs(24);
	VFD_in=((a >> i)& 1);
	delay_tcsh;//DelayUs(24);
	VFD_clk=1;
	delay_tcsh;//DelayUs(24);
	}
VFD_blk=1;
VFD_clk=1;
VFD_in=0;
delay_tcsh;
VFD_cs=1;
delay_tcsh;	
}
//*******************************************************
void pt6312_send_cmd_withoutSTB(unsigned char a)
{
int i;
VFD_in=0;
VFD_clk=1;
//aqui nao tenho blk para permitir o incremento directo
	for (i=0; i<8;i++) // 8 bit 0-7 // aqui inverti para ficar com 0x88 correccto
	{
	// exp: com i=0, uso de ++; 0x31 sai 13, 0x38 sai 83, 0b10001111 sai F0
	VFD_clk=0;
	delay_tcsh;//DelayUs(24);
	VFD_in=((a >> i)& 1);
	//VFD_in=(a>>=1);
	delay_tcsh;//DelayUs(24);
	VFD_clk=1;
	delay_tcsh;//DelayUs(24);
	}
VFD_in=0;
VFD_clk=1;
}
//****************************************************
void pt6312_init(int shiftA, int shiftB)
{
int i,j;
VFD_cs=0;
i=shiftA;
j=shiftB;

	_delay_ms(100); //power_up delay
	
	//cmd1 Configure VFD display (grids) 
	pt6312_send_cmd(0b00000001);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;  // 5/16 5 grids & 16 segments
	
	//cmd2 Write to memory display, increment address, normal operation 
	pt6312_send_cmd(0b01000000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 

	VFD_cs=1;
	delay_tcsh;
	
	//cmd3 Address 00H - 15H ( total of 11*2Bytes=176 Bits)
	pt6312_send_cmd_withoutSTB(0b11000000);//(BIN(01100110)); //(BIN(01100110))); 
	delay_tcsh; 
	
	//data1		Grid A
	pt6312_send_cmd_withoutSTB(0b00000001 << i);
	delay_tcsh; 
	//data2
	pt6312_send_cmd_withoutSTB(0b00000001 << j);
	delay_tcsh; 
	//data3		Grid B
	pt6312_send_cmd_withoutSTB(0b00000001 << i);
	delay_tcsh; 
	//data4		
	pt6312_send_cmd_withoutSTB(0b00000001 << j);
	delay_tcsh; 
	//data5		Grid C
	pt6312_send_cmd_withoutSTB(0b00000001 << i);
	delay_tcsh; 
	//data6
	pt6312_send_cmd_withoutSTB(0b00000001 << j);
	delay_tcsh; 	
	//data7		Grid D
	pt6312_send_cmd_withoutSTB(0b00000001 << i);
	delay_tcsh; 
	//data8
	pt6312_send_cmd_withoutSTB(0b00000001 << j);
	delay_tcsh; 
	//data7		Grid E
	pt6312_send_cmd_withoutSTB(0b00000001 << i);
	delay_tcsh; 
	//data8
	pt6312_send_cmd_withoutSTB(0b00000001 << j);
	delay_tcsh; 
	
	//cmd4 set DIMM/PWM to value
	pt6312_send_cmd(0b10001111);// | 7);//0 min - 7 max  )(0b01010000)
	delay_tcsh; 

}

//***********************************************
void pt6312_Clear_MapMemory(int nBytes)
{
int i,j;
VFD_cs=0;

	_delay_ms(1); //power_up delay
	
	//cmd1 Configure VFD display (grids) 
	pt6312_send_cmd(0b00000001);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;  // 4/16 4 grids & 16 segments
	
	//cmd2 Write to memory display, increment address, normal operation 
	pt6312_send_cmd(0b01000000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 

	VFD_cs=1;
	delay_tcsh;
	
	//cmd3 Address 00H - 15H ( total of 11*2Bytes=176 Bits)
	pt6312_send_cmd_withoutSTB(0b11000000);//(BIN(01100110)); //(BIN(01100110))); 
	delay_tcsh; 
	
	//data1		Grid Memory
	for (i=0;i<nBytes;i++)
	{
	pt6312_send_cmd_withoutSTB(0b00000000);
	delay_tcsh; 
	}	
	//cmd4 set DIMM/PWM to value
	pt6312_send_cmd(0b10001111);// | 7);//0 min - 7 max  )(0b01010000)
	delay_tcsh; 

}
//******************************************************

//****************************************************
void pt6312_Count(int units,int tens,int undred)
{
int i,j;
VFD_cs=0;

	//cmd1 Configure VFD display (grids) 
	pt6312_send_cmd(0b00000001);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;  // 4/16 4 grids & 16 segments
	//DelayMs(1);
	//cmd2 Write to memory display, increment address, normal operation 
	pt6312_send_cmd(0b01000000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 
	//DelayMs(1);

	VFD_cs=1;
	delay_tcsh;
	
	//cmd3 Address 00H - 15H ( total of 11*2Bytes=176 Bits)
	pt6312_send_cmd_withoutSTB(0b11000000);//(BIN(01100110)); //(BIN(01100110))); 
	delay_tcsh; 
	
	//data1		Grid A
	pt6312_send_cmd_withoutSTB((0b00000000) | units);
	delay_tcsh; 
	//data2
	pt6312_send_cmd_withoutSTB(0b00000000);//
	delay_tcsh; 
	//data3		Grid B
	pt6312_send_cmd_withoutSTB((0b00000000) | tens);
	delay_tcsh; 
	//data4		
	pt6312_send_cmd_withoutSTB(0b00000000);
	delay_tcsh; 
	//data5		Grid C
	pt6312_send_cmd_withoutSTB((0b00000000) | undred);
	delay_tcsh; 
	//data5	
	pt6312_send_cmd_withoutSTB(0b00000000);
	delay_tcsh; 
	
	//cmd4 set DIMM/PWM to value
	pt6312_send_cmd(0b10001111);// | 7);//0 min - 7 max  )(0b01010000)
	delay_tcsh; 
}
//**************************

//****************************************************
void pt6312_Hexa(int hex)
{
int i,j;
VFD_cs=0;

	//cmd1 Configure VFD display (grids) 
	pt6312_send_cmd(0b00000001);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	//delay_tcsh;  // 4/16 4 grids & 16 segments
	DelayMs(1);
	//cmd2 Write to memory display, increment address, normal operation 
	pt6312_send_cmd(0b01000000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	//delay_tcsh; 
	DelayMs(1);

	VFD_cs=1;
	delay_tcsh;
	
	//cmd3 Address 00H - 15H ( total of 11*2Bytes=176 Bits)
	pt6312_send_cmd_withoutSTB(0b11000110);//(BIN(01100110)); //(BIN(01100110))); 
	delay_tcsh; 
	
	//data1		Grid A
	pt6312_send_cmd_withoutSTB((0b00000000) | hex);
	delay_tcsh; 
	//data2
	pt6312_send_cmd_withoutSTB(0b00000000);//
	delay_tcsh; 
	
	
	//cmd4 set DIMM/PWM to value
	pt6312_send_cmd(0b10001111);// | 7);//0 min - 7 max  )(0b01010000)
	delay_tcsh; 
}
//****************************************************

//**************************
void pt6312_cls(void)
{
unsigned char i;
VFD_blk=0;
DelayMs(20);
	pt6312_send_cmd_withoutSTB(0b00010000);//(0b00010000);
	for(i=0;i<16;i++)
	{
	pt6312_send_cmd_withoutSTB(' ');
	}
	delay_tcsh; 
	DelayMs(20);
VFD_blk=1;
DelayMs(20);

VFD_blk=0;
DelayMs(20);
	pt6312_send_cmd_withoutSTB(0b00110000);//(0b00110000); 
	for(i=0;i<16;i++) 
	{	
	pt6312_send_cmd_withoutSTB(0); 
	}
	delay_tcsh;
VFD_blk=1;
DelayMs(20);
}
//***************************************************
void pt6312_print(unsigned char address, char *text)
{
unsigned char c;
VFD_blk=0;
DelayMs(2);
	pt6312_send_cmd_withoutSTB((0b00010000) + (address & 0x0F));//)(0b00010000
	while ( (c = *text++) )
		{
		pt6312_send_cmd_withoutSTB(c);// & 0x7F);
		}
	delay_tcsh;	
VFD_blk=1;
DelayMs(2);
}
//************************************************
void check_run(unsigned int b)
{
int n ,g;
for (n=0;n<b;n++)
{
VFD_bit8= (0);
	for(g=0;g<5;g++)
	{
	DelayMs(50);
	}
VFD_bit8= (1);
	for(g=0;g<5;g++)
	{
	DelayMs(50);
	}
}
}

//****************************************

void init_16BD08GINK()
{
int i;
VFD_reset=1;
		for (i=0;i<20;i++)
		{
		DelayMs(5);
		}
	VFD_reset=0;
		for (i=0;i<20;i++)
		{
		DelayMs(5);
		}
	VFD_reset=1;
	
	for (i=0;i<4;i++)
	{
	VFD_bit8=1;
	DelayMs(50);
	VFD_bit8=0;
	DelayMs(50);
	}
	//cmd1 Configure general output port set command
	// peso: b7,b6,b5,b4,b3,b2,b1,b0
	pt6312_send_cmd(0b01000000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;  // 5/16 5 grids & 16 segments
	DelayMs(1);
	//cmd2 Number of digits set
	pt6312_send_cmd(0b01100000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 
	DelayMs(1);
	//cmd3 display dutty set command
	pt6312_send_cmd(0b01010000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;
	DelayMs(1);
	
}

void Wr_direct(char *text)
{
//this is the block common of code to start VFD
	//cmd3 Number of digits set
	pt6312_send_cmd(0b01100000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 
	
	//cmd4 duty set cmd
	pt6312_send_cmd(0b01010000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 
	
	//char 1
	// peso: b7,b6,b5,b4,b3,b2,b1,b0
	//grid cmd write (part1
	VFD_cs=0;

	pt6312_print(0x00,text);

	VFD_cs=1;
	//command last
	//display light all on or off
	pt6312_send_cmd(0b01110000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 
	
}

void VFD_16BD08GINK_cls()
{
unsigned int t;
for (t=0;t<16;t++){
	//this is the block common of code to start VFD
	//cmd3 Number of digits set
	pt6312_send_cmd(0b01100000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 
	
	//cmd4 duty set cmd
	pt6312_send_cmd(0b01010000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 
	
	//char 1
	// peso: b7,b6,b5,b4,b3,b2,b1,b0
	//grid cmd write (part1
	VFD_cs=0;
	
		pt6312_send_cmd_withoutSTB((0b00010000) + (t & 0x0F));//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
		delay_tcsh; 
		
		//data on grid of previous command
		pt6312_send_cmd_withoutSTB(0b00100000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
		
		delay_tcsh; 
		}
	VFD_cs=1;
		//command last
		//display light all on or off
		pt6312_send_cmd(0b01110000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
		delay_tcsh; 

}
void main()
{
unsigned char i,j,k,nBytes, x,z;
unsigned char s, t;
int shiftA, shiftB;
unsigned char font[] ={
//font data
0x03, 0xA2, 0xE2, 0x11, 0x55,
0x03, 0xA2, 0xE2, 0x39, 0xCE,
0x03, 0xA2, 0xE2, 0x7E, 0xB5,
0x07, 0xE3, 0x18, 0xC6, 0x3F,
0x00, 0x1C, 0xA5, 0x29, 0xC0,
0x00, 0x00, 0x42, 0x10, 0x00,
0x03, 0x94, 0x47, 0x11, 0x4A,
0x03, 0x94, 0x4F, 0x91, 0xD1
};	

unsigned char segmnts[] ={
//font data
    0b01110111, 0b00100100, 0b01101011, 0b01101101, // 0,1,2,3
    0b00111100, 0b01011101, 0b01011111, 0b01100100, // 4,5,6,7
    0b01111111, 0b01111100, //  8,9
    0b01111110, 0b00011111, 0b01010011, 0b00101111, // A,B,C,D
    0b01011011, 0b01011010, //  E,F
	};
	
init_16BD08GINK();

	while (1)
	{

	/* for (i=0;i<4;i++)
	{
	VFD_bit8=1;
	DelayMs(50);
	VFD_bit8=0;
	DelayMs(50);
	}*/
				VFD_16BD08GINK_cls();

			Wr_direct("  Display  VFD  ");
				for (i=0;i<50;i++)
				{
				DelayMs(50);
				} 
				
			Wr_direct(" 16-BD-08GINK  ");
				for (i=0;i<50;i++)
				{
				DelayMs(50);
				} 
				
				VFD_16BD08GINK_cls();
				
			Wr_direct("    FUTABA      ");
				for (i=0;i<50;i++)
				{
				DelayMs(50);
				} 

			
			
	for (s=32;s<128;s++){
	
	
		for (t=0;t<16;t++){
	//this is the block common of code to start VFD
	//cmd3 Number of digits set
	pt6312_send_cmd(0b01100000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 
	
	//cmd4 duty set cmd
	pt6312_send_cmd(0b01010000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 
	
	//char 1
	// peso: b7,b6,b5,b4,b3,b2,b1,b0
	//grid cmd write (part1
	VFD_cs=0;
	
		pt6312_send_cmd_withoutSTB((0b00010000) + (t & 0x0F));//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
		delay_tcsh; 
		
		//data on grid of previous command
		pt6312_send_cmd_withoutSTB((0b00000000) | s);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
		
		delay_tcsh; 
		 	for (i=0;i<8;i++)
			{
			VFD_bit8=1;
			DelayMs(20);
			VFD_bit8=0;
			DelayMs(20);
			} 

		
		}
		VFD_cs=1;
		//command last
		//display light all on or off
		pt6312_send_cmd(0b01110000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
		delay_tcsh; 
	}
	
	}
}

/*  // write 16 chars from B to P ( I remake it by usinf a for cycle)
//char 2 
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//grid cmd write (part1)
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01000010);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 

	//char 3 
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01000011);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;

	//char 4
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01000100);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 
	//char 5
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01000101);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;  
//char 6
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01000110);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;  

	//char 7
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01000111);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;  

	//char 8
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01001000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 

	//char 9
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01001001);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh; 
	//char 10
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01001010);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;    

	//char 11
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01001011);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;  

	//char 12
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01001100);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;  

	//char 13
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01001101);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;   

	//char 14
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01001110);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;
	
	//char 15
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01001111);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;  

	//char 16
	//peso: b7,b6,b5,b4,b3,b2,b1,b0
	//data on grid of previous command
	pt6312_send_cmd_withoutSTB(0b01010000);//(BIN(01000000)); //(BIN(01000000)));//  (0b01000000)
	delay_tcsh;     
	*/  