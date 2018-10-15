#include <FastLED.h>
#include <LiquidCrystal.h>
#include <Servo.h>

/*debug start*/
boolean g_bDebugMode = false;
/*debug end*/

/*Laser tripwire start*/
#define LASER_PIN      22                                                  //Dxx--->LASERMOD.S
#define LDR_PWR       23                                                  //Dxx--->LDR.++
#define LDR_SENSE 0
#define MA_WIN_LSR 3
#define LASER_LDR_READ_DELAY 1 //ms
int ldrReading[MA_WIN_LSR] = {0};
int totalldrReading = 0;
boolean bLaserOn = false;
unsigned long currLaserMillis = 0;
unsigned long prevLaserMillis = 0;
#define LASER_TRIPWIRE_FREQ_LOW                        17000
#define LASER_TRIPWIRE_FREQ_HIGH                        19000
#define LASER_TRIPWIRE_PERIOD_LOW                    5000
#define LASER_TRIPWIRE_PERIOD_HIGH                    7000
#define LASER_THRESH                                                  980 //may need to be changed. Ensure immovable positions for the laser and ldr else this fails!
/*Laser tripwire end*/

/*Vib Sensor's ISR related start*/
//Since this is for the Mega 2560 R3, here are the digital pins servicing interrupts - 2, 3, 18, 19, 20, 21
#define VIB_SNSR_HIT_FREQ_BUFFER 500 //register points only after this period between successive hits (ISR invocations)
#define VIB_SNSR3_HIT_FREQ_BUFFER 750 //seperate timeout as it seems more sensitive
boolean bInterruptsInitialized = false;

#define VIBSNSR1_PIN  2 //input of vib sensor connected to D2
#define VIBSNSR1_PWR 24
#define VIBSNSR1_LED 12
volatile boolean bVibSnsr1Trig = false;
unsigned long last_vib1_hit_time = 0;

#define VIBSNSR2_PIN  3 //input of vib sensor connected to D3
#define VIBSNSR2_PWR 25
#define VIBSNSR2_LED 13
volatile boolean bVibSnsr2Trig = false;
unsigned long last_vib2_hit_time = 0;

#define VIBSNSR3_PIN  18 //ball drop collector
#define VIBSNSR3_PWR 26
volatile boolean bVibSnsr3Trig = false;
unsigned long last_vib3_hit_time = 0;

#define VIBSNSR4_PIN  19
#define VIBSNSR4_PWR 40
#define VIBSNSR4_LED 14
volatile boolean bVibSnsr4Trig = false;
unsigned long last_vib4_hit_time = 0;
/*Vib Sensor's ISR related end*/

/*LED strip start*/
#define LED_PIN                                                    27
#define NUM_LEDS                                                30
#define BRIGHTNESS                                            20
CRGBArray<NUM_LEDS> leds;
/*LED strip end*/

/*tones start*/
#define TONE_PIN 4
#define ALERT_DURATION 200
const int f_incr = 200;//349; //points added!
const int cH_decr = 225;//523; //points subtracted!
const int gSH_ball_lost = 280;//830;
const int a = 440;
const int f = 349;
const int cH = 523;
/*tones end*/

/*LCD start*/
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 28, en = 30, d4 = 32, d5 = 34, d6 = 36, d7 = 38;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
/*LCD end*/

/*Extra ball start*/
#define EXTRA_BALL_SERVO_PIN 10
//const int EXTRA_BALL_REL_TIME_LOW = 30000;
//const int EXTRA_BALL_REL_TIME_HIGH = 35000;
long randRelTime = 0;
const int EXTRA_BALL_GATE_CLOSED_ANGLE = 140;
const int EXTRA_BALL_GATE_OPEN_ANGLE = 180;
const int EXTRA_BALL_SERVO_MOVE_WAIT = 100;
Servo xtraBallServo;
boolean g_bXtraBallReleased = false;
/*Extra ball end*/

