
// Rev 2A8: Scratch Rev of alternating blade colors.
//..............................................................

// Rev 2B1: All strips change color together, solid color disk.
// Color changes each time breath stops and restarts.

// Rev 2B2: Preliminary structure for breath controlled disk radius.
//          Lights each strip in a separate line of code.

// Rev 2B3: NUMPIXELS is no longer a constant, it is calulated from the value stripPixels.

//          Added strip.setBrightness(n) and variable 'brightness' for hardcoded LED brightness.
//          This can be used in future revs for dynamic brighness if desired.

// Rev 2B4:  RainBow Gradient replaces solid color disk.

// Rev 2B5:  RainBow Gradient and solid color disk are combined. Rainbow is treated as a colorOffset 4 (0 - 4 = White, Blue, Green, Red, rainBow)

// Rev 2C:   Cleaned up version of Rev 2B5. Unused variables and comments removed. No functional changes.

// Rev 3A:   Initial motor drive test.

// Rev 3A2:  Testing code snip intended for ref Demo_1D:
//           The strip.fill starting indexes for strips  1,2, & 3 are calculated from stripPixels instead of hard coded.

// ..........................................................................................................................................................

// Rev 4A:   Scratch Rev (Root: 3A2) for testing color gradients and patterns: Testing 'switch' statement, for manual fill map selection by name.
//           --> Not yet working with pattern names. Works well with pattern numbers, e.g. 0 - 3 for four patterns.
// ==========================================================================================================================================================
//  Rev 4B Added additional pattern handling at artists request, sequence key is below.  Interim rev.

//  Rev 4C The new pattern maps are gradients, generated in a spreadsheet. The spreadsheet generates a table that can be cut and pasted directly into code.
//         IMPORTANT: The spreadsheet maps include braces and commas around the data. They to not include the requiste 'int myVar =' or the star and end braces and semicolon.
//         Pattern auto sequencing is working; steps through all the patterns below, including solid colors.
//         The 'if' block  that increments colorOffset from 0 - 2 also increments patNum from 0 - 4.
//         This isn't quite what the artist is looking for; it cycles both solid colors between each gradient pattern.
//         The solid color fill routine should be duplicated, and placed into switch cases 0 and 1, next rev.

//  Rev 4D Moved the solid color fill routine into the switch / case pattern selector.
//         Each solid color is now treated as one of the patterns, even though it doesn't use a map.


/*  ---------------------------------- Breath Consciousness Beta Release [Bench Test Rev for Arduino Mega]  -------------------------------------------------

  [Root: LedTst 4d]

  Release Rev for video and related promotional use.

  When breath is present, disk radius follows breath as usual.
  LED's will step automatically through the pattern sequence key below.

  The patDwell time at each pattern is set by the User variable 'patDwell' , in the User Variable Section below. patDwell units are milliseconds.
  LED overall brightness is set by User variable 'brightness'. Brightness is unitless, 0 - 255.

  Sequence Key:

  0. solid color, White
  1. solid color, Blue

  2. gradient 1  [Yellow to Green to Blue]
  3  gradient 2  [Orange to Red to Pink]
  4. stripes
  5. checker
  6. Rainbow

  [Beta 1A]

  Added new User Variable 'autoSeq' that allows the user to enable or disable auto sequencing through the patterns.
  When auto sequencing is disabled (off) the patterns increment each time breath stops, as in prior versions.

  Added 250 mS delay to the Breath Not Present 'else' block to briefly delay response to breath stopping, per empirical testing.
  The primary effect of this is to prevent inadvertent sequencing of patterns if there is a brief break in breath.

  [Beta 1B]
  The diskRadius calculation routine had a hard coded variable for the number of pixels. That algorithm now uses 'stripPixels'.

  Updated nongradient patters, Stripes, Checkers, and Rainbow, for 82 pixel strips, in release version only, NOT this Bench Test rev.

  [Beta 2A]  Add Breath Duration Option; longer breath duration behaves like increased breath volume, higher fan speed and larger disk radius.

  [Beta 2B]  Corrected Stripe, Checker, and Rainbow pattern maps for 82 pixel strips.

  [Beta 2C]  Added disk radius deflate dwell that starts when breath stops.



**********************************************************************************************************************************************************************
*/

