unsigned long TimeStart = millis() ;
unsigned long TimeNow ;


int configuration[11] = {
							0,		// 0 Language - 0 Deutsch, 1 English
							0,		// 1 Unit - 0 Celsuis, 1 Kelvin, 2 Fahrenheit
							1,		// 2 Poti direction 0 or 1
							64,		// 3 Display Brightness Normal 0 - 255
							10,		// 4 Display Brightness StandBy 0 - 255
							900,	// 5 Time to tempdrop
							900,	// 6 Time from tempdrop to Buzzer
							255,	// 7 MaxPWM 0 - 255
							450,	// 8 Maximum Temp
							255,	// 9 StatusLEDBrightnes - Soldering
							50		// 10 StatusLEDBrightnes - StandBy
} ;


unsigned long IntervallArray[][2] = {

									{0, 250},	// 0 Potitime
									{0, 199},	// 1 TFTSolderscreen
									{0, 200},	// 2 SetStandBy
									{0, 1000},	// 3 SerialOutput
									{0, 50},	// 4 Buttonstates
									{0, 500},	// 5 Checkled
									{0, 100},	// 6 TFTStandByScreen
									{0, 30},	// 7 getTemperature
									{0, 200},	// 8 set time
									{0, 999},	// 9 StandBy clear
									{0, 1000},	// 10 Buzzertime
									{0, 101},	// 11 WS2812b
									{0, 2001}	// 12 EEprom saveconfig
								};



int PotiDelta = 0 ;
int PotiDeltaPM = 5 ; // Value plus minus
int MaxTemp = 450 ; // Be carefull!

//int MaxPwm = 255 ; //0 - 255
bool HeaterOn = false ;
int HeaterPWM = 0 ;
int HeaterPresets[3] = {330,360,400} ;

int ArrayTemps[3] = {0, 0, 0}; // Current, Target, Delta

int StatusMain = 23 ; // 0 - Solder, 1 - StandBy, 2 - Menu
int StatusTFT = 23 ; // 0 - Solder, 1 - StandBy, 2 - Menu

int TFTOldTemps[10] = {999, 999, 999, 999, 999, 999, 999, 999, 999, 999} ;
int TFTOldButtons[4][2] = {
								{0, 0},
								{0, 0},
								{0, 0},
								{0, 0}
							} ;
int TFToldPresets[9] = {999, 999, 999, 999, 999, 999, 999, 999, 999} ;
int TFToldClock[5] = {0,0,0,0,0} ;

int TimesArray[7] = {0, 0, 0, 0, 0, 0, 0} ; // hour, month, second, day, month, year, temp


int ArrayButtons[4][5] = {
							{0, 0}, //Shortpress, Longpress, lastloop, activetime, startcheck
							{0, 0},
							{0, 0},
							{0, 0}
						} ;

unsigned long StandByArray[3] = {0, 0, 0} ;
int StandByRuntime = 0 ;

int statusledarray[] = {0, 0, 0, 0, 0, 0} ; // 0 - HSVPos


int ButtonValues[4] = {780, 680, 585, 400} ;





#define sclk 			13	// SPI
#define mosi 			11	// SPI
#define cs_tft			10	// SPI
#define dc   			9	// Display
#define rst  			12	// Display 
#define BLpin			5   // DisplayBacklihgt 5-PWM 7-IO

#define LED_PIN 		6	// WS2812b
//#define BRIGHTNESS 		200	// WS2812b
#define NUM_LEDS 		8	// WS2812b

//#define RTCSDA A4		// I2C RTC
//#define RTCSCDL A5	// I2C RTC

#define PiezoPin		7	// Piezo Buzzer 

#define STANDBYin 		A1	// Old A4 -> A1
#define POTI   			A3	// Old A5 -> A3
#define TEMPin 			A7	// IronSensor
#define PWMpin 			3	// IronHeater
#define Analogbuttons 	A6
//#define CheckLED	A0

#define DELAY_MEASURE 		25 //50
//#define ADC_TO_TEMP_GAIN 	0.415
#define ADC_TO_TEMP_GAIN 	0.53 //Mit original Weller Station verglichen
#define ADC_TO_TEMP_OFFSET 25.0
#define STANDBY_TEMP			175

#define PWM_DIV 1024						//default: 64   31250/64 = 2ms







