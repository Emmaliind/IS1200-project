/* mipslabfunc.c
   This file written 2015 by F Lundevall
   Some parts are original code written by Axel Isaksson

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */
#include <stdlib.h>
#include <string.h>

/* globala variabler */
int ormx = 30;                   // ormens x-koordiant
int ormy = 13;                   // ormens y-koordinat
int gamestate=0;                 // spelets status
int ran = 0;                     // random för matbit, ska vara global för ska sparas mellan de olika faserna
int dealytime_food = 50;        // ormens dealytid ska minska om den träffar en matbit
int foodx;                       // koordinater för matbit
int foody;                       // koordinater för matbit

/* Declare a helper function which is local to this file */
static void num32asc( char * s, int );

#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

/* quicksleep:
   A simple function to create a small delay.
   Very inefficient use of computing resources,
   but very handy in some special cases. */
void quicksleep(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
}



/* tick:
   Add 1 to time in memory, at location pointed to by parameter.
   Time is stored as 4 pairs of 2 NBCD-digits.
   1st pair (most significant byte) counts days.
   2nd pair counts hours.
   3rd pair counts minutes.
   4th pair (least significant byte) counts seconds.
   In most labs, only the 3rd and 4th pairs are used. */
void tick( unsigned int * timep )
{
  /* Get current value, store locally */
  register unsigned int t = * timep;
  t += 1; /* Increment local copy */

  /* If result was not a valid BCD-coded time, adjust now */

  if( (t & 0x0000000f) >= 0x0000000a ) t += 0x00000006;
  if( (t & 0x000000f0) >= 0x00000060 ) t += 0x000000a0;
  /* Seconds are now OK */

  if( (t & 0x00000f00) >= 0x00000a00 ) t += 0x00000600;
  if( (t & 0x0000f000) >= 0x00006000 ) t += 0x0000a000;
  /* Minutes are now OK */

  if( (t & 0x000f0000) >= 0x000a0000 ) t += 0x00060000;
  if( (t & 0x00ff0000) >= 0x00240000 ) t += 0x00dc0000;
  /* Hours are now OK */

  if( (t & 0x0f000000) >= 0x0a000000 ) t += 0x06000000;
  if( (t & 0xf0000000) >= 0xa0000000 ) t = 0;
  /* Days are now OK */

  * timep = t; /* Store new value */
}

/* display_debug
   A function to help debugging.

   After calling display_debug,
   the two middle lines of the display show
   an address and its current contents.

   There's one parameter: the address to read and display.

   Note: When you use this function, you should comment out any
   repeated calls to display_image; display_image overwrites
   about half of the digits shown by display_debug.
*/
void display_debug( volatile int * const addr )
{
  display_string( 1, "Addr" );
  display_string( 2, "Data" );
  num32asc( &textbuffer[1][6], (int) addr );
  num32asc( &textbuffer[2][6], *addr );
  display_update();
}

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 1));
	return SPI2BUF;
}

void display_init(void) {
        DISPLAY_CHANGE_TO_COMMAND_MODE;
	quicksleep(10);
	DISPLAY_ACTIVATE_VDD;
	quicksleep(1000000);

	spi_send_recv(0xAE);
	DISPLAY_ACTIVATE_RESET;
	quicksleep(10);
	DISPLAY_DO_NOT_RESET;
	quicksleep(10);

	spi_send_recv(0x8D);
	spi_send_recv(0x14);

	spi_send_recv(0xD9);
	spi_send_recv(0xF1);

	DISPLAY_ACTIVATE_VBAT;
	quicksleep(10000000);

	spi_send_recv(0xA1);
	spi_send_recv(0xC8);

	spi_send_recv(0xDA);
	spi_send_recv(0x20);

	spi_send_recv(0xAF);
}

void display_string(int line, char *s) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;

	for(i = 0; i < 16; i++)
		if(*s) {
			textbuffer[line][i] = *s;
			s++;
		} else
			textbuffer[line][i] = ' ';
}

