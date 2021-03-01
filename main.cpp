/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
 /*
* Author: Amanda La  
* Date: October 25th 2020
* Course: CSE321
* Purpose: Security System implementation
* Extra modules/functions in file: c1isr, c2isr, c3isr, c4isr, open, closed, buildPassEntry, resetPass,
*                                   checkPass, tryAgain, passwordMatch, turnOnLED
* Assignment: Project 2
*********************************
* Inputs: PB_8, PB_9, PB_12, PB_15
* Outputs: PG_0, PG_1, PC_8, PC_9, PC_10, PC_11
* Constraints:   4 digit code is last 4 digits of person number,
*                everytime a value is entered an LED lights up
*                must have a response of some kind if the code entered is wrong
*                must run "forever", lock/unlock mode must be displayed on LCD,
*                code must be entered through the matrix keypad 
*********************************
* References: Managing Interrupts lecture video
* Summary of File:
*    This file works with the lcd1802.h file to print out whether the system is locked or unlocked
*    based on whether the complete inputted code is correct. The inputted code must be a length of four
*    and must equal "8632". If the code does not match "8632", then the system informs the user that
*    the inputted code was incorrect and they need to try again.      
*    
*/

//link to headers and other files
#include "mbed.h"
#include "lcd1802.h"

//additional function prototype declarations
void c1isr(void);
void c2isr(void);
void c3isr(void);
void c4isr(void);
void open();
void closed();
void buildPassEntry(const char c);
void resetPass();
void checkPass();
void tryAgain();
bool passwordMatch(char arr[], char pass[]);
void turnOnLED();

//variable declarations
Ticker t1;
Ticker t2;

EventQueue qu(32 * EVENTS_EVENT_SIZE);
CSE321_LCD lcd(16,2,LCD_5x10DOTS,PF_0,PF_1);

Thread t;
int row = 0; // var to use to determine row
int numEntries = 0; //counts number of inputs
char passcode[] = "8632";
char passEntry[4];
int ledSwitch = 0;

// setup interrupt objects
InterruptIn int1(PB_8, PullDown);
InterruptIn int2(PB_9, PullDown);
InterruptIn int3(PB_15,PullDown);
InterruptIn int4(PB_13,PullDown);

//main function
int main() {
    lcd.begin();
    lcd.clear();
    lcd.print("locked");
    t.start(callback(&qu, &EventQueue::dispatch_forever));
    // RCC
    RCC->AHB2ENR |= 6;
    // MODER
    //set up inputs for columns
    GPIOB->MODER &= ~(0xCC0F0000);

    //set up row outputs
    GPIOC->MODER &= ~(0xAA0000);
    GPIOC->MODER |= 0x550000;

    //sets up LED outputs
    GPIOG->MODER |= 5;
    GPIOG->MODER &= ~(0xA);

    //enables clock
    RCC->AHB2ENR |= 0x40;

    // set up interrupt behavior
    int1.rise(qu.event(c1isr));
    int2.rise(qu.event(c4isr));
    int3.rise(qu.event(c2isr));
    int4.rise(qu.event(c3isr));

    int1.enable_irq();
    int2.enable_irq();
    int3.enable_irq();
    int4.enable_irq();
    while (true) { 
        //checks last row
        if (row == 0) {
        row = 1;
        GPIOC->ODR = 0x200;
        } 
        //checks first row
        else if(row == 1){
        row = 2;
        GPIOC->ODR = 0x100;
        }
        //check 2nd row
        else if(row ==2){
            row = 3;
            GPIOC->ODR = 0x400;
        }
        //check 3rd row
        else{
            row = 0;
            GPIOC->ODR = 0x800;
        }
        // delay
        thread_sleep_for(100); // 50 ms
    }//end while
}//end main

