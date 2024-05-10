#include <msp430.h>
#include <libTimer.h>
#include "lcdutils.h"
#include "lcddraw.h"
#include "buzzer.h"

// WARNING: LCD DISPLAY USES P1.0.  Do not touch!!! 

#define LED BIT6		/* note that bit zero req'd for display */

#define SW1 1
#define SW2 2
#define SW3 4
#define SW4 8

#define SWITCHES 15


u_char button = 0;
u_char switch_state_changed = 0;

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

void
switch_interrupt_handler()
{
  char p2val = switch_update_interrupt_sense();
  if( !(p2val & SW1) ){
     button = 1;
     switch_state_changed = 1;
     button_update();
  }
  else if( !(p2val & SW2) ){
    button = 2;
    switch_state_changed = 1;
    button_update();
  }
  else if( !(p2val & SW3) ){
    button = 3;
    switch_state_changed = 1;
    button_update();
  }
  else if( !(p2val & SW4) ){
    button = 4;
    switch_state_changed = 1;
    button_update();
  }
}
  //game balls
typedef struct ball{
    u_char col;
    u_char row;
}ball;
  
ball blueBall = {8,155};//screenWidth = 128, so if i want it slightly on the edge col = 8
ball redBall = {120,5}; //screenHeight = 160, if i want it slightly up, it should be on row = 5

void
draw_ball(int col, int row, unsigned short color)
{
  fillRectangle(col-1, row-1, 3, 3, color);
}
 
void update_shape();
void horizontalLine(u_char row, u_int color);
void createGame();
void button_update();
void moveBallLeft(u_char player);
void moveBallRight(u_char player);
void shoot(u_char player);
void checkWin();

//this struct will be used to keep the data of a cup so that it is easy to delete and create them 
typedef struct cup{
  u_char col;
  u_char row;
} cup;

//cups
//center of screen is 64
cup redL = {66,10};
//so that the cups aren't directly on top of each other, 2 pixel offset
cup redR = {42,10};
cup redT = {54,32};//to center the cup on top, 10 + 20 + 2 for offset
cup blueL = {66,130};
cup blueR = {42,130};
cup blueT = {54,108};

//to keep track of who is winning
u_char redCups = 3;
u_char blueCups = 3;

void main()
{
  
  configureClocks();
  lcd_init();
  switch_init();
  buzzer_init();
  //enableWDTInterrupts();      /**< enable periodic interrupt */
  or_sr(0x8);	              /**< GIE (enable interrupts) */
  clearScreen(COLOR_PINK);
  drawString5x7(screenWidth/5,screenHeight/2,"msp430 Cup Pong", COLOR_RED, COLOR_PINK); 
  
}

u_char win = 0;// for the buzzer

void createGame()
{
  clearScreen(COLOR_SKY_BLUE);
  horizontalLine( (screenHeight/2), COLOR_BLACK);
  
  //playerBlue Cups
  drawRectOutline(blueL.col,blueL.row,20,20,COLOR_BLUE);//left cup
  drawRectOutline(blueR.col,blueR.row,20,20,COLOR_BLUE);//right cup
  drawRectOutline(blueT.col,blueT.row,20,20,COLOR_BLUE);//top cup
  
  //playerRed Cups
  drawRectOutline(redL.col,redL.row,20,20,COLOR_RED);//left cup
  drawRectOutline(redR.col,redR.row,20,20,COLOR_RED);//right cup
  drawRectOutline(redT.col,redT.row,20,20,COLOR_RED);//the top cup
  
  //make sure the balls restart
  blueBall.col = 8;
  blueBall.row = 155;
  redBall.col = 120;
  redBall.row = 5;
  
  draw_ball(blueBall.col, blueBall.row, COLOR_RED);
  draw_ball(redBall.col, redBall.row, COLOR_ORANGE);

  //make sure cup counts restart
  redCups = 3;
  blueCups = 3;
  win = 0;
}
//global player
u_char playerTurn = 0;

