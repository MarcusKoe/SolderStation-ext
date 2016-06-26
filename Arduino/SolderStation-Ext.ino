
#include <FastLED.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <DS3232RTC.h>    //http://github.com/JChristensen/DS3232RTC
#include "Time.h"
#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <EEPROM.h>

#include "config.h"
#include "conftables.h"


#define VERSION "0.9"		//Version der Steuerung





Adafruit_ST7735 tft = Adafruit_ST7735(cs_tft, dc, rst);  // Invoke custom library

CRGB leds[NUM_LEDS];




// Problem: Configvals are int = 2byte. Eeprom save only 1byte.
// Solution: Split in two*1 bytes. Using nice functions

// Matthias Busse 5.2014 V 1.0
// modified by marcus: using update instead write (save write cycles == lifetime)
void eepromWriteInt(int adr, int wert) {
byte low, high;
  low=wert&0xFF;
  high=(wert>>8)&0xFF;
  EEPROM.update(adr, low); // dauert 3,3ms 
  EEPROM.update(adr+1, high);
  return;
} 

// Matthias Busse 5.2014 V 1.0
int eepromReadInt(int adr) {
byte low, high;
  low=EEPROM.read(adr);
  high=EEPROM.read(adr+1);
  return low + ((high << 8)&0xFF00);
} 





void SaveConfig(){
	//Serial.println("Save config");
	for(int i=0; i<11; i++){
		//Serial.println(i);
	    eepromWriteInt(i*2, configuration[i]) ;
	}
	int c = 0 ;
	for(int i=50; i<53; i++){
	    eepromWriteInt(i*2, HeaterPresets[c]) ;
	    c++;
	}
	eepromWriteInt(76, ArrayTemps[1]) ;
	eepromWriteInt(254, 1337) ;
}



void LoadConfig(){
	int checkval =  eepromReadInt(254) ;
	if (checkval != 1337) {
		Serial.println("Seems like a new Arduino :)");
		Serial.println("Save config first time");
		//delay(10000);
		SaveConfig() ;
	}
	for(int i=0; i<11; i++){
	    configuration[i] = eepromReadInt(i*2) ;
	    //Serial.println(configuration[i]) ;
	}
	int c = 0 ;
	for(int i=50; i<53; i++){
	    HeaterPresets[c] = eepromReadInt(i*2) ;
	    c++ ;
	}
	ArrayTemps[1] = eepromReadInt(76) ;
	// Check if config is valid
	if(configuration[0] < 0 || configuration[0] > 1){
		configuration[0] = 0 ;
		SaveConfig() ;
	} else if(configuration[1] < 0 || configuration[1] > 1){
		configuration[1] = 0 ;
		SaveConfig() ;
	} else if(configuration[2] < 0 || configuration[2] > 1){
		configuration[2] = 0 ;
		SaveConfig() ;
	} else if(configuration[3] < 0 || configuration[3] > 255){
		configuration[3] = 64 ;
		SaveConfig() ;
	} else if(configuration[4] < 0 || configuration[4] > 255){
		configuration[4] = 10 ;
		SaveConfig() ;
	} else if(configuration[5] < 0 || configuration[5] > 32765){
		configuration[5] = 900 ;
		SaveConfig() ;
	} else if(configuration[6] < 0 || configuration[6] > 32765){
		configuration[6] = 900 ;
		SaveConfig() ;
	} else if(configuration[7] < 0 || configuration[7] > 255){
		configuration[7] = 255 ;
		SaveConfig() ;
	} else if (configuration[8] < 0 || configuration[8] > MaxTemp){
		configuration[8] = 400 ;
		SaveConfig() ;
	} else if(configuration[9] < 0 || configuration[9] > 255){
		configuration[9] = 255 ;
		SaveConfig() ;
	} else if(configuration[10] < 0 || configuration[10] > 255){
		configuration[10] = 50 ;
		SaveConfig() ;
	} 	
}





