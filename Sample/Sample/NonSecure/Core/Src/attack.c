/*
 * attack.h
 *
 *  Created on: Jun 16, 2022
 *      Author: aj4775
 */
#include "attack.h"
#include "main.h"
#include "secure_nsc.h"
#include <string.h>
#define _OPEN_SYS_ITOA_EXT
#include <stdlib.h>

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim5;

void __reducePriorityTIM2(){
    __HAL_RCC_TIM2_CLK_ENABLE();
    /* TIM5 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void __increasePriorityTIM2(){
    __HAL_RCC_TIM2_CLK_ENABLE();
    /* TIM5 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

void syncDebbugTimer(){
	/* This code m ake the timer of the MCU stop counting when the
	 * Debbuger reach a breakpoint
	 */
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim5);
	HAL_DBGMCU_EnableDBGStandbyMode();
	HAL_DBGMCU_EnableDBGStopMode();
	DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_TIM2_STOP;
	DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_TIM5_STOP;
}

#define ATTACK_ADDRESS	100
#define ATTACK_DELAY	INTERRUPTION_MAX-10

int firstFlag2 = False;
int firstFlag5 = False;
int tAuth = False;

int flagAttackOn = False;

uint32_t aReturnAddress;
char aReturnAddressChars[9];

int neverChangeFlag = False;
void attack(void);

void __setAuth(){
	tAuth = True;
	return;
}

unsigned int tPrivilegedFlag = 0;

void _privilegedCode(){
	tPrivilegedFlag = 1;
}

void TIM5_IRQHandler(){

	if (firstFlag5 == False) {
		HAL_TIM_IRQHandler(&htim5);
		return;
	}

	HAL_TIM_IRQHandler(&htim5);
	char redirectAddress[4];
	char vulnerability[1];
	// Buffer Attack

	redirectAddress[0] = (char) 0xFF & aReturnAddress;
	redirectAddress[1] = (char) 0xFF & aReturnAddress >> 8;
	redirectAddress[2] = (char) 0xFF & aReturnAddress >> 16;
	redirectAddress[3] = (char) 0xFF & aReturnAddress >> 24;

	memcpy( vulnerability + 40, redirectAddress, 4 * sizeof(char));

	//strcpy(stringAttack + 0 ,"\x65\x02\x04\x08");
}

void TIM2_IRQHandler(){
	if (firstFlag2 == False) {
		HAL_TIM_IRQHandler(&htim2);
		return;
	}
	HAL_TIM_IRQHandler(&htim2);

	// Buffer Overflow Attack Simulation
	char stringAttack[1];
	//strcpy(stringAttack + 8 ,"\xc5\x03\x04\x08"); // Jump to attack address
	aReturnAddress = * ( (uint32_t*) (stringAttack + 36) );
	strcpy(stringAttack + 36 ,"\xb9\x02\x04\x08");
	SET_CLOCK(htim5,INTERRUPTION_5_MAX-39);
	//SET_CLOCK(htim5,0);
}


void _mainCode(){

	if (neverChangeFlag) attack(); // Just to appear in the assembly

	while(1){
		SET_CLOCK(htim2,INTERRUPTION_2_MAX);
		for(int i=0;i<10;i++){} // Random code execution
		if (neverChangeFlag) __setAuth();
		SET_CLOCK(htim2,INTERRUPTION_2_MAX);
		for(int i=0;i<10;i++){} // Random code execution
		if(tAuth) _privilegedCode();
	}
}


void attack(){
	/*	Buffer Overflow:
	 * 		Jump to the middle of the privileged code
	 * 		and schedule the interruption of TIM5(timer 5)
	 * 		to trigger before the attestation log
	 */
	if (!flagAttackOn){
		__reducePriorityTIM2();
		flagAttackOn = True;
	}

	asm("pop {r6,r7} \n"
		"ldr r6, EXC_RETURN \n"
		"ldr lr, TempLR \n"
		"push {r6}\n"
		"pop {pc}\n");

	char stringAttack[1];
	// Buffer Overflow
	strcpy(stringAttack + 12 ,"\x45\x1f\x04\x08");
	// Schedule interruption TIM5


	return;
}
asm("nop");
asm("EXC_RETURN:\n"
	"\t.word 0xffffffa8");
asm("TempLR:\n"
	"\t.word 0x8041f45");

void _init_(){
	syncDebbugTimer();
	firstFlag2 = True;
	firstFlag5 = True;
}

void mainLoop(){
	_init_();
	_mainCode();
}
