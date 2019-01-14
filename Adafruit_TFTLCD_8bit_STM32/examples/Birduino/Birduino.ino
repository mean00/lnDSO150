/*  Arduino Game Proejct
 *  Program made by Dejan Nedelkovski,
 *  www.HowToMechatronics.com 
 */
 
/*  This program uses the UTFT and UTouch libraries
 *  made by Henning Karlsen. 
 *  You can find and download them at:
 *  www.RinkyDinkElectronics.com
 */
/*
#include <UTFT.h> 
#include <UTouch.h>
#include <EEPROM.h>
*/

#include <Adafruit_TFTLCD_8bit_STM32.h> // Hardware-specific library
#include <TouchScreen_STM32.h>

#include "flappy_title.h"		// the latest version
#include "bird05.h"		// the latest version
#include "pillar01.h"	// first version
#include "pillar_end.h"
#include "bottom_deco.h"

#define BIRD_BG 0x7639 // first value from the bird header file
// Assign human-readable names to some common 16-bit color values:

#define LED_PIN PC13

// overlaping:
#define XM TFT_RS // 330 Ohm // must be an analog pin !!!
#define YP TFT_CS // 500 Ohm // must be an analog pin !!!
#define XP PB0 //TFT_D0 // 330 Ohm // can be a digital pin
#define YM PB1 //TFT_D1 // 500 Ohm // can be a digital pin

TouchScreen ts = TouchScreen(XP, YP, XM, YM);
TSPoint p;	// a point object holds x, y and z coordinates
int x, y; // Variables for the coordinates where the display has been pressed

//==== Creating Objects
Adafruit_TFTLCD_8bit_STM32    myGLCD;

//==== Defining Fonts
extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];

#define TEXT_POSITION_LEFT 50
#define TEXT_POSITION_CENTER 110
#define BOTTOM_AREA_Y 205
#define BOTTOM_AREA_BG_COLOR 0x// 221,216,150
#define BOTTOM_LINE_1_COLOR
// Floppy Bird
#define PILLAR_GAP 60 // pixels
#define BIRD_X PILLAR_BMP_X // from the pillar header file
#define MOV_RATE_X0 3 // starting moving rate
#define FALL_RATE_Y 3 // starting falling rate

static int xP, yP = 100, yB;
static int movingRate, fallRateInt, steps;
static uint32_t fallRate;
static int score, level;
static int lastSpeedUpScore, highscore;
//static int screenPressed;
static boolean screenPressed, gameStarted, paused;
static uint32_t crt_millis, prev_millis;