void SetupRTC(){
	//setTime(12, 7, 0, 15, 5, 2016); // Set Time in RTC
    //RTC.set(now());
    //delay(500);
    setSyncProvider(RTC.get);   // get time from the RTC
    setSyncInterval(30007);
	if(timeStatus() != timeSet) {
        Serial.println("Unable to sync with the RTC");
        setTime(12, 58, 10, 22, 12, 2016); 
    } else {
        Serial.println("RTC has set the system time"); 
    }
}





void setup(void) {

	Serial.begin(115200);
	Serial.println("Loeten!");


    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    SetupRTC();


    LoadConfig();




	FastLED.addLeds<WS2812B, LED_PIN,GRB>(leds, NUM_LEDS);
	for(int i=0; i<NUM_LEDS; i++){
       leds[i] = CRGB( 0, 0, 0); 
    }
    FastLED.show() ;
	
	pinMode(BLpin, OUTPUT);
	digitalWrite(BLpin, LOW);
  	analogWrite(BLpin, configuration[3]);

	pinMode(STANDBYin, INPUT_PULLUP);
	
	pinMode(PWMpin, OUTPUT);
	digitalWrite(PWMpin, LOW);
	setPwmFrequency(PWMpin, PWM_DIV);
	digitalWrite(PWMpin, LOW);

	pinMode(rst, OUTPUT);
	digitalWrite(rst, LOW);
	delay(10) ;
	digitalWrite(rst, HIGH);	
	tft.initR(INITR_BLACKTAB);
	SPI.setClockDivider(SPI_CLOCK_DIV4);  // 4MHz

	tft.setRotation(0);	// 0 - Portrait, 1 - Lanscape
	tft.fillScreen(ST7735_BLACK);
	tft.setTextWrap(false);




	pinMode(PiezoPin, OUTPUT) ;
	digitalWrite(PiezoPin, LOW);

	// Start Soldering
	StatusMain = 1 ;

}

void loop() {
	
	TimeNow = millis();
	SetTargetTemp();
	Heating();

	if(intervallcheck(8) == true){
		GetTime() ;
	}
	if(intervallcheck(7) == true){
		getTemperature();
	}

   	if(intervallcheck(2) == true){
		SetStandBy() ;
	}

  	// Solderloop
	if(StatusMain == 0){
  		PresetChange() ;
	}

	if(intervallcheck(1) == true){
		if(StatusMain == 0 || StatusMain == 1){
			TFTwriteHEATING();
		}
	}

	



	GetPotiDelta() ;
	//delay(DELAY_MAIN_LOOP);		//wait for some time
	
	if(intervallcheck(4) == true){
		buttonstates() ;
	}


	if(intervallcheck(11) == true){
		StatusLED() ;
		FastLED.show() ;
	}

	if(intervallcheck(12) == true){
		SaveConfig();
	}

	if(intervallcheck(3) == true){
		//SerialOutput();
	}
	//Serial.println("Loop") ;
}





void StatusLED(){

	if(StatusMain == 0){
		FastLED.setBrightness(configuration[9]);
		for(int i=0; i<NUM_LEDS; i++){
			if(ArrayTemps[2] >= 5){
		    	leds[i] = CRGB( 0, 0, 255); 
			} else if (abs(ArrayTemps[2]) < 5) {
		    	leds[i] = CRGB( 0, 255, 0); 
			} else {
				leds[i] = CRGB( 255, 0, 0); 
			}	
		}
	} else if (StatusMain == 1){
		FastLED.setBrightness(configuration[10]);
		int val = statusledarray[0] ;
		for(int i=0; i<NUM_LEDS; i++){
			leds[i].setHSV( val, 255, 255);	
		}
		if (val == 255) {
			val = -1 ;			
		}
		statusledarray[0] = val + 1 ;
	} else {
		for(int i=0; i<NUM_LEDS; i++){
			leds[i] += CRGB( 0, 0, 0); 	
		}
	}

}





