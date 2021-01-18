/* mipslabwork.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):

   This file modified 2017-04-31 by Ture Teknolog

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */


int timeoutcount = 0;                                        // initierar en räknare
const int delaytime = 1000;
int dealytime_snake;

/* Interrupt Service Routine */
/* Written by Johanna Peterson and Emma Lind */
void user_isr( void ){

    /* KNAPP 2 */
    if(IFS(0) & 0x1){                                     // om flaggan är 0x1, det vill säga btn 2 är nedtryckt
      snakebtn2();
      display_image(0, litenOrm);
      clear_uppat();                                      // anropar funktionen för att av makera pixlarna när ormen svänger
      mark_playground();                                  // anropar funtionen för att markera spelplanen igen, ormen kan avmarkera spelplanen när den svänger
    }

    /* KNAPP 3 */
    if(IFS(0) & 0x2){                                     // om flaggan är 0x2, det vill säga btn 3 är nedtryckt
      snakebtn3();
      display_image(0, litenOrm);
      clear_nedat();                                      // anropar funktionen för att av makera pixlarna när ormen svänger
      mark_playground();                                  // anropar funtionen för att markera spelplanen igen, ormen kan avmarkera spelplanen när den svänger
    }

    /* KNAPP 4 */
   if(IFS(0) & 0x4){                                      // om flaggan är 0x4, det vill säga btn 3 är nedtryckt
      snakebtn4();
      display_image(0, litenOrm);
      clear_vanster();                                    // anropar funktionen för att av makera pixlarna när ormen svänger
      mark_playground();                                  // anropar funtionen för att markera spelplanen igen, ormen kan avmarkera spelplanen när den svänger
    }

    /* KNAPP 1 */
 if(IFS(0) & 0x200){                                      // om flaggan är 0x200, det vill säga btn 1 är nedtryckt
      snakebtn1();
      display_image(0, litenOrm);
      clear_hoger();                                      // anropar funktionen för att av makera pixlarna när ormen svänger
      mark_playground();                                  // anropar funtionen för att markera spelplanen igen, ormen kan avmarkera spelplanen när den svänger
    }
    IFS(0) = 0;                                           // nollställ flaggan
}
/* Written by Johanna Peterson and Emma Lind */
/* Lab-specific initialization goes here */
void labinit( void ){

    volatile int * d = (volatile int *) 0xbf886100;          // initerar portd att peka till minnesadressen
    *d = *d & 0xff00;                                        // initierar portd till output genom att nolla de 16 lsb
    TRISD |= (126 << 5);                                     // initierar port D som en input, eller 0x0fe0

    T2CON = 0x70;                                            // initierar timer
    PR2 = ((80000000/256)/100);                              // prescale ändrar 10 hundra för fler timeoutevent per s
    TMR2 = 0;                                                // reset the timer
    T2CONSET = 0x8000;                                       // startar timer (set bit 15 to 1)

    IPC(2) = 4;                                              // 100, prioritet
    IEC(0) = 0x100;                                          // bit nr 8 till 100 innan

    enable_interrupt();

    return;
}