/*game lifetime management start*/
boolean g_bStartGame = false;
long g_pointsScored = 0;
unsigned long g_gmTime = 0;
int g_ballsLeft = 0;
const int GAME_SESSION_TIME = 90;//seconds
/*game lifetime management end*/

void setup() {

  Serial.begin(9600);

  randomSeed(analogRead(15));

  //so that people know its booting...
  BootLEDs();

  InitGame();

  Serial.println("Ready.  Start Playing !!");
  Serial.println( "Xtra Ball to be released at time: " + String(randRelTime) );

  g_gmTime = 0;
  g_pointsScored = 0;
  g_ballsLeft = 3;

}

void BootLEDs()
{
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  for (int i = 0; i < (NUM_LEDS); i++)
  {
    leds[i] = CRGB(255, 0, 0); FastLED.show();
  }
  delay(200);
  for (int i = 0; i < (NUM_LEDS); i++)
  {
    leds[i] = CRGB(0, 255, 0); FastLED.show();
  }
  delay(200);
  for (int i = 0; i < (NUM_LEDS); i++)
  {
    leds[i] = CRGB(0, 0, 255); FastLED.show();
  }

  delay(300);
}

void beep(int note, int duration)
{
  tone(TONE_PIN, note, duration);
  delay(duration);
  noTone(TONE_PIN);
  delay(50);
}

void InitGame()
{
  pinMode(LASER_PIN, OUTPUT);
  pinMode(LDR_PWR, OUTPUT);
  pinMode(LDR_SENSE, INPUT);
  pinMode(VIBSNSR3_PWR, OUTPUT);
  digitalWrite(VIBSNSR3_PWR, HIGH);
  pinMode(VIBSNSR4_PWR, OUTPUT);
  digitalWrite(VIBSNSR4_PWR, HIGH);
  pinMode(VIBSNSR1_PWR, OUTPUT);
  digitalWrite(VIBSNSR1_PWR, HIGH);
  pinMode(VIBSNSR2_PWR, OUTPUT);
  digitalWrite(VIBSNSR2_PWR, HIGH);
  pinMode(TONE_PIN, OUTPUT);
  digitalWrite(LDR_PWR, HIGH);//todo - later move to high only when laser trip on

  //SetupExtHWInterrupts();

  SetupBatmobileLauncher();

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("BATMAN!");
  lcd.setCursor(0, 1);
  lcd.print("Mini Pinball!!");

  //some music
  RunStartMusic();

  g_bStartGame = true;
  GameStartLEDAnim();

  lcd.clear();

  SetFloatingPins();

  GenRandomXtraBallReleaseTime();
}

void RunStartMusic()
{
  noTone(TONE_PIN);
  beep(a, 1);
  beep(a, 1);
  beep(a, 1);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 500);
  beep(f, 350);
  beep(cH, 150);
  beep(a, 650);
}

void SetupBatmobileLauncher()
{
  xtraBallServo.attach(EXTRA_BALL_SERVO_PIN);
  xtraBallServo.write(EXTRA_BALL_GATE_CLOSED_ANGLE);
  delay(EXTRA_BALL_SERVO_MOVE_WAIT);
  xtraBallServo.detach();
  g_bXtraBallReleased = false;

}

void GenRandomXtraBallReleaseTime()
{
  randRelTime = random(GAME_SESSION_TIME - 75, GAME_SESSION_TIME - 15);
}

void SetFloatingPins()
{
  pinMode(VIBSNSR3_PIN, INPUT_PULLUP);
  pinMode(VIBSNSR1_PIN, INPUT_PULLUP);
  pinMode(VIBSNSR2_PIN, INPUT_PULLUP);
  pinMode(VIBSNSR4_PIN, INPUT_PULLUP);
}