void display_image(int x, const uint8_t *data) {
	int i, j;

	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22);
		spi_send_recv(i);

		spi_send_recv(x & 0xF);
		spi_send_recv(0x10 | ((x >> 4) & 0xF));

		DISPLAY_CHANGE_TO_DATA_MODE;

		for(j = 0; j < 32*4; j++)
			spi_send_recv(~data[i*32*4 + j]);
	}
}

void display_update(void) {
	int i, j, k;
	int c;
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;
		spi_send_recv(0x22);
		spi_send_recv(i);

		spi_send_recv(0x0);
		spi_send_recv(0x10);

		DISPLAY_CHANGE_TO_DATA_MODE;

		for(j = 0; j < 16; j++) {
			c = textbuffer[i][j];
			if(c & 0x80)
				continue;

			for(k = 0; k < 8; k++)
				spi_send_recv(font[c*8 + k]);
		}
	}
}

/* Helper function, local to this file.
   Converts a number to hexadecimal ASCII digits. */
static void num32asc( char * s, int n )
{
  int i;
  for( i = 28; i >= 0; i -= 4 )
    *s++ = "0123456789ABCDEF"[ (n >> i) & 15 ];
}

/*
 * nextprime
 *
 * Return the first prime number larger than the integer
 * given as a parameter. The integer must be positive.
 */
#define PRIME_FALSE   0     /* Constant to help readability. */
#define PRIME_TRUE    1     /* Constant to help readability. */
int nextprime( int inval )
{
   register int perhapsprime = 0; /* Holds a tentative prime while we check it. */
   register int testfactor; /* Holds various factors for which we test perhapsprime. */
   register int found;      /* Flag, false until we find a prime. */

   if (inval < 3 )          /* Initial sanity check of parameter. */
   {
     if(inval <= 0) return(1);  /* Return 1 for zero or negative input. */
     if(inval == 1) return(2);  /* Easy special case. */
     if(inval == 2) return(3);  /* Easy special case. */
   }
   else
   {
     /* Testing an even number for primeness is pointless, since
      * all even numbers are divisible by 2. Therefore, we make sure
      * that perhapsprime is larger than the parameter, and odd. */
     perhapsprime = ( inval + 1 ) | 1 ;
   }
   /* While prime not found, loop. */
   for( found = PRIME_FALSE; found != PRIME_TRUE; perhapsprime += 2 )
   {
     /* Check factors from 3 up to perhapsprime/2. */
     for( testfactor = 3; testfactor <= (perhapsprime >> 1) + 1; testfactor += 1 )
     {
       found = PRIME_TRUE;      /* Assume we will find a prime. */
       if( (perhapsprime % testfactor) == 0 ) /* If testfactor divides perhapsprime... */
       {
         found = PRIME_FALSE;   /* ...then, perhapsprime was non-prime. */
         goto check_next_prime; /* Break the inner loop, go test a new perhapsprime. */
       }
     }
     check_next_prime:;         /* This label is used to break the inner loop. */
     if( found == PRIME_TRUE )  /* If the loop ended normally, we found a prime. */
     {
       return( perhapsprime );  /* Return the prime we found. */
     }
   }
   return( perhapsprime );      /* When the loop ends, perhapsprime is a real prime. */
}

/*
 * itoa
 *
 * Simple conversion routine
 * Converts binary to decimal numbers
 * Returns pointer to (static) char array
 *
 * The integer argument is converted to a string
 * of digits representing the integer in decimal format.
 * The integer is considered signed, and a minus-sign
 * precedes the string of digits if the number is
 * negative.
 *
 * This routine will return a varying number of digits, from
 * one digit (for integers in the range 0 through 9) and up to
 * 10 digits and a leading minus-sign (for the largest negative
 * 32-bit integers).
 *
 * If the integer has the special value
 * 100000...0 (that's 31 zeros), the number cannot be
 * negated. We check for this, and treat this as a special case.
 * If the integer has any other value, the sign is saved separately.
 *
 * If the integer is negative, it is then converted to
 * its positive counterpart. We then use the positive
 * absolute value for conversion.
 *
 * Conversion produces the least-significant digits first,
 * which is the reverse of the order in which we wish to
 * print the digits. We therefore store all digits in a buffer,
 * in ASCII form.
 *
 * To avoid a separate step for reversing the contents of the buffer,
 * the buffer is initialized with an end-of-string marker at the
 * very end of the buffer. The digits produced by conversion are then
 * stored right-to-left in the buffer: starting with the position
 * immediately before the end-of-string marker and proceeding towards
 * the beginning of the buffer.
 *
 * For this to work, the buffer size must of course be big enough
 * to hold the decimal representation of the largest possible integer,
 * and the minus sign, and the trailing end-of-string marker.
 * The value 24 for ITOA_BUFSIZ was selected to allow conversion of
 * 64-bit quantities; however, the size of an int on your current compiler
 * may not allow this straight away.
 */