void SetStandBy(){

	boolean StandIn = digitalRead(STANDBYin) ;
	if (StandIn == false) {
		//Serial.println("Input Low") ;
		StandByArray[0] = TimeNow ;
	}
	unsigned long LastFalse = StandByArray[0] ;
	unsigned long DeltaTime = TimeNow - LastFalse ;
	//Serial.println(DeltaTime) ;
	if(DeltaTime < 1000){
		//Serial.println("StandBy Aktiv") ;
		//Serial.println(DeltaTime) ;
		StatusMain = 1 ;
		StandByArray[2] = TimeNow - StandByArray[1];
		StandByRuntime = StandByArray[2] / 1000 ;
		if(StandByRuntime > 32000){
		    StandByArray[1] = TimeNow - 3600000;
		}
	} else {
		StatusMain = 0 ;
		StandByArray[1] = TimeNow ;
		HeaterOn = true ;
	}
} 










void GetTime(){
	TimesArray[0] = hour();
	TimesArray[1] = minute() ;
	TimesArray[2] = second() ;
	// TimesArray[3] = day() ;
	// TimesArray[4] = month() ;
	// TimesArray[5] = year() ;
	// TimesArray[6] = RTC.temperature() ;
}




void buttonstates(){
	unsigned long TimeDelta ;
	unsigned long TimeStart = TimeNow ;
	int whilestatus = 1 ;
	int ResultValue ;
	//Serial.println(" Butt0rnch3ck");
	for(int i=0; i<4; i++){
	    ArrayButtons[i][0] = 0 ;
		ArrayButtons[i][1] = 0 ;
	}

	while(whilestatus == 1){
		int ButtonValue = analogRead(Analogbuttons);
		if(ButtonValue < 1024 && ButtonValue > ButtonValues[0]){
		    ResultValue = 0 ;
		    analogWrite(PWMpin, 0);
		} else if (ButtonValue < ButtonValues[0] - 1 && ButtonValue > ButtonValues[1]){
		    ResultValue = 1 ;
		    analogWrite(PWMpin, 0);
		} else if (ButtonValue < ButtonValues[1] - 1 && ButtonValue > ButtonValues[2]){
		    ResultValue = 2 ;
		    analogWrite(PWMpin, 0);
		} else if (ButtonValue < ButtonValues[2] - 1 && ButtonValue > ButtonValues[3]){
		    ResultValue = 3 ;
		    analogWrite(PWMpin, 0);
		} else if (ButtonValue < ButtonValues[3] - 1){
			ResultValue = 23 ;
			whilestatus = 0 ;
		}
		for(int Bt=0; Bt<4; Bt++){
			if ( Bt == ResultValue) {
				//delay(10) ;
				whilestatus = 1 ;
				TimeDelta = millis() - TimeStart;
				//Serial.print("Button: ") ;
				//Serial.print(Bt) ;
				//Serial.print(" Deltatime: ") ;
				//Serial.println(TimeDelta) ;
				if(TimeDelta < 3000 && TimeDelta > 50){
				    // Shortpress
				    // Serial.println("Shortpress");
				    ArrayButtons[Bt][0] = 1 ;
				    ArrayButtons[Bt][1] = 0 ;
				} else {
				    // Longpress
				    // Serial.println("Longpress");
				    ArrayButtons[Bt][0] = 0 ;
					ArrayButtons[Bt][1] = 1 ;
				}
			} else {
				//ArrayButtons[Bt][0] = 0 ;
				//ArrayButtons[Bt][1] = 0 ;
			}
		}
		TFTButtons() ;
	}
}





bool intervallcheck(int RowTimeArray) {
	unsigned long TimeNow = millis();
	unsigned long TimeLastcheck = IntervallArray[RowTimeArray][0] ;
	unsigned long TimeIntervall = IntervallArray[RowTimeArray][1] ; 
	bool Result ;
	if(TimeNow >= (TimeLastcheck + TimeIntervall)){
	    Result = true ;
	    IntervallArray[RowTimeArray][0] = TimeNow ;
	} else {
	    Result = false ;
	}
	return(Result) ;
}



void GetPotiDelta(){
	int potiValue = analogRead(POTI);
	int deltaValue = map(potiValue, 0, 1024, -PotiDeltaPM, PotiDeltaPM + 1);
	int result ;
	if(intervallcheck(0) == true){
		if(configuration[2] == 0){
	        result = deltaValue ;
	    } else {
	    	result = deltaValue * -1;
	    }
	} else {
	    result = 0 ;
	}
	PotiDelta = result ;
}




