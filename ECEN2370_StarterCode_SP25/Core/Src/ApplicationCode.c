/*
 * ApplicationCode.c
 *
 *  Created on: Dec 30, 2023 (updated 11/12/2024) Thanks Donavon! 
 *      Author: Xavion
 */

#include "ApplicationCode.h"

/* Static variables */


extern void initialise_monitor_handles(void); 

#if COMPILE_TOUCH_FUNCTIONS == 1
static STMPE811_TouchData StaticTouchData;
#endif // COMPILE_TOUCH_FUNCTIONS

static bool oneplayer_twoplayer;
bool gameReset = true;
bool gameState = true;

void ApplicationInit(void)
{
	initialise_monitor_handles(); // Allows printf functionality
	LTCD__Init();
	LTCD_Layer_Init(0);
	LCD_Clear(0,LCD_COLOR_WHITE);

#if COMPILE_TOUCH_FUNCTIONS == 1
	InitializeLCDTouch();

	// This is the orientation for the board to be direclty up where the buttons are vertically above the screen
	// Top left would be low x value, high y value. Bottom right would be low x value, low y value.
	StaticTouchData.orientation = STMPE811_Orientation_Portrait_2;

#endif // COMPILE_TOUCH_FUNCTIONS
}

void LCD_Visual_Demo(void)
{
	visualDemo();
}

void LCD_Game_Start(void){
	LCD_Clear(0,LCD_COLOR_GREEN);
	while (1) {
		/* If touch pressed */
		if (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed) {
			/* Touch valid */
			printf("\nX: %03d\nY: %03d\n", StaticTouchData.x, StaticTouchData.y);
			LCD_Clear(0, LCD_COLOR_RED);
		} else {
			/* Touch not pressed */
			printf("Not Pressed\n\n");
			LCD_Clear(0, LCD_COLOR_GREEN);
		}
	}
}

#if COMPILE_TOUCH_FUNCTIONS == 1
void LCD_Touch_Polling_Demo(void)
{
	//	LCD_Clear(0,LCD_COLOR_GREEN);
	while (1) {
		/* If touch pressed */
		if ((returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed)) {
			/* Touch valid */
			gameReset = true;
				if (StaticTouchData.x < 120){
					oneplayer_twoplayer = true;
					gameplayScreen();
					while(gameReset){
						moveCoin(126, 150);
					}
				}
				else if (StaticTouchData.x > 120){
					oneplayer_twoplayer = false;
					gameplayScreen();
					while(gameReset){
						moveCoin(126, 150);
					}

				}
		}
		break;
	}
}
#endif // COMPILE_TOUCH_FUNCTIONS



	void EXTI0_IRQHandler(){
		disable_IRQ(EXTI0_IRQn);
		if(oneplayer_twoplayer){
			dropBallPlayer();
			dropBallAI();
		}
		else{
			dropBallPlayer();
			player1Mode();
		}
		EXTI_ClearPendingIRQ(0);
		enable_IRQ(EXTI0_IRQn);
	}