// ****************************************************************************************************************************************************

#include <Adafruit_DotStar.h>

#include <SPI.h>

int stripPixels = 82;  // [Rev 2A8] TEST BENCH STRIPS HAVE 30 LEDS EACH. This has to match the TOTAL number of pixels; it's used in the AdaFruit_DotStar function declaration.
//                        --->> CHANGE TO 82 TO MATCH ACTUAL PIXELS PER STRIP. <<-----

int NUMPIXELS = (stripPixels * 4) ; // [Rev 2B3] Calculate total Number of LEDs, all four strips from stripPixels.  >> THE '+1' IS NOT NEEDED, ONLY FOR BENCH TEST.

// Set LED control pins to SPI ~ MOSI and SCK.
#define DATAPIN    11
#define CLOCKPIN   13

/*
  [Rev 1C Scratch] [LedTst 4B]  There is an internal error in DotStars. The term 'DOTSTAR_BGR' is required for DotStar to use the standard RGB sequence.
                             'DOTSTAR_GRB' etc is NOT correct.  THIS CODE USES Green/Red/Blue (GRB) THROUGHOUT,
*/

Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);   // Correct this in future rev.

const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to

int fanVal = 0;          // << [Rev 3A] Motor control analog output.

int windSenVal = 0;
int adjLedWindVal = 0;      // Gain Adj for Wind Sensor. Max raw output is ~2V.

int adjFanWindVal = 0;       // [Rev Demo 1D] Separate adjWinVal for motor.

int diskRadius = 0;   // [Rev 2B2] Breath proportional disk Radius (Pixels per strip.)

int i = 0;
int j = 0;

unsigned long Delta = 0;

unsigned long timeNow = 0;   // [LedTst 4C] These two variable comprise a patDwell timer that increments the pattern sequence.


// .............................. [Beta 2A] Breath Duration Tracking ......................

long prevBreathMillis = 0;
long breathpatDwell = 0;

// ............................. [Beta 2C] Gradual deflate when no breath.  ..............

long prevDeflateMillis = 0;
long deflateDwell = 0;

int deflateTrim = 1;




/*   ***************************************************************************************************************************************************************** */

/*   ************************************************************** USER CONFIGURATION VARIABLES ********************************************************************* */


int brightVal  = 16;        //  LED Brightness Setting [Range 0 - 255]

int patDwell = 5000;         //  Pattern dwell time, milliseconds. [Range 100 - 10000]

int autoSeq = 0;             //  [Beta 1A]  Auto pattern sequencing: 0 = Off   1 = On


int RadiusMode = 1;          //  [Beta 2A]  If = 0, breath volume is used for fan speed and diskRadius, as prior revs.
//                                          If = 1, breath volume is used for fan speed, and breath duration is used for diskRadius.

int maxBreath = 3000;        //  [Beta 2A] Breath duration required for maximum disk radius, milliseconds. [Range 100 - 10000]



int deflateMode = 1;             //  [Beta 2C] If = 0, Stop fan and turn off all LEDs at once when breath stops.
//                                             If = 1, Stop fan, and then deflate disk radius over 'deflateDwell’ seconds.

int maxDeflate = 2000;       //  [Beta 2C2]   Deflate duration, milliseconds. [Range 100 - 10000]


/*   ************************************************************ END of User Configuration Variables **************************************************************** */

/*   ***************************************************************************************************************************************************************** */

// .....................................................................................................................................................

//                                                ============== Pattern Maps  Grn, Red, Blu [GRB] Sequence ================




// Stripes
int stripeFill [ ] [3] =      // [Rev ]

