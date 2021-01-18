  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #
.global hexac
.global delay
.global time2string
.global enable_interrupt

.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  #
  # Please add your own code below this line
  #
.data
.align 2
.text

  hexasc:

    	andi $t0,$a0,0xf		# tar de fyra sista bitarna, and 15 = 0xf = 1111, 4 lsb

 	ble $t0,9,number                # mindre eller lika med 9, hoppa till number
 	nop
 	ble $t0,15,letters              # mindre eller lika med 15, hoppa till letters
 	nop

  	number:
  		addi $v0,$t0,0x30	# register $v0 har seven least significant bits, kmr aldrig överskrida de 7 sista, an ASCII code
  		jr $ra			# 0x30 konverterar nummer till ASCII
  		nop

 	letters:
 		addi $v0,$t0,0x37	# register $v0 har seven least significant bits, kmr aldrig överskrida de 7 sista, 0x37 konverterar
 		jr $ra			# nummer till bokstäver ASCII, så allt efter 10,
 		nop
delay:
PUSH $ra

li $t2, 4711   # t2 = should be 4711
#li $a0, 200
#addi $a0, $a0, $0

  while:
   li $t0, 0                   # t0 = i = 0 #a0 = ms
   ble $a0, 0, end             # när a0 är mindre än eller lika med gå till end
   nop

   addi $a0, $a0, -1             # ms = ms - 1

     for:
       beq $t0, $t2, while         # t0=t2 få till while
       nop
       addi $t0, $t0, 1            # öka t0 med 1
       j for                       # hoppa till for
       nop
end:
POP $ra
jr $ra
nop



 time2string:


PUSH $ra                 # we have to save the location
PUSH $s1
PUSH $s2                # saving what is on s1 before using it
move $s2, $a1
move $s1, $a0              # copy what is on a0 to s1, dvs adressen för timstr

#FIRST DIGIT
andi $t0, $s2, 0xf000      # Now we begin with the first digit, everything else in t0 is 0
srl $a0, $t0, 12           # shift right 12 binaries, so we on a0 now only have the four binaries representing the first digit
jal hexasc                 # jump and link to hexasc which uses a0
nop
move $a0, $v0              # copy v0 to a0, we get v0 from hexac
sb $a0, 0($s1)             # store byte a0, in memory location for s1

#SECOND DIGIT
andi $t0, $s2, 0xf00       # now we begin with the second digit, everything else in t0 is 0
srl $a0, $t0, 8            # shift right 8 binaries, so we on a0 now only have the four binaries representing the first digit
jal hexasc                 #jump and link to hexasc which uses a0
nop
move $a0, $v0              # copy v0 to a0, we get v0 from hexac
sb $a0, 1($s1)             # store byte a0, in memory location for 1+s1

#COLON
li $a0, 0x3a               # we put 0x3a = : in ASCII in t0
sb $a0, 2($s1)             # we store byte a0, in memory loaction for 2+s2

#THIRD DIGIT
andi $t0, $s2, 0xf0        # now we begin with the third digit, everything else in t0 is 0
srl $a0, $t0, 4            # shift right 4 binaries, so we on a0 now only have the four binaries representing the first digit
jal hexasc                 # jump and link to hexasc which uses a0
nop
move $a0, $v0              # copy v0 to a0, we get v0 from hexac
sb $a0, 3($s1)             # store byte a0, in memory location for 3+s1


#FOURTH DIGIT
andi $t0, $s2, 0xf
move $a0, $t0             #Now we begin with the first digit, everything else in t0 is 0
beq $a0, 9, nine
nop                       #We do not need to shift the fourth digit
jal hexasc                #jump and link to hexasc which uses a0 to represent it in ascii
nop
move $a0, $v0
sb $a0, 4($s1)
#null
li $a0, 0x00        #we load t0 with the ascii representation of null
sb $a0, 5($s1)

j stopp
nop

nine:

li $t1, 0x4e
li $a1, 0x49
li $a2, 0x45

sb $t1, 4($s1)
sb $a1, 5($s1)
sb $t1, 6($s1)
sb $a2, 7($s1)
#We take what we got from hexasc and move it to a2
#we storebyte a2 to the fifth location in s1

#NULL
li $a0, 0x00        #we load t0 with the ascii representation of null
sb $a0, 8($s1)        #we storebyte t0 to the sixth location in s1
stopp:
POP $s2
POP $s1         #pop s1 back
POP $ra         #pop ra back
jr $ra             #jump to current location
nop

enable_interrupt:
ei
jr $ra
nop