void GameStartLEDAnim()
{
  /*for (int i = 0; i < NUM_LEDS; i++)
    {
    leds[i] = CRGB::HotPink; FastLED.show(); delay(30);
    leds[i + 1] = CRGB::Maroon; FastLED.show(); delay(30);
    leds[i + 2] = CRGB::Aqua; FastLED.show(); delay(30);
    leds[i + 3] = CRGB::Gray; FastLED.show(); delay(30);
    }

    for (int i = NUM_LEDS; i > 3; i--)
    {
    leds[i] = CRGB::Yellow; FastLED.show(); delay(30);
    leds[i - 1] = CRGB::Lime; FastLED.show(); delay(30);
    leds[i - 2] = CRGB::Teal; FastLED.show(); delay(30);
    leds[i - 3] = CRGB::Purple; FastLED.show(); delay(30);
    }

    delay(500);
  */

  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0, 0, 0); FastLED.show(); delay(30);
  }
}

/*
  From - http://gammon.com.au/interrupts
  TL;DR :
  When writing an Interrupt Service Routine (ISR):

  Keep it short
  Don't use delay ()
  Don't do serial prints
  Make variables shared with the main code volatile
  Variables shared with main code may need to be protected by "critical sections" (see below)
  Don't try to turn interrupts off or on
*/
void SetupExtHWInterrupts()
{
  //clear the interrupts and set them
  EIFR = (1 << INTF3);
  attachInterrupt( digitalPinToInterrupt(VIBSNSR3_PIN), VibSnsr3Trig, LOW );
  EIFR = (1 << INTF4);
  attachInterrupt( digitalPinToInterrupt(VIBSNSR1_PIN), VibSnsr1Trig, LOW );
  EIFR = (1 << INTF5);
  attachInterrupt( digitalPinToInterrupt(VIBSNSR2_PIN), VibSnsr2Trig, LOW );
  EIFR = (1 << INTF2);
  attachInterrupt( digitalPinToInterrupt(VIBSNSR4_PIN), VibSnsr4Trig, LOW );
}

/*ISR's start*/
void VibSnsr4Trig()
{
  bVibSnsr4Trig = true;
}

void VibSnsr3Trig()
{
  bVibSnsr3Trig = true;
}

void VibSnsr2Trig()
{
  bVibSnsr2Trig = true;
}

void VibSnsr1Trig()
{
  bVibSnsr1Trig = true;
}
/*ISR's end*/

void MngVibSnsr3Hit()
{
  unsigned long vib3_curr_hit_tm = millis();
  if ( ( vib3_curr_hit_tm - last_vib3_hit_time) <= VIB_SNSR3_HIT_FREQ_BUFFER)
  {
    last_vib3_hit_time = vib3_curr_hit_tm;
    return;
  }
  else
  {
    //Flash relevant LED
    for (int i = 10; i < 19; i++)//@todo - reduce # of LEDs if game is sluggish
    {
      leds[i] = CRGB(255, 0, 0); FastLED.show();
    }

    //sound ball lost alert
    BallLostAlert();

    DbgPrint("BALL LOST!");
    last_vib3_hit_time = vib3_curr_hit_tm;

    //display update on LCD
    g_ballsLeft --;
    if (g_ballsLeft <= 0)
    {
      g_ballsLeft = 0;
      //@todo - game over
    }

    return;
  }
}

void MngVibSnsr4Hit()
{
  //check if there was a hit immediately before this...
  unsigned long vib4_curr_hit_tm = millis();
  if ( ( vib4_curr_hit_tm - last_vib4_hit_time) <= VIB_SNSR_HIT_FREQ_BUFFER)
  {
    last_vib4_hit_time = vib4_curr_hit_tm;
    return;
  }
  else
  {
    //Flash relevant LED
    leds[VIBSNSR4_LED] = CRGB(0, 0, 255);
    FastLED.show();

    //update points
    g_pointsScored += 150;

    //sound incr alert
    IncrementAlert();

    DbgPrint("hit vib4!");
    last_vib4_hit_time = vib4_curr_hit_tm;
    return;
  }
}