static char s[250]; // for sprintf
static void drawTitle(void);
/*****************************************************************************/
void setup()
{
/*
  // Initiate display
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
*/
	Serial.begin(115200);
	
	//delay(6000); // allow time for OS to enumerate USB as COM port
	
	Serial.print("\n*** Paint demo with easy touch calibration process ***\n");
	
	myGLCD.begin(0x9328);

 /**/
	// Touch screen range setting
	// set display X and Y range of the display screen
	// the returned coordinates will be automatically mapped to these values.
	// If this is not used, the default values form the header file are used.
	ts.rangeSet(TFTWIDTH, TFTHEIGHT);	
/*
// Calibration
// needs sequentially touching two, diagonally opposite corners (check serial output!)
// the touching can include small circular movements around the corners
// it will keep calibrating the corner as long as pressure is applied
  myGLCD.setRotation(0);
  myGLCD.fillScreen(WHITE);
  myGLCD.setCursor(0, 30);
  myGLCD.setTextColor(BLACK);
  myGLCD.setTextSize(2);
  myGLCD.println("Calibration\n");
  myGLCD.println("Press top corner...\n");
	Serial.println("Calibrating the touch surface");
	Serial.print("> please press one corner...");
  //myGLCD.fillRect(20, 0, 239, 19, GREEN);
  myGLCD.fillRect(TFTWIDTH-10, 0, 10, 10, GREEN);
	ts.calibratePoint();
  myGLCD.fillRect(TFTWIDTH-10, 0, 10, 10, BLUE);
  delay(1000);
  myGLCD.println("Now press the bottom corner...");
	Serial.print("ok.\n> and now press the diagonally opposite corner...");
  myGLCD.fillRect(0, TFTHEIGHT-10, 10, 10, GREEN);
	ts.calibratePoint();
  //myGLCD.setTextSize(2);
  myGLCD.setCursor(0, 200);
  myGLCD.println("Calibration done.");
	Serial.println("ok.\nCalibration done.");
  myGLCD.fillRect(0, TFTHEIGHT-10, 10, 10, BLUE);

	delay(1000);
*/
	uint32_t crt_millis = millis();
	uint32_t prev_millis = crt_millis;
	highscore = 0; //EEPROM.read(0); // Read the highest score from the EEPROM

	myGLCD.setRotation(1);
	myGLCD.fillScreen(BIRD_BG);
	drawTitle();

	initiateGame(); // Initiate the game
}
void drawScore(void);
/*****************************************************************************/
static int tick;
/*****************************************************************************/
void loop()
{
	//==== Controlling the bird
	if ( ts.getPoint(&p) && !screenPressed ) {
		// adapt coordinates to rotation(1)
		x = p.y;
		y = TFTWIDTH - p.x;
		//myGLCD.fillCircle(x, y, 3, BLACK); // test only, remove it
		if ( y>BOTTOM_AREA_Y ) { // pause the game
			while ( ts.getPoint(&p) ); // wait to release touch
			while ( !ts.getPoint(&p) ); // wait for next touch
		} else {
			tick = 0;
			fallRate = 0; // Setting the fallRate negative will make the bird jump
			fallRateInt = -10; // Setting the fallRate negative will make the bird jump
			screenPressed = true;
		}
	}
	
	// only do the rest when the time has come...
	crt_millis = millis();
#define TIME_UNIT 50 // game ticker, in millis
	if ( (crt_millis-prev_millis)<TIME_UNIT ) return;
	
	prev_millis = crt_millis;
    
	if ( (tick++)>0 ) {
		fallRate = tick*tick; // Setting the fallRate negative will make the bird jump
		fallRateInt = int(fallRate/15);
	}

    drawPilars(xP, yP); // Draws the pillars 
    
   // yB - y coordinate of the bird which depends on value of the fallingRate variable
    yB+=fallRateInt;
	// clip bird coordinates
	if ( yB>(BOTTOM_AREA_Y-BIRD_BMP_Y) ) yB=(BOTTOM_AREA_Y-BIRD_BMP_Y);
	if ( yB<0 ) yB=0;

    // Draws the bird
    drawBird(yB);

    // Checks for collision
	// top and bottom
	//if ( yB>(BOTTOM_AREA_Y-BIRD_BMP_Y) || yB<0 )	gameOver();
	// upper or lower pillar
    if ( xP<(BIRD_X+BIRD_BMP_X-5) && xP>(BIRD_X-PILLAR_BMP_X+5) && ( yB<(yP) || yB>(yP+PILLAR_GAP-BIRD_BMP_Y) ) )	gameOver();


    // After the pillar has passed through the screen
    if (xP<=-PILLAR_END_BMP_X){
      xP += TFTHEIGHT+PILLAR_END_BMP_X; // Restart xP
      yP = map(rand() % 100, 0, 100, PILLAR_END_BMP_Y, (BOTTOM_AREA_Y-PILLAR_END_BMP_Y-PILLAR_GAP)); // Random number for the pillars height
      score++; // Increase score by one
    }
    xP -= movingRate; // xP - x coordinate of the pillars; range: 319 - (-51)   
    // After each five points, increases the moving rate of the pillars
    if ((score - lastSpeedUpScore) == 5) {
      lastSpeedUpScore = score;
	  level ++;
      movingRate++;
    }
	drawScore();
    // Doesn't allow holding the screen / you must tap it
    if ( !ts.getPoint(&p) && screenPressed )	screenPressed = false;
}
void drawTitle(void);
/*****************************************************************************/
void initiateGame(void)
{
	while ( ts.getPoint(&p) ); // wait to release touch
	myGLCD.fillScreen(BIRD_BG);
  // Ground
  drawGround();
  // Text
   myGLCD.setTextColor(BLACK, myGLCD.color565(221, 216, 148));
  myGLCD.setTextSize(2);
  myGLCD.setCursor(5,220);
  myGLCD.print("Score:");
  //myGLCD.setTextSize(1);//SmallFont);
  myGLCD.setCursor(170,220);
  myGLCD.print("Level:");//, 140, 220);
  //myGLCD.setColor(0, 0, 0);
  //myGLCD.setBackColor(114, 198, 206);
  myGLCD.setTextColor(BLACK, BIRD_BG);
  myGLCD.setCursor(5,5);
  myGLCD.print("Highscore:");//,5,5);
  myGLCD.setCursor(130,5);
  myGLCD.print(highscore);//, 120, 6);
//  myGLCD.setCursor(255,5);
//  myGLCD.print("| RESET |");//,255,5);

	// Resets the variables to start position values
	xP = TFTHEIGHT;
	yB = 50;
	fallRate = 0;
	fallRateInt = 0;
	score = 0;
	level = 0;
	lastSpeedUpScore = 0;
	movingRate = MOV_RATE_X0;
	//screenPressed = -1;
	screenPressed = false;
	gameStarted = false;
	paused = false;
	steps = 0;
	tick = 0;

	drawBird(yB); // Draws the bird
	drawScore();
	// write tap to start
	myGLCD.drawFastHLine(0,23,TFTHEIGHT, BLACK);
	myGLCD.setTextSize(1);
	myGLCD.setTextColor(BLACK, BIRD_BG);
while (!gameStarted) {
	uint32_t tim = millis();
	myGLCD.setCursor(TEXT_POSITION_CENTER,100);
	myGLCD.print("TAP TO START");//,CENTER,100);
	while ( (millis()-tim)<500 ) {
		if ( ts.getPoint(&p) ) { gameStarted = true; break; }
	}
	myGLCD.fillRect(TEXT_POSITION_CENTER, 100, 100, 10, BIRD_BG); // clear text
	tim = millis();
	while ( (millis()-tim)<500 ) {
		if ( ts.getPoint(&p) ) { gameStarted = true; break; }
	}
}
	myGLCD.fillRect(TEXT_POSITION_CENTER, 100, 100, 10, BIRD_BG); // clear text
	myGLCD.fillRect(0, 0, TFTHEIGHT, 32, BIRD_BG);	// delete top status row, leave room for the game
	delay(100); // wait touch end
}
/*****************************************************************************/
void drawTitle(void)
{
	myGLCD.drawBitmap((TFTHEIGHT-FLAPPY_TITLE_X)/2, (TFTWIDTH-FLAPPY_TITLE_Y)/3, FLAPPY_TITLE_X, FLAPPY_TITLE_Y, flappy_title);
	// write info
	myGLCD.setTextColor(BLACK);//, color565(221, 216, 148));
	myGLCD.setTextSize(2);
	myGLCD.setCursor(20,150);
	myGLCD.print("STM32F103C8T6 & ILI9328");
while (1) {
	uint32_t tim = millis();
	myGLCD.setTextSize(1);
	myGLCD.setCursor(100,210);
	myGLCD.print("TAP TO CONTINUE");
	//delay(1000);
	while ( (millis()-tim)<500 ) {
		if ( ts.getPoint(&p) ) goto dT_end;
	}
	myGLCD.fillRect(100, 210, 100, 10, BIRD_BG);
	//delay(1000);
	tim = millis();
	while ( (millis()-tim)<500 ) {
		if ( ts.getPoint(&p) ) goto dT_end;
	}
}
dT_end:
	//delay(3000);
	myGLCD.fillScreen(BIRD_BG);
}
/*****************************************************************************/
void drawGroundDeco(void)
{
	//int offs = xP<0 ? (-xP)%12 : xP%12; // template repetition length = 12
	int offs = (steps+=movingRate)%12; // template repetition length = 12
	// draw 6 lines of deco
	myGLCD.drawBitmap(0, BOTTOM_AREA_Y+2, TFTHEIGHT, 1, bottom_deco+((offs++)%12));
	myGLCD.drawBitmap(0, BOTTOM_AREA_Y+3, TFTHEIGHT, 1, bottom_deco+((offs++)%12));
	myGLCD.drawBitmap(0, BOTTOM_AREA_Y+4, TFTHEIGHT, 1, bottom_deco+((offs++)%12));
	myGLCD.drawBitmap(0, BOTTOM_AREA_Y+5, TFTHEIGHT, 1, bottom_deco+((offs++)%12));
	myGLCD.drawBitmap(0, BOTTOM_AREA_Y+6, TFTHEIGHT, 1, bottom_deco+((offs++)%12));
	myGLCD.drawBitmap(0, BOTTOM_AREA_Y+7, TFTHEIGHT, 1, bottom_deco+((offs++)%12));
}
/*****************************************************************************/
void drawGround(void)
{
	//myGLCD.fillRect(0, BOTTOM_AREA_Y, TFTHEIGHT, 10, color565(47,175,68));
	myGLCD.drawFastHLine(0, BOTTOM_AREA_Y, TFTHEIGHT, BLACK);
	myGLCD.drawFastHLine(0, BOTTOM_AREA_Y+1, TFTHEIGHT, (int)bottom_deco[0]); // the light green
	drawGroundDeco();
	myGLCD.drawFastHLine(0, BOTTOM_AREA_Y+8, TFTHEIGHT, (int)bottom_deco[6]); // the dark green
	myGLCD.drawFastHLine(0, BOTTOM_AREA_Y+9, TFTHEIGHT, (int)bottom_deco[0]);
	// text area
	myGLCD.fillRect(0, BOTTOM_AREA_Y+10, TFTHEIGHT, TFTWIDTH-(BOTTOM_AREA_Y+10), myGLCD.color565(221,216,148));
}
/*****************************************************************************/
// ===== drawPlillars - Custom Function
/*****************************************************************************/
void drawPilars(int x, int y)
{
//x = 250; // test
	//sprintf(s, "drawPillars x: %3i, y: %3i\n", x, y); Serial.print(s);

	// clip y coordinates:
	if ( y<PILLAR_END_BMP_Y ) y = PILLAR_END_BMP_Y;
	if ( y>(BOTTOM_AREA_Y-PILLAR_GAP-PILLAR_END_BMP_Y) ) y = (BOTTOM_AREA_Y-PILLAR_GAP-PILLAR_END_BMP_Y);
	
	// draw pillar body till (y-PILLAR_END_BMP_Y)
	for (int i=0; i<PILLAR_BMP_X; i++) {
		myGLCD.drawFastVLine(x+i, 0, (y-PILLAR_END_BMP_Y), pillar[i]);
	}
	// draw pillar end
	myGLCD.drawBitmap(x, y-PILLAR_END_BMP_Y, (int)PILLAR_END_BMP_X, (int)PILLAR_END_BMP_Y, pillar_end);
	// delete old top pillar
	myGLCD.fillRect(x+PILLAR_END_BMP_X, 0, movingRate, y, BIRD_BG);
	
	// now draw lower pillar. First the pillar end
	myGLCD.drawBitmap(x, y+PILLAR_GAP, (int)PILLAR_END_BMP_X, (int)PILLAR_END_BMP_Y, pillar_end);
	// then the body
	for (int j=0; j<PILLAR_BMP_X; j++) {
		myGLCD.drawFastVLine(x+j, y+PILLAR_GAP+PILLAR_END_BMP_Y, (BOTTOM_AREA_Y-y-PILLAR_GAP-PILLAR_END_BMP_Y), pillar[j]);
		//myGLCD.drawBitmap(x, j, (int)PILLAR_BMP_X, (int)PILLAR_BMP_Y, pillar);
	}
	// delete old columns
	myGLCD.fillRect(x+PILLAR_END_BMP_X, y+PILLAR_GAP, movingRate, BOTTOM_AREA_Y-(y+PILLAR_GAP), BIRD_BG);

	drawGroundDeco();
}

