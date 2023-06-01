#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned char sec_tick=0,min_tick=0,hour_tick=0; //three variables act as counters for second, minutes and hours.
void INT0_RESET(void)
{
	DDRD&=~(1<<PD2); //set PD2 as input for INT0.
	PORTD|=(1<<PD2); //enable internal pull-up resistor.
	MCUCR|=(1<<ISC01); //generate INT0 request (reset) at falling edge.
	GICR|=(1<<INT0); //INT0 request enable.
}
ISR(INT0_vect)
{
	// resetting all counters at reset interrupt.
	TCNT1=0;
	sec_tick=0;
	min_tick=0;
	hour_tick=0;
}
void INT1_PAUSE(void)
{
	DDRD&=~(1<<PD3); //set PD3 as input for INT1.
	MCUCR|=(1<<ISC10)|(1<<ISC11); //generate INT1 (pause) request at raising edge.
	GICR|=(1<<INT1); //INT1 request enable.
}
ISR(INT1_vect)
{
	TCCR1B&=~(1<<CS10)&~(1<<CS12); //stop timer.
}
void INT2_RESUME(void)
{
	DDRB&=~(1<<PB2);
	PORTB|=(1<<PB2); //enable internal pull-up resistor.
	MCUCSR&=~(1<<ISC2); //generate INT2 (resume) request at raising edge.
	GICR|=(1<<INT2);//INT2 request enable.
}
ISR(INT2_vect)
{
	TCCR1B|=(1<<CS10)|(1<<CS12); //resume timer (1024 prescaler).
}
void TIMER1_Init(void)
{
	TCCR1A=(1<<FOC1A); //force compare match mode to compare unit A.
	TCCR1B=(1<<WGM12)|(1<<CS10)|(1<<CS12); //set the timer on mode 4 (ctc) and operate with 1024 prescaler.
	OCR1A=977; //set compare value of compare unit A at 977 to  generate interrupt request every one second.
	TIMSK|=(1<<OCIE1A); //output compare A match enable.
	sei(); //enable global interrupt.
}
ISR(TIMER1_COMPA_vect)
{
	/*
	 incrementing seconds at every ISR and resetting seconds counter at 60 while incrementing
	  minutes counter by one and the same goes between minutes and hours.
	 */
	sec_tick++;

	if(sec_tick==60)
	{
		min_tick++;
		sec_tick=0;
	}
	if(min_tick==60)
	{
		hour_tick++;
		min_tick=0;
	}
	/*
	 As the connected device has only two 7-segments for displaying hours, the max number
	of hours to be displayed is 99. so all the counters have to be reset after displaying 99:59:59.
	*/
	if(hour_tick==100)
	{
		sec_tick=0;
		min_tick=0;
		hour_tick=0;
	}
}
void seven_seg_init(void)
{
	DDRC |= 0x0F; //set the first 4 pins of port C as output.
	PORTC &= 0xF0;
	DDRA|=0x3F; //set the first 6 pins of port A as output to control the  display of the six 7-segments.
	PORTA&=0xC1;
}

void display_secs(void)
{
	PORTC = (PORTC & 0xF0) | ((sec_tick-((sec_tick/10)*10)) & 0x0F); //pass the ones value of the seconds to port C.
	PORTA|=0x01;  //display on 1st 7-segment.
	_delay_ms(2);
	PORTA&=0xC0;  //turn the 7-segment off before switching the display to the next one.
	PORTC = (PORTC & 0xF0) | ((sec_tick/10) & 0x0F); //pass the tens value of the seconds to port C.
	PORTA|=0x02;  //display on 2nd 7-segment.
	_delay_ms(2);
	PORTA&=0xC0;
}

void display_mins(void)
{
	PORTC = (PORTC & 0xF0) | ((min_tick-((min_tick/10)*10)) & 0x0F); //pass the ones value of the minutes to port C.
	PORTA|=0x04;  //display on 3rd 7-segment.
	_delay_ms(2);
	PORTA&=0xC0;
	PORTC=(PORTC & 0xF0) | ((min_tick/10) & 0x0F);  //pass the tens value of the minutes to port C.
	PORTA|=0x08;  //display on 4th 7-segment.
	_delay_ms(2);
	PORTA&=0xC0;
}
void display_hours(void)
{
	PORTC = (PORTC & 0xF0) | ((hour_tick-((hour_tick/10)*10)) & 0x0F);  //pass the ones value of the hours to port C.
	PORTA|=0x10; //display on 5th 7-segment.
	_delay_ms(2);
	PORTA&=0xC0;
	PORTC=(PORTC & 0xF0) | ((hour_tick/10) & 0x0F);   //pass the tens value of the hours to port C.
	PORTA|=0x20;  //display on 6th 7-segment.
	_delay_ms(2);
	PORTA&=0xC0;
}

int main(void)
{
	seven_seg_init();
	TIMER1_Init();
	INT0_RESET();
	INT2_RESUME();
	INT1_PAUSE();
	while(1)
	{
		display_secs();
		display_mins();
		display_hours();
	}
}