void getTemperature() {
	analogWrite(PWMpin, 0);		//switch off heater
	delay(DELAY_MEASURE) ;			//wait for some time (to get low pass filter in steady state)
	int adcValue = analogRead(TEMPin); // read the input on analog pin 7:
	if(HeaterOn == true && (StatusMain == 0 || StatusMain == 1)){
	    analogWrite(PWMpin, HeaterPWM);	//switch heater back to last value
	}    
	ArrayTemps[0] = round(((float) adcValue)*ADC_TO_TEMP_GAIN+ADC_TO_TEMP_OFFSET); //apply linear conversion to actual temperature
	//Serial.println(ArrayTemps[0]);
	
	if(StatusMain == 0 || StatusMain == 1){
		ArrayTemps[2] = ArrayTemps[1] - ArrayTemps[0] ; 
	}   

}

 


void SetTargetTemp(){
	int SollTemp = ArrayTemps[1] ;
	SollTemp += PotiDelta ;
	if(SollTemp < 0 ){
		SollTemp = 0 ;
	} else if (SollTemp > MaxTemp) {
		SollTemp = MaxTemp ;
	}
	ArrayTemps[1] = SollTemp ;
	//Serial.print("SollTemp: ");
	//Serial.println(SollTemp);
}




void Heating(){
	//TODO: Put in Funktion
	//Done: By Marcus

	if(StatusMain == 0 || StatusMain == 1){

		// a = PWM 0		b= PWM 100
		int a = -2 ;
		int b = 15 ;
		int value = constrain(ArrayTemps[2], a, b);
		HeaterPWM = map(value, a, b, 0, configuration[7]); 
	}

}





void PresetChange(){

	int sp = 0 ;
	int lp = 0 ;
	for(int i=0; i<3; i++){
	    sp = ArrayButtons[i][0] ;
		lp = ArrayButtons[i][1] ;
		if (sp == 1){
		    ArrayTemps[1] = HeaterPresets[i] ;
		} else if (lp == 1) {
			HeaterPresets[i] = ArrayTemps[1] ;
		}
	}
}






void TFTWritePresets(int newrender){

	int yPreset = 144 ;
	int xpos = 0 ;
	int Xstep = 45 ;
	for(int i=0; i<3; i++){
	    int Preset = HeaterPresets[i] ;
    	//Serial.println(Preset);
	    if(Preset == ArrayTemps[1]){
			//Serial.print("Preset Active: ");
			//Serial.println(Preset) ;
			TFToldPresets[3+i] = 1 ;
		} else {
			TFToldPresets[3+i] = 0 ;
		}
	}
	if(newrender == 1 || TFToldPresets[0] != HeaterPresets[0] || TFToldPresets[1] != HeaterPresets[1] || TFToldPresets[2] != HeaterPresets[2] || TFToldPresets[6] != TFToldPresets[3] || TFToldPresets[7] != TFToldPresets[4] || TFToldPresets[8] != TFToldPresets[5]){
		//Serial.println("Presetbackgrounds") ;
		tft.fillRect(xpos, yPreset - 5,  128,  160 - yPreset +5,  ST7735_BLACK);
		tft.drawLine(xpos, yPreset - 4, 128, yPreset -4, ST7735_YELLOW);
		tft.drawLine(xpos, yPreset - 5, 128, yPreset -5, ST7735_YELLOW);
		tft.setTextSize(2);
		for(int i=0; i<3; i++){
		    if(TFToldPresets[3+i] == 1){
		        tft.fillRect(Xstep * i, yPreset -2 ,  Xstep - 5,  160 - yPreset + 2,  ST7735_GREEN);
		    } 
			TFToldPresets[6+i] = TFToldPresets[3+i] ; 
			tft.setCursor(Xstep * i, yPreset);
			tft.setTextColor(ST7735_MAGENTA);
			int value = getTempUnit(configuration[1], HeaterPresets[i]);
			tft.print(value);
			TFToldPresets[i] = HeaterPresets[i] ;
		}
	}
}






