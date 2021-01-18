  # hexmain.asm
  # Written 2015-09-04 by F Lundevall
  # Copyright abandonded - this file is in the public domain.

	.text
main:
	li	$a0,17 	        # change this to test different values, skriver in det värde jag vill omvandla till ASCII
	                        # om vi väljer 17 = 10001, blir det endast 0001 = 1 efter som den endast tar 4 lsb

	jal	hexasc		# call hexasc
	nop			# delay slot filler (just in case)	

	move	$a0,$v0		# copy return value to argument register, för att kunna skriva ut 

	li	$v0,11		# syscall with v0 = 11 will print out
	syscall			# one byte from a0 to the Run I/O window
	
stop:	j	stop		# stop after one run
	nop			# delay slot filler (just in case)

  # You can write your own code for hexasc here
  #
  
hexasc:

    	andi $t0,$a0,0xf		# tar de fyra sista bitarna, and 15 = 0xf = 1111, 4 lsb
  
 	ble $t0,9,number                # mindre eller lika med 9, hoppa till number
 	nop		
 	ble $t0,15,letters              # mindre eller lika med 15, hoppa till letters
 	nop
  
  	number:
  		addi $v0,$t0,0x30	# register $v0 har seven least significant bits, kmr aldrig överskrida de 7 sista, an ASCII code 
  		jr $ra	
  		nop		        # 0x30 konverterar nummer till ASCII
  		                        
  	
 	letters:
 		addi $v0,$t0,0x37	# register $v0 har seven least significant bits, kmr aldrig överskrida de 7 sista, 0x37 konverterar
 		jr $ra
 		nop			# nummer till bokstäver ASCII, så allt efter 10, 
                                        # max att returnera äe 0x5a = 0101 1010 

