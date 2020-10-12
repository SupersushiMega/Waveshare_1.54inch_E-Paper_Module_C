#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

//~ #define CS_ON PORTD |= (1<<PD0)
//~ #define CS_OFF PORTD &= ~(1<<PD0)

//~ #define DS_ON PORTD |= (1<<PD1)
//~ #define DS_OFF PORTD &= ~(1<<PD1)

//~ #define RESET_ON PORTD |= (1<<PD2)
//~ #define RESET_OFF PORTD &= ~(1<<PD2)

#define BUSY !(PIND & (1<<PD3))

const unsigned char lut_vcom0[] =
{
    0x0E, 0x14, 0x01, 0x0A, 0x06, 0x04, 0x0A, 0x0A,
    0x0F, 0x03, 0x03, 0x0C, 0x06, 0x0A, 0x00
};
 
const unsigned char lut_w[] =
{
    0x0E, 0x14, 0x01, 0x0A, 0x46, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x86, 0x0A, 0x04
};
 
const unsigned char lut_b[] = 
{
    0x0E, 0x14, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x4A, 0x04
};
 
const unsigned char lut_g1[] = 
{
    0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04
};
 
const unsigned char lut_g2[] = 
{
    0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04
};
 
const unsigned char lut_vcom1[] = 
{
    0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
 
const unsigned char lut_yellow0[] = 
{
    0x83, 0x5D, 0x01, 0x81, 0x48, 0x23, 0x77, 0x77,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
 
const unsigned char lut_yellow1[] = 
{
    0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t font[][8] PROGMEM = 
{
	{//Space
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000
	},
	
	{//!
		0b00100000,
		0b00100000,
		0b00100000,
		0b00100000,
		0b00000000,
		0b00100000
	},
	
	{//"
		0b01001000,
		0b01001000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000
	},
	
	{//#
		0b00100000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000,
		0b00000000
	},
};

void Init();
void SPI_Com (uint8_t data);
void SPI_Data (uint16_t data);
void drawImage (uint8_t image[], uint8_t width, uint8_t height);
void refresh();

void SPI_Com (uint8_t data)
{/* Start transmission */
	//_delay_ms(10);
	PORTD &= ~(1<<PD0);
	PORTD &= ~(1<<PD1);
	SPDR = data;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
}

void SPI_Data (uint16_t data)
{/* Start transmission */
	PORTD &= ~(1<<PD0);
	PORTD |= (1<<PD1);
	SPDR = data;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)))
	;
}

void Wait4Idle(void)
{
	while(BUSY)
	{
	_delay_ms(100);
	}
}

void SetLutBw(void) {
    unsigned int count;     
    SPI_Com(0x20);         //g vcom
    for(count = 0; count < 15; count++) {
        SPI_Data(lut_vcom0[count]);
    } 
    SPI_Com(0x21);        //g ww --
    for(count = 0; count < 15; count++) {
        SPI_Data(lut_w[count]);
    } 
    SPI_Com(0x22);         //g bw r
    for(count = 0; count < 15; count++) {
        SPI_Data(lut_b[count]);
    } 
    SPI_Com(0x23);         //g wb w
    for(count = 0; count < 15; count++) {
        SPI_Data(lut_g1[count]);
    } 
    SPI_Com(0x24);         //g bb b
    for(count = 0; count < 15; count++) {
        SPI_Data(lut_g2[count]);
    } 
}

void SetLutY(void) {
    unsigned int count;     
    SPI_Com(0x25);
    for(count = 0; count < 15; count++) {
        SPI_Data(lut_vcom1[count]);
    } 
    SPI_Com(0x26);
    for(count = 0; count < 15; count++) {
        SPI_Data(lut_yellow0[count]);
    } 
    SPI_Com(0x27);
    for(count = 0; count < 15; count++) {
        SPI_Data(lut_yellow1[count]);
    } 
}

void Init()
{
	PORTD |= (1<<PD2);
	_delay_ms(10000);
	PORTD &= ~(1<<PD2);
	_delay_ms(10000);
	PORTD |= (1<<PD2);
	_delay_ms(10000);
	
	SPI_Com(0x01);
	SPI_Data(0x07);
	SPI_Data(0x00);
	SPI_Data(0x08);
	SPI_Data(0x00);
	
	SPI_Com(0x06);
	SPI_Data(0x07);
	SPI_Data(0x07);
	SPI_Data(0x07);
	
	SPI_Com(0x04); //Power ON
	while(BUSY)
	{
		SPI_Com(0x71);
	}
	Wait4Idle();
	
	SPI_Com(0x00);
	SPI_Data(0xCF);
	
	SPI_Com(0x50);
	SPI_Data(0x17);
	
	SPI_Com(0x30);
	SPI_Data(0x39);
	
	SPI_Com(0x61);
	SPI_Data(0xC8);
	SPI_Data(0x00);
	SPI_Data(0xC8);
	
	SPI_Com(0x82);
	SPI_Data(0x0E);
	
	SetLutBw();
	SetLutY();
}

