/*
 * LCD_Driver.c
 *
 *  Created on: Sep 28, 2023
 *      Author: Xavion
 */

#include "LCD_Driver.h"
#include "stdio.h"

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */

static LTDC_HandleTypeDef hltdc;
static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
static FONT_t *LCD_Currentfonts;
static uint16_t CurrentTextColor = 0xFFFF;
static STMPE811_TouchData StaticTouchData;
static gameCoin coin;
static bool player1;
static int gameboard[7][6];
extern RNG_HandleTypeDef hrng;
static int endTime;
static int startTime;
extern bool gameReset;
static int playerOneScore;
static int playerTwoScore;


//void rng_init(void){
//	hrng->Instance = RNG;
//	HAL_RNG_Init(hrng);
//}



/*
 * fb[y*W+x] OR fb[y][x]
 * Alternatively, we can modify the linker script to have an end address of 20013DFB instead of 2002FFFF, so it does not place variables in the same region as the frame buffer. In this case it is safe to just specify the raw address as frame buffer.
 */
//uint32_t frameBuffer[(LCD_PIXEL_WIDTH*LCD_PIXEL_WIDTH)/2] = {0};		//16bpp pixel format. We can size to uint32. this ensures 32 bit alignment


//Someone from STM said it was "often accessed" a 1-dim array, and not a 2d array. However you still access it like a 2dim array,  using fb[y*W+x] instead of fb[y][x].
uint16_t frameBuffer[LCD_PIXEL_WIDTH*LCD_PIXEL_HEIGHT] = {0};			//16bpp pixel format.


void LCD_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable the LTDC clock */
  __HAL_RCC_LTDC_CLK_ENABLE();

  /* Enable GPIO clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* GPIO Config
   *
    LCD pins
   LCD_TFT R2 <-> PC.10
   LCD_TFT G2 <-> PA.06
   LCD_TFT B2 <-> PD.06
   LCD_TFT R3 <-> PB.00
   LCD_TFT G3 <-> PG.10
   LCD_TFT B3 <-> PG.11
   LCD_TFT R4 <-> PA.11
   LCD_TFT G4 <-> PB.10
   LCD_TFT B4 <-> PG.12
   LCD_TFT R5 <-> PA.12
   LCD_TFT G5 <-> PB.11
   LCD_TFT B5 <-> PA.03
   LCD_TFT R6 <-> PB.01
   LCD_TFT G6 <-> PC.07
   LCD_TFT B6 <-> PB.08
   LCD_TFT R7 <-> PG.06
   LCD_TFT G7 <-> PD.03
   LCD_TFT B7 <-> PB.09
   LCD_TFT HSYNC <-> PC.06
   LCDTFT VSYNC <->  PA.04
   LCD_TFT CLK   <-> PG.07
   LCD_TFT DE   <->  PF.10
  */

  /* GPIOA configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 |
                           GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
  GPIO_InitStructure.Alternate= GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

 /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_8 | \
                           GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

 /* GPIOC configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

 /* GPIOD configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_3 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

 /* GPIOF configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);

 /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | \
                           GPIO_PIN_11;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);

  /* GPIOB configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStructure.Alternate= GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* GPIOG configuration */
  GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}


void LTCD_Layer_Init(uint8_t LayerIndex)
{
	LTDC_LayerCfgTypeDef  pLayerCfg;

	pLayerCfg.WindowX0 = 0;	//Configures the Window HORZ START Position.
	pLayerCfg.WindowX1 = LCD_PIXEL_WIDTH;	//Configures the Window HORZ Stop Position.
	pLayerCfg.WindowY0 = 0;	//Configures the Window vertical START Position.
	pLayerCfg.WindowY1 = LCD_PIXEL_HEIGHT;	//Configures the Window vertical Stop Position.
	pLayerCfg.PixelFormat = LCD_PIXEL_FORMAT_1;  //INCORRECT PIXEL FORMAT WILL GIVE WEIRD RESULTS!! IT MAY STILL WORK FOR 1/2 THE DISPLAY!!! //This is our buffers pixel format. 2 bytes for each pixel
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 0;
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	if (LayerIndex == 0){
		pLayerCfg.FBStartAdress = (uintptr_t)frameBuffer;
	}
	pLayerCfg.ImageWidth = LCD_PIXEL_WIDTH;
	pLayerCfg.ImageHeight = LCD_PIXEL_HEIGHT;
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;
	if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, LayerIndex) != HAL_OK)
	{
		LCD_Error_Handler();
	}
}

