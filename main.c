/*  UART Transmitter (8 data bits, 1 stop bit, no parity)
   
 */

// change these to use another pin
#define TX_PORT PORTB
#define TX_PIN  PB0
#define TX_DDR  DDRB
#define TX_DDR_PIN DDB0 
/*
		BAUD = F_CPU / ( TIMER0_PRESCALER * (OCR0 + 1) 
		so we can modify
		 + the prescaler
		 + the OCR0A value to achieve a certain baud rate.
		 

*/
#define F_CPU 8000000UL 
#define BAURATE 9600
#define TIMER0_PRESCALER 8
#define OCR0_VALUE ((F_CPU/BAURATE/TIMER0_PRESCALER)) - 1
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//a variable that holds our data to be sent.
volatile uint16_t tx_shift_reg = 0;

void UART_tx(char character)
{	
   uint16_t local_tx_shift_reg = tx_shift_reg;
   //if sending the previous character is not yet finished, return
   //transmission is finished when tx_shift_reg == 0
   if(local_tx_shift_reg){return;}
   //fill the TX shift register witch the character to be sent and the start & stop bits (start bit (1<<0) is already 0)
   local_tx_shift_reg = (character<<1) | (1<<9); //stop bit (1<<9)
   tx_shift_reg = local_tx_shift_reg;
   TCCR0 |= (1<<CS01);  //start timer0 with a prescaler of 8
}

void UART_tx_str(char* string){
    while( *string ){
        UART_tx( *string++ );
        //wait until transmission is finished
        while(tx_shift_reg);
    }
}

void UART_init(){
   //set TX pin as output
   TX_DDR |= (1<<TX_DDR_PIN);
   TX_PORT |= (1<<TX_PIN);
   TCCR0 |= (1<<WGM01);         //turn on CTC mode
   OCR0 = OCR0_VALUE;     //set output compare register 
   TIMSK|=(1<<OCIE0);    //enable timer compare interrupt
      //set compare value to 103 to achieve a 9600 baud rate (i.e. 104µs)
   //together with the 8MHz/8=1MHz timer0 clock
   /*NOTE: since the internal 8MHz oscillator is not very accurate, this value can be tuned
     to achieve the desired baud rate, so if it doesn't work with the nominal value (103), try
     increasing or decreasing the value by 1 or 2 */
   sei();       //enable interrupts
}

ISR(TIMER0_COMP_vect)
{
   uint16_t local_tx_shift_reg = tx_shift_reg;
   //output LSB of the TX shift register at the TX pin
   if( local_tx_shift_reg & 0x01)
   {
      TX_PORT |= (1<<TX_PIN);
   }
   else
   {
      TX_PORT &= ~(1<<TX_PIN);
   }
   //shift the TX shift register one bit to the right
   local_tx_shift_reg >>= 1;
   tx_shift_reg = local_tx_shift_reg;
    //if the stop bit has been sent, the shift register will be 0
    //and the transmission is completed, so we can stop & reset timer0 (clear prescale)
    if(!local_tx_shift_reg)
    {
	    TCCR0 &= ~ (1<<CS01);
	    TCNT0 = 0;
    }
}

int main(void)
{
	UART_init();
	

	
	while(1)
	{
		_delay_ms(1000);
			UART_tx_str("\tHello World 2022.\n");
			UART_tx_str("\tThere are many variations of passages of Lorem Ipsum available, but the majority have suffered alteration in some form, by injected humour, or randomised words which don't look even slightly believable. If you are going to use a passage of Lorem Ipsum, you need to be sure there isn't anything embarrassing hidden in the middle of text. All the Lorem Ipsum generators on the Internet tend to repeat predefined chunks as necessary, making this the first true generator on the Internet. It uses a dictionary of over 200 Latin words, combined with a handful of model sentence structures, to generate Lorem Ipsum which looks reasonable. The generated Lorem Ipsum is therefore always free from repetition, injected humour, or non-characteristic words etc.\n");
		
	}
}