{
  {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201},
  {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201},
  {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201},
  {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201},

  {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201},
  {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201},
  {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201},
  {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201}, {124, 53, 145}, {58, 191, 201},
  {124, 53, 145}, {58, 191, 201}
};

// Checkers1  [Green / Sky Blue, 2 pixels per color.]
int checkers1  [ ] [3] =

{
  {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9},
  {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145},
  {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9},
  {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145},

  {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9},
  {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145},
  {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9},
  {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145}, {253, 3, 9}, {253, 3, 9}, {102, 45, 145}, {102, 45, 145},
  {253, 3, 9}, {253, 3, 9}
};


// Rainbow
int rainBow [ ] [3] =

{
  {255, 0, 0}, {255, 20, 0}, {255, 58, 0}, {255, 151, 0}, {255, 233, 0}, {225, 255, 0}, {148, 255, 0}, {60, 255, 0}, {7, 255, 5}, {0, 255, 52},
  {0, 255, 134}, {0, 255, 214}, {0, 243, 254}, {0, 175, 255}, {0, 79, 255}, {2, 14, 255}, {41, 0, 255}, {120, 0, 255}, {202, 0, 255}, {254, 0, 247},
  {255, 0, 189}, {255, 0, 0}, {255, 20, 0}, {255, 58, 0}, {255, 151, 0}, {255, 233, 0}, {225, 255, 0}, {148, 255, 0}, {60, 255, 0}, {7, 255, 5},
  {0, 255, 52}, {0, 255, 134}, {0, 255, 214}, {0, 243, 254}, {0, 175, 255}, {0, 79, 255}, {2, 14, 255}, {41, 0, 255}, {120, 0, 255}, {202, 0, 255},

  {7, 255, 5}, {0, 255, 52}, {0, 255, 134}, {0, 255, 214}, {0, 243, 254}, {0, 175, 255}, {0, 79, 255}, {2, 14, 255}, {41, 0, 255}, {120, 0, 255},
  {202, 0, 255}, {254, 0, 247}, {255, 0, 189}, {255, 0, 0}, {255, 20, 0}, {255, 58, 0}, {255, 151, 0}, {255, 233, 0}, {225, 255, 0}, {148, 255, 0},
  {60, 255, 0}, {7, 255, 5}, {0, 255, 52}, {0, 255, 134}, {0, 255, 214}, {0, 243, 254}, {0, 175, 255}, {0, 79, 255}, {2, 14, 255}, {41, 0, 255},
  {120, 0, 255}, {202, 0, 255}, {120, 0, 255}, {202, 0, 255}, {254, 0, 247}, {255, 0, 189}, {255, 0, 0}, {255, 20, 0}, {255, 58, 0}, {255, 151, 0},
  {255, 233, 0}, {225, 255, 0}
};

// .....................................................................................................................................................


// ----------------------------------------------------------- Spreadhseet Generated Gradient Maps ------------------------------------------------------------

// gradient1 [Yellow --> Green --> Blue]

