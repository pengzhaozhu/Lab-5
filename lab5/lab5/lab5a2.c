/* Lab 5 Part A2
   Name: Pengzhao Zhu
   Section#: 112D
   TA Name: Chris Crary
   Description: This Program toggles on/off the red LED of the RBG package upon receiving a string of characters
*/



#include <avr/io.h>
void CLK_32MHZ(void);
void USARTD0_init(void);
void OUT_CHAR(uint8_t data);
uint8_t IN_CHAR(void);
void OUT_STRING(volatile uint8_t* data); //pointing the point at the first address. we have to pass in the address
										//without the dereferencing mark


#define BSELHIGH (((4)*((32000000/(16*57600))-1))>>8)   //bscale of -2
#define BSEL ((4)*((32000000/(16*57600))-1))			//bscale of -2

volatile uint8_t name[]="Pengzhao Zhu";   
											



int main(void)
{

	
	CLK_32MHZ();
	USARTD0_init();
	PORTD_DIRSET=0x10;   //set red led as output
	PORTD_OUTSET=0X10;  //turn off red led
	

	
	
	CHECK:;
	volatile uint8_t character=IN_CHAR();
	OUT_CHAR(character);
	
	
    if ((character != 'R') && (character !='r')) {
			goto CHECK;
		}
	
	character=IN_CHAR();
	OUT_CHAR(character);
	
	
	if ((character != 'E') && (character != 'e')){
		goto CHECK;
	}
			
	character=IN_CHAR();
	OUT_CHAR(character);
	
	if ((character != 'D') && (character != 'd')){
		goto CHECK;
	}
	
		PORTD_OUTTGL=0x10;
		goto CHECK;
			
		
	}
	
	
	

void USARTD0_init(void)
{
	PORTD_DIRSET=0x08;   //set transmitter as output
	PORTD_DIRCLR=0X04;	 //set receiver as input
	
	USARTD0_CTRLB=0x18;  //enable receiver and transmitter
	USARTD0_CTRLC= 0X33; //USART asynchronous, 8 data bit, odd parity, 1 stop bit
	
	USARTD0_BAUDCTRLA= (uint8_t) BSEL;    //load lowest 8 bits of BSEL
	USARTD0_BAUDCTRLB= (((uint8_t) BSELHIGH) | 0xE0); //load BSCALE and upper 4 bits of BSEL. bitwise OR them
	
	PORTD_OUTSET= 0x08;   //set transit pin idle
	
	
}
	



void CLK_32MHZ(void)
{
	
	//volatile uint8_t *p=&OSC_STATUS;       //inner volatile saying pointer p could change. 
													//outer volative saying data in p could change
													//reference to OSC_STATUS
	
	OSC_CTRL=0x02;     //select the 32Mhz osciliator
	while ( ((OSC_STATUS) & 0x02) != 0x02 );   //check if 32Mhz oscillator is stable
											//if not stable. keep looping
		
	CPU_CCP= 0xD8;                       //write IOREG to CPU_CCP to enable change
	CLK_CTRL= 0x01;						//select the 32Mhz oscillator
	CPU_CCP= 0xD8;						//write IOREG to CPU_CCP to enable change
	CLK_PSCTRL= 0x00;					//0x00 for the prescaler
	
}




void OUT_CHAR(volatile uint8_t data) {
	
	//volatile uint8_t *p=&USARTD0_STATUS;      //load the status flag data
	while( ((USARTD0_STATUS) & 0x20) != 0x20);			//keep looping if DREIF flag is not set
	
	USARTD0_DATA= (uint8_t) data;
	
}



void OUT_STRING(volatile uint8_t* data) {      //pointing the pointer at that address
	
	for (int i=0; data[i]!=0x00; i++) {      //go through the whole string except the null terminator
		OUT_CHAR((uint8_t) data[i]);			//output the value
}
	/*
	while(*data != 0)						//dereferencing
	{
		OUT_CHAR((uint8_t)*data);			//output the value
		data++;
	} */
	
	}
	

uint8_t IN_CHAR(void) {
	
	//volatile uint8_t *p=&USARTD0_STATUS;      //load the status flag data
	while( (USARTD0_STATUS & 0x80) != 0x80);			//keep looping if DREIF flag is not set
	
	return USARTD0_DATA;
	
}




