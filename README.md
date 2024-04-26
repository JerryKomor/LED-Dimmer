
Version 2.03 for dual output PWM board.
Example program uses first driver as PWM for driving LED string. 
The 2 onboard buttons control duty cycle of first MOSFET transistor from 5 to 100%
A trigger pin on IO connector triggers 90 second count down timer for second MOSFET
Rev 2.03 adds auto shut off for MOSFET 1
Both values adjustable in setup loop

On / Off button - available through connector
  short press  GRADUAL ON / OFF
  double press instant ON / OFF

up button

  short press  gradual on till last level then step up
  long press   continous level up  till  max
  double press instant On till last level  or min if last was 0
  
down

  short press   if on redure by step or off if at last step
  long press    continous step down till off 
  double press  auto gradual off 


PCB board information is found [here](http:\\PCBhut.com\Products\NightLightLEDdimmer_Dual\index.html)