int main(void)
{
	// Set MOSI, SCK as Output
    DDRB |= (1<<PB3) | (1<<PB5) | (1<<PB2) | (1<<PB1);
	DDRB &= ~(1<<PB4);//Input (Miso)
	PORTB |= (1<<PB1);
	PORTB &= ~(1<<PB2);
	//PORTD |= (1<<PD0);//epaper-BUSY; activate pullup
    // Enable SPI, Set as Master
    // Prescaler: Fosc/16, Enable Interrupts
    //The MOSI, SCK pins are as per ATMega8
    // Enable SPI as master, set clock rate fck/2
    //Init SPI		CLK/2
	//==================================================================
	SPCR = (1<<SPE) | (1<<MSTR);
	SPSR |= (1<<SPI2X);
	//==================================================================
 
    // Enable Global Interrupts
    sei();
	
	DDRD |= (1<<PD0) | (1<<PD1) | (1<<PD2);	//Pin definition CS DS Reset
	PORTD &= ~(1<<PD1);
	PORTD &= ~(1<<PD0);	//set CS to low
	PORTD |= (1<<PD2);	//set Reset to high
	
	DDRC &= ~(1<<PC1);	//Pin definition Input Button
	PORTC |= (1<<PC1);	//activate Pull up
	
	DDRC &= ~(1<<PD3);	//Pin definition Busy Input
	PORTD |= (1<<PD3);	//activate Pull up
	
	//uint8_t Image[154][154] = {0};
	
	//~ const uint8_t ImageWidth = 80;	//Width of image in Pixels
	//~ const uint8_t ImageHeight = 80;	//Height of image in Pixels
	
	static const uint8_t image[800] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfa, 0x5f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xbf, 0xee, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0x76, 0x5f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xb5, 0xba, 0xeb, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9e, 0xdc, 0x5e, 0x8f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0x97, 0xe8, 0xf5, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9a, 0xbc, 0xbe, 0x3b, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0x9f, 0x68, 0xd6, 0x2f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xca, 0xf4, 0x78, 0x5f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0x9f, 0xac, 0x54, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc5, 0x78, 0xf8, 0x5f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0x87, 0xaf, 0xac, 0x77, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc1, 0x7a, 0xf6, 0x3f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xe1, 0xd7, 0xae, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x7d, 0x7b, 0x87, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfc, 0x57, 0xdd, 0x41, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7a, 0xb7, 0xb0, 0x07, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xfe, 0x2f, 0xed, 0xda, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x75, 0x5b, 0x6d, 0xb0, 0x0f, 0xff, 0xff, 
0xff, 0xff, 0xff, 0x3f, 0xf6, 0xbb, 0xdf, 0x03, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x6a, 0xaf, 0x16, 0xb5, 0xc3, 0xff, 0xff, 
0xff, 0xf0, 0x10, 0xdd, 0xfb, 0x1f, 0xff, 0x70, 0xff, 0xff, 0xff, 0xe2, 0x05, 0xee, 0xad, 0x15, 0x55, 0xd8, 0xff, 0xff, 
0xff, 0xe7, 0xbf, 0x5b, 0xde, 0x1e, 0xee, 0xec, 0xff, 0xff, 0xff, 0xca, 0xea, 0xf6, 0xf5, 0x2b, 0xbb, 0x74, 0x7f, 0xff, 
0xff, 0x8f, 0x77, 0xbb, 0x5f, 0x3d, 0x6d, 0xbc, 0xff, 0xff, 0xff, 0x9b, 0xad, 0x6e, 0xea, 0xeb, 0xdf, 0xd4, 0x5f, 0xff, 
0xff, 0x96, 0xff, 0xdb, 0xbf, 0xbe, 0xf5, 0x78, 0xff, 0xff, 0xff, 0x8b, 0x54, 0xb6, 0xea, 0xeb, 0x5d, 0xa8, 0xbf, 0xff, 
0xff, 0xdd, 0xf9, 0xfd, 0xbd, 0xbd, 0xeb, 0xe1, 0xff, 0xff, 0xff, 0xf7, 0x51, 0x57, 0x6b, 0xd7, 0x5e, 0xa2, 0xff, 0xff, 
0xff, 0xff, 0xf1, 0xed, 0xde, 0xba, 0xf4, 0x07, 0xbf, 0xff, 0xff, 0xfe, 0xa1, 0x77, 0x75, 0xef, 0xa8, 0x0a, 0xff, 0xff, 
0xff, 0xff, 0xc3, 0xbb, 0xbf, 0x5a, 0x80, 0x2f, 0xff, 0xff, 0xff, 0xff, 0x02, 0xdd, 0x6a, 0xf6, 0x01, 0xf5, 0xff, 0xff, 
0xff, 0xff, 0x05, 0xeb, 0xdf, 0xbc, 0x0b, 0x5f, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x5e, 0xea, 0xd0, 0x1d, 0xff, 0xff, 0xff, 
0xff, 0xfe, 0x1a, 0xf7, 0x7d, 0xb8, 0x2b, 0xbf, 0xff, 0xff, 0xff, 0xfc, 0x3d, 0xa8, 0x07, 0x68, 0x7f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xeb, 0x41, 0x01, 0xf8, 0x5f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xbf, 0x1e, 0xa0, 0x50, 0xf7, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xf7, 0xf8, 0xf8, 0x5f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdd, 0x58, 0x54, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x6c, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x38, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfe, 0x6c, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x34, 0x5f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7e, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xd5, 0x1f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0x7b, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0xd5, 0x8f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf1, 0xbd, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xf3, 0x94, 0x8b, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xf8, 0x3f, 0xf2, 0xc9, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe2, 0x9f, 0xf3, 0x4c, 0x8b, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xc7, 0x47, 0xf1, 0xce, 0x4f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcb, 0xe3, 0xc0, 0x94, 0x8f, 0xff, 0xff, 0xff, 
0xff, 0xff, 0x8d, 0x58, 0x00, 0x4c, 0xb7, 0xff, 0xff, 0xff, 0xff, 0xff, 0x9f, 0xb4, 0x06, 0x11, 0xaf, 0xff, 0xff, 0xff, 
0xff, 0xff, 0x35, 0x7f, 0xd6, 0x06, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x2f, 0xd5, 0x3b, 0x05, 0x5b, 0xff, 0xff, 0xff, 
0xff, 0xfe, 0x7a, 0xb0, 0x5e, 0x32, 0x1f, 0x7f, 0xff, 0xff, 0xff, 0xfe, 0x2f, 0xc1, 0x6e, 0xe2, 0x00, 0x0f, 0xff, 0xff, 
0xff, 0xfe, 0x75, 0x1e, 0xde, 0x51, 0x00, 0x87, 0xff, 0xff, 0xff, 0xfe, 0x2c, 0x57, 0x7e, 0x60, 0x3f, 0xc3, 0xff, 0xff, 
0xff, 0xfe, 0x00, 0xfd, 0xfe, 0x60, 0x00, 0xaf, 0xff, 0xff, 0xff, 0xff, 0x07, 0x57, 0x5c, 0x1e, 0x00, 0x76, 0xff, 0xff, 
0xff, 0xff, 0x6a, 0xfe, 0x00, 0x9b, 0xda, 0x1b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x6b, 0x76, 0xbc, 0x1f, 0xff, 0xff, 
0xff, 0xff, 0xdb, 0xf8, 0x3d, 0xdd, 0xd2, 0x57, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0xbf, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xc2, 0xa2, 0x57, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xdf, 0xba, 0xef, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xfe, 0xb7, 0x7a, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};
	//~ SPI_Com(0x12);
	//~ Wait4Idle();
	Init();
	//_delay_ms(10000);
	//drawImage(&fonty[], 8, 9)
	uint8_t counter;
	refresh();
	
	//~ //_delay_ms(2);
	//~ SPI_Data(x);
	//~ SPI_Data(0x00);
	//~ SPI_Data(0x00);
	Wait4Idle();
	SPI_Com(0x02);
	while(1);
}//end of main

void drawImage (uint8_t *image, uint8_t width, uint8_t height)
{
	uint16_t counter = 0;
	uint16_t x = 0;
	//~ uint8_t y = 0;
	SPI_Com(0x10);
	for (counter = 0; counter < ((160*160)/8); counter++)
	{
		if(((counter*8)%160) < width && (x / (width / 8)) < height)
		{
			Wait4Idle();
			SPI_Data(image[x % ((width / 8) * height)]);
			x++;
		}
		else
		{
			Wait4Idle();
			SPI_Data(0x00);
		}
	}
	
	Wait4Idle();
	SPI_Com(0x13);
	for (counter = 0; counter < ((160*160)/8); counter++)
	{
		Wait4Idle();
		SPI_Data(0x00);
	} //end while
}

void refresh()
{
	Wait4Idle();
	SPI_Com(0x12);
}