int gradient1 [ ] [3] =
{
  //G    R    B
  {  225 , 251 , 14  },
  { 226 , 245 , 14  },
  { 226 , 239 , 13  },
  { 227 , 233 , 13  },
  { 228 , 227 , 13  },
  { 229 , 220 , 12  },
  { 229 , 214 , 12  },
  { 230 , 208 , 12  },
  { 231 , 202 , 11  },
  { 232 , 196 , 11  },
  { 232 , 190 , 11  },
  { 233 , 184 , 10  },
  { 234 , 178 , 10  },
  { 235 , 171 , 10  },
  { 235 , 165 , 9 },
  { 236 , 159 , 9 },
  { 237 , 153 , 9 },
  { 237 , 147 , 8 },
  { 238 , 141 , 8 },
  { 239 , 135 , 8 },
  { 240 , 129 , 7 },
  { 240 , 122 , 7 },
  { 241 , 116 , 6 },
  { 242 , 110 , 6 },
  { 243 , 104 , 6 },
  { 243 , 98  , 5 },
  { 244 , 92  , 5 },
  { 245 , 86  , 5 },
  { 245 , 80  , 4 },
  { 246 , 73  , 4 },
  { 247 , 67  , 4 },
  { 248 , 61  , 3 },
  { 248 , 55  , 3 },
  { 249 , 49  , 3 },
  { 250 , 43  , 2 },
  { 251 , 37  , 2 },
  { 251 , 31  , 2 },
  { 252 , 24  , 1 },
  { 253 , 18  , 1 },
  { 254 , 12  , 1 },
  { 254 , 6 , 0 },
  { 255 , 0 , 0 },
  { 255 , 0 , 6 },
  { 255 , 0 , 12  },
  { 255 , 1 , 18  },
  { 255 , 1 , 24  },
  { 255 , 1 , 31  },
  { 255 , 1 , 37  },
  { 255 , 2 , 43  },
  { 255 , 2 , 49  },
  { 255 , 2 , 55  },
  { 255 , 2 , 61  },
  { 254 , 2 , 67  },
  { 254 , 3 , 73  },
  { 254 , 3 , 80  },
  { 254 , 3 , 86  },
  { 254 , 3 , 92  },
  { 254 , 4 , 98  },
  { 254 , 4 , 104 },
  { 254 , 4 , 110 },
  { 254 , 4 , 116 },
  { 254 , 4 , 122 },
  { 254 , 5 , 129 },
  { 254 , 5 , 135 },
  { 254 , 5 , 141 },
  { 254 , 5 , 147 },
  { 254 , 5 , 153 },
  { 254 , 6 , 159 },
  { 254 , 6 , 165 },
  { 254 , 6 , 171 },
  { 254 , 6 , 178 },
  { 254 , 7 , 184 },
  { 253 , 7 , 190 },
  { 253 , 7 , 196 },
  { 253 , 7 , 202 },
  { 253 , 7 , 208 },
  { 253 , 8 , 214 },
  { 253 , 8 , 220 },
  { 253 , 8 , 227 },
  { 253 , 8 , 233 },
  { 253 , 9 , 239 },
  { 253 , 9 , 245 }
};


// gradient2 [Orange --> Red --> Pink]

int gradient2 [ ] [3] =
{
  {  145 , 253 , 11  },
  { 141 , 253 , 11  },
  { 138 , 253 , 10  },
  { 134 , 253 , 10  },
  { 131 , 253 , 10  },
  { 127 , 253 , 10  },
  { 124 , 253 , 9 },
  { 120 , 253 , 9 },
  { 117 , 253 , 9 },
  { 113 , 253 , 9 },
  { 110 , 253 , 8 },
  { 106 , 254 , 8 },
  { 103 , 254 , 8 },
  { 99  , 254 , 8 },
  { 95  , 254 , 7 },
  { 92  , 254 , 7 },
  { 88  , 254 , 7 },
  { 85  , 254 , 6 },
  { 81  , 254 , 6 },
  { 78  , 254 , 6 },
  { 74  , 254 , 6 },
  { 71  , 254 , 5 },
  { 67  , 254 , 5 },
  { 64  , 254 , 5 },
  { 60  , 254 , 5 },
  { 57  , 254 , 4 },
  { 53  , 254 , 4 },
  { 50  , 254 , 4 },
  { 46  , 254 , 3 },
  { 42  , 254 , 3 },
  { 39  , 254 , 3 },
  { 35  , 255 , 3 },
  { 32  , 255 , 2 },
  { 28  , 255 , 2 },
  { 25  , 255 , 2 },
  { 21  , 255 , 2 },
  { 18  , 255 , 1 },
  { 14  , 255 , 1 },
  { 11  , 255 , 1 },
  { 7 , 255 , 1 },
  { 4 , 255 , 0 },
  { 0 , 255 , 0 },
  { 0 , 255 , 6 },
  { 0 , 255 , 12  },
  { 0 , 255 , 18  },
  { 0 , 254 , 24  },
  { 1 , 254 , 29  },
  { 1 , 254 , 35  },
  { 1 , 254 , 41  },
  { 1 , 254 , 47  },
  { 1 , 254 , 53  },
  { 1 , 254 , 59  },
  { 1 , 253 , 65  },
  { 1 , 253 , 71  },
  { 2 , 253 , 76  },
  { 2 , 253 , 82  },
  { 2 , 253 , 88  },
  { 2 , 253 , 94  },
  { 2 , 253 , 100 },
  { 2 , 252 , 106 },
  { 2 , 252 , 112 },
  { 2 , 252 , 118 },
  { 3 , 252 , 123 },
  { 3 , 252 , 129 },
  { 3 , 252 , 135 },
  { 3 , 251 , 141 },
  { 3 , 251 , 147 },
  { 3 , 251 , 153 },
  { 3 , 251 , 159 },
  { 3 , 251 , 165 },
  { 4 , 251 , 170 },
  { 4 , 251 , 176 },
  { 4 , 250 , 182 },
  { 4 , 250 , 188 },
  { 4 , 250 , 194 },
  { 4 , 250 , 200 },
  { 4 , 250 , 206 },
  { 4 , 250 , 212 },
  { 5 , 250 , 217 },
  { 5 , 249 , 223 },
  { 5 , 249 , 229 },
  { 5 , 249 , 235 }
};