#define ITOA_BUFSIZ ( 24 )
char * itoaconv( int num )
{
  register int i, sign;
  static char itoa_buffer[ ITOA_BUFSIZ ];
  static const char maxneg[] = "-2147483648";

  itoa_buffer[ ITOA_BUFSIZ - 1 ] = 0;   /* Insert the end-of-string marker. */
  sign = num;                           /* Save sign. */
  if( num < 0 && num - 1 > 0 )          /* Check for most negative integer */
  {
    for( i = 0; i < sizeof( maxneg ); i += 1 )
    itoa_buffer[ i + 1 ] = maxneg[ i ];
    i = 0;
  }
  else
  {
    if( num < 0 ) num = -num;           /* Make number positive. */
    i = ITOA_BUFSIZ - 2;                /* Location for first ASCII digit. */
    do {
      itoa_buffer[ i ] = num % 10 + '0';/* Insert next digit. */
      num = num / 10;                   /* Remove digit from number. */
      i -= 1;                           /* Move index to next empty position. */
    } while( num > 0 );
    if( sign < 0 )
    {
      itoa_buffer[ i ] = '-';
      i -= 1;
    }
  }
  /* Since the loop always sets the index i to the next empty position,
   * we must add 1 in order to return a pointer to the first occupied position. */
  return( &itoa_buffer[ i + 1 ] );
}

