
#include <avr/io.h>
#include <avr/interrupt.h>
#define hello 6

int main(void) {
	
	uint8_t a;
	if(USARTF0_DATA != 'G') {
		a=1;
	} else if (USARTF0_DATA=2 ) {
		a=0;
	}
	
	return 0;
}