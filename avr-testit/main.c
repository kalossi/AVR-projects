#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "twi.h"

void adc_init(void)
{
    ADMUX = (1<<REFS0);     //select AVCC as reference
    ADCSRA = (1<<ADEN) | (1 << ADPS0);  //enable and prescale = 128 (16MHz/128 = 125kHz)



	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0); // Set ADC prescalar to 128 - 125KHz sample rate @ 16MHz

	ADMUX |= (1 << REFS0); // Set ADC reference to AVCC
	ADMUX |= (0 << REFS1); // Set ADC reference to AVCC
	ADMUX |= (1 << ADLAR); // Left adjust ADC result to allow easy 8 bit reading

	// No MUX values needed to be changed to use ADC0

//	ADCSRA |= (1 << ADFR);  // Set ADC to Free-Running Mode
	ADCSRA |= (1 << ADEN);  // Enable ADC
	ADCSRA |= (1 << ADSC);  // Start A2D Conversions

}

uint16_t readAdc(char ch)
{
	ADMUX=ch;
	ADCSRA |= (1<<ADSC);
	while(!(ADCSRA&(1<<ADIF)));
	ADCSRA|=(1<<ADIF);
	return(ADC);
}

uint8_t read_pin(uint8_t reg, uint8_t pin)
{
	return (reg >> pin) & 1;  // divided by 2 AND 0000 0001
}

int main()
{
	DDRB = 0xff;	// all outputs (LEDs)
	DDRD = 0x00;  // all inputs (buttons)
	PORTB = 0xff;
	PORTD = 0xff;
	DDRC=255;
	DDRC &= ~(1 << 1);
	PORTC = 255;
	_delay_ms(50);
	twi_init();

	sei();
	adc_init();


	while(1){
		uint16_t b = readAdc(1);
		if (read_pin(PIND, 0) == 1) {
			PORTB = ~b;
		} else {
			PORTB = ~b>>8;
		}
		_delay_ms(50);

		b = b;
		uint8_t *pt = (uint8_t *)&b;
		twi_writeTo(1, pt,2,1);
	}
	

}