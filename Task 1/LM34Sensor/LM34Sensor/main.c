/*
 * LM34Sensor.c
 *
 * Created: 3/22/2018 1:24:47 PM
 * Author : Oji
 */ 
#define F_CPU 16000000UL
#define FOSC 16000000	// Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#include <avr/io.h> //standard AVR header
#include <stdint.h> // need for uint8_t
#include <util/delay.h> //delay header
#include <avr/interrupt.h>

void USARTinit (void)
{
	/* set baud rate */
	UBRR0H = (MYUBRR >> 8); //high value of baud rate
	UBRR0L = MYUBRR; //  low value of baud rate
	
	UCSR0B |= (1 << RXEN0) | (1<<TXEN0);  //enable receiver and transmitter
	UCSR0B |= (1 << RXCIE0);			 // enable receiver input
	UCSR0C = ((1<<UCSZ01)|(1<<UCSZ00)); //asynchronous
	
}


volatile uint8_t ADCvalue; // Global Variable of the returned value from ADC
void ADCinit(void) // function for Initializing ADC
{
	ADMUX |= (1 << REFS0);	//use AVcc as ref
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 128 Prescale for 16MHz
	ADCSRA |= (1 << ADATE); // Set ADC Auto Trigger Enable
	ADCSRB = 0; // Free running mode
	ADCSRA |= (1 << ADEN); // Enable the ADC
	ADCSRA |= (1 << ADIE); // Enable interrupts	
	ADCSRA |= (1 << ADSC); // start conversion
}

void USARTsend(unsigned char Data) // function for sending data to the stream
{
	while (!(UCSR0A & (1<<UDRE0))); 
	UDR0=Data;
}


ISR (ADC_vect)
{
	char repeatTemp[13]= "Temperature: "; //header for temperature
	char temperature[2] = "00";			  //temperature string array
	char temp;							  //temporary place holder
	
	/*
	convert the read ADCvalue to temperature
	500.0=>(Vref * 100)=>(5V * 100)
	divide by 1024, the max for the ADC values (0-1024)
	*/
	ADCvalue = ADC; 
	ADCvalue = (ADCvalue)*(500.0/1024.0);
	
	/* converts value into ascii */
	temp = (ADCvalue/10);			//divide by 10 for tenths place digit
	temperature[0] = temp + 48;		//add ASCII '0' 
	temp= (ADCvalue%10);			//modulo by 10 for ones place digit
	temperature[1] = temp + 48;		//add ASCII '0' 
	
	/* Displaying onto terminal */
	for (int i; i < 13; i++){
		USARTsend(repeatTemp[i]); //loop to send the header "Temperature: "
	}
	USARTsend(temperature[0]);  //sends 10s space of temperature
	USARTsend(temperature[1]);  //sends 1 space of temperature
	USARTsend('°');				//sends degree sign
	USARTsend('F');				//sends F
	USARTsend('\n');			//send line feed
	_delay_ms(1000);			//wait 1 seconds before retrieving data again
}

int main(void)
{
	USARTinit();   //initialize USART
	ADCinit();	   //initialize the ADC
	sei();         //enable interrupts
	while(1);
	return 0;
}

