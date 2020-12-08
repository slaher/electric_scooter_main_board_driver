#include <avr/io.h>
#include <util/delay.h>               
#include <avr/interrupt.h> 
#include <string.h>    
#include <stdio.h>   
volatile char REC;
char data[30];
int k = 0;
char enable[] = "enable";
char disable[] = "disable";
char mystates[] = "get_states";
char speed_limit_on[] = "speed_limit_on";
char speed_limit_off[] = "speed_limit_off";
int state = 0 ;  //0 - disabled; 1 - enabled
int speedLimitState = 0 ;  //0 - disabled; 1 - enabled

void uart_init(void)
{
	int baud = 1200;
	int ubrr = (F_CPU/(16UL*baud))-1;
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	/* Enable receiver and transmitter */
	UCSRB = (1<<RXEN)|(1<<TXEN);
	/* Set frame format: 8data, 1stop bit */
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);      
	UCSRA=0; 
	UCSRC = (1<<URSEL)|(1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0); // 8 bitów, 2 bit stopu
}


void put_c_uart(char c)
{
	UDR=c;
	loop_until_bit_is_set(UCSRA,TXC);
	UCSRA|=_BV(TXC);
}


void print(char *_char)
{	
	for (int z = 0; z < strlen(_char); z++)
	{ 
		put_c_uart(_char[z]);
	}
}

void send_scooter_key_status(void)
{
	if (state == 1)
	{
		print("my_state_is_enabled\n");
	}
	if (state == 0)
	{
		print("my_state_is_disabled\n");
	}
}

void send_speed_limit_status(void)
{
	if (speedLimitState == 1)
	{
		print("speed_limit_is_enabled\n");
	}
	if (speedLimitState == 0)
	{
		print("speed_limit_is_disabled\n");
	}
}

void send_states()
{
	send_scooter_key_status();
	send_speed_limit_status();
}

int main(void)
{

	DDRD = 0x02;
	PORTD= 0xFF;
	DDRC = 0xFF;
	PORTC = 0xFF;

	uart_init();
	sei();
	print("Hello!\n");  // only for debug
	while(1)
	{}
}

	
void get_message_and_action(void)
{	
	while(!(UCSRA & (1<<RXC)));
	{
		REC = UDR;
	
		if(REC == '\r' || REC == '\n')
		{	
			if(strcmp(data, enable) == 0)
			{
				PORTC &= ~(1<<PC3);
				state = 1;
				send_scooter_key_status();
			}
			
			if(strcmp(data, disable) == 0)
			{
				PORTC |= (1<<PC3);
				state = 0;
				send_scooter_key_status();
			}
			
			if(strcmp(data, speed_limit_on) == 0)
			{
				PORTC &= ~(1<<PC5);
				speedLimitState = 1;
				send_speed_limit_status();
			}
			
			if(strcmp(data, speed_limit_off) == 0)
			{
				PORTC |= (1<<PC5);
				speedLimitState = 0;
				send_speed_limit_status();
			}
			
			if(strcmp(data, mystates) == 0)
			{
				send_states();
			}
			
			k = 0;
			memset(data, '\0' , strlen(data));
		}
	
		else
		{
			data[k] = REC;
			k++;
		}
	}
}

ISR(USART_RXC_vect) 
{	
	get_message_and_action();
}