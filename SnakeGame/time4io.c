//
//  time4io.c
//
//
//  Created by Emma Lind on 2018-02-06.
//

#include "time4io.h"
#include <stdint.h>
#include <pic32mx.h>
#include "mipslab.h"
/* Written by Johanna Peterson and Emma Lind */
int getsw (void){
    TRISD |= (0xf<<8);                             // initierar PORTD till input
    return (PORTD >> 8) & 0xf;                     // returnerar input från switchar som LSB

}
/* Written by Johanna Peterson and Emma Lind */
int getbtns (void){
    TRISD |= (0x7<<5);                             // initierar PORTD till input
    return (PORTD >> 5) & 0x7;                     // returnerar input från btn 2-3 som LSB

}
/* Written by Johanna Peterson and Emma Lind */
int getbtn1(void){
  TRISF = (0x1<<1);                                // initierar PORTF som input
  return ((PORTF>>1) & 0x1)<<9;                    // returnerar input som bit nr 9 
}
