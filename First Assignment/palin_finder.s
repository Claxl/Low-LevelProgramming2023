.global _start


// Please keep the _start method and the input strings name ("input") as
// specified below
// For the rest, you are free to add and remove functions as you like,
// just make sure your code is clear, concise and well documented.

_start:
	ldr r0, =input //takes the input
	mov r1, #0 //i
	mov r2, #0 // j
	ldr r5,  =0xFF200000 //load led
	str r1,[r5] //reset them
	b check_input

//This function just check the input string, to get the length
check_input:
	ldrb r3, [r0,r2] // loading input[j], I use j so it's already in len(input)+1					
	cmp r3, #0
	beq init // if we arrive at the end of the string we start our computation
	add r2,r2,#1
	b check_input
is_space:
	//Are they a space?
	cmp r4, #0x20 
	addeq r1,r1,#1 // I go to next char
	cmp r5, #0x20
	subeq r2,r2,#1
	mov r15,r14
	
load_byte:
	ldrb r4,[r0,r1] // a char is a byte so I use ldrb instaed of ldr
	ldrb r5,[r0,r2]
	mov r15,r14

go2Lower:
	//Are they capital letter?
	cmp r4, #0x40 
	cmpgt r4, #0x5b
	addlt r4, #0x20 //the difference between the a and A in ascii
	
	cmp r5, #0x40
	cmpgt r5,#0x5b
	addlt r5,#0x20
	mov r15,r14
	
init: // this start the computation by substracting one
	sub r2,r2,#1 //put j = len(input)

//Here we check if is palindrome or not
check_palindrom:
	// Here you could check whether input is a palindrom or not
	cmp r1,r2 // if j is less then i ofc we have a palindrom
	bge is_palindrom //found
	
	bl load_byte //load the char
	
	bl is_space //first of all we check if there is any space
	
	bl load_byte // we reload the byte 
	
	bl go2Lower //we need to have all lower char

	cmp r4,r5 // check if they are the same
	bne is_no_palindrom
	
	add r1,r1,#1
	sub r2,r2,#1
	b check_palindrom



is_palindrom:
	ldr r5,  =0xFF200000 //reusing the r5 for led
	ldr r7, =0xfe0 // 5 rightmost
	str r7,[r5] // lights 5 leftmost
	ldr r6, =palindrom //loading "Palindrome detected"
	mov r9,#0 //this is here for setup the index of uart 
	b uart
	
is_no_palindrom:
	// same as palindrome
	ldr r5,  =0xFF200000
	ldr r7, =0x1f //5 leftmost
	str r7,[r5]
	ldr r6, =nopalindrom
	mov r9,#0
	b uart
	
uart:
	ldr r1, = 0xFF201000 //load uart address
	ldrb r8, [r6,r9] //load char
	cmp r8,#0 //cycle until the end
	beq _exit
	str r8,[r1]
	add r9,r9,#1
	b uart
	
	
_exit:
	// Branch here for exit
	b .
	
.data
.align
	// This is the input you are supposed to check for a palindrom
	// You can modify the string during development, however you
	// are not allowed to change the name 'input'!
	input: .asciz "Grav ned den varg"
	nopalindrom: .asciz "Not a palindrome\n"
	palindrom: .asciz "Palindrome detected\n"
.end