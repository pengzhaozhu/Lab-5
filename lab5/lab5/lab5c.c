/* Lab 5 Part C
   Name: Pengzhao Zhu
   Section#: 112D
   TA Name: Chris Crary
   Description: This Program continuously transmit 0x53. Will be used to verify the functionality of functions written in Part B and
				the transibility of my SPI module.
				
*/



#include <avr/io.h>
#include "LSM.h"
void CLK_32MHZ(void);
void SPI(void);    //SPI Initialization function
uint8_t SPI_WRITE(uint8_t data);   //SPI write function. returns data written to the SPIF Data register
uint8_t SPI_READ(void) ;     //read function to read from slave by writing junk data. return
//the two functions will be used separately?




int main(void){
	
	CLK_32MHZ();
	SPI();   //call function to initialize SPI
	
	while(1){
		SPI_WRITE(0x53);
	}
	return 0;
}



void SPI(void){
	
	PORTF_DIRCLR= 0b01000000; //set MISO as input
	PORTF_DIRSET=0b10111100; //set as output. the 1011 is SCK (SPI) enable, MOSI (SPI), and SSG (gyroscope)
	//why do I have to set the gyroscope as output?????
	//the 1100 is low true SSA signal of accelerometer and Sensor_sel of the mux (to accelerometer)
	
	
	SPIF_CTRL=0b01011111;    // enable SPI (bit 6), MSB first(bit 5), master mode(bit 4), (falling setup, rising sample)=11, 32MHZ/64=11
	PORTF_OUTSET=0b00011000; //set SSA and SSG high so it doesn't start. I will initialize in the write routine
	
}

uint8_t SPI_WRITE(uint8_t data){    //returns data written to the SPIF Data register
	PORTF_OUTCLR=0x08;   //enable slave(accelerometer) device by setting it low
	SPIF_DATA=data;    //write a byte of data to DATA register
	while((SPIF_STATUS & 0x80) != 0x80); //keep looping until interrupt flag is set. Also act as step one (reading STATUS REGISTER)
	//OF clearing the interrupt flag.
	PORTF_OUTSET=0x08;   //enable slave(accelerometer) device by setting it low
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