void MngVibSnsr2Hit()
{
  //check if there was a hit immediately before this...
  unsigned long vib2_curr_hit_tm = millis();
  if ( ( vib2_curr_hit_tm - last_vib2_hit_time) <= VIB_SNSR_HIT_FREQ_BUFFER)
  {
    last_vib2_hit_time = vib2_curr_hit_tm;
    return;
  }
  else
  {
    //Flash relevant LED
    leds[VIBSNSR2_LED] = CRGB(0, 0, 255);
    FastLED.show();

    //update points
    g_pointsScored += 120;

    //sound incr alert
    IncrementAlert();

    DbgPrint("hit vib2!");
    last_vib2_hit_time = vib2_curr_hit_tm;
    return;
  }
}

void MngVibSnsr1Hit()
{
  //check if there was a hit immediately before this...
  unsigned long vib1_curr_hit_tm = millis();
  if ( ( vib1_curr_hit_tm - last_vib1_hit_time) <= VIB_SNSR_HIT_FREQ_BUFFER)
  {
    last_vib1_hit_time = vib1_curr_hit_tm;
    return;
  }
  else
  {
    //Flash relevant LED
    leds[VIBSNSR1_LED] = CRGB(0, 0, 255);
    FastLED.show();

    //update points
    g_pointsScored += 100;

    //sound incr alert
    IncrementAlert();

    DbgPrint("hit vib1!");
    last_vib1_hit_time = vib1_curr_hit_tm;
    return;
  }
}

/*check all the vib sensors trigger states*/
void ManageTriggeredVibSnsrs()
{

  //crit section start - save all shared volatiles to locals here
  byte oldSREG = SREG; //save current interrupt state

  noInterrupts(); //disable all interrupts so that it doesnt interfere with shared var access
  boolean bVibS1 = bVibSnsr1Trig;
  boolean bVibS2 = bVibSnsr2Trig;
  boolean bVibS3 = bVibSnsr3Trig;
  boolean bVibS4 = bVibSnsr4Trig;
  bVibSnsr1Trig = false;
  bVibSnsr2Trig = false;
  bVibSnsr3Trig = false;
  bVibSnsr4Trig = false;

  SREG = oldSREG; //restore interrupt state
  //interrupts();
  //crit section end

  /*vib sensor 1 start*/
  if (bVibS1)
  {
    MngVibSnsr1Hit();
  }
  else
  {
    //turn off LED for vib1snsr
    leds[VIBSNSR1_LED] = CRGB(0, 0, 0);
    FastLED.show();
  }
  /*vib sensor 1 end*/

  /*vib sensor 2 start*/
  if (bVibS2)
  {
    MngVibSnsr2Hit();
  }
  else
  {
    //turn off LED for vib2snsr
    leds[VIBSNSR2_LED] = CRGB(0, 0, 0);
    FastLED.show();
  }
  /*vib sensor 2 end*/

  /*vib sensor 3 start*/
  if (bVibS3)
  {
    MngVibSnsr3Hit();
  }
  else
  {
    //turn off LED for vib3snsr
    for (int i = 10; i < 19; i++)
    {
      leds[i] = CRGB(0, 0, 0); FastLED.show();
    }
  }
  /*vib sensor 3 end*/

  if (bVibS4)
  {
    MngVibSnsr4Hit();
  }
  else
  {
    //turn off LED for vib4snsr
    leds[VIBSNSR4_LED] = CRGB(0, 0, 0);
    FastLED.show();
  }

}

void DbgPrint(String str)
{
  if (g_bDebugMode)
    Serial.println(str);
}

void BallLostAlert()
{
  noTone(TONE_PIN);
  beep(gSH_ball_lost, ALERT_DURATION);
  beep(gSH_ball_lost, ALERT_DURATION);
  //tone(TONE_PIN, gSH_ball_lost, ALERT_DURATION * 2);
}

void DecrementAlert()
{
  //sound off a note noting the decrement of points
  noTone(TONE_PIN);
  beep(cH_decr, ALERT_DURATION);
  //tone(TONE_PIN, cH_decr, ALERT_DURATION);
}

