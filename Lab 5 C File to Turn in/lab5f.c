/* Lab 5 Part F
   Name: Pengzhao Zhu
   Section#: 112D
   TA Name: Chris Crary
   Description: This program will ultilize all functions written in this lab to plot real time data from the accelerometer
				
*/



#include <avr/io.h>
#include <avr/interrupt.h>
#include "LSM.h"
void CLK_32MHZ(void);

//SPI
void SPI(void);    //SPI Initialization function
uint8_t SPI_WRITE(uint8_t data);   //SPI write function. returns data written to the SPIF Data register
uint8_t SPI_READ(void) ;     //read function to read from slave by writing junk data. return
//the two functions will be used separately?
void ACCEL_WRITE(uint8_t addr, uint8_t data);
uint8_t ACCEL_READ(uint8_t addr);
void ACCEL_INIT(void);

//USART
void USARTD0_init(void);
void OUT_CHAR(uint8_t data);
uint8_t IN_CHAR(void);
void OUT_STRING(uint8_t* data); //pointing the point at the first address. we have to pass in the address
//without the dereferencing mark


#define BSELHIGH (((4)*((32000000/(16*1000000))-1))>>8)   //bscale of -2
#define BSEL ((4)*((32000000/(16*1000000))-1))			//bscale of -2

volatile uint8_t intbit;

int main(void){
	
	CLK_32MHZ();
	SPI();   //call function to initialize SPI
	ACCEL_INIT(); //call function to initialize accelerometer 
	USARTD0_init(); //call function to initialize USART system
	
	uint8_t XL;
	uint8_t XH;
	uint8_t YL;
	uint8_t YH;
	uint8_t ZL;
	uint8_t ZH;
	
	while(1) {
	while(intbit != 1);     //keep checking if the interrupt is set
	XL= ACCEL_READ (OUT_X_L_A);                    //read measurements from accelerometer
	XH= ACCEL_READ (OUT_X_H_A);
	YL= ACCEL_READ(OUT_Y_L_A);
	YH= ACCEL_READ(OUT_Y_H_A);
	ZL= ACCEL_READ(OUT_Z_L_A);
	ZH= ACCEL_READ(OUT_Z_H_A);
	OUT_CHAR(0x03);					//start byte
	OUT_CHAR(XL);
	OUT_CHAR(XH);
	OUT_CHAR(YL);
	OUT_CHAR(YH);
	OUT_CHAR(ZL);
	OUT_CHAR(ZH);
	OUT_CHAR(0xFC);					//end byte. inverse of start byte. One's complement
	
	
	intbit=0;           //set the bit to zero. until the ISR to change the intbit to 1 to output data to data stream
	}
	
	
	
	return 0;
}



void SPI(void){
	
	PORTF_DIRCLR= 0b01000000; //set MISO as input
	PORTF_DIRSET=0b10111100; //set as output. the 1011 is SCK (SPI) enable, MOSI (SPI), and SSG (gyroscope)
	                         //why do I have to set the gyroscope as output?????
	                         //the 1100 is low true SSA signal of accelerometer and Sensor_sel of the mux (to accelerometer)
	
	
	SPIF_CTRL=0b01011100;    // enable SPI (bit 6), MSB first(bit 5), master mode(bit 4), (falling setup, rising sample)=11, 32MHZ/64=00. changed
	PORTA_DIRSET=0x10; //set PROTOCOL_SEL as output
	PORTA_OUTCLR=0x10; //clear PROTOCOL_SEL to configure it as SPI. I2C is when i set it.
	PORTF_OUTSET=0b00011000; //set SSA and SSG high so it doesn't start. I will initialize in the write routine
	
}  //GOOD


uint8_t SPI_WRITE(uint8_t data){    //returns data written to the SPIF Data register
	//PORTF_OUTCLR=0x08;   //enable slave(accelerometer) device by setting it low. gotta take it out for ACCEL WRITE
	SPIF_DATA=data;    //write a byte of data to DATA register
	while((SPIF_STATUS & 0x80) != 0x80); //keep looping until interrupt flag is set. Also act as step one (reading STATUS REGISTER)
	                                     //OF clearing the interrupt flag.
	                                     //PORTF_OUTSET=0x08;   //disable slave(accelerometer) device by setting it low. gotta take it out for ACCEL WRITE
	return SPIF_DATA;
	
}

