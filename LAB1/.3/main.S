.section .text
/////////////////////////////////////////////////////////////////////////////////////
// GPIO General Purpose I/0 Subroutine package                         0.91 AC
/////////////////////////////////////////////////////////////////////////////////////
.equ GPIOA,  0x40010800     // GPIOB Base address.
.equ GPIOB,  0x40010C00     // GPIOB Base address.
.equ CTR0,   0x00           // Port control register 0 offset
.equ ISTAT,  0x08           // Port input status register
.equ OCTL,   0x0C           // Port output control register
.equ BOP,    0x10           // Port bit operate register offset
.equ BC,     0x14           // Port bit clear register offset
.equ GPIO_AM, 0x0           // Mode: Input, Analog Mode
.equ GPIO_50, 0x3           // Mode: Output, max speed 50MHz
.equ GPIO_FI, 0x4           // Conf: (Input) Floating input
.equ GPIO_UD, 0x8           // Conf: (Input) Pull-up/-down
.equ GPIO_PP, 0x0           // Conf: (Output) Push-pull
.equ GPIO_OD, 0x4           // Conf: (Output) Open-drain

// GPIOI Initialize ////////////////////////////////////////////////// 1.0 / AC ////
// a0 Targeted GPIO-module
// a1 Configuration bits (<<2)
// a2 Mode bits
// a3 Targeted pin
gpioi:  mv t0, a0           // t0 points to GPIOX...
        mv t4, a3           //    t4 marks selected pin...
        li t1,0x8           //    Use port control register 0 or 1,
        blt t4,t1, 1f       //    thus is pin > 7?
        addi t0, t0, 4      //       Yes, adjust base address,
        addi t4,t4,-8       //            and adjust selected pin.

1:      slli t4,t4,2        //    Multiply pin nb by 4 to get steps to shift!
        li t1, 0xF          //    Create inverted configuration mask...
        sll t1, t1, t4      //    ...and move it into right position.
        not t1, t1          //    ...invert it back, then...
        mv t2, a1           //    Create configuration bits...
        or t2, t2, a2       //    
        sll t2, t2, t4      //    ...and move it into right position.

        lw t3,CTR0(t0)      //    retreive port control register 0(/1)...
        and t3,t3,t1        //    ...clear out selected pin's config/mode bits...
        or t3, t3, t2       //    ...configure new config/mode bits...
        sw t3,CTR0(t0)      //    ...and write it back.
        ret                 //  then go home!

// GPIOBO Bit Operations ///////////////////////////////////////////// 1.0 / AC ///
// a0 Targeted GPIO-module
// a1 Targeted pattern
// a2 Targeted pin(s)
gpiobo: and a1, a1, a2      // (Clear all patterns bits that are not target bits)
        xor a2, a2, a1      // Find zeros in targeted bits...
        slli a2, a2, 16     // ...position clear requests...
        or a1, a1, a2       // ...add set requests...
        sw a1, BOP(a0)      // ...then execute...
        ret                 // ...and return to caller!

// GPIBC Bit Clear /////////////////////////////////////////////////// 1.0 / AC ///
// a0 Targeted GPIO-module
// a1 Targeted pin(s)
gpiobc: sw a1, BC(a0)       // Execute request...
        ret                 // ...and return to caller!

// GPIOC Output Control ////////////////////////////////////////////// 1.0 / AC ///
// a0 Targeted GPIO-module
// a1 value
gpiooc: sw a1, OCTL(a0)      // Execute request...
        ret                 // ...and return to caller!

// GPIOIS Input Status //////////////////////////////////////////////// 1.0 / AC ///
// a0 Targeted GPIO-module
// a1 read value
gpiois: lw a1, ISTAT(a0)    // Execute request...
        ret                 // ...and return to caller!

// GPIO General Purpose I/0 Subroutine package ////////////////////////// END ///////

/////////////////////////////////////////////////////////////////////////////////////
// RCU Reset and Clock Unit Subroutine package                         0.91 AC
/////////////////////////////////////////////////////////////////////////////////////
.equ RCU,    0x40021000
.equ APB1EN, 0x1C
.equ T5EN,   0x10
.equ APB2EN, 0x18
.equ PAEN,   0x4
.equ PBEN,   0x8
// RCU1EN Enabel APB1 bus devices //////////////////////////////////// 1.0 / AC ///
rcu1en: li t0,RCU           // t0 points to RCU...
        lw t1,APB1EN(t0)    // ...retrieve current configuration...
        or t1,t1,a0         // ...and update the configuration...
        sw t1,APB1EN(t0)    // ...then save back the updated config...
        ret                 // and go home!

