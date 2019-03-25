// BRIAN SUMNER
// IN AFFILIATION WITH KAMIL ADYLOV AND PHI HUYNH
// UCDENVER CSCI 4287
// SPRING 2019
// LAZERBOY ENTERTAINMENT SYSTEM:
// LAZERGUN DRIVER
// MODEL M9B2
// VERSION: ALPHA_04


#include <avr/io.h>
#include <avr/interrupt.h>


#define PIN_TRIGGER_IN    2
#define PIN_SLIDE_IN      3

#define PIN_TRIGGER_OUT   7
#define PIN_SLIDE_OUT     8

#define PIN_LASER_OUT     11

#define DELAY_PULSE_LASER   100
#define DELAY_RACK_SLIDE    20


// ISR FLAGS MUST BE DECLARED VOLATILE
volatile int isTriggerEnabled = 1;
volatile int isSlideEnabled = 1;
volatile int doFireLaser = 0;
volatile int doRackSlide = 0;


void setup() 
{

  pinMode(PIN_TRIGGER_IN, INPUT_PULLUP);
  pinMode(PIN_SLIDE_IN, INPUT_PULLUP);

  pinMode(PIN_TRIGGER_OUT, OUTPUT);
  pinMode(PIN_SLIDE_OUT, OUTPUT);

  pinMode(PIN_LASER_OUT, OUTPUT);


  cli();
  attachInterrupt(digitalPinToInterrupt(PIN_TRIGGER_IN), ISR_pin_trigger_in, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_SLIDE_IN), ISR_pin_slide_in, FALLING);
  sei();

}


void loop() 
{

  if (doFireLaser)
  {
    // FIRE LASER
    {
      digitalWrite(PIN_LASER_OUT, HIGH);
      digitalWrite(PIN_TRIGGER_OUT, HIGH);
      
      delay(DELAY_PULSE_LASER);
    }
    
    // STOP FIRING LASER
    {
      digitalWrite(PIN_LASER_OUT, LOW);
      digitalWrite(PIN_TRIGGER_OUT, LOW);
  
      doFireLaser = 0;
    }
  }


  if (doRackSlide)
  {
    // GENERATE RACK SLIDE SOUND
    {
      digitalWrite(PIN_SLIDE_OUT, HIGH);
      delay(DELAY_RACK_SLIDE);
    }
    
    // STOP RACKING SLIDE
    {
      digitalWrite(PIN_SLIDE_OUT, LOW);
      doRackSlide = 0;
    }
  }

  
}


// INTERRUPT SERVICE ROUTINE FOR TRIGGER SWITCH
void ISR_pin_trigger_in() 
{
  if (isTriggerEnabled)
  {
    // FIRE LASER ON NEXT LOOP ITERATION
    doFireLaser = 1;

    // DISABLE TRIGGER DUE TO SWITCH BOUNCE
    isTriggerEnabled = 0;

    // RESET SLIDE SWITCH (SINGLE-ACTION MODE)
    isSlideEnabled = 1;
  }
}


// INTERRUPT SERVICE ROUTINE FOR SLIDE SWITCH
void ISR_pin_slide_in()
{
  if (isSlideEnabled)
  {
    // GENERATE RACK SLIDE SOUND ON NEXT LOOP ITERATION
    doRackSlide = 1;

    // DISABLE SLIDE DUE TO SWITCH BOUNCE   
    isSlideEnabled = 0;

    // RESET TRIGGER (SINGLE-ACTION MODE)
    isTriggerEnabled = 1;
  }
}