/* Written by Johanna Peterson and Emma Lind */
/* Funktion som lyser upp de pixlar som skickats in som argument till metoden. För varje page på skärmen så anävnds olika x värden */
void markPixel (int x, int y){

if(x<129 && y<64){

	if(y<8){ // Page 0

		//om y = 0 så skickas ~1
			if(y==0){
				int write = ~1;
				litenOrm[x] = litenOrm[x] & write;
			}
			// Om y != 0 så skickas 2^y in och inverteras till skärmen
			else
			{
				int write = 1;
				int i;
				for (i = 0; i < y; i++)
				{
					write *= 2;
				}
				litenOrm[x] = litenOrm[x] & (~write);
			}
}
			//tidigare
			/*if(y==1){
				int write = ~2;
				litenOrm[x] = litenOrm[x] & write;
			}*/

	 if(y>= 8 && y<16){ // Page 1
			y = y - 8;

			if(y==0){
				int write = ~1;
				litenOrm[128+x] = litenOrm[128+x] & write;
			}
			// Om y != 0 så skickas 2^y in och inverteras till skärmen
			else
			{
				int write = 1;
				int i;
				for (i = 0; i < y; i++)
				{
					write *= 2;
				}
				litenOrm[128+x] = litenOrm[128+x] & (~write);
			}
		}

		if(y>= 16 && y<24){ // Page 2
 			y = y - 16;

			if(y==0){
				int write = ~1;
				litenOrm[256+x] = litenOrm[256+x] & write;
			}
			// Om y != 0 så skickas 2^y in och inverteras till skärmen
			else
			{
				int write = 1;
				int i;
				for (i = 0; i < y; i++)
				{
					write *= 2;
				}
				litenOrm[256+x] = litenOrm[256+x] & (~write);
			}
 		}

		if(y>= 24 && y<32){ // Page 2
 			y = y - 24;

			if(y==0){
				int write = ~1;
				litenOrm[384+x] = litenOrm[384+x] & write;
			}
			// Om y != 0 så skickas 2^y in och inverteras till skärmen
			else
			{
				int write = 1;
				int i;
				for (i = 0; i < y; i++)
				{
					write *= 2;
				}
				litenOrm[384+x] = litenOrm[384+x] & (~write);
			}
		}
}
}
/* Written by Johanna Peterson and Emma Lind */
/* Funktion som avmarkera upp de pixlar som skickats in som argument till metoden. För varje page på skärmen så anävnds olika x värden */
void unmarkPixel (int x, int y){

if(x<129 && y<64){

	if(y<8){ // Page 0
			if(y==0){
				int write = 1;
				litenOrm[x] = litenOrm[x] |  write;
			}
			//Om y != 0 så skickas 2^y in till skärmen
			else{
				int write = 1;
				int i;
				for (i = 0; i < y; i++)
				{
					write *= 2;
				}
				litenOrm[x] = litenOrm[x] |  write;
			}
		}

	 if(y>= 8 && y<16){ // Page 1
			y = y - 8;

			if(y==0){
				int write = 1;
				litenOrm[128+x] = litenOrm[128+x] |  write;
			}
			//Om y != 0 så skickas 2^y in till skärmen
			else{
				int write = 1;
				int i;
				for (i = 0; i < y; i++)
				{
					write *= 2;
				}
				litenOrm[128+x] = litenOrm[128+x] |  write;
			}
		}

		if(y>= 16 && y<24){ // Page 2
 			y = y - 16;

 			if(y==0){
 				int write = 1;
 				litenOrm[256+x] = litenOrm[256+x] |  write;
 			}
			//Om y != 0 så skickas 2^y in till skärmen
			else{
				int write = 1;
				int i;
				for (i = 0; i < y; i++)
				{
					write *= 2;
				}
				litenOrm[256+x] = litenOrm[256+x] |  write;
			}
 		}

		if(y>= 24 && y<32){ // Page 2
 			y = y - 24;

 			if(y==0){
 				int write = 1;
 				litenOrm[384+x] = litenOrm[384+x] |  write;
 			}
			//Om y != 0 så skickas 2^y in till skärmen
			else{
				int write = 1;
				int i;
				for (i = 0; i < y; i++)
				{
					write *= 2;
				}
				litenOrm[384+x] = litenOrm[384+x] |  write;
			}
 		}
}
}
/* Written by Johanna Peterson and Emma Lind */
/*GÅ TILL HÖGER*/
/* Funtion som gör att ormen rör sig åt höger, ormen är 3 pixlar bred och 6 pixlar lång */
void snakebtn1 (void){

 int b = 0;                                     // variabel för att hålla värden från btn 1
 int i = 0x200;                                 // variabel för att hålla värdet för den nedtryckta knappen

  while (i != 2 & i != 4 & i !=1 ){             // upprepa sålänge ingen annan knapp är nedtryckt
	  markPixel(ormx, ormy);                      // markerar ormen
	  markPixel(ormx, ormy +1);
	  markPixel(ormx, ormy +2);

	/* Kollar om ormen kör på en matbit, jämför ormens koordinater med matens koordinater */
	/* Matbiten består av 4 pixlar, jämför alla möjliga kombinationer */
	if(ormx == foodx & ormy == foody || ormx == (foodx+1) & ormy == foody ||ormx == foodx & ormy == (foody+1) || ormx == (foodx+1) & ormy == (foody+1)){
		dealytime_snake -= dealytime_food;
		food2();
	}
	else if(ormx == foodx & (ormy+1) == foody || ormx == (foodx+1) & (ormy+1) == foody || ormx == foodx & (ormy+1)==(foody+1) ||ormx == (foodx+1) & (ormy+1) == (foody+1)){
		dealytime_snake -= dealytime_food;
		food2();
	}
	else if(ormx == foodx & (ormy+2)== foody ||ormx == (foodx+1) & (ormy+2) == foody ||ormx ==foodx & (ormy+2) ==(foody+1) || ormx ==(foodx+1) & (ormy+2) == (foody+1)){
		dealytime_snake -= dealytime_food;
		food2();
	}

	unmarkPixel(ormx-6, ormy); 										// avmarkera svansen
	unmarkPixel(ormx-6, ormy +1);
	unmarkPixel(ormx-6, ormy +2);

	mark_playground();                           // markera spelplanen igen om ormen avmarkera någon kant
	delay(dealytime_snake);
	display_image(0, litenOrm);

	/* GAME OVER om vi går in någon vägg, gamestate = 1, avbryter loopen och GAMEOVER skrivs ut från funtionen labwork */
	if (ormx>125 || ormx<1 || (ormy+2)> 29 || ormy < 1){
	gamestate = 1;
	break;
	}
		b = getbtn1();                             // uppdatera om knapp 1 trycks ner
		i = getbtns();                             // uppdatera om någon av knapp 2-3 trycks ner

		if (b == 0x200){                           // kollar om knapp 1 är nedtryckt
			i = 0x200;
		}
		ormx++;                                    // ökar ormens x-koordiant eftersom vi går åt höger
	}
}