void TFTButtons(){
	int ystart = 20 ;
	int xpos = 118 ;
	int ydelta = 25 ;
	int ypos ;
	int sp ;
	int lp ;
	for(int i=0; i<4; i++){
		ypos = ystart + ydelta * i ;
		sp = ArrayButtons[i][0] ;
		lp = ArrayButtons[i][1] ;
		if(sp == 1){
		    tft.drawRect( xpos,  ypos,  10,  28,  ST7735_WHITE);
   			tft.fillRect( xpos,  ypos,  10,  28,  ST7735_YELLOW);
   			TFTOldButtons[i][0] = 1 ;
		} else if(lp == 1){
    		tft.drawRect( xpos,  ypos,  10,  28,  ST7735_WHITE);
    		tft.fillRect( xpos,  ypos,  10,  28,  ST7735_CYAN);
    		TFTOldButtons[i][1] = 1 ;
		} else if (TFTOldButtons[i][0] == 1 || TFTOldButtons[i][1] == 1 ) {
			tft.drawRect( xpos,  ypos,  10,  28,  ST7735_BLACK);
    		tft.fillRect( xpos,  ypos,  10,  28,  ST7735_BLACK);
    		TFTOldButtons[i][0] = 0 ;
    		TFTOldButtons[i][1] = 0 ;
    		//StatusTFT = 99 ;
		}
	}
}




void TFTwriteClock(){

	int Dlength = 128 ;
	int yClock = 0 ;
	int xstart ;
	int Slength = 0;
	int charlength = 10 ;
	if (TimesArray[2] != TFToldClock[2]) {
		for(int i=0; i<3; i++){
		    if (TimesArray[i] <= 9){
		    		Slength += charlength ;
		    	} else {
		    		Slength += charlength * 2 ;
		    	}
		}
		Slength += charlength * 2 ;

		if(TFToldClock[3] != Slength){
			tft.fillRect( 0, 0,  128,  15,  ST7735_BLACK);
			TFToldClock[3] = Slength ;
		}
		tft.drawLine(0, yClock + 16, 128, yClock + 16, ST7735_YELLOW);
		tft.drawLine(0, yClock + 17, 128, yClock + 17, ST7735_YELLOW);
		xstart = (Dlength - Slength) / 2 ;
		tft.setTextSize(2);
		tft.setCursor(xstart,yClock);
		for(int i=0; i<3; i++){
			tft.setTextColor(ST7735_BLACK);
			tft.print(TFToldClock[i]);
			if(i != 2){
			    tft.print(":");
			}		
		}
		tft.setTextSize(2);
		tft.setCursor(xstart,yClock);
		for(int i=0; i<3; i++){
			tft.setTextColor(ST7735_WHITE);
			tft.print(TimesArray[i]);
			if(i != 2){
			    tft.print(":");
			}
			TFToldClock[i] = TimesArray[i] ;
		}
	}

}