/*****************************************************************************/
void drawScore(void)
{  // Draws the score
	myGLCD.setTextSize(2);
	myGLCD.setTextColor(BLACK, myGLCD.color565(221, 216, 148));
	myGLCD.setCursor(90, 220);
	myGLCD.print(score);
	myGLCD.setCursor(260, 220);
	myGLCD.print(level+1);
}
/*****************************************************************************/
//====== drawBird() - Custom Function
/*****************************************************************************/
void drawBird(int y)
{
	// delete previous bird
	//myGLCD.fillRect(BIRD_X, y-fallRateInt, BIRD_BMP_X, BIRD_BMP_Y, BIRD_BG);
	// Draws the new bird bitmap
	myGLCD.drawBitmap (BIRD_X, y, BIRD_BMP_X, BIRD_BMP_Y, bird);
/**/
  // Draws blue rectangles above and below the bird to clear its previous state
	if (fallRateInt>0) {
		myGLCD.fillRect(BIRD_X, y-fallRateInt, BIRD_BMP_X, fallRateInt, BIRD_BG);
	} else if (fallRateInt<0) {
		//Serial.println("bird up");
		myGLCD.fillRect(BIRD_X, y+BIRD_BMP_Y, BIRD_BMP_X, 0-fallRateInt, BIRD_BG);
	}

}
/*****************************************************************************/
//======== gameOver() - Custom Function
/*****************************************************************************/
void gameOver(void)
{
	// blink display
	for (int i = 0; i<10; i++) {
		myGLCD.invertDisplay(i&1);
		delay(100);
	}
/**/
	Serial.println("*** GAME OVER ***");
	sprintf(s, "xP: %i, yP: %i, yB: %i\n", xP, yP, yB ); Serial.print(s);

	delay(500); // 1 second
  myGLCD.setCursor(150,50);
  myGLCD.setTextSize(3);
  myGLCD.setTextColor(BLACK);//, BIRD_BG);
  myGLCD.print("GAME");
  myGLCD.setCursor(180,80);
  myGLCD.print("OVER!");
  myGLCD.setTextSize(1);
  myGLCD.setTextColor(BLACK);//, BIRD_BG);

  boolean t = false;
while ( !t ) {
	uint32_t tim = millis();
	myGLCD.setCursor(180,150);
	myGLCD.print("TAP TO CONTINUE");
	while ( (millis()-tim)<500 && !t ) {	//delay(1000);
		if ( ts.getPoint(&p) ) t = true;
	}
	myGLCD.fillRect(150, 150, 150, 10, BIRD_BG);
	tim = millis();
	while ( (millis()-tim)<500 && !t ) {	//delay(1000);
		if ( ts.getPoint(&p) ) t = true;
	}
}
	delay(100);
	while( ts.getPoint(&p) ); // wait for touch release
	// Writes the highest score in the EEPROM
	if (score > highscore) {
		highscore = score;
		//EEPROM.write(0,highscore);
	}
	// Restart game
	initiateGame();
}