/* Written by Johanna Peterson and Emma Lind */
/* GÅ UPPÅT */
/* Funtion som gör att ormen rör sig åt uppåt, ormen är 3 pixlar bred och 6 pixlar lång */
void snakebtn2 (void){

	int i = 1;                                  // variabel för att hålla värden från btn 1
	int b = 0;                                  // variabel för att hålla värdet för den nedtryckta knappen

	while (i != 2 & i !=4 & i != 0x200) {       // upprepa sålänge ingen annan knapp är nedtryckt
		markPixel(ormx, ormy);
		markPixel(ormx +1, ormy);                 // markerar ormen
		markPixel(ormx +2, ormy);

		/* Kollar om ormen kör på en matbit, jämför ormens koordinater med matens koordinater */
		/* Matbiten består av 4 pixlar, jämför alla möjliga kombinationer */
		if(ormx == foodx & ormy == foody || ormx == (foodx+1) & ormy == foody || ormx == foodx & ormy == (foody+1) || ormx == (foodx+1) & ormy == (foody+1)){
			dealytime_snake -= dealytime_food;
			food2();
		}
		else if((ormx+1) == foodx & ormy == foody ||(ormx+1) == (foodx+1) & ormy == foody ||(ormx+1) == foodx & ormy == (foody+1) ||(ormx+1) == (foodx+1) & ormy == (foody+1)){
			dealytime_snake -= dealytime_food;
			food2();
		}
		else if((ormx+2) == foodx & ormy == foody || (ormx+2) == (foodx+1) & ormy == foody ||(ormx+2) == foodx & ormy == (foody+1) ||(ormx+2) == (foodx+1) & ormy == (foody+1)){
			dealytime_snake -= dealytime_food;
			food2();
		}

		unmarkPixel(ormx, ormy +7); 							// avmarkera svansen
		unmarkPixel(ormx +1, ormy +7);
		unmarkPixel(ormx +2, ormy +7);

		mark_playground();                        // markera spelplanen igen om ormen avmarkera någon kant
		delay(dealytime_snake);
		display_image(0, litenOrm);

		/* GAME OVER om vi går in någon vägg, gamestate = 1, avbryter loopen och GAMEOVER skrivs ut från funtionen labwork */
		if (ormy<1 || ormy>31 || ormx<2 || ormx>125 ){
		gamestate = 1;
		break;
		}
		b = getbtn1();                            // uppdatera om knapp 1 trycks ner
		i = getbtns();                            // uppdatera om någon av knapp 2-3 trycks ner

		if (b == 0x200){                          // kollar om knapp 1 är nedtryckt
			i = 0x200;
		}
		ormy--;                                   // minskar ormens y-koordiant eftersom vi går åt uppåt
	}
}
/* Function snakebtn3 is */
/* Written by Johanna Peterson and Emma Lind */
/* GÅ NEDÅT */
/* Funtion som gör att ormen rör sig åt nedåt, ormen är 3 pixlar bred och 6 pixlar lång */
void snakebtn3 (void){

	int b = 0;                                   // variabel för att hålla värden från btn 1
	int i = 2;                                   // variabel för att hålla värdet för den nedtryckta knappen

	while (i != 1 & i !=4 & i !=0x200) {         // upprepa sålänge ingen annan knapp är nedtryckt
		markPixel(ormx, ormy);                     // markerar ormen
		markPixel(ormx +1,ormy);
		markPixel(ormx +2,ormy);

		/* Kollar om ormen kör på en matbit, jämför ormens koordinater med matens koordinater */
		/* Matbiten består av 4 pixlar, jämför alla möjliga kombinationer */
		if(ormx == foodx & ormy == foody || ormx == (foodx+1) & ormy == foody || ormx == foodx & ormy == (foody+1) || ormx == (foodx+1) & ormy == (foody+1)){
			dealytime_snake -= dealytime_food;
			food2();
		}
		else if((ormx+1) == foodx & ormy == foody ||(ormx+1) == (foodx+1) & ormy == foody ||(ormx+1) == foodx & ormy == (foody+1) ||(ormx+1) == (foodx+1) & ormy == (foody+1)){
			dealytime_snake -= dealytime_food;
			food2();
		}
		else if((ormx+2) == foodx & ormy == foody || (ormx+2) == (foodx+1) & ormy == foody ||(ormx+2) == foodx & ormy == (foody+1) ||(ormx+2) == (foodx+1) & ormy == (foody+1)){
			dealytime_snake -= dealytime_food;
			food2();
		}

		unmarkPixel(ormx, ormy -6);                // avmarkera svansen
		unmarkPixel(ormx +1, ormy -6);
		unmarkPixel(ormx +2, ormy -6);

		mark_playground();                         // markera spelplanen igen om ormen avmarkera någon kant
		delay(dealytime_snake);
		display_image(0, litenOrm);

   /* GAME OVER om vi går in någon vägg, gamestate = 1, avbryter loopen och GAMEOVER skrivs ut från funtionen labwork */
		if (ormy<1 || ormy >30 || ormx<2 || ormx>125){
			gamestate = 1;
			break;
		}

		b = getbtn1();                             // uppdatera om knapp 1 trycks ner
		i = getbtns();                             // uppdatera om någon av knapp 2-3 trycks ner
		if (b == 0x200){                           // kollar om knapp 1 är nedtryckt
			i = 0x200;
		}
		ormy++;                                     // ökar ormens y-koordiant eftersom vi går åt nedåt
	}
}