void IncrementAlert()
{
  //sound off a note noting the increment of points
  noTone(TONE_PIN);
  //beep(f_incr, ALERT_DURATION);
  tone(TONE_PIN, f_incr, ALERT_DURATION);
}

void GameOverAlert()
{
  noTone(TONE_PIN);
  beep(gSH_ball_lost, ALERT_DURATION);
  beep(cH_decr, ALERT_DURATION);
  beep(cH_decr, ALERT_DURATION);
  beep(cH_decr, ALERT_DURATION);
  beep(gSH_ball_lost, ALERT_DURATION);
  noTone(TONE_PIN);
}

void UpdateLCD()
{
  if ( ( (g_gmTime / 1000) >= GAME_SESSION_TIME) || (g_ballsLeft == 0) )
  {
    //@todo - game over processing
    lcd.clear();
    lcd.home();
    lcd.print("GAME OVER!!");
    long score = g_pointsScored;
    lcd.setCursor(0, 1);
    lcd.print("TOTSCR: " + String(score));
    //GameOverAlert();
    g_bStartGame = false;
  }
  else
  {
    UpdatePointsLCD();
    UpdateGameTimeBallsLCD();
  }

}

//@todo - sometimes the score >1000 retains its trailing zero which temporarily
//leads to a wrong score being displayed. Need to correct it somehow.
void UpdatePointsLCD()
{
  //display latest score on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  String strScore(g_pointsScored);
  lcd.print("SCORE: " + strScore);
}

void UpdateGameTimeBallsLCD()
{
  lcd.setCursor(0, 1);

  String strTR(g_gmTime / 1000);
  String strBR(g_ballsLeft);
  lcd.print("TM: " + strTR + " BL: " + strBR);
}

void MiscStateCheck()
{
  g_gmTime = millis(); //http://www.gammon.com.au/millis

  if ( randRelTime <= ( g_gmTime / 1000 ) ) //&& ( g_gmTime <= randRelTime + 3 ) )
  {
    XtraBallRelease();
  }

}

void XtraBallRelease()
{
  if (!g_bXtraBallReleased)
  {
    xtraBallServo.attach(EXTRA_BALL_SERVO_PIN);
    xtraBallServo.write(EXTRA_BALL_GATE_OPEN_ANGLE);
    FlashXtraBallRelLEDs(true);
    delay(EXTRA_BALL_SERVO_MOVE_WAIT * 5); //wait for the xtra ball to roll out
    g_bXtraBallReleased = true;
    xtraBallServo.write(EXTRA_BALL_GATE_CLOSED_ANGLE);
    delay(EXTRA_BALL_SERVO_MOVE_WAIT);
    xtraBallServo.detach();
    g_ballsLeft++;
    g_pointsScored += 300;
    FlashXtraBallRelLEDs(false);
  }

}

void FlashXtraBallRelLEDs(boolean bFlash)
{
  if (bFlash)
  {
    leds[7] = CRGB(217, 25, 255);
    FastLED.show();
    leds[8] = CRGB(217, 25, 255);
    FastLED.show();
    leds[9] = CRGB(217, 25, 255);
    FastLED.show();
    leds[10] = CRGB(217, 25, 255);
    FastLED.show();
  }
  else
  {
    leds[7] = CRGB(0, 0, 0);
    FastLED.show();
    leds[8] = CRGB(0, 0, 0);
    FastLED.show();
    leds[9] = CRGB(0, 0, 0);
    FastLED.show();
    leds[10] = CRGB(0, 0, 0);
    FastLED.show();
  }
}