void clearScreen(void)
{
  LCD_Clear(0,LCD_COLOR_WHITE);
}

void LTCD__Init(void)
{
	hltdc.Instance = LTDC;
	/* Configure horizontal synchronization width */
	hltdc.Init.HorizontalSync = ILI9341_HSYNC;
	/* Configure vertical synchronization height */
	hltdc.Init.VerticalSync = ILI9341_VSYNC;
	/* Configure accumulated horizontal back porch */
	hltdc.Init.AccumulatedHBP = ILI9341_HBP;
	/* Configure accumulated vertical back porch */
	hltdc.Init.AccumulatedVBP = ILI9341_VBP;
	/* Configure accumulated active width */
	hltdc.Init.AccumulatedActiveW = 269;
	/* Configure accumulated active height */
	hltdc.Init.AccumulatedActiveH = 323;
	/* Configure total width */
	hltdc.Init.TotalWidth = 279;
	/* Configure total height */
	hltdc.Init.TotalHeigh = 327;
	/* Configure R,G,B component values for LCD background color */
	hltdc.Init.Backcolor.Red = 0;
	hltdc.Init.Backcolor.Blue = 0;
	hltdc.Init.Backcolor.Green = 0;

	/* LCD clock configuration */
	/* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
	/* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
	/* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 Mhz */
	/* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_8 = 48/4 = 6Mhz */

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	/* Polarity */
	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	LCD_GPIO_Init();

	if (HAL_LTDC_Init(&hltdc) != HAL_OK)
	 {
	   LCD_Error_Handler();
	 }

	ili9341_Init();
}

/* START Draw functions */


/*
 * This is really the only function needed.
 * All drawing consists of is manipulating the array.
 * Adding input sanitation should probably be done.
 */
void LCD_Draw_Pixel(uint16_t x, uint16_t y, uint16_t color)
{
	if((x < 240) & (y < 320))
	{
		frameBuffer[y*LCD_PIXEL_WIDTH+x] = color;  //You cannot do x*y to set the pixel.
	}
	else
	{
		if (x > 240){
			printf("x is out of bounds");
		}
		else if (y > 320){
			printf("y is out of bounds");
		}
	}
}

/*
 * These functions are simple examples. Most computer graphics like OpenGl and stm's graphics library use a state machine. Where you first call some function like SetColor(color), SetPosition(x,y), then DrawSqure(size)
 * Instead all of these are explicit where color, size, and position are passed in.
 * There is tons of ways to handle drawing. I dont think it matters too much.
 */
void LCD_Draw_Circle_Fill(uint16_t Xpos, uint16_t Ypos, uint16_t radius, uint16_t color)
{
    for(int16_t y=-radius; y<=radius; y++)
    {
        for(int16_t x=-radius; x<=radius; x++)
        {
            if(x*x+y*y <= radius*radius)
            {
            	LCD_Draw_Pixel(x+Xpos, y+Ypos, color);
            }
        }
    }
}

void LCD_Draw_Vertical_Line(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
  for (uint16_t i = 0; i < len; i++)
  {
	  LCD_Draw_Pixel(x, i+y, color);
  }
}

void LCD_Clear(uint8_t LayerIndex, uint16_t Color)
{
	if (LayerIndex == 0){
		for (uint32_t i = 0; i < LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT; i++){
			frameBuffer[i] = Color;
		}
	}
  // TODO: Add more Layers if needed
}

//This was taken and adapted from stm32's mcu code
void LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

//This was taken and adapted from stm32's mcu code
void LCD_SetFont(FONT_t *fonts)
{
  LCD_Currentfonts = fonts;
}

//This was taken and adapted from stm32's mcu code
void LCD_Draw_Char(uint16_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, counter = 0;
  for(index = 0; index < LCD_Currentfonts->Height; index++)
  {
    for(counter = 0; counter < LCD_Currentfonts->Width; counter++)
    {
      if((((c[index] & ((0x80 << ((LCD_Currentfonts->Width / 12 ) * 8 ) ) >> counter)) == 0x00) && (LCD_Currentfonts->Width <= 12)) || (((c[index] & (0x1 << counter)) == 0x00)&&(LCD_Currentfonts->Width > 12 )))
      {
         //Background If want to overrite text under then add a set color here
      }
      else
      {
    	  LCD_Draw_Pixel(counter + Xpos,index + Ypos,CurrentTextColor);
      }
    }
  }
}