/* Written by Johanna Peterson and Emma Lind */
/* GÅ TILL VÄNSTER */
/* Funtion som gör att ormen rör sig åt vänster, ormen är 3 pixlar bred och 6 pixlar lång */
void snakebtn4 (void){

	int b = 0;                                     // variabel för att hålla värden från btn 1
  int i = 4;                                     // variabel för att hålla värdet för den nedtryckta knappen

	  while (i != 1 & i !=2 & i != 0x200) {        // upprepa sålänge ingen annan knapp är nedtryckt
		markPixel(ormx, ormy);                       // markerar ormen
		markPixel(ormx, ormy +1);
		markPixel(ormx, ormy +2);

		/* Kollar om ormen kör på en matbit, jämför ormens koordinater med matens koordinater */
		/* Matbiten består av 4 pixlar, jämför alla möjliga kombinationer */
		if(ormx == foodx & ormy == foody || ormx == (foodx+1) & ormy == foody ||ormx == foodx & ormy == (foody+1) || ormx == (foodx+1) & ormy == (foody+1)){
			dealytime_snake -= dealytime_food;
			food2();
		}
		else if(ormx == foodx & (ormy+1) == foody || ormx == (foodx+1) & (ormy+1) == foody || ormx == foodx & (ormy+1)==(foody+1) ||ormx == (foodx+1) & (ormy+1) == (foody+1)){
			dealytime_snake -= dealytime_food;
			food2();
		}
		else if(ormx == foodx & (ormy+2)== foody ||ormx == (foodx+1) & (ormy+2) == foody ||ormx ==foodx & (ormy+2) ==(foody+1) || ormx ==(foodx+1) & (ormy+2) == (foody+1)){
			dealytime_snake -= dealytime_food;
			food2();
		}

		unmarkPixel(ormx +6, ormy);                    // avmarkera svansen
		unmarkPixel(ormx +6, ormy +1);
		unmarkPixel(ormx +6, ormy +2);

		mark_playground();                             // markera spelplanen igen om ormen avmarkera någon kant
		delay(dealytime_snake);
		display_image(0, litenOrm);

		/* GAME OVER om vi går in någon vägg, gamestate = 1, avbryter loopen och GAMEOVER skrivs ut från funtionen labwork */
		if (ormx>125 || ormx<1 || (ormy+2) > 29 || ormy < 1){
		gamestate = 1;
		break;
			}

			b = getbtn1();                                 // uppdatera om knapp 1 trycks ner
			i = getbtns();                                 // uppdatera om någon av knapp 2-3 trycks ner
			if (b == 0x200){                               // kollar om knapp 1 är nedtryckt
				i = 0x200;
			}
			ormx--;                                        // minskar ormens x-koordiant eftersom vi går åt vänster
		}
	}