void TFTwriteHEATING(){
	//TFT Anzeige

	int yAct = 32 ;
	int xAct = 0 ;
	int yTar = 78 ; 
	int xTar = 0 ;
	int yPWM = 120 ;

	int pwmVAL = map(HeaterPWM, 0, 255, 0, 100);

	if(StatusTFT != 0 && StatusMain == 0){
		
		analogWrite(BLpin, configuration[3]);
		digitalWrite(PiezoPin, LOW);
		tft.fillScreen(ST7735_BLACK);
		StatusTFT = 0 ;
		TFTwriteClock() ;
		TFTWritePresets(1) ;
		TFTOldTemps[1] = 999 ;

		// Actual:
		TFTWriteunits(xAct, yAct, 1, configuration[1]) ;
		// Target
		TFTWriteunits(xTar, yTar, 2, configuration[1]) ;
		// pwm
		tft.setTextSize(1);
		tft.setCursor(0,yPWM);
		tft.setTextColor(ST7735_WHITE);
		tft.print("PWM:");
		tft.setTextSize(2);
		tft.setCursor(80,yPWM);
		tft.print("%");
	} else if (StatusTFT != 1 && StatusMain == 1) {
		digitalWrite(PiezoPin, LOW);
		tft.fillScreen(ST7735_BLACK);
		StatusTFT = 1 ;
		TFTOldTemps[1] = 999 ;

		// Actual:
		TFTWriteunits(xAct, yAct, 1, configuration[1]) ;
		// Target
		TFTWriteunits(xTar, yTar, 2, configuration[1]) ;
	}



	// Ist - Actual
	TFTActualTemp(45, yAct) ;

	// Soll - Target State
	TFTTarget(45, yTar) ;


	

	
	TFTwriteClock() ;

	if(StatusMain == 0){
	    TFTWritePresets(0) ;
	    TFTpwm(45, yPWM, pwmVAL) ;
	}


	if(StatusMain == 1){
		int y = yPWM - 5 ;
		int TimeTotempdrop = configuration[5] ;
		int TimeFromtempdrop = configuration[6] ;
		if(StandByRuntime < TimeTotempdrop){
			TimeTotempdrop = TimeTotempdrop - StandByRuntime ;
			if(TFTOldTemps[4] != TimeTotempdrop){
				tft.setTextColor(ST7735_YELLOW);
		    	tft.setTextSize(2);
				tft.setCursor(0,y);
				tft.print(Langs[8][configuration[0]]);
				tft.setCursor(0,y+30);
				tft.print(Langs[5][configuration[0]]);
				tft.print(":");
				tft.setTextColor(ST7735_CYAN);
		    	tft.setTextSize(3);
				tft.setCursor(40,y+24);
				tft.setTextColor(ST7735_BLACK);
		    	tft.setTextSize(3);
				tft.setCursor(40,y+24);
			    tft.print(TFTOldTemps[4]);
			    tft.setTextColor(ST7735_CYAN);
		    	tft.setTextSize(3);
				tft.setCursor(40,y+24);
			    tft.print(TimeTotempdrop);
			    TFTOldTemps[4] = TimeTotempdrop ;
			}
		} else if(StandByRuntime >= TimeTotempdrop){
			analogWrite(BLpin, configuration[4]);
			// Slow Temperature Drop maybe later
		    int TimeTempdrop = (TimeTotempdrop + TimeFromtempdrop) - StandByRuntime ;
		    if(StandByRuntime == TimeTotempdrop && intervallcheck(9) == true){
		        //tft.drawRect( xpos,  ypos,  10,  28,  ST7735_BLACK);
    			tft.fillRect( 0,  y,  128,  160-y,  ST7735_BLACK);
		    }
		    Serial.println(TimeTempdrop) ;
			tft.setTextColor(ST7735_YELLOW);
	    	tft.setTextSize(2);
			tft.setCursor(0,y + 15);
			tft.print(Langs[9][configuration[0]]);
			HeaterOn = false ;
		} 



		if(StandByRuntime > (TimeTotempdrop + TimeFromtempdrop)){
		    if (intervallcheck(10) == true) {
		    	UseBuzzer() ;
		    }
		}
	}
}













void TFTpwm(int x, int y, int pwmVAL){
	if(TFTOldTemps[3] != pwmVAL){
		tft.setTextSize(2);
		tft.setCursor(x,y);
		tft.setTextColor(ST7735_BLACK);
		tft.print(TFTOldTemps[3]);
		tft.setCursor(x,y);
		tft.setTextColor(ST7735_WHITE);
		tft.print(pwmVAL);
		TFTOldTemps[3] = pwmVAL;
	}
}





void TFTActualTemp(int x, int y){
	int Value = getTempUnit(configuration[1], ArrayTemps[0]);	
	if(TFTOldTemps[0] != Value){
		tft.setTextSize(4);
		tft.setCursor(x,y);
		tft.setTextColor(ST7735_BLACK);
		tft.print(TFTOldTemps[0]);
		tft.setCursor(x,y);
		if(ArrayTemps[2] >= 5){
		    tft.setTextColor(ST7735_BLUE);
		} else if (abs(ArrayTemps[2]) < 5) {
		    tft.setTextColor(ST7735_GREEN);
		} else {
			tft.setTextColor(ST7735_RED);
		}	
		tft.print(Value);
		TFTOldTemps[0] = Value;
	}
}