//This was taken and adapted from stm32's mcu code
void LCD_DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii)
{
  Ascii -= 32;
  LCD_Draw_Char(Xpos, Ypos, &LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height]);
}

void gameplayScreen(void){


	coin.xpos = 120;
	coin.ypos = 200;
	player1 = true;

	LCD_Clear(0,LCD_COLOR_WHITE);
	for(int i = 14; i < 228; i++){
		LCD_Draw_Vertical_Line(i,100,220, LCD_COLOR_BLUE);
	}
	LCD_Draw_Circle_Fill(120,60,14,LCD_COLOR_RED);
	//
	LCD_Draw_Circle_Fill(30,120,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(30,150,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(30,180,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(30,210,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(30,240,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(30,270,14,LCD_COLOR_WHITE);

	LCD_Draw_Circle_Fill(60,120,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(60,150,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(60,180,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(60,210,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(60,240,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(60,270,14,LCD_COLOR_WHITE);

	LCD_Draw_Circle_Fill(90,120,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(90,150,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(90,180,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(90,210,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(90,240,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(90,270,14,LCD_COLOR_WHITE);

	LCD_Draw_Circle_Fill(120,120,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(120,150,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(120,180,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(120,210,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(120,240,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(120,270,14,LCD_COLOR_WHITE);

	LCD_Draw_Circle_Fill(150,120,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(150,150,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(150,180,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(150,210,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(150,240,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(150,270,14,LCD_COLOR_WHITE);

	LCD_Draw_Circle_Fill(180,120,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(180,150,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(180,180,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(180,210,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(180,240,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(180,270,14,LCD_COLOR_WHITE);

	LCD_Draw_Circle_Fill(210,120,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(210,150,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(210,180,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(210,210,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(210,240,14,LCD_COLOR_WHITE);
	LCD_Draw_Circle_Fill(210,270,14,LCD_COLOR_WHITE);

	startTime = HAL_GetTick();

}

void moveCoin(uint16_t x, uint16_t y){


	if(player1 == true){
		LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_RED);
	}
	else{
		LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_BLACK);
	}


	if (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed){

			//LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_RED);
		if (StaticTouchData.x > 120){
			LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_WHITE);
			if(coin.xpos == 30){
				coin.xpos = 210;
				if(player1 == true){
					LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_RED);
				}
				else{
					LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_BLACK);
				}
				HAL_Delay(500);
			}
			else{
				coin.xpos = coin.xpos - 30;
				if(player1 == true){
					LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_RED);
				}
				else{
					LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_BLACK);
				}

				HAL_Delay(500);
			}
		}
		if (StaticTouchData.x < 120){

			LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_WHITE);
			if(coin.xpos == 210){
				coin.xpos = 30;
				if(player1 == true){
					LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_RED);
				}
				else{
					LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_BLACK);
				}
				HAL_Delay(500);
			}
			else{
				coin.xpos = coin.xpos + 30;
				if(player1 == true){
					LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_RED);
				}
				else{
					LCD_Draw_Circle_Fill(coin.xpos,60,14,LCD_COLOR_BLACK);
				}
				HAL_Delay(500);
			}
		}

	}

//	if (buttonPressed()){
//		for (int i )
//	}



}

void drawBallonBoard(void){

	for (int i = 0; i < 7; i++){
		for (int j = 0; j < 6; j++){
			if (gameboard[i][j] == 1){
				LCD_Draw_Circle_Fill(30+(i*30),270-(j*30),14,LCD_COLOR_RED);
			}
			else if (gameboard[i][j] == 2){
				LCD_Draw_Circle_Fill(30+(i*30),270-(j*30),14,LCD_COLOR_BLACK);
			}

		}

	}
}

void dropBallAI(void){

	uint32_t random = 0;

	HAL_RNG_GenerateRandomNumber(&hrng, &random);
	int column = random % 7;

		    if (column < 0 || column > 6) return;

		    for (int row = 0; row < 6; row++) {
		        if (gameboard[column][row] == 0) {
		            gameboard[column][row] = 2;

		            int winner = checkWin();

		            if (winner == 1) {
		            	clearGameBoard();
		            	displayWinScreen(winner);
		            } else if (winner == 2) {
		            	clearGameBoard();
		            	displayWinScreen(winner);
		            }

		            int tie = checkTie();

		            if(tie == 1){
		            	displayTieScreen();
		            }

		            drawBallonBoard();
		            break;
		        }
		    }

}

void dropBallPlayer(void){

	int column = (coin.xpos - 30) / 30;

	    if (column < 0 || column > 6) return;

	    for (int row = 0; row < 6; row++) {
	        if (gameboard[column][row] == 0) {
	            gameboard[column][row] = player1 ? 1 : 2;

	            int winner = checkWin();

	            if (winner == 1) {
	            	clearGameBoard();
	            	displayWinScreen(winner);

	            } else if (winner == 2) {
	            	clearGameBoard();
	            	displayWinScreen(winner);
	            }

	            int tie = checkTie();

	            if(tie == 1){
	            	displayTieScreen();
	            }

	            drawBallonBoard();

	            break;
	        }
	    }
}

int checkWin(void){

	for (int col = 0; col < 7; col++) {
	        for (int row = 0; row < 6; row++) {
	            int player = gameboard[col][row];
	            if (player == 0) continue;

	            //horizontal

	            if (col <= 3 && player == gameboard[col+1][row] &&
	                            player == gameboard[col+2][row] &&
	                            player == gameboard[col+3][row])
	                return player;

	            //verticle

	            if (row <= 2 && player == gameboard[col][row+1] &&
	                            player == gameboard[col][row+2] &&
	                            player == gameboard[col][row+3])
	                return player;

	            //diagonal down right
	            if (col <= 3 && row <= 2 && player == gameboard[col+1][row+1] &&
	                                         player == gameboard[col+2][row+2] &&
	                                         player == gameboard[col+3][row+3])
	                return player;

	            //diagonal up right
	            if (col <= 3 && row >= 3 && player == gameboard[col+1][row-1] &&
	                                         player == gameboard[col+2][row-2] &&
	                                         player == gameboard[col+3][row-3])
	                return player;
	        }
	    }

	    return 0;

}

int checkTie(void) {

	if (checkWin() != 0) {
        return 0;
    }

    for (int col = 0; col < 7; col++) {
        for (int row = 0; row < 6; row++) {
            if (gameboard[col][row] == 0) {
                return 0;
            }
        }
    }


    return 1;
}

void displayWinScreen(int winner) {


		if (winner == 1){
			LCD_Clear(0, LCD_COLOR_RED);
			LCD_SetTextColor(LCD_COLOR_BLACK);
			LCD_SetFont(&Font16x24);
			LCD_DisplayChar(85,90,'R');
			LCD_DisplayChar(105,90,'E');
			LCD_DisplayChar(125,90,'D');
			LCD_DisplayChar(75,120,'W');
			LCD_DisplayChar(95,120,'I');
			LCD_DisplayChar(115,120,'N');
			LCD_DisplayChar(135,120,'S');
			LCD_DisplayChar(85,220,'P');
			LCD_DisplayChar(105,220,'L');
			LCD_DisplayChar(125,220,'A');
			LCD_DisplayChar(145,220,'Y');
			LCD_DisplayChar(50,250,'A');
			LCD_DisplayChar(75,250,'G');
			LCD_DisplayChar(95,250,'A');
			LCD_DisplayChar(115,250,'I');
			LCD_DisplayChar(135,250,'N');
			LCD_DisplayChar(60,150,'T');
			LCD_DisplayChar(75,150,'i');
			LCD_DisplayChar(90,150,'m');
			LCD_DisplayChar(105,150,'e');
			LCD_DisplayChar(120,150,':');

			LCD_DisplayChar(10,185,'B');
			LCD_DisplayChar(25,185,'L');
			LCD_DisplayChar(40,185,'A');
			LCD_DisplayChar(55,185,'C');
			LCD_DisplayChar(70,185,'K');
			LCD_DisplayChar(80,185,':');

			LCD_DisplayChar(150,185,'R');
			LCD_DisplayChar(165,185,'E');
			LCD_DisplayChar(180,185,'D');
			LCD_DisplayChar(190,185,':');

						playerOneScore += 1;

						endTime = (HAL_GetTick() - startTime)/1000;

						char buffer[10];

						sprintf(buffer, "%d", endTime);

						for (int i = 0; buffer[i] != '\0'; i++) {
						    LCD_DisplayChar(145 + (i * 20), 150, buffer[i]);
						}

						char scoreBuffer3[10];

						sprintf(scoreBuffer3, "%d", playerTwoScore);

						for(int i = 0; scoreBuffer3[i] != '\0'; i++){
							LCD_DisplayChar(110 + (i*20),185, scoreBuffer3[i]);
						}

						char scoreBuffer4[10];

						sprintf(scoreBuffer4, "%d", playerOneScore);

						for(int i = 0; scoreBuffer4[i] != '\0'; i++){
								LCD_DisplayChar(200 + (i*20),185, scoreBuffer4[i]);
						}


						while(gameReset)
						{
							if(returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed){

								if(StaticTouchData.y > 200){
									gameReset = false;
								}

							}
						}

		}

		if (winner == 2){

			LCD_Clear(0, LCD_COLOR_BLACK);
			LCD_SetTextColor(LCD_COLOR_WHITE);
			LCD_SetFont(&Font16x24);
			LCD_DisplayChar(85,90,'B');
			LCD_DisplayChar(105,90,'L');
			LCD_DisplayChar(125,90,'A');
			LCD_DisplayChar(145,90,'C');
			LCD_DisplayChar(165,90,'K');
			LCD_DisplayChar(75,120,'W');
			LCD_DisplayChar(95,120,'I');
			LCD_DisplayChar(115,120,'N');
			LCD_DisplayChar(135,120,'S');

			LCD_DisplayChar(85,220,'P');
			LCD_DisplayChar(105,220,'L');
			LCD_DisplayChar(125,220,'A');
			LCD_DisplayChar(145,220,'Y');
			LCD_DisplayChar(50,250,'A');
			LCD_DisplayChar(75,250,'G');
			LCD_DisplayChar(95,250,'A');
			LCD_DisplayChar(115,250,'I');
			LCD_DisplayChar(135,250,'N');
			LCD_DisplayChar(60,150,'T');
			LCD_DisplayChar(75,150,'i');
			LCD_DisplayChar(90,150,'m');
			LCD_DisplayChar(105,150,'e');
			LCD_DisplayChar(120,150,':');

			LCD_DisplayChar(10,185,'B');
			LCD_DisplayChar(25,185,'L');
			LCD_DisplayChar(40,185,'A');
			LCD_DisplayChar(55,185,'C');
			LCD_DisplayChar(70,185,'K');
			LCD_DisplayChar(80,185,':');

			LCD_DisplayChar(150,185,'R');
			LCD_DisplayChar(165,185,'E');
			LCD_DisplayChar(180,185,'D');
			LCD_DisplayChar(180,185,':');



			playerTwoScore += 1;

			char scoreBuffer[10];

			sprintf(scoreBuffer, "%d", playerTwoScore);

			for(int i = 0; scoreBuffer[i] != '\0'; i++){
				LCD_DisplayChar(110 + (i*20),185, scoreBuffer[i]);
			}

			char scoreBuffer2[10];

			sprintf(scoreBuffer2, "%d", playerOneScore);

			for(int i = 0; scoreBuffer2[i] != '\0'; i++){
				LCD_DisplayChar(200 + (i*20),185, scoreBuffer2[i]);
			}


			endTime = (HAL_GetTick() - startTime)/1000;

			char buffer[10];

			sprintf(buffer, "%d", endTime);

			for (int i = 0; buffer[i] != '\0'; i++) {
			    LCD_DisplayChar(145 + (i * 20), 150, buffer[i]);
			}


			while(gameReset)
			{

				if(returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed){

					if(StaticTouchData.y > 200){
						gameReset = false;
					}

				}
			}


		}

}

void displayTieScreen(void){

	LCD_Clear(0, LCD_COLOR_BLACK);
	LCD_SetTextColor(LCD_COLOR_WHITE);
	LCD_SetFont(&Font16x24);
	LCD_DisplayChar(110,100,'T');
	LCD_DisplayChar(125,100,'I');
	LCD_DisplayChar(140,100,'E');

	LCD_DisplayChar(85,220,'P');
	LCD_DisplayChar(105,220,'L');
	LCD_DisplayChar(125,220,'A');
	LCD_DisplayChar(145,220,'Y');
	LCD_DisplayChar(50,250,'A');
	LCD_DisplayChar(75,250,'G');
	LCD_DisplayChar(95,250,'A');
	LCD_DisplayChar(115,250,'I');
	LCD_DisplayChar(135,250,'N');
	LCD_DisplayChar(60,150,'T');
	LCD_DisplayChar(75,150,'i');
	LCD_DisplayChar(90,150,'m');
	LCD_DisplayChar(105,150,'e');
	LCD_DisplayChar(120,150,':');

	endTime = (HAL_GetTick() - startTime)/1000;

			char buffer[10];

			sprintf(buffer, "%d", endTime);

			for (int i = 0; buffer[i] != '\0'; i++) {
			    LCD_DisplayChar(145 + (i * 20), 150, buffer[i]);
			}


	while(gameReset)
	{

		if(returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed){
			if(StaticTouchData.y > 200){
				gameReset = false;
			}

		}
	}


}

void clearGameBoard(void) {
    for (int col = 0; col < 7; col++) {
        for (int row = 0; row < 6; row++) {
            gameboard[col][row] = 0;
        }
    }
}


void visualDemo(void)
{
	// This for loop just illustrates how with using logic and for loops, you can create interesting things
	// this may or not be useful ;)
	LCD_Clear(0, LCD_COLOR_GREY);
	LCD_Draw_Vertical_Line(120,100,215,LCD_COLOR_BLACK);

	LCD_SetTextColor(LCD_COLOR_MAGENTA);
	LCD_SetFont(&Font16x24);

	LCD_DisplayChar(55,30,'C');
	LCD_DisplayChar(65,30,'o');
	LCD_DisplayChar(75,30,'n');
	LCD_DisplayChar(85,30,'n');
	LCD_DisplayChar(95,30,'e');
	LCD_DisplayChar(105,30,'c');
	LCD_DisplayChar(115,30,'t');
	LCD_DisplayChar(135,30,'4');

//	LCD_Clear(0,LCD_COLOR_BLUE);
	LCD_SetTextColor(LCD_COLOR_RED);
	LCD_SetFont(&Font16x24);
//
	LCD_DisplayChar(5,160,'O');
	LCD_DisplayChar(15,160,'n');
	LCD_DisplayChar(25,160,'e');
	LCD_DisplayChar(45,160,'P');
	LCD_DisplayChar(55,160,'l');
	LCD_DisplayChar(65,160,'a');
	LCD_DisplayChar(75,160,'y');
	LCD_DisplayChar(85,160,'e');
	LCD_DisplayChar(95,160,'r');

	LCD_SetTextColor(LCD_COLOR_BLUE);

	LCD_DisplayChar(135,160,'T');
	LCD_DisplayChar(145,160,'w');
	LCD_DisplayChar(155,160,'o');
	LCD_DisplayChar(175,160,'P');
	LCD_DisplayChar(185,160,'l');
	LCD_DisplayChar(195,160,'a');
	LCD_DisplayChar(205,160,'y');
	LCD_DisplayChar(215,160,'e');
	LCD_DisplayChar(225,160,'r');
}

void player1Mode(void){

	if(player1 == true){
		player1 = false;
	}
	else if (player1 == false){
		player1 = true;
	}

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void LCD_Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

// Touch Functionality   //

#if COMPILE_TOUCH_FUNCTIONS == 1

void InitializeLCDTouch(void)
{
  if(STMPE811_Init() != STMPE811_State_Ok)
  {
	 for(;;); // Hang code due to error in initialzation
  }
}

STMPE811_State_t returnTouchStateAndLocation(STMPE811_TouchData * touchStruct)
{
	return STMPE811_ReadTouch(touchStruct);
}

void DetermineTouchPosition(STMPE811_TouchData * touchStruct)
{
	STMPE811_DetermineTouchPosition(touchStruct);
}

uint8_t ReadRegisterFromTouchModule(uint8_t RegToRead)
{
	return STMPE811_Read(RegToRead);
}

void WriteDataToTouchModule(uint8_t RegToWrite, uint8_t writeData)
{
	STMPE811_Write(RegToWrite, writeData);
}

#endif // COMPILE_TOUCH_FUNCTIONS