// RCU2EN Enabel APB2 bus devices //////////////////////////////////// 1.0 / AC ///
rcu2en: li t0,RCU           // t0 points to RCU...
        lw t1,APB2EN(t0)    // ...retrieve current configuration...
        or t1,t1,a0         // ...and update the configuration...
        sw t1,APB2EN(t0)    // ...then save back the updated config...
        ret                 // and go home!
        
// RCU Reset and Clock Unit Subroutine package ////////////////////////// END /////// 

.global main
main:   li a0, PBEN+PAEN    // Prepare to turn on GPIOB...
        call rcu2en         // ...do the set-up!

        li a0, GPIOB        // Prepare to init GPIOB...
        li a1, GPIO_PP      // ...as Push-Pull Output...
        li a2, GPIO_50      // ...with max 50MHz rate...
        li a3, 0            // ...for pin 0...
        li a4, 3            // ...to pin 2...
1:      call gpioi          // ...do the set-up!
        addi a3,a3,1        // ...advance to next pin...
        bne a3,a4, 1b       // ...continue, if not done!  

        li a0, GPIOA        // Prepare to init GPIOA...
        li a1, GPIO_AM      // ...in Analoge mode...
        li a2, GPIO_FI      // ...with High-Z...
        li a3, 5            // ...for pin 5...
        li a4, 9            // ...to pin 8...
1:      call gpioi          // ...do the set-up!
        addi a3,a3,1        // ...advance to next pin...
        bne a3,a4, 1b       // ...continue, if not done!   
### EDIT HERE
reset:
        la t0, code // Load memory address of code + LED pattern 
        lh t1, 0(t0) // Load Code
        li t2, 0 // Switch check counter (off-on-off)
        li t3, 0 // Correct answers
        li t4, 0 // Rounds
        lh a7, 2(t0) //Load LED pattern

1:      li a0, GPIOA        // Prepare to read GPIOA...
        call gpiois         // ...and get the value!
        srli a1,a1,5        // ...shift it down 5 steps...
        andi a1,a1,0xF     // ...mask out 4 lsb...
        
        srli a3, a1, 3 // Shift to get only trigger switch
        call MSB_check // Check if trigger switch has changed
        call sleep // Delay :P
        li t0, 0x2 // Load t0 with 2 temporarily
        bne t2, t0, 1b // if trigger switch hasnt been off-on-off, loop again
        li t2, 0 // otherwise reset counter

        call numb_check // check if number inputted matches code

        li a0, GPIOB        // Prepare to write GPIOB...
        andi a1, a7, 0x07 // mask out LED pattern to only get LSB
        li t0, 4 // Set limit for number entries temporarily
        bne t0, t3, 2f // if all answers correct, i.e t3 = 4 (only in final round)
        not a1, a1 // then invert pattern 000->111 
2:      srli a7, a7, 4 // right shift LED pattern to get next led formation
        srli t1, t1, 4 // also right shift code to next expected digit
        li a2, 0x07         // ...the 3 lsb...
        call gpiobo         // ...and write the new value!

        beq t4, t0, 3f  // check if 4 rounds have passed
        j 1b                // if no, jump back to start and read another digit
3:      bne t3, t0, reset // if yes, check if there have been 4 correct answers
lock:   j lock // if yes, lock device by looping foreverrrrrrr

sleep:  li a4, 12500000 // set time for sleep, 50MhZ -> 12500000 = 0.25 seconds
1:      addi a4, a4, -1 // remove 1 from counter
        bnez a4, 1b // if timer is not zero, go back to start of loop
        ret // otherwise return to where you came from

MSB_check:
        bnez t2, 2f //check if t2 is not 0, if yes, go to nearest title 2 forward
        j 1f //if no, go to nearest title 1 forward
1:
        bnez a3, 4f // - check if a1 is not 0, if yes, go to nearest 4 forward
        ret // if no, return
2:
        beqz a3, 4f // check if a3 is 0, if yes, go to nearest 4 forward
        ret // if no, return

4:      addi t2, t2, 1 // add 1 to switch check counter (off-on-off) 
        ret // then return

numb_check:
        andi a1, a1, 0x07 //mask out 3 LSB from input
        andi t0, t1, 0x07 //mask out 3 LSB from code
        bne a1, t0, 1f // Compare input to code, if not the same, jump to nearest 1 forward
        addi t3, t3, 1 // if the same, add 1 to correct answer counter
1:      addi t4, t4, 1 // add 1 to round counter
        ret // go back home
.data
code:   .half 0x5027
        .half 0x0624

.end