// ......................................................................................................................................................


int patNum = 0;     //Counter for pattern sequence loop.

/*
  Sequence Key:

  0. solid color, White
  1. solid color, Blue

  2. gradient 1  [Yellow to Green to Blue]
  3  gradient 2  [Orange to Red to Pink]
  4. stripes
  5. checker
  6. Rainbow
*/


// Color Constants (Sequence: GRB) [Rev 2A8] :

uint32_t Wht = 0x7F7F7F;
uint32_t Blu = 0x0000FF;
uint32_t Grn = 0xFF0000;
uint32_t Red = 0x00FF00;
uint32_t Violet = 0x00FFC0;   // [Rev Demo  1E] Artist's request.

uint32_t color [5] = {Wht, Blu, Grn, Violet, 0} ;     // White, Blue, Green, Red, rainBow  [B/C Beta uses only White and Blue.]

int colorOffset = 0;

int risingEdge = 0;

int motorPin = A21;      // <<< MOTOR PIN for Teensy 3.5 only, true analog output.
// int motorPin = 12;          // Test Motor Pin for Arduino Mega 2560.  [Rev 3A]


void setup() {        /* =============================================== Setup =============================================== */

  Serial.begin(9600);
  pinMode(2, OUTPUT);

  strip.begin(); // Initialize pins for output
  strip.clear( );
  strip.setBrightness(brightVal);                          // [Rev 2B3]  Global strip brightenss  (0 = Off  255 = Max)

  strip.show();  // Turn all LEDs off.

}


