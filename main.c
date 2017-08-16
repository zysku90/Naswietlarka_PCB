/*
 * main.c
 *
 *  Created on: 24 sty 2016
 *      Author: Michal
 */
/*
 * main.c
 *
 *  Created on: 24 sty 2016
 *      Author: Michal
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

// definicje makr
#define DISP_1 PC0
#define DISP_2 PC1
#define DISP_3 PC2
#define DISP_4 PC3
#define DISP_5 PC4
#define DISP_PORT PORTC
#define DISP_DIR DDRC
#define SEG_PORT PORTD
#define SEG_DIR DDRD
#define UV PB1
#define UV_PORT PORTB
#define UV_DIR DDRB
#define LED_RED PB2
#define LED_RED_PORT PORTB
#define LED_RED_DIR DDRB
#define SW_PORT PORTB
#define SW_DIR DDRB
#define SW1 PB0
#define SW2 PB3
#define SW3 PB4
#define SW4 PB5
#define RED_ON 	LED_RED_PORT |= (1<<LED_RED)
#define RED_OFF LED_RED_PORT &= ~(1<<LED_RED)
#define UV_OFF 	UV_PORT &= ~(1<<UV)
#define UV_ON 	UV_PORT |= (1<<UV)
#define CLS_SW1 !(PINB & (1<<SW1))
#define CLS_SW2 !(PINB & (1<<SW2))
#define CLS_SW3 !(PINB & (1<<SW3))
#define CLS_SW4 !(PINB & (1<<SW4))

// deklaracja funkcji
void timer1_init (void);
void konwersja (void);
void curring (void);
void switches (void);

// Zmienne globalne
static EEMEM uint8_t ee_minuty, ee_sekundy;
uint8_t old_minuty, old_sekundy, start = 0, minuty, sekundy,
		key_pressed1 = 0, key_pressed2 = 0, key_pressed3 = 0, plus = 0, minus = 0,
		key_pressed4 = 0, next = 0;
volatile uint8_t numbers[10] = {0b01000000, // number 0
					 0b01111001, // number 1
					 0b00100100, // number 2
					 0b00110000, // number 3
					 0b00011001, // number 4
					 0b00010010, // number 5
					 0b00000010, // number 6
					 0b01111000, // number 7
					 0b00000000, // number 8
					 0b00010000, // number 9
};
volatile uint8_t flag_1s = 0, flag_4s = 0, cy1, cy2, cy3, cy4;
int main (void){
//************Ustawienia portow************
DISP_DIR = 255;
DISP_PORT = 255;
SEG_DIR = 255;
SEG_PORT = 255;
UV_DIR |= (1<<UV);
UV_PORT |= (1<<UV);
LED_RED_DIR |= (1<<LED_RED);
LED_RED_PORT |= (1<<LED_RED);
SW_PORT |= (1<<SW1) | (1<<SW2) | (1<<SW3) | (1<<SW4);
timer1_init();  //Inicjalizacja Timera1
sei();
old_minuty = eeprom_read_byte(&ee_minuty);
old_sekundy = eeprom_read_byte(&ee_sekundy);
minuty = old_minuty;
sekundy = old_sekundy;
next = 0;
while(1){
	konwersja();
	RED_ON;
	UV_OFF;
	if (next == 1 || next == 2){
		if (plus == 1){
			if(next == 2) minuty ++;
			else sekundy ++;
			if(sekundy > 59) sekundy = 59;
			if(minuty > 30) minuty = 30;
			plus = 0;
		}
		if (minus == 1){
			if(next == 2) minuty --;
			else sekundy --;
			if(sekundy > 100) sekundy = 0;
			if(minuty > 100) minuty = 0;
			minus = 0;
		}
	}
	if (start == 1 && next == 0){
		start = 0;
		curring();
	}
	switches();
}
}
void curring (void){
	flag_4s = 0;
	RED_OFF;
	while (flag_4s == 0){
		if ((minuty != old_minuty) || (sekundy != old_sekundy)){
			eeprom_write_byte (&ee_minuty, minuty);
			eeprom_write_byte (&ee_sekundy, sekundy);
			old_minuty = minuty;
			old_sekundy = sekundy;
		}
	}
	flag_1s = 0;
	while(1){
		konwersja();
		UV_ON;
		if (CLS_SW1 && !key_pressed1){
				key_pressed1 = 1;
				start = 1;
		}
		else if (key_pressed1 && !CLS_SW1) key_pressed1++;

		if (flag_1s == 1){
			if((minuty == 0 && sekundy == 0) || start == 1){
				minuty = old_minuty;
				sekundy = old_sekundy;
				start = 0;
				break;
			}
			if (sekundy == 0){
				sekundy = 60;
				minuty --;
			}
			sekundy --;
			flag_1s = 0;
		}
	}
}


void konwersja (void){
	cy1 = minuty/10;
	cy2 = minuty%10;
	cy3 = sekundy/10;
	cy4 = sekundy%10;
}

void switches (void){
	if (CLS_SW1 && !key_pressed1){
		key_pressed1 = 1;
		start++;
		if (start>1) start=0;
	}
	else if (key_pressed1 && !CLS_SW1) key_pressed1++;

	if (CLS_SW2 && !key_pressed2){
		key_pressed2 = 1;
		plus=1;
	}
	else if (key_pressed2 && !CLS_SW2){
		key_pressed2++;
		plus=0;
	}

	if (CLS_SW3 && !key_pressed3){
		key_pressed3 = 1;
		next++;
		if(next>2) next=0;
	}
	else if (key_pressed3 && !CLS_SW3) key_pressed3++;

	if (CLS_SW4 && !key_pressed4){
		key_pressed4 = 1;
		minus=1;
	}
	else if (key_pressed4 && !CLS_SW4){
		key_pressed4++;
		minus=0;
	}
}

void timer1_init (void){
	TCCR1B |= (1<<WGM12) | (1<<CS10)| (1<<CS11); //CTC, Prescaler 64
	TIMSK1 |= (1<<OCIE1A);
	OCR1A = 250; // 500Hz
}

ISR(TIMER1_COMPA_vect){
	static uint8_t mpx = 1, cnt = 0;
	static uint16_t sek = 0;
	switch (mpx){
		case 1:
			DISP_PORT = 0b11111110;
			if (cy1 == 0) SEG_PORT = 255;
			else SEG_PORT = numbers[cy1];
			if (next==1) SEG_PORT = 255;
			break;
		case 2:
			DISP_PORT = 0b11111101;
			SEG_PORT = numbers[cy2];
			if (next==1) SEG_PORT = 255;
			break;
		case 3:
			DISP_PORT = 0b11111111;
			SEG_PORT = 0;
			break;
		case 4:
			DISP_PORT = 0b11111011;
			SEG_PORT = numbers[cy3];
			if (next==2) SEG_PORT = 255;
			break;
		case 5:
			DISP_PORT = 0b11110111;
			SEG_PORT = numbers[cy4];
			if (next==2) SEG_PORT = 255;
			break;
	}
	mpx ++;
	if (mpx > 5) mpx = 1;
	sek ++;
	if (sek == 500){
		cnt++;
		flag_1s = 1;
		sek = 0;
	}
	if (cnt == 6){
		flag_4s = 1;
		cnt = 0;
	}
}