void Monitor_Laser_Trap()
{

  if (bLaserOn)
  {
    for (int i = 0; i < MA_WIN_LSR; i++)
    {
      analogRead(LDR_SENSE); //first reading is useless
      delay(LASER_LDR_READ_DELAY);
      ldrReading[i] = analogRead(LDR_SENSE);
    }

    for (int x = 0; x < MA_WIN_LSR; x++)
    {
      totalldrReading += ldrReading[x];
    }

    //DbgPrint(String(totalldrReading / MA_WIN_LSR));

    if (totalldrReading / MA_WIN_LSR <= LASER_THRESH)
    {
      //tripped! Decrement points!
      DecrementAlert();
      g_pointsScored -= 20;
      FlashLaserLDRLeds(true);
      DbgPrint(String(totalldrReading / MA_WIN_LSR));
    }
  }
  else
    FlashLaserLDRLeds(false);
  totalldrReading = 0;
}

void FlashLaserLDRLeds(boolean bHit)
{
  if (bHit)
  {
    //red
    leds[4] = CRGB(255, 0, 0);
    FastLED.show();
    leds[5] = CRGB(255, 0, 0);
    FastLED.show();
    leds[24] = CRGB(255, 0, 0);
    FastLED.show();
    leds[23] = CRGB(255, 0, 0);
    FastLED.show();
  }
  else
  {
    //none
    leds[4] = CRGB(0, 0, 0);
    FastLED.show();
    leds[5] = CRGB(0, 0, 0);
    FastLED.show();
    leds[24] = CRGB(0, 0, 0);
    FastLED.show();
    leds[23] = CRGB(0, 0, 0);
    FastLED.show();
  }
}

void Disengage_Laser_TripWire()
{
  digitalWrite(LASER_PIN, LOW);
  bLaserOn = false;
}

void Engage_Laser_TripWire()
{
  digitalWrite(LASER_PIN, HIGH);
  bLaserOn = true;
}

void DoLaserOps()
{
  currLaserMillis = millis();

  if (!bLaserOn)
  {
    unsigned long laserFreq = currLaserMillis - prevLaserMillis;
    if ( ( (LASER_TRIPWIRE_FREQ_LOW) <= ( laserFreq ) ) &&  ( ( laserFreq ) <= (LASER_TRIPWIRE_FREQ_HIGH) ) )
    {
      DbgPrint("Engaging laser!");
      //digitalWrite(LDR_PWR, HIGH); //@todo - see if the quick succession of LDR PWR then laser on causes a stray reading? If yes, move this elsewhere
      Engage_Laser_TripWire();
      prevLaserMillis = currLaserMillis;
    }
  }

  if (bLaserOn)
  {
    unsigned long laserFreq = currLaserMillis - prevLaserMillis;
    if ( (LASER_TRIPWIRE_PERIOD_LOW <= laserFreq) && (laserFreq <= LASER_TRIPWIRE_PERIOD_HIGH) )
    {
      DbgPrint("Disengaging laser!");
      //digitalWrite(LDR_PWR, LOW);
      Disengage_Laser_TripWire();
      prevLaserMillis = currLaserMillis;
    }
  }

  Monitor_Laser_Trap();
}

void GameLoop()
{

  ManageTriggeredVibSnsrs();

  if ( !bInterruptsInitialized )
  {
    //do this here to avoid issues (e.g. noise due to the motors etc.)
    //though filtering the circuit is highly recommended
    SetupExtHWInterrupts();
    bInterruptsInitialized = true;
    g_gmTime = 0;
  }

  DoLaserOps();

  UpdateLCD();

  MiscStateCheck();

  //check to see if we're in debug mode
  if (Serial.available() > 0) //something is waiting for us in the Rx buffer. Inspect it.
  {
    String str1 = Serial.readString();
    if ( str1 == "DBG" )
    {
      g_bDebugMode = true;
      Serial.println("DEBUG MODE ON!");
    }
    if ( str1 == "NODBG")
    {
      g_bDebugMode = false;
      Serial.println("DEBUG MODE OFF!");
    }
  }

}

void loop() {

  if (g_bStartGame)
  {
    GameLoop();
  }
  else
  {
    //run demo mode
  }

}

