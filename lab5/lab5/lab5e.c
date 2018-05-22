/* Lab 5 Part E
   Name: Pengzhao Zhu
   Section#: 112D
   TA Name: Chris Crary
   Description: This program initialize and configure the necessary LSM330 accelerometer registers needed for Part F (real-time plotting)
				of the lab.
				
*/




#include <avr/io.h>
#include "LSM.h"
#include <avr/interrupt.h>
void CLK_32MHZ(void);
void SPI(void);    //SPI Initialization function
uint8_t SPI_WRITE(uint8_t data);   //SPI write function. returns data written to the SPIF Data register
uint8_t SPI_READ(void) ;     //read function to read from slave by writing junk data. return
//the two functions will be used separately?
void ACCEL_WRITE(uint8_t addr, uint8_t data);
uint8_t ACCEL_READ(uint8_t addr);
void ACCEL_INIT(void);





int main(void){
	
	CLK_32MHZ();
	SPI();   //call function to initialize SPI
	
	uint8_t hello;
	hello=ACCEL_READ(WHO_AM_I_A);


	
	while(1);
	
	return 0;
}





void SPI(void){
	
	PORTF_DIRCLR= 0b01000000; //set MISO as input
	PORTF_DIRSET=0b10111100; //set as output. the 1011 is SCK (SPI) enable, MOSI (SPI), and SSG (gyroscope)
	//why do I have to set the gyroscope as output?????
	//the 1100 is low true SSA signal of accelerometer and Sensor_sel of the mux (to accelerometer)
	
	
	SPIF_CTRL=0b01011111;    // enable SPI (bit 6), MSB first(bit 5), master mode(bit 4), (falling setup, rising sample)=11, 32MHZ/64=11
	PORTA_DIRSET=0x10; //set PROTOCOL_SEL as output
	PORTA_OUTCLR=0x10; //clear PROTOCOL_SEL to configure it as SPI. I2C is when i set it.
	PORTF_OUTSET=0b00011000; //set SSA and SSG high so it doesn't start. I will initialize in the write routine
	
}

uint8_t SPI_WRITE(uint8_t data){    //returns data written to the SPIF Data register
	PORTF_OUTCLR=0x08;   //enable slave(accelerometer) device by setting it low
	SPIF_DATA=data;    //write a byte of data to DATA register
	while((SPIF_STATUS & 0x80) != 0x80); //keep looping until interrupt flag is set. Also act as step one (reading STATUS REGISTER)
	//OF clearing the interrupt flag.
	PORTF_OUTSET=0x08;   //disable slave(accelerometer) device by setting it low
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
	
	SPIF_DATA=addr;    //writing the address byte. MSB bit is RW, Write=0, read=1 (need to be 0). second bit=MS=0
	while((SPIF_STATUS & 0x80) != 0x80); //keep looping until interrupt flag is set. Also act as step one (reading STATUS REGISTER)
	//OF clearing the interrupt flag.

	SPIF_DATA=data;    //write the actual data
	while((SPIF_STATUS & 0x80) != 0x80); //keep looping until interrupt flag is set. Also act as step one (reading STATUS REGISTER)
	//OF clearing the interrupt flag.
	
	PORTF_OUTSET=0x08;   //disable slave(accelerometer) device by setting it high. SPI have no automatic control of the SS line
}

uint8_t ACCEL_READ(uint8_t addr){
	PORTF_OUTCLR=0x08;   //enable slave(accelerometer) device by setting it low. SPI have no automatic control of the SS line
	PORTF_OUTSET=0x04; //enable sensor_sel, make it high. sensor_sel = accelerometer
	
	addr=addr | 0b10000000;    //bitwise OR so RW (bit 7) is always 1 (Read). Gotta be careful of the MS signal
	
	SPIF_DATA=addr;							//writing the address byte. MSB bit is RW, Write=0, read=1 (need to be 1). second bit=MS=0
	while((SPIF_STATUS & 0x80) != 0x80);	//keep looping until interrupt flag is set. Also act as step one (reading STATUS REGISTER)
	//OF clearing the interrupt flag.
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
	ACCEL_WRITE(CTRL_REG2_A, 0x01);    //resetting the LSM system
	ACCEL_WRITE(CTRL_REG2_A,0b11101000); //data routed to to INT_A, interrupt signal active high, edge triggered, INT1_A signal enable
	ACCEL_WRITE(CTRL_REG5_A,0b10010111); //fastest output rate, BDU continous update, X Y Z enabled
}