#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#define BUF_LEN 256
#define BUF_HZ (10000.0/BUF_LEN) // 20kHz per two clocks, so 20000/2 = 10000

volatile uint32_t buf_cursor = 0;
volatile uint8_t buf[BUF_LEN];
volatile uint32_t current_speed = 256*1;
volatile uint8_t note = 0;
volatile uint8_t pressed = 0;

void set_speed(float hz){
	current_speed = (hz*256.0)/BUF_HZ; // adjust cursor speed
}

uint8_t read_pin(uint8_t reg, uint8_t pin)
{
	return (reg >> pin) & 1;
}

void generate_wave(uint8_t wave){
	PORTC = 1;
	for (uint16_t i = 0; i<BUF_LEN; i++){	// buffer "shape"
		//buf[i] = 127.5*sin(i*2.0*M_PI/BUF_LEN)+127.5;
		//buf[i] =  abs(((i/(BUF_LEN/256))-(BUF_LEN/2)));

		if (i < BUF_LEN / 2) {
			buf[i] = 2*(i / (BUF_LEN / 256));
		} else {
			buf[i] = 2*(255 - (i / (BUF_LEN / 256)));
		}
	}
	PORTC = 0;
}


int main(){

	DDRD = 0xff;  // all outputs
	PORTD = 0x00; //active HIGH
	DDRC = 0xff;
	PORTC = 0x00;
	DDRB = 0x00; // all inputs (buttons)
	PORTB = 0xff; // active LOW (see circuit)

	TCCR0A |= (1 << WGM01); // Timer/counter register A masking: enable CTC mode
	OCR0A = 99;			// Compare value = 100 (Output -Compare Register) (16 MHz / 8 / 2)
	TIMSK0 |= (1 << OCIE0A);// Select output register, enable 0a (OCR0A)

	TCCR0B |= (1 << CS01); // Prescaler 8MHz / 8 = 1MHz (16MHz with external crystal)
	sei(); // enables interrupts

	float speed = 50;

	for (uint16_t i = 0; i<BUF_LEN; i++){
		buf[i] = 127.5*sin(i*2.0*M_PI/BUF_LEN)+127.5;
	}

	

	while(1){	// otherwise routine stops
		speed *= 1.005;
		set_speed(speed);
		if (pressed == 0 && read_pin(PINB, 0) == 0) {		// checks if button is pressed only once
			generate_wave(1);
			pressed = 1;
		}
		if (pressed == 1 && read_pin(PINB, 0) == 1){
			pressed = 0;
		}
		_delay_ms(10);
		if (speed > 10000000){
			speed = 1;
		}
	}


}

ISR (TIMER0_COMPA_vect) { // Timer/Counter0 Compare Match A 
	note = buf[(buf_cursor>>8)%BUF_LEN]; // to get the int value
	//note = note ^ (1<<3); // exlusive OR works as mask to invert only third bit (distortion)
	PORTD = note;
	buf_cursor = (buf_cursor+current_speed);	// move cursor to insert next value (%maximum value)
}