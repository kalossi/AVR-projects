#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint32_t modulation;
volatile uint32_t current_delay;
volatile uint32_t delay_cursor = 0;
volatile uint8_t pressed = 0;
volatile int32_t vibrato = 0;

uint8_t read_pin(uint8_t reg, uint8_t pin)
{
	return (reg >> pin) & 1;
}

void play_hz(float hz)
{
	current_delay = 256.0*(20000.0/hz);
}

void play_notes(float *notes, uint8_t i)
{
	play_hz(notes[i]);
}

void set_vibrato(int8_t percent)
{
	if (percent < 0)
		vibrato = (uint32_t)(current_delay * (-percent)*vibrato_max) >> 7;
	else
		vibrato = (uint32_t)(current_delay * percent*vibrato_max) >> 7;
}

float vibrato_max = 0.020;

int main()
{
	DDRB = 0xff;	// all outputs (LEDs)
	DDRD = 0x00;  // all inputs (buttons)
	PORTB = 0xff;
	PORTD = 0xff;

	TCCR0A |= (1 << WGM01); // Timer/counter register A masking: enable CTC mode
	OCR0A = 99;			// Compare value = 100 (Output -Compare Register) (16 MHz / 8 / 2)
	TIMSK0 |= (1 << OCIE0A);// Select output register, enable 0a (OCR0A)

	TCCR0B |= (1 << CS01); // Prescaler 8MHz / 8 = 1MHz (16MHz with external crystal)
	sei(); // enables interrupts

	modulation = 50;
	current_delay = 22.727272*256;
	uint8_t direction = 1;
	float notes[][3] = {
		{523.25, 659.26, 783.99},
		{440.0, 523.25, 659.26},
		{349.23, 440.0, 523.25},
		{392.00, 493.22, 587.33},
	};
	uint8_t note_cursor = 0;
	uint32_t loop_cursor = 0;
	uint8_t sequence_cursor = 0;
	int8_t percent = 0;

	while(1){
		if (modulation == 24){
			direction = 1;
		}
		if (modulation == 127){
			direction = 0;
		}

		if (direction == 0){
			modulation--;
		} else{
			modulation++;
		}
		_delay_ms(4);
		loop_cursor++;
		pressed = 1;

		if (read_pin(PINB, 0) == 0) {
			play_hz(659.26);
		} else if (read_pin(PINB, 1) == 0) {
			play_hz(783.99);
		} else if (read_pin(PINB, 2) == 0) {
			play_hz(987.77);
		} else if (read_pin(PINB, 3) == 0){
			play_notes(notes[sequence_cursor], note_cursor%3);
		} else{
			play_hz(523.25);
		}

		if (loop_cursor % 10 == 0){
			note_cursor = (note_cursor+1)%3;
		}
		if (loop_cursor % 200 == 0) {
			sequence_cursor = (sequence_cursor + 1) % 4;
		}

		percent -= 3;
		set_vibrato(percent);
	}
	

}

ISR (TIMER0_COMPA_vect) { // Interrupt Service Routine (interrupt vector name)
	uint32_t tmp_delay = current_delay + vibrato;
	if (pressed == 1){
		if (delay_cursor < ((tmp_delay * modulation) >> 7)){
			PORTD = 255;
		}
		else{
			PORTD = 0;
		}
		if (delay_cursor >= tmp_delay) {
			delay_cursor = delay_cursor % tmp_delay;
		}
		delay_cursor += 256;
	}
}