  # analyze.asm
  # This file written 2015 by F Lundevall
  # Copyright abandoned - this file is in the public domain.

	.text
main:
	li	$s0,0x30        # hämtar värdet som finns på 0x30 och lägger i $s0, första är a
loop:
	move	$a0,$s0		# copy from s0 to a0, för att det går bara att skriva ut argument 
	addi    $a0, $a0,0      # tillagd BEHÖVS DENNA?
	
	li	$v0,11		# syscall with v0 = 11 will print out
	syscall			# one byte from a0 to the Run I/O window

	addi	$s0,$s0,3	# what happens if the constant is changed?/vi ökar med tre efter varje utskrift 
	
	li	$t0,0x5d        # 0x5d för att vi ökar sista varvet med 3, dvs när vi har skrivit ut z 
	                        # ökar vi sedan med 3 och måste då jämföra med 3 längre bort 
	bne	$s0,$t0,loop    # om dessa är olika hoppar vi till loop och fortsätter 
	nop			# delay slot filler (just in case)

stop:	j	stop		# loop forever here behövs denna??
	nop			# delay slot filler (just in case)