/* Written by Johanna Peterson and Emma Lind */
/* Funktion som avmarkerar ormens svans när ormen svänger */
void clear_uppat (void){
	int r = ormy;
	int i;
	for (i = 0; i < 10; i++)
		{
				unmarkPixel(ormx, r);
				unmarkPixel (ormx +1, r);
				unmarkPixel (ormx +2, r);
				r++;
			}
	}

/* Written by Johanna Peterson and Emma Lind */
/* Funktion som avmarkerar ormens svans när ormen svänger */
void clear_nedat(void){
		int i;
		int r = ormy;
		for (i = 0; i < 10; i++)
			{
					unmarkPixel(ormx, r);
					unmarkPixel (ormx +1, r);
					unmarkPixel (ormx +2, r);
					r--;
	}
}
/* Written by Johanna Peterson and Emma Lind */
/* Funktion som avmarkerar ormens svans när ormen svänger */
void clear_hoger (void){
		int i;
		int r= ormx -1;
		for (i = 0; i < 10; i++)
			{
					unmarkPixel(r, ormy);
					unmarkPixel (r , ormy +1);
					unmarkPixel (r , ormy +2);
					r--;
				}
	}
/* Written by Johanna Peterson and Emma Lind */
/* Funktion som avmarkerar ormens svans när ormen svänger */
void clear_vanster (void){
		int i;
		int r = ormx +1;
		for (i = 0; i < 10; i++)
			{
					unmarkPixel(r, ormy);
					unmarkPixel (r , ormy+1);
					unmarkPixel (r , ormy+2);
					r++;
				}
}
/* Written by Johanna Peterson and Emma Lind */
/* Funtion som avmarkerar hela skärmen genom att 0xff på alla platser i arrayen */
void clear_display (void){
	int i;
	for (i = 0; i < 512; i++)
	litenOrm[i] = 0xff;
}

/* Written by Johanna Peterson and Emma Lind */
/* Funtion som markerar spelplanen, markerar alla kanter */
void mark_playground (void){
	int i;
	for (i = 0; i<128; i++){
		markPixel(i, 0);
		markPixel(i, 31);
	}
	int j;
	for (j = 0; j < 32; j++){
	markPixel (0, j);
	markPixel (127, j);
  }
}
/* Written by Johanna Peterson and Emma Lind */
/* Funktion som skriver ut en matbit 2*2 pixlar på olika platser till skärmen */
void food2 (void) {

	int randomy1[11] = {12,3,7,26,13,11,3,27,22,15};
	int randomx1[11] = {4,67,44,5,2,72,11,23,56,3};
	int randomy2[11] = {26,2,24,17,12,20,18,7,24,9};
	int randomx2[11] = {120,79,45,16,28,99,113,90,33,22};

	if (ran < 10){                                         // ran är en global variabel som ökas efter varje anrop av funktionen
	foodx = randomx1[ran];
	foody = randomy1[ran];

	markPixel (foodx, foody);                              // matbiten är "2*2" pixlar
	markPixel (foodx+1, foody);
	markPixel (foodx, foody+1);
	markPixel (foodx+1, foody+1);
	ran++;
}
else{                                                   // när första arrayen random1 tar slut påbörjas nästa array random2
	foodx = randomx2[ran - 10];
	foody = randomy2[ran - 10];

	markPixel (foodx, foody);
	markPixel (foodx+1, foody);
	markPixel (foodx, foody+1);
	markPixel (foodx+1, foody+1);
	ran++;
}
 if (ran > 20){                                           // när ran blir större än 20 nollställs ran
	ran = 0;
 }
}
