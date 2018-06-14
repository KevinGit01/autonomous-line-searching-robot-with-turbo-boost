/*******************************************************************************
 System Interrupts File

  File Name:
    system_interrupt.c

  Summary:
    Raw ISR definitions.

  Description:
    This file contains a definitions of the raw ISRs required to support the
    interrupt sub-system.

  Summary:
    This file contains source code for the interrupt vector functions in the
    system.

  Description:
    This file contains source code for the interrupt vector functions in the
    system.  It implements the system and part specific vector "stub" functions
    from which the individual "Tasks" functions are called for any modules
    executing interrupt-driven in the MPLAB Harmony system.

  Remarks:
    This file requires access to the systemObjects global data structure that
    contains the object handles to all MPLAB Harmony module objects executing
    interrupt-driven in the system.  These handles are passed into the individual
    module "Tasks" functions to identify the instance of the module to maintain.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2011-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "system/common/sys_common.h"
#include "app.h"
#include "system_definitions.h"

int sError = 0,error= 0,left=0,right=0,MAX_DUTY = 2400;
float ki = 0.033;

int comH = 0;
int comL = 0;
int comL_old = 0;
int eTotal = 0;
int counter = 0;

void __ISR(_USB_1_VECTOR, ipl4AUTO) _IntHandlerUSBInstance0(void)
{

    DRV_USBFS_Tasks_ISR(sysObj.drvUSBObject);
}


void __ISR(_TIMER_4_VECTOR, IPL4SOFT) Timer4ISR(void) {
  // code for PI control goes here
        comH = getCOMH();
        comL = getCOML();
        sError = getdiff();
        float kp = 2.6;
   
        //check if on the bridge 
        
     
        if(-17 <= sError && sError <= 17){
            MAX_DUTY = 2199;
        }else if(sError< -17){
            sError = -sError;
            MAX_DUTY = 2018 - sError*5 - eTotal*ki;
            if(MAX_DUTY < 1200){
                MAX_DUTY = 1200;
            }
        }else{
            MAX_DUTY = 2018 - sError*5 - eTotal*ki;
            if(MAX_DUTY < 1200){
                MAX_DUTY = 1200;
            }
        }
         
       
        if(comL == 0){
            MAX_DUTY = 1510;
            comL = comL_old;
            kp = 6;
        }
 
        
        error = comL - 319; // 240 means the dot is in the middle of the screen
        if(error<-30) { // slow down the left motor to steer to the left
            error  = -error;
            left = 0.85*MAX_DUTY - kp*error;
            right = 0.85*MAX_DUTY-error;
            if (left < 0){
                left = 0;
            }
        }
        else if(error > 30){ // slow down the right motor to steer to the right
            right = 0.85*MAX_DUTY - kp*error;
            left = 0.85*MAX_DUTY-error;
            if (right<0) {
                right = 0;
            }
        }else{
         right = MAX_DUTY;
         left = MAX_DUTY;
        }
        OC1RS = right;
        OC4RS = left;
        
        comL_old = comL;
        eTotal = eTotal + error;
        counter++;
        
        if(counter > 500){
            counter = 0;
            eTotal = 0;   
        }
                
  IFS0bits.T4IF = 0; // clear interrupt flag, last line
}