/* This function is called repetitively from the main program */
/* Written by Johanna Peterson and Emma Lind */
void labwork( void ){

    int button;                                         // varibel för att hålla värden från knapp 2-3
    int button1;                                        // variabel för att hålla värden från knapp 1
    int sw;                                             // variabel för att hålla värden från switchar
    sw = getsw();                                       // anropar för att veta vilka switchar som är på

/*EASY ******************************************************/
/* Spelet börjar på level EASY */
if (sw == 0x1){                                         // SW1

    display_string(0, "EASY");
    display_string(1, "");
    display_string(2, "");
    display_string(3, "");
    display_update();
    delay( delaytime );

    display_string(0, "       3");
    display_string(1, "");
    display_string(2, "");
    display_string(3, "");
    display_update();
    delay( delaytime );

    display_string(0, "");
    display_string(1, "       2");
    display_string(2, "");
    display_string(3, "");
    display_update();
    delay( delaytime );

    display_string(0, "");
    display_string(1, "");
    display_string(2, "       1");
    display_string(3, "");
    display_update();
    delay( delaytime );

    display_string(0, "");
    display_string(1, "");
    display_string(2, "");
    display_string(3, " PRESS TO PLAY");
    display_update();
    delay( delaytime );

    display_string(0, "");
    display_string(1, "");
    display_string(2, "");
    display_string(3, "");
    display_update();

    clear_display();                               // avmarkerar alla pixalar på skärmen
    mark_playground();                             // markerar spelplanen
    food2();                                       // markerar en matbit på planen
    dealytime_snake = 500;                         // EASY, sätter hastigehten för ormen, startar på 500 för easy
    gamestate = 0;                                 // nollställer gamestate
    ormx = 30;                                     // ormens start x-koordiant
    ormy = 13;                                     // ormens start y-koordiant

    /* Oändlig loop som uppdaterar input från knapparna */
    while (1){
      /* Kollar om ormen gått in i spelplanen i mipslabfunc,, gamestate == 1, GAMEOVER */
      if (gamestate == 1){
        clear_display();                           // avmarkerar alla pixlar på skärmen
        display_image(0, litenOrm);
        display_string(0,"GAME OVER");
				display_string(1, "");
				display_string(2, "");
				display_string(3, "");
				display_update();
				delay(delaytime);
        dealytime_snake = 500;                      // EASY, sätter hastigehten för ormen, startar på 500 för easy
        main();                                     // anropar main för att komma till startmeny
      }
      button1 = getbtn1();                          // hämtar värden från btn 1 i time4io.c
      button = getbtns();                           // hämtar värden från btn 2 i time4io.c
      if (button1 == 0x200){                        // kollar om btn 1 är nedtryckt
        button = 0x200;
      }
      IFS(0) = button;                              // sätter flaggan till motsv knapp som är nedtryckt
    }
}
/*DIFFICULT***********************************************/
/* Spelet börjar på level DIFFICULT */
    if (sw == 0x2){                                  // SW2
      display_string(0, "DIFFICULT");
      display_string(1, "");
      display_string(2, "");
      display_string(3, "");
      display_update();
      delay( delaytime );

      display_string(0, "       3");
      display_string(1, "");
      display_string(2, "");
      display_string(3, "");
      display_update();
      delay( delaytime );

     display_string(0, "");
     display_string(1, "       2");
     display_string(2, "");
     display_string(3, "");
     display_update();
     delay( delaytime );

     display_string(0, "");
     display_string(1, "");
     display_string(2, "       1");
     display_string(3, "");
     display_update();
     delay( delaytime );

     display_string(0, "");
     display_string(1, "");
     display_string(2, "");
     display_string(3, " PRESS TO PLAY");
     display_update();
     delay( delaytime );

     display_string(0, "");
     display_string(1, "");
     display_string(2, "");
     display_string(3, "");
     display_update();

     clear_display();                               // avmarkerar alla pixalar på skärmen
     mark_playground();                             // markerar spelplanen
     food2();                                       // markerar en matbit på planen
     dealytime_snake = 200;                         // DIFFICULT, sätter start hastigheten för ormen
     gamestate = 0;                                 // nollställer gamestate
     ormx = 30;                                     // ormens start x-koordiant
     ormy = 13;                                     // ormens start y-koordiant

    /* Oändlig loop som uppdaterar input från knapparna */
     while (1) {
       /* Kollar om ormen gått in i spelplanen i mipslabfunc, gamestate == 1, GAMEOVER */
       if (gamestate == 1){
         clear_display();                           // avmarkerar alla pixlar på skärmen
         display_image(0, litenOrm);
         display_string(0, "GAME OVER");
 				 display_string(1, "");
 				 display_string(2, "");
 				 display_string(3, "");
 				 display_update();
 				 delay(delaytime);
         dealytime_snake = 200;                         // DIFFICULT, sätter start hastigheten för ormen
         main();                                     // anropar main för att komma till startmeny
       }
       button1 = getbtn1();                          // hämtar värden från btn 1 i time4io.c
       button = getbtns();                           // hämtar värden från btn 2 i time4io.c
       if (button1 == 0x200){                        // kollar om btn 1 är nedtryckt
         button = 0x200;
       }
       IFS(0) = button;                              // sätter flaggan till mots knapp som är nedtryckt
  }
}
}
