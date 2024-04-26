/*
 *  Non blocking version of Night Light Dimmer for either version 1  and 
 *  on version 2 first driver (D8) with input signal ie PIR to drive timed down second driver (D6)
 *  
 *  
 *  PCB project boards
 *  version 1  http:\\PCBhut.com\Products\NightLightLEDdimmer\index.html
 *  version 2  http:\\PCBhut.com\Products\NightLightLEDdimmer_Dual\index.html
 *  
 *  
 * On / Off
 *   short press  GRADUAL ON / OFF
 *   double press instant on / off
 * 
 *   rev 2.03
 *   - added auto shut off timer
 * 
 * up
 *   short press  gradual on till last level then step up
 *   long press   continous level up  till  max
 *   double press instant On till last level  or min if last was 0
 * 
 * down
 *   short press   if on redure by step or off if at last step
 *   long press    continous step down till off 
 *   double press  auto gradual off 
 *   
 *   
 */

#include "OneButton.h"        // https://github.com/mathertel/OneButton

#if defined (__AVR_ATtiny84__)  // default controller
   //Debug.println("__AVR_ATtiny84__ defined");
   #define oo     A0             //  on / off switch
   #define up     A1             //  brighter switch
   #define dwn    A2             //  darker  switch
   #define fet_1   8             //  Transistor 1 driver
   #define fet_2   6             //  Transistor 2 driver            
   #define bl      7             //  3 backlit driver   A7
   #define PIR    A3             // motion input

//#elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)   //other micros
    //Debug.println("__AVR_ATmega328__ defined");
    //Code here       

#else                           // other Arduino
   //Debug.println("__Arduino__ defined");
   #define oo     A0
   #define up     A1
   #define dwn    A2
   #define fet_1  5   // D5  pwm Transistor 1 driver            
   #define fet_2  6   // D6  pwm Transistor 2 driver            
   #define bl     3   // D3  pwm backlit driver                 
   #define PIR   A3   // 3 A3  motion input
#endif




   
OneButton buttonOO(oo, true);    //  A0 on / off switch  button_0       
OneButton buttonUp(up, true);    //  A1 up / brighter switch  button_1  
OneButton buttonDn(dwn, true);   //  A2 dn / darker  switch  button_2  



int PWM_1 = 5;     // default FET_1 level
int PWM_2 = 100;   // default FET_2 level
int state_1 = 0;   // FET_1 status
int state_2 = 0;   // FET_2 status
int state_PIR = 0;
int bl_off = 20;     //back light low
int bl_on = 120;     //back light high level
int x = 0;          //floater variable
int countDwn=0;


#include "Countimer.h"      //  https://github.com/inflop/Countimer
Countimer timer;
Countimer pDown;


void setup() {
  #if !defined (__AVR_ATtiny84__)
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
    }
  Serial.println("Starting Buttons...");
  #endif                                         
  // link the button 1 functions.
  buttonOO.attachClick(clickO);
  buttonOO.attachDoubleClick(doubleclickO);
  //buttonOO.attachLongPressStart(longPressStartO);
  //buttonOO.attachLongPressStop(longPressStopO);
  //buttonOO.attachDuringLongPress(longPressO);
  buttonUp.attachClick(clickU);
  buttonUp.attachDoubleClick(doubleclickU);
  //buttonUp.attachLongPressStart(longPressStartU);
  //buttonUp.attachLongPressStop(longPressStopU);
  buttonUp.attachDuringLongPress(longPressU);
  buttonDn.attachClick(clickD);
  buttonDn.attachDoubleClick(doubleclickD);
  //buttonDn.attachLongPressStart(longPressStartD);
  //buttonDn.attachLongPressStop(longPressStopD);
  buttonDn.attachDuringLongPress(longPressD);


  
  pinMode(fet_1, OUTPUT);         // main driver
  pinMode(fet_2, OUTPUT);         // second driver
  pinMode(bl, OUTPUT);            // back light
  pinMode(PIR, INPUT_PULLUP);     // motion sensor
  analogWrite(bl, bl_off);
  //
  //   timers
  // 00h:00m:10s
  timer.setCounter(0, 1, 30, timer.COUNT_DOWN, PIRcomplete);    // PIR timeout period
  pDown.setCounter(0, 59, 00, timer.COUNT_DOWN, AutoOff);       // Auto Shutdown PIR timeout period
  //countDwn=tDown.getCurrentTime();
  timer.setInterval(refreshClock, 1000);
  pDown.setInterval(refreshAutoOff, 1000);
} // setup


