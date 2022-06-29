/*
 * attack.h
 *
 *  Created on: Jun 22, 2022
 *      Author: nneto
 */

#ifndef INC_ATTACK_H_
#define INC_ATTACK_H_

#define GET_CLOCK_COUNT(TIMx) 	TIMx.Instance->CNT
#define SET_CLOCK(TIMx,VAL) 	TIMx.Instance->CNT = VAL
#define INTERRUPTION_2_MAX		4294967295
#define INTERRUPTION_5_MAX		4294967295
#define False  					0
#define True					1

void interruptionHandler1(void);
void mainLoop(void);
void _mainCode(void);
//void attack_IRQHandler(void);
void TIM2_IRQHandler(void);
void TIM5_IRQHandler(void);


#endif /* INC_ATTACK_H_ */