//definitions for additional functions
// ISR for C1 - 1 or *
void c1isr(void) {
    ++numEntries;
  // which row
    if (row == 0) {
        turnOnLED();
        printf("*\n");
        numEntries = 0;
        lcd.clear();
        lcd.print("reset");
    } else if(row ==1) {
        turnOnLED();
        printf("1\n");
        buildPassEntry('1');
    }
    else if(row ==2){
        turnOnLED();
        printf("4\n");
        buildPassEntry('4');
    }
    else{
        turnOnLED();
        printf("7\n");
        buildPassEntry('7');
    }
  //checks if we have entered a pass
     if(numEntries == 4){
         if(row == 0){
             resetPass();
         }
         else{
            checkPass();
         }
    }
    else{
        if(row == 0){
            resetPass();
        }
        else{
            closed();
        }
    }
    wait_us(500); // 500 us
}

// ISR for C4 - A or D
void c4isr(void) {
    ++numEntries;
    if (row == 0) {
        turnOnLED();
        printf("D\n");
        buildPassEntry('D');
    } else if(row == 1){
        turnOnLED();
        printf("A\n");
        buildPassEntry('A');
    }
    else if(row == 2){
        turnOnLED();
        printf("B\n");
        buildPassEntry('B');
    }
    else{
        turnOnLED();
        printf("C\n");
        buildPassEntry('C');
    }
    //check if pass entered
    checkPass();
    wait_us(500); // 500 us
}

//ISR for actual column 2 - 2 or 0
void c2isr(void){
    ++numEntries;
    if(row == 0){
        turnOnLED();
        printf("0\n");
        buildPassEntry('0');
    }
    else if(row==1){
        turnOnLED();
        printf("2\n");
        buildPassEntry('2');
    }
    else if(row ==2){
        turnOnLED();
        printf("5\n");
        buildPassEntry('5');
    }
    else{
        turnOnLED();
        printf("8\n");
        buildPassEntry('8');
    }
      //check if pass entered
    checkPass();
    wait_us(500);
}

//ISR for actual column 3 - 3 or #
void c3isr(void){
    ++numEntries;
    if(row==0){
        turnOnLED();
        printf("#\n");
        buildPassEntry('#');
    }
    else if(row == 1){
        turnOnLED();
        printf("3\n");
        buildPassEntry('3');
    }
    else if(row ==2){
        turnOnLED();
        printf("6\n");
        buildPassEntry('6');
    }
    else{
        turnOnLED();
        printf("9\n");
        buildPassEntry('9');
    }
      //check if pass entered
    checkPass();
    wait_us(500);
}

//displays on lcd that we unlocked the system
void open(){
    lcd.clear();
    lcd.print("unlocked");
}
//displays on lcd that the system is still locked
void closed(){
    lcd.clear();
    lcd.print("locked"); 
}
//displays on lcd that the entered password is reset
void resetPass(){
    lcd.clear();
    lcd.print("reset entries");
}
//builds a string from the entered passwords
void buildPassEntry(const char c){
    passEntry[numEntries-1] = c;
}
//informs user that the complete entered password is incorrect through lcd display
void tryAgain(){
    lcd.clear();
    lcd.print("locked,try again");
}
//checks if the entered password matches with the hardcoded password
bool passwordMatch(char enteredPass[], char pass[]){
    for(int i = 0; i < 4; ++i){
        if(enteredPass[i] != pass[i]){
            return false;
        }
    }
    return true;
}
/* checks if we have entered 4 entries, and checks if entered password matches
*  the hardcoded password. if the passwords match, then we unlock the system
*  and reset the number of password entries. else, we reset the password entries
*  and tell the user to try again to input the password.
*  if we don't have 4 entries, then the system remains locked. */
void checkPass(){
    if(numEntries == 4){
        if(passwordMatch(passEntry,passcode)){
            open();
            printf("%s\n", passEntry);
            numEntries =0;
        }
        else{
            printf("%s\n",passEntry);
            numEntries = 0;
            tryAgain();
        }
    }
    else{
        closed();  
    }
}

/* turns on a different LED based on the value stored in ledSwitch
   used whenever a keypad entry is recognized by the system */
void turnOnLED(){
    if(ledSwitch == 0){
        GPIOG->ODR = 2;
        ledSwitch = 1;
    }
    else{
        GPIOG->ODR = 1;
        ledSwitch = 0;
    }
}