uint8_t SPI_READ(void) {      //read function to read from slave by writing junk data
	
	return (SPI_WRITE(0xFF));
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

void ACCEL_WRITE(uint8_t addr, uint8_t data){
	PORTF_OUTCLR=0x08;   //enable slave(accelerometer) device by setting it low. SPI have no automatic control of the SS line
	PORTF_OUTSET=0x04; //enable sensor_sel, make it high. sensor_sel = accelerometer
	
	addr= addr & 0b00111111 ;   //RW is always 0 (write) and MS is always 0
	
	SPI_WRITE(addr);
	SPI_WRITE(data);
	
	
	PORTF_OUTSET=0x08;   //disable slave(accelerometer) device by setting it high. SPI have no automatic control of the SS line
}

uint8_t ACCEL_READ(uint8_t addr){
	PORTF_OUTCLR=0x08;   //enable slave(accelerometer) device by setting it low. SPI have no automatic control of the SS line
	PORTF_OUTSET=0x04; //enable sensor_sel, make it high. sensor_sel = accelerometer
	
	addr=addr | 0b10000000;    //bitwise OR so RW (bit 7) is always 1 (Read). Gotta be careful of the MS signal
	
	SPI_WRITE(addr);
	uint8_t hi=SPI_READ();
	
	PORTF_OUTSET=0x08;   //disable slave(accelerometer) device by setting it high. SPI have no automatic control of the SS line
	
	return hi;   //data read from the ACCEL register
}



void ACCEL_INIT(void){
	PORTC_INTCTRL=0x01;   //enable low level interrupt
	PORTC_INT0MASK=0x80;  //set pin 7 on C as source for interrupt
	PORTC_DIRCLR=0x80;    //set pin 7 as input
	PORTC_PIN7CTRL=0x01; //rising edge trigger
	PMIC_CTRL=0x01;// enable low level interrupt in the PMIC
	sei();   //enable global interrupt flag
	ACCEL_WRITE(CTRL_REG4_A, 0x01);    //resetting the LSM system
	ACCEL_WRITE(CTRL_REG4_A,0b11101000); //data routed to to INT_A, interrupt signal active high, pulsed, INT1_A signal enable 0b11101000
	ACCEL_WRITE(CTRL_REG5_A,0b10010111); //fastest output rate, BDU continous update, X Y Z enabled
	
}    //also enabled PORT C pin 7 interrupt in the XMEGA


void USARTD0_init(void)
{
	PORTD_DIRSET=0x08;   //set transmitter as output
	PORTD_DIRCLR=0X04;	 //set receiver as input
	
	USARTD0_CTRLB=0x18;  //enable receiver and transmitter
	USARTD0_CTRLC= 0x03; //USART asynchronous, 8 data bit, odd parity, 1 stop bit
	
	USARTD0_BAUDCTRLA= (uint8_t) BSEL;    //load lowest 8 bits of BSEL
	USARTD0_BAUDCTRLB= (((uint8_t) BSELHIGH) | 0xE0); //load BSCALE and upper 4 bits of BSEL. bitwise OR them
	
	PORTD_OUTSET= 0x08;   //set transit pin idle
}


void OUT_CHAR(uint8_t data) {      //changed it to 8 bit sign for accelerometer
	
	//volatile uint8_t *p=&USARTD0_STATUS;      //load the status flag data
	while( ((USARTD0_STATUS) & 0x20) != 0x20);			//keep looping if DREIF flag is not set
	
	USARTD0_DATA= data;
	
}



void OUT_STRING(uint8_t* data) {      //pointing the pointer at that address
	
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

ISR(PORTC_INT0_vect) {
	uint8_t status=CPU_SREG;   //push status register
	PORTC_INTFLAGS=0x01 ;    //clear the interrupt flag
	intbit=1;      //change intbit to 1 so we can read and transmit measured data from the accelerometer 
	CPU_SREG= status;			//pop the status register
	
}