void button_update()
{
  if( button == 1){//S1 is responsible for setting fresh cups, 3 on each side, blue and red. 
    createGame();
  }
  else if( button == 2){
    moveBallLeft(playerTurn);
  }
  else if( button == 3){
    moveBallRight(playerTurn);
  }
  else if( button == 4){
    if( win ){
      buzzer_set_period(0); //turn off buzzer after win
    }
    else{
    shoot(playerTurn);
    }
  }
}

void moveBallRight(u_char player)
{
  
  if(player == 1){//player RED
    draw_ball(redBall.col, redBall.row, COLOR_SKY_BLUE); //erase the ball
    redBall.col -= 2;
    if(redBall.col == 0){
      redBall.col = 120;
    }
    draw_ball(redBall.col, redBall.row, COLOR_ORANGE);
  }
  else{ //player == 0, player BLUE
    draw_ball(blueBall.col, blueBall.row, COLOR_SKY_BLUE);
    blueBall.col += 2;
    if(blueBall.col == 128){
      blueBall.col = 8;
    }
    draw_ball(blueBall.col, blueBall.row, COLOR_RED);
  }
}
 
void moveBallLeft(u_char player)
{
  
  if(player == 1){//player RED
    draw_ball(redBall.col, redBall.row, COLOR_SKY_BLUE); //erase the ball
    redBall.col += 2;
    if(redBall.col == 128){//edge of screen
      redBall.col = 8;
    }
    draw_ball(redBall.col, redBall.row, COLOR_ORANGE);
    
  }
  else{ //player == 0, player BLUE
    draw_ball(blueBall.col, blueBall.row, COLOR_SKY_BLUE);
    blueBall.col -= 2;
    if(blueBall.col == 0){
      blueBall.col = 120;
    }
    draw_ball(blueBall.col, blueBall.row, COLOR_RED);
    
  }
}

void shoot(u_char player)
{
  if(player == 1){//player RED is shooting
    
    if( redBall.col == blueL.col +10 ){//if the ball is aligned to the center of the cup
      drawRectOutline( blueL.col, blueL.row, 20, 20, COLOR_SKY_BLUE);//erase the cup
      blueCups--;
    }
    else if( redBall.col == blueR.col +10 ){//if the ball is aligned to the center of the cup
      drawRectOutline( blueR.col, blueR.row, 20, 20, COLOR_SKY_BLUE);//erase the cup
      blueCups--;
    }
    else if( redBall.col == blueT.col +10 ){//if the ball is aligned to the center of the cup
      drawRectOutline( blueT.col, blueT.row, 20, 20, COLOR_SKY_BLUE);//erase the cup
      blueCups--;
    }
    checkWin();
    playerTurn = 0;
  }
  else{//player == 0, player BLUE is shooting
    
    if( blueBall.col == redL.col +10 ){//if the ball is aligned to the center of the cup
      drawRectOutline( redL.col, redL.row, 20, 20, COLOR_SKY_BLUE);//erase the cup
      redCups--;
    }
    else if( blueBall.col == redR.col +10 ){//if the ball is aligned to the center of the cup
      drawRectOutline( redR.col, redR.row, 20, 20, COLOR_SKY_BLUE);//erase the cup
      redCups--;
    }
    else if( blueBall.col == redT.col +10 ){//if the ball is aligned to the center of the cup
      drawRectOutline( redT.col, redT.row, 20, 20, COLOR_SKY_BLUE);//erase the cup
      redCups--;
    }
    checkWin();
    playerTurn = 1;
  }
  
}


void checkWin()
{
  if(redCups == 0){
    win = 1;
    clearScreen(COLOR_BLUE);
    drawString5x7(screenWidth/5, screenHeight/2, "PLAYER BLUE WON!", COLOR_WHITE, COLOR_BLUE);

    buzzer_set_period(LITTLE_C);
    
    P1DIR |= BIT0;
    P1OUT ^= BIT0;
   
  }
  if(blueCups == 0){
    win = 1;
    clearScreen(COLOR_RED);
    drawString5x7(screenWidth/5,screenHeight/2, "PLAYER RED WON!", COLOR_WHITE, COLOR_RED);
    buzzer_set_period(BIG_C);
    P1DIR |= LED;
    P1OUT |= LED;
  }
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