void loop() {        /* =============================================== Main Loop ============================================= */

  // Teensy Operational Voltage Readings:
  // Gain Adj for Wind Sensor. Max raw output is ~2V.
  // IDLE windSenVal is 400 - 415
  // MAX windSenVal (strong breath) is about 550.  Occasional peaks to 650 ~ 700 but rare.
  // With offset and no gain, range is then: 0 = 150



  windSenVal = analogRead(analogInPin);     // Raw wind sensor output.

  // ....................................... Determine LED WindVal ...........................................................

  adjLedWindVal = windSenVal - 450;                  // [Demo 1E] Offset updated empirically. True number when no breath is now 0.

  if (adjLedWindVal < 0)  adjLedWindVal = 0;         // PREVENTS ANYTHING FROM GOING BELOW ZERO

  adjLedWindVal = adjLedWindVal * 5.5;               // Gain. This peaks at about 700 with nominal max breath velocity.
  //                                                           Updated to Gain of 5.5 empirically.

  if (adjLedWindVal > 1000) adjLedWindVal = 1000;    // Limit to 1000 (Analog input range is 0 - 1024)

  // ......................................... Determine Fan WindVal .........................................................

  adjFanWindVal = windSenVal - 450;                  // [Demo 1G] Separate Gain and Offset for motor.

  if (adjFanWindVal < 0)  adjFanWindVal = 0;         //Prevents negative values.

  adjFanWindVal = adjFanWindVal * 5.5;               // Gain. This peaks at about 700 with nominal max breath velocity.
  //                                                     Updated to Gain of 5.5 empirically.

  if (adjFanWindVal > 1000) adjFanWindVal = 1000;    // Limit to 1000 (Analog input range is 0 - 1024)


  // -------------------------------------- [Rev 3A] Motor control output gain. -----------------------

  fanVal = (adjFanWindVal / 58) + 108;
  //

  if (fanVal > 123) fanVal = 123;  //  [Rev 3A]  Motor output is 8 Bit.  Limit motor output to less than 4 volts; e.g. 3.8V.


  // =========================== Test for Breath Presence [Rev2A8] / Track Breath Duration [Beta 2A] ======================================


  if (adjLedWindVal > 125) {           // Minimum starting velocity for breath.   Default = 125

    breathpatDwell = millis() - prevBreathMillis;

    deflateDwell = 0;                      // [Beta 2C2]
    prevDeflateMillis = millis();


    analogWrite(motorPin, fanVal);     // [Beta 2A] If breath present, set fan speed. Moved here to group all breath present actions.


    // =========================== Set Disk Radius, Proportional to Breath Velocity or Duration. [Beta 2A] ==========================

    if (RadiusMode == 0) {      // [Beta 2A]                 // Then use breath volume for disk radius.

      diskRadius = adjLedWindVal / (1000 / stripPixels);     // Max adjLedWindVal of 1000 / 82 pixels per strip.  [Demo 1G] Separate adjWindVal for LEDs and motor.
      //                                                         Divisor may be trimmed to adjust disk radius breath sensitivity.

      if (diskRadius >= stripPixels) diskRadius = stripPixels; // [Rev Demo 1D] Don't allow diskRadius to exceed pixels per strip.


    } else {                                                  // Then use breath duration for disk radius.

      diskRadius = stripPixels * breathpatDwell / maxBreath;  // [Beta 2A]

      if (diskRadius <= 0) diskRadius = 1;

      if (diskRadius >= stripPixels) diskRadius = stripPixels; // Don't allow diskRadius to exceed pixels per strip -1 (0 start index).

    }

    //  ======================================  Fill With Selected Fill Pattern =================================


    strip.clear( );


    switch (patNum) {

      // solidColor White

      case 0:


        strip.fill (Wht, 0, diskRadius);
        strip.fill (Wht, stripPixels, diskRadius);
        strip.fill (Wht, (2 * stripPixels), diskRadius);
        strip.fill (Wht, (3 * stripPixels), diskRadius);

        //      strip.show();                     // Refresh strip

        break;


      // solidColor Blue

      case 1:

        strip.fill (Blu,  0, diskRadius);
        strip.fill (Blu, stripPixels, diskRadius);
        strip.fill (Blu, (2 * stripPixels), diskRadius);
        strip.fill (Blu, (3 * stripPixels), diskRadius);

        //      strip.show();                     // Refresh strip

        break;


      // gradient1
      case 2:

        // First Strip:

        j = 0;
        for (i = 0; i < diskRadius; i++) {
          strip.setPixelColor(i, gradient1[i][0], gradient1[i][1], gradient1[i][2]);
        }

        // Second Strip:
        for (i = stripPixels; i < (stripPixels + diskRadius); i++) {

          j = i - stripPixels;
          strip.setPixelColor(i, gradient1[j][0], gradient1[j][1], gradient1[j][2]);

        }

        //Third Strip:
        for (i = (2 * stripPixels); i < ((2 * stripPixels) + diskRadius); i++) {

          j = i - (2 * stripPixels);
          strip.setPixelColor(i, gradient1[j][0], gradient1[j][1], gradient1[j][2]);

        }

        // Fourth Strip:
        for (i = (3 * stripPixels); i < ((3 * stripPixels) + diskRadius); i++) {

          j = i - (3 * stripPixels);
          strip.setPixelColor(i, gradient1[j][0], gradient1[j][1], gradient1[j][2]);
        }


        break;


      // gradient2
      case 3:


        // First Strip:
        j = 0;
        for (i = 0; i < diskRadius; i++) {
          strip.setPixelColor(i, gradient2[i][0], gradient2[i][1], gradient2[i][2]);

        }

        // Second Strip:
        for (i = stripPixels; i < (stripPixels + diskRadius); i++) {

          j = i - stripPixels;
          strip.setPixelColor(i, gradient2[j][0], gradient2[j][1], gradient2[j][2]);

        }

        //Third Strip:
        for (i = (2 * stripPixels); i < ((2 * stripPixels) + diskRadius); i++) {

          j = i - (2 * stripPixels);
          strip.setPixelColor(i, gradient2[j][0], gradient2[j][1], gradient2[j][2]);

        }

        // Fourth Strip:
        for (i = (3 * stripPixels); i < ((3 * stripPixels) + diskRadius); i++) {

          j = i - (3 * stripPixels);
          strip.setPixelColor(i, gradient2[j][0], gradient2[j][1], gradient2[j][2]);

        }

        break;


      // stripeFill
      case 4:

        // First Strip:
        j = 0;
        for (i = 0; i < diskRadius; i++) {

          strip.setPixelColor(i, stripeFill[i][0], stripeFill[i][1], stripeFill[i][2]);

        }

        // Second Strip:
        for (i = stripPixels; i < (stripPixels + diskRadius); i++) {

          j = i - stripPixels;
          strip.setPixelColor(i, stripeFill[j][0], stripeFill[j][1], stripeFill[j][2]);

        }

        //Third Strip:
        for (i = (2 * stripPixels); i < ((2 * stripPixels) + diskRadius); i++) {

          j = i - (2 * stripPixels);
          strip.setPixelColor(i, stripeFill[j][0], stripeFill[j][1], stripeFill[j][2]);

        }

        // Fourth Strip:
        for (i = (3 * stripPixels); i < ((3 * stripPixels) + diskRadius); i++) {

          j = i - (3 * stripPixels);
          strip.setPixelColor(i, stripeFill[j][0], stripeFill[j][1], stripeFill[j][2]);

        }

        break;


      // Checker
      case 5:


        // First Strip:
        j = 0;

        for (i = 0; i < diskRadius; i++) {
          strip.setPixelColor(i, checkers1[i][0], checkers1[i][1], checkers1[i][2]);

        }

        // Second Strip:
        for (i = stripPixels; i < (stripPixels + diskRadius); i++) {

          j = i - stripPixels;
          strip.setPixelColor(i, checkers1[j][0], checkers1[j][1], checkers1[j][2]);

        }

        //Third Strip:
        for (i = (2 * stripPixels); i < ((2 * stripPixels) + diskRadius); i++) {

          j = i - (2 * stripPixels);
          strip.setPixelColor(i, checkers1[j][0], checkers1[j][1], checkers1[j][2]);

        }

        // Fourth Strip:
        for (i = (3 * stripPixels); i < ((3 * stripPixels) + diskRadius); i++) {

          j = i - (3 * stripPixels);
          strip.setPixelColor(i, checkers1[j][0], checkers1[j][1], checkers1[j][2]);

        }

        break;


      // Rainbow
      case 6:

        // First Strip:
        j = 0;

        for (i = 0; i < diskRadius; i++) {
          strip.setPixelColor(i, rainBow[i][0], rainBow[i][1], rainBow[i][2]);

        }

        // Second Strip:
        for (i = stripPixels; i < (stripPixels + diskRadius); i++) {

          j = i - stripPixels;
          strip.setPixelColor(i, rainBow[j][0], rainBow[j][1], rainBow[j][2]);

        }

        //Third Strip:
        for (i = (2 * stripPixels); i < ((2 * stripPixels) + diskRadius); i++) {

          j = i - (2 * stripPixels);
          strip.setPixelColor(i, rainBow[j][0], rainBow[j][1], rainBow[j][2]);

        }

        // Fourth Strip:
        for (i = (3 * stripPixels); i < ((3 * stripPixels) + diskRadius); i++) {

          j = i - (3 * stripPixels);
          strip.setPixelColor(i, rainBow[j][0], rainBow[j][1], rainBow[j][2]);
        }
        break;

    }

    strip.show();                     // Refresh strip

    risingEdge = 1;                  // Reset risingEdge flag for next breath.


    // User may disable auto sequencing by setting autoSeq to 0.  When autoSeq is 0 the next pattern is selected only when breath stops.

    if ((millis() > timeNow + patDwell) && (autoSeq >= 1)) {   //  [LedTst 4C] [Beta 1A] If patDwell has elapsed AND autoSeq = 1 then patterns auto sequence at interval set by 'patDwell'.
      
      timeNow = millis();

      if (risingEdge == 1) {    // Test risingEdge flag. (Run this block only once per continuous breath.)

        risingEdge = 0;         // Prevent this block from running until next breath.

        //  if (colorOffset++ >= 2)  colorOffset = 0;  // If color shift has rolled over, then start sequence again.

        if (patNum++ >= 6) patNum = 0; // <<<---- [LedTst 4C]  Increment to the next pattern in the sequence.

      }

    }

  } else {                     //  ========== IF no breath present, then clear all pixels. ==========


    deflateDwell = millis() - prevDeflateMillis;  //  [Beta 2C2]

    fanVal = 0;                          // [Rev Demo 1E] Fan always stops when breath not present.
    analogWrite(motorPin, fanVal);

    breathpatDwell = 0;                      //  [Beta 2A] Clear breath duration if there is no breath.
    prevBreathMillis = millis();

    /********************************************************************** [Beta 2C] Taper Deflate **************************************************************/



    if (deflateMode == 0) {      // [Beta 2C]                 // Then turn off all LEDs at once.

      diskRadius = 0;
      strip.clear();

    } else if  (diskRadius > 1) {                    // Then taper down disk radius.

      deflateTrim = stripPixels * deflateDwell / maxDeflate;

      if (deflateTrim <= 0) deflateTrim = 1;

      if (deflateTrim >= 29) deflateTrim = 29;

      diskRadius = stripPixels - deflateTrim;

   
      strip.fill (0, diskRadius, deflateTrim);
      strip.fill (0, stripPixels + diskRadius, deflateTrim);
      strip.fill (0, (2 * stripPixels) + diskRadius, deflateTrim);
      strip.fill (0, (3 * stripPixels) + diskRadius, deflateTrim);


    } else {


      diskRadius = 1;
      strip.clear( );

      deflateTrim = 1;

    }

    // ............................................................................................................................................................

    strip.show();


    /************************************************************************[END: Taper Deflate ************************************************************************/


    if (autoSeq == 0) { //  [Beta 1A] If autoSeq = 1 then auto sequences patterns at interval set by 'patDwell'.

      if (risingEdge == 1) {    // Test risingEdge flag. (Run this block only once per continuous breath.)

        risingEdge = 0;         // Prevent this block from running until next breath.

        if (patNum++ >= 6) patNum = 0; // <<<---- [LedTst 4C]  Increment to the next pattern in the sequence.

      }
    }
  }
}