void TFTTarget(int x, int y){
	int Value = getTempUnit(configuration[1], ArrayTemps[1]);	
	if(TFTOldTemps[1] != Value){
		tft.setTextSize(4);
		tft.setCursor(x,y);
		tft.setTextColor(ST7735_BLACK);
		tft.print(TFTOldTemps[1]);
		tft.setCursor(x,y);
		tft.setTextColor(ST7735_WHITE);
		tft.print(Value);
		TFTOldTemps[1] = Value;
	}
}




void TFTWriteunits(int x, int y, int w0rd, int unit){
	if(unit == 0){
	    tft.setTextSize(1);
		tft.setCursor(x,y);
		tft.setTextColor(ST7735_WHITE);
		tft.print(Langs[w0rd][configuration[0]]);
		tft.drawCircle(x + 5, y + 15, 2,ST7735_WHITE) ;
		tft.setCursor(x + 11,y + 14);
		tft.setTextSize(2);
		tft.print("C");
	} else if (unit == 1){
		tft.setTextSize(1);
		tft.setCursor(x,y);
		tft.setTextColor(ST7735_WHITE);
		tft.print(Langs[w0rd][configuration[0]]);
		tft.setCursor(x + 11,y + 14);
		tft.setTextSize(2);
		tft.print("K");
	} else if (unit == 2) {
	    tft.setTextSize(1);
		tft.setCursor(x,y);
		tft.setTextColor(ST7735_WHITE);
		tft.print(Langs[w0rd][configuration[0]]);
		tft.drawCircle(x + 5, y + 15, 2,ST7735_WHITE) ;
		tft.setCursor(x + 11,y + 14);
		tft.setTextSize(2);
		tft.print("F");
	}
}







void UseBuzzer(){
	int val = !digitalRead(PiezoPin) ;
	digitalWrite(PiezoPin, val);
	digitalWrite(BLpin, val);
}


void SerialOutput(){
	Serial.print(ArrayTemps[0]);
	Serial.print(" / ") ;
	Serial.print(ArrayTemps[1]) ;
	Serial.print(" / ") ;
	Serial.print(ArrayTemps[2]) ;
	Serial.print(" - ") ;
	Serial.print(TimesArray[0]);
	Serial.print(" : ") ;
	Serial.print(TimesArray[1]) ;
	Serial.print(" : ") ;
	Serial.println(TimesArray[2]) ;
	Serial.print("Active Button: ") ;
	for(int i=0; i<4; i++){
	    int sp = ArrayButtons[i][0] ;
	    int lp = ArrayButtons[i][1] ;
	    Serial.print(i) ;
	    Serial.print(": ") ;
	    Serial.print(sp) ;
	    Serial.print(lp) ;
	    Serial.print("   ") ;
	}
	Serial.println("") ;
}




//uint16_t Color565(uint8_t r, uint8_t g, uint8_t b) {
//	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
//}



int getTempUnit(int Unit, int Value){
	int result ;
	if(Unit == 0){
	    result = Value ;
	} else if (Unit == 1) {
		result = Value + 273 ;
	} else if (Unit ==2) {
	    float Fahrenheit = ((9/5) * Value) + 32 ;
		result = Fahrenheit ;
	}
	return result ;
}


void setPwmFrequency(int pin, int divisor) {
	byte mode;
	if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
		switch(divisor) {
			case 1: 		mode = 0x01; break;
			case 8: 		mode = 0x02; break;
			case 64: 		mode = 0x03; break;
			case 256: 	mode = 0x04; break;
			case 1024: 	mode = 0x05; break;
			default: return;
		}
		
		if(pin == 5 || pin == 6) {
			TCCR0B = TCCR0B & 0b11111000 | mode;
		} else {
			TCCR1B = TCCR1B & 0b11111000 | mode;
		}
	} else if(pin == 3 || pin == 11) {
		switch(divisor) {
			case 1: 		mode = 0x01; break;
			case 8: 		mode = 0x02; break;
			case 32: 		mode = 0x03; break;
			case 64: 		mode = 0x04; break;
			case 128: 	mode = 0x05; break;
			case 256: 	mode = 0x06; break;
			case 1024: 	mode = 0x07; break;
			default: return;
		}
		
		TCCR2B = TCCR2B & 0b11111000 | mode;
	}
}
