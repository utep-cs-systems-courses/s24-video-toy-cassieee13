#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"

// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 

#define LED BIT6		/* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15

char blue = 31, green = 0, red = 31;
unsigned char step = 0;

static char 
switch_update_interrupt_sense()
{
  char p2val = P2IN;
  /* update switch interrupt to detect changes from current buttons */
  P2IES |= (p2val & SWITCHES);	/* if switch up, sense down */
  P2IES &= (p2val | ~SWITCHES);	/* if switch down, sense up */
  return p2val;
}

void 
switch_init()			/* setup switch */
{  
  P2REN |= SWITCHES;		/* enables resistors for switches */
  P2IE |= SWITCHES;		/* enable interrupts from switches */
  P2OUT |= SWITCHES;		/* pull-ups for switches */
  P2DIR &= ~SWITCHES;		/* set switches' bits for input */
  switch_update_interrupt_sense();
}

int switches = 0;

void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  switches = ~p2val & SWITCHES;
}


// axis zero for col, axis 1 for row

short drawPos[2] = {1,10}, controlPos[2] = {2, 10};
short colVelocity = 1, colLimits[2] = {1, screenWidth/2};

void
draw_ball(int col, int row, unsigned short color)
{
  fillRectangle(col-1, row-1, 3, 3, color);
}


void
screen_update_ball()
{
  for (char axis = 0; axis < 2; axis ++) 
    if (drawPos[axis] != controlPos[axis]) /* position changed? */
      goto redraw;
  return;			/* nothing to do */
 redraw:
  draw_ball(drawPos[0], drawPos[1], COLOR_BLUE); /* erase */
  for (char axis = 0; axis < 2; axis ++) 
    drawPos[axis] = controlPos[axis];
  draw_ball(drawPos[0], drawPos[1], COLOR_WHITE); /* draw */
}
  

short redrawScreen = 1;
u_int controlFontColor = COLOR_GREEN;

void wdt_c_handler()
{
  static int secCount = 0;

  secCount ++;
  if (secCount >= 25) {		/* 10/sec */
   
    {				/* move ball */
      short oldCol = controlPos[0];
      short newCol = oldCol + colVelocity;
      if (newCol <= colLimits[0] || newCol >= colLimits[1])
	colVelocity = -colVelocity;
      else
	controlPos[0] = newCol;
    }

    {				/* update hourglass */
      if (switches & SW3) green = (green + 1) % 64;
      if (switches & SW2) blue = (blue + 2) % 32;
      if (switches & SW1) red = (red - 3) % 32;
      if (step <= 30)
	step ++;
      else
	step = 0;
      secCount = 0;
    }
    if (switches & SW4) return;
    redrawScreen = 1;
  }
}
  
void update_shape();
void horizontalLine(u_char row, u_int color);
void verticalLine(u_char col, u_int color);
void createGame();

//this struct will be used to keep the data of a cup so that it is easy to delete and create them 
struct cup{
  u_char row;
  u_char col;
}
u_char midWidth = screenWidth/2;
//global cups
struct cup redL;
redL.col = midWidth+2;//so that the cups aren't directly on top of each other, 2 pixel offset
redL.row = 10;//to act as thumbing the table
struct cup redR;
redR.col = midWidth-22;//-22 comes from the width of redL
redR.row = 10;
struct cup redT;
redT.col = midWidth - 11;//to center the cup on top
redT.row = 32;//10 + 20 + 2 for offset
struct cup blueL;
blueL.col = midWidth+2;
blueL.row = screenHeight-30;
struct cup blueR;
blueR.col = midWidth-22;
blueR.row = screenHeight-30;
struct cup blueT;
blueT.col = midWidth-11;
blueT.row = screenHeight-52;
  
void main()
{
  
  P1DIR |= LED;		/**< Green led on when CPU on */
  P1OUT |= LED;
  configureClocks();
  lcd_init();
  switch_init();
  
  //enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  
  createGame();
}

void createGame()
{
  
  clearScreen(COLOR_SKY_BLUE);
  horizontalLine( (screenHeight/2), COLOR_BLACK);
  verticalLine( (screenWidth/2), COLOR_BLACK);
  
  //playerBlue Cups
  drawRectOutline(blueL.col,blueL.row,20,20,COLOR_BLUE);//left cup
  drawRectOutline(blueR.col,blueR.row,20,20,COLOR_BLUE);//right cup
  drawRectOutline(blueT.col,blueT.row,20,20,COLOR_BLUE);//top cup
  
  //playerRed Cups
  drawRectOutline(redL.col,redL.row,20,20,COLOR_RED);//left cup
  drawRectOutline(redR.col,redR.row,20,20,COLOR_RED);//right cup
  drawRectOutline(redT.col,redT.row,20,20,COLOR_RED);//the top cup
  
}

void verticalLine(u_char col, u_int color)
{
  /*
   * we want it to start at the beginning of the screen: row = 0
   * col = whereever you want the line to start on the width of the screen.
   * width = 1 because that determines the thickness
   * height = screenHeight so that it goes across the screen
   */
  fillRectangle(col, 0, 1, screenHeight, color);
}

void horizontalLine(u_char row, u_int color)
{
  /*
   * we want it to start at the beginning of the screen: col = 0
   * row = whereever you want the line to start. E.g. row = screenwidth/2 is a horizontal line in the middle of the screen
   * height = 1 because we dont want a thick line
   */
  fillRectangle(0, row, screenWidth, 1, color);
}

void
__interrupt_vec(PORT2_VECTOR) Port_2(){
  if (P2IFG & SWITCHES) {	      /* did a button cause this interrupt? */
    P2IFG &= ~SWITCHES;		      /* clear pending sw interrupts */
    switch_interrupt_handler();	/* single handler for all switches */
  }
}
