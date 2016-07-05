/*
 * systick.c - 1ms system tick timer & related services.
 * 	- also handles switches, encoder and button debounce
 */

#include "systick.h"

uint32_t SysTick_Counter;

/*
 * SysTick_Init - sets up all the System Tick and UI state
 */
void SysTick_Init(void)
{
	GPIO_InitTypeDef       GPIO_InitStructure;

	/* Enable GPIOB Periph clock for diags */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	/* Configure PC14 PC15 as input w/ pullup */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Init tick counter */
	SysTick_Counter = 0;
	
	/* start Tick IRQ */
	if(SysTick_Config(SystemCoreClock/1000))
	{
		/* Hang here to capture error */
		while(1);
	}
}

/*
 * SysTick_Handler - Called by System Tick IRQ @ 1ms intervals to update UI elements
 */
void SysTick_Handler(void)
{
	/* Update SysTick Counter */
	SysTick_Counter++;
}