// main code here, to run repeatedly: 
void loop() {
  // keep watching the push buttons:
  buttonOO.tick();
  buttonUp.tick();
  buttonDn.tick();
  timer.run();
  pDown.run();
  
  //Serial.print(" PIR pin:  ");
  //Serial.println(digitalRead(PIR));
  if (digitalRead(PIR) == LOW){
    if (state_2 == 0){
      #if !defined (__AVR_ATtiny84__)
      Serial.println("PIR  ON. ");
      #endif
      state_2 = !state_2;
      analogWrite(fet_2, PWM_2);
      timer.start();
      delay(500);
    }else{
      #if !defined (__AVR_ATtiny84__)
      Serial.println("PIR  running "); 
      #endif
      delay(500);
    }
  }
  delay(15);
} // loop



// ----- PRI end function
// ---------------------------------
void PIRcomplete(){
  analogWrite(fet_2,0);
  timer.stop();
  state_2 = !state_2;
  #if !defined (__AVR_ATtiny84__)
  Serial.println(" ilumination OFF");
  #endif
  stats();
}

void refreshClock() {
  #if !defined (__AVR_ATtiny84__)
  Serial.print("Current count time is: ");
  Serial.println(timer.getCurrentTime());
  #endif
}



// ----- PRI end function
// ---------------------------------
void AutoOff(){
  analogWrite(fet_1,0);
  pDown.stop();
  state_1 = !state_1;
  #if !defined (__AVR_ATtiny84__)
  Serial.println(" Auto OFF");
  #endif
  stats();
}


void refreshAutoOff() {
  #if !defined (__AVR_ATtiny84__)
  Serial.print("AutoOff count time is: ");
  Serial.println(pDown.getCurrentTime());
  #endif
}





// ----- button OnOFF callback functions
// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void clickO() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button OnOff click.");
  #endif
  if (state_1 == 1){      // check if powered on
    x = PWM_1;
    digitalWrite(bl,0);
    do{
      x=x-5;
      analogWrite(fet_1,x);
      delay(150);
    }while(x >=15);      //
    digitalWrite(fet_1,0);
    analogWrite(bl,bl_off);
    pDown.stop();
  }else{
    x = 0;
    digitalWrite(bl,0);
    do{
      x = x + 5;
      analogWrite(fet_1,x);
      delay(250);
    }while(x <=PWM_1);
    pDown.start();
    analogWrite(bl,bl_on);
  }
  state_1 = !state_1;
  stats();
}  // clickO


// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclickO() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button OnOff doubleclick.");
  #endif
  if (state_1 == 0){
    analogWrite(bl,bl_on);
    pDown.start();
    if (PWM_1 <=4){
      PWM_1 = 5;
      analogWrite (fet_1,PWM_1);
    }else{
      analogWrite (fet_1,PWM_1);
    }
  }else{
    digitalWrite(fet_1,0);
  }
  analogWrite(bl,bl_off);
  pDown.stop();
  state_1 = !state_1;
  stats();
}  // doubleclickO


// This function will be called once, when the button1 is pressed for a long time.
void longPressStartO() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button OnOff longPress start");
  #endif
}  // longPressStartO


// This function will be called often, while the button1 is pressed for a long time.
void longPressO() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button OnOff longPress...");
  #endif
}  // longPressO


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStopO() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button OnOff longPress stop");
  #endif
}  // longPressStopO



// ----- button OnOFF callback functions
// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void clickU() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button Up click.");
  #endif
  if (state_1 == 0){      // check if off
    state_1 = !state_1;
    if (PWM_1 >= 5){      // check if PWM more them 5
      x = 0;
      digitalWrite(bl,0);  // turn BL off during on loop
      do {                // gradually turn lights on
        x = x +5;
        analogWrite(fet_1,x);
        delay(150);
      }while(x <= PWM_1);
      analogWrite(bl,bl_on);    // turn BL back on
      pDown.start();       
    }else{                // if less then 5 inclease brighnrss and power on
      PWM_1 = PWM_1 +5;
      analogWrite(fet_1,PWM_1);
      analogWrite(bl,bl_on);
      pDown.start();
    }
  }else{
    if (PWM_1 <= 240){
      PWM_1 = PWM_1 + 5;         // just inclease brightness
      analogWrite(fet_1,PWM_1);
      analogWrite(bl,bl_on);
    }else{
      #if !defined (__AVR_ATtiny84__)
      Serial.println("Up reached maximun.");
      #endif
    }
  //  
  }
  stats();
}  // clickU


// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclickU() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button Up doubleclick.");
  #endif
  if (state_1 == 0){    // check if powered odd
    state_1 = !state_1;
    analogWrite(bl,bl_on);
    if (PWM_1 <=4){
      PWM_1 = 5;
      analogWrite (fet_1,PWM_1);
    }else{
      analogWrite (fet_1,PWM_1);
    }
  }
  stats();
}  // doubleclickU


// This function will be called once, when the button1 is pressed for a long time.
void longPressStartU() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button Up longPress start");
  #endif
}  // longPressStartU


// This function will be called often, while the button1 is pressed for a long time.
void longPressU() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button Up longPress...");
  #endif
  if (state_1 == 1){
    if (PWM_1 <= 240){
       PWM_1 = PWM_1 + 5;
       analogWrite(fet_1,PWM_1);
    }else{
      #if !defined (__AVR_ATtiny84__)
      Serial.println("long Press up at maximum");
      #endif
    }
  //
  }else{
    state_1 = !state_1;      // since is off turn status and output on 
    if (PWM_1 <= 4){
      PWM_1 = 5;
      analogWrite(fet_1,PWM_1);
      pDown.start();
    }else{
      analogWrite(fet_1,PWM_1);
      pDown.start();
    }
  }
  stats();
  delay(150);
}  // longPressU


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStopU() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button Up longPress stop");
  #endif
  for (int x = 0; x < 5; x = x+1) {
  analogWrite(bl,bl_on);
  delay(250);
  analogWrite(bl,bl_off);
  delay(250);
  }
}  // longPressStopU





// ----- button Down callback functions
// This function will be called when the button1 was pressed 1 time (and no 2. button press followed).
void clickD() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button Down click.");
  #endif
  if (state_1 == 1){
    if (PWM_1 >=15){
      PWM_1 = PWM_1 - 5;         // decrease PWM
      analogWrite(fet_1,PWM_1);  // update O/P
    }
    else{
      state_1 = !state_1;       // its below threshold
      PWM_1 = 1;
      digitalWrite(fet_1,0);    // shut down O/P
      analogWrite(bl,bl_off);   // lower back lit
      pDown.stop();             // shut timer off
    }
  }
  stats();  
}  // clickD


// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclickD() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button Down doubleclick.");
  #endif
  if (state_1 == 1){
    if (PWM_1 >= 15){
      x = PWM_1;
      digitalWrite(bl,0);
      do {
        x = x - 5;
        analogWrite(fet_1,x);
        delay(150);
      }while (x >=10);
      analogWrite(bl,bl_off);
      digitalWrite(fet_1,0);
      state_1 = !state_1;
    }else{
      analogWrite(bl,bl_off);
      digitalWrite(fet_1,0);
      pDown.stop();               // shut off timer
      state_1 = !state_1;
    }
  }else{
    #if !defined (__AVR_ATtiny84__)
    Serial.println("Down reached minimal");
    #endif
  }
  stats();
}  // doubleclickD


// This function will be called once, when the button1 is pressed for a long time.
void longPressStartD() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button Down longPress start");
  #endif
}  // longPressStartD


// This function will be called often, while the button1 is pressed for a long time.
void longPressD() {
  #if !defined (__AVR_ATtiny84__)
  Serial.println("Button Down longPress...");
  #endif
  if (state_1 == 1){
    if (PWM_1 >= 15){
      PWM_1 = PWM_1 - 5;
      analogWrite(fet_1,PWM_1);
    }else{
      digitalWrite(fet_1,0);
      analogWrite(bl,bl_off);
      pDown.stop();
      state_1 = !state_1;
    }
  }else{
    #if !defined (__AVR_ATtiny84__)
    Serial.println("Powered Off...");
    #endif
  }
  stats();
  delay(150);
}  // longPressD


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStopD() {
  #if !defined (__AVR_ATtiny84__)
   Serial.println("Button Down longPress stop");
  #endif 
  for (int x = 0; x < 5; x = x+1) {
  analogWrite(bl,bl_on);
  delay(150);
  analogWrite(bl,bl_off);
  delay(150);
  }
}  // longPressStopD








void stats(){
  #if !defined (__AVR_ATtiny84__)
  
  //Serial.println( );
  Serial.print("  PWM_1:  ");
  Serial.println(PWM_1);
  Serial.print("state_1:  ");
  Serial.println(state_1);
  //Serial.println(" ");
  
  Serial.print("  PWM_2:  ");
  Serial.println(+ PWM_2);
  Serial.print("state_2:  ");
  Serial.println(state_2);
  
  //Serial.print("     BL:  ");
  //Serial.println(bl_off);
  
  //Serial.println();
  //Serial.print("cntr  " + (x));
  //Serial.println(x);
  //Serial.println();
  Serial.println();
  #endif
} // stats




// End
