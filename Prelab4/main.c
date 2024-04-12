/*
 * Prelab4.c
 *
 * Created: 04/04/2024 04:44:18 p. m.
 * Author : josei
 */ 

//DEFINICIONES
#define F_CPU 16000000

//LIBRERIAS

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>

//VARIABLES
uint8_t contador = 0;
uint8_t Receptor = 0;
const uint8_t tabla[]={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

//PROTOTIPOS
void setup(void);
void initADC(void);
void initTimer1(void);

//MAIN
int main(void)
{
    setup(); //MANDAMOS A LLAMAR A SETUP
    
	//LOOP PRINCIPAL
	while (1) 
    {
		
		PORTB |= (1<<PORTB3);		//ENCENDER TRANSISTOR 1
		PORTD = tabla[(Receptor & 0x0F)];
		_delay_ms(4);
		
		PORTB &= ~(1<<PORTB3);		//APAGAR TRANSISTOR 1
		PORTB |= (1<<PORTB4);		//ENCENDER TRANSISTOR 2
		PORTD = tabla[Receptor>>4];
		_delay_ms(4);
		
		PORTB &= ~(1<<PORTB4);		//APAGAR TRANSISTOR 1
		PORTB |= (1<<PORTB2);		//ENCENDER TRANSISTOR 2
		PORTD = contador;
		_delay_ms(4);
		PORTB &= ~(1<<PORTB2);	
		
    }
}

void setup(void){
	cli();
	DDRD = 0xFF; //PONER EL PORTD COMO SALIDAS
	UCSR0B = 0;	//ACTIVAR RX Y TX COMO PINES NORMALES
	PORTD &= ~(1 << PORTD0); //LIMPIAR PORTD0
	
	DDRC  &= ~((1 << DDC1)| (1 << DDC0));	//COMO ENTRADAS 
	PORTC |= (1 << PORTC1 | 1<< PORTC0);	//ACTIVAR PULLUP
	
	DDRB |= (1<<DDB1)|(1<<DDB2)|(1<<DDB3)|(1<<DDB4);	//SALIDAS
	
	PCMSK1 |= (1 << PCINT9)|(1 << PCINT8); //HABILITANDO PCINT EN LOS PINES PCINT9 y PCINT8
	//IMPORTANTE: ESTAMOS EN LA DIRECCION DEL ISR_PCINT1 POR TENER LOS BOTONES EN EL PUERTO C

	PCICR |= (1 << PCIE1);	//HABILITANDO LA ISR PCINT[14:8]
	initADC();
	initTimer1();
	sei();					//HABILITAR INTERRUPCIONES
	
}


void initADC(void){
	ADMUX = 0;
	ADMUX |= (1<<REFS0);				//REFERENCIA AVCC = 5V
	ADMUX |= (1<<ADLAR);				//JUSTIFICACIÓN A LA IZQUIERDA
	ADMUX |= ((1<<MUX2) | (1<<MUX1));	//AL COLOCAR 0110 INDICAMOS EL ADC6 COMO EL RECEPTOR DE LA SEÑAL ANALÓGICA
	
	ADCSRA = 0;
	ADCSRA |= (1<<ADIE);				//HABILITaR INTERRUPCIONES DE ADC
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);	//PRESCALER DE 128 ----> 125kHz
	ADCSRA |= (1<<ADEN);				//HABILITANDO EL ADC
	
}

void initTimer1(void){
	TCCR1B |= (1 << CS12)|(1 << CS10);		//CONFIGURAR PRESCALER DE 1024
	TCNT1 = 62411;						//CARGAR VALOR DE DESBORDAMIENTO
	TIMSK1 |= (1 << TOIE1);
}

ISR (TIMER1_OVF_vect){
	TCNT1 = 62411;						//CARGAR VALOR DE DESBORDAMIENTO
	TIFR1 |= (1 << TOV1);				//APAGAR BANDERA DEL TIMER1
	ADCSRA |= (1<<ADSC);				//ADC START CONVERSION
}

ISR (PCINT1_vect){
	
	if (PINC== 0b00000001)				//SI PC0 ESTA EN 1
	{
		contador++;						//AUMENTAR CONTADOR
	}
	else if (PINC== 0b00000010)			//SI PC0 ESTA EN 1
	{
		contador--;						//DECREMENTAR CONTADOR
	}
	else
	{
		contador=contador;				//CONTADOR SIN CAMBIOS
	}
}


ISR (ADC_vect){
	
	Receptor = ADCH;
	if (Receptor>contador)
	{
		PORTB |= (1<<PORTB1);
	}
	else
	{
		PORTB &= ~(1<<PORTB1);
	}
	ADCSRA |= (1<<ADIF);
	
	
}