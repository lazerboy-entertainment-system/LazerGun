// BRIAN SUMNER
// IN AFFILIATION WITH KAMIL ADYLOV AND PHI HUYNH
// UCDENVER CSCI 4287
// SPRING 2019
// LAZERBOY ENTERTAINMENT SYSTEM:
// LAZERGUN DRIVER
// MODEL M9B2
// VERSION: BETA_02


// INCLUDED LIBRARIES
#include <avr/io.h>
#include <avr/interrupt.h>


// MACRO DEFINITIONS
#define PIN_TRIGGER_IN    2
#define PIN_SLIDE_IN      3

#define PIN_TRIGGER_OUT   7
#define PIN_SLIDE_OUT     8

#define PIN_LASER_OUT     5

#define MODE_SAFETY             0   // UNUSED
#define MODE_SEMI_AUTOMATIC     1
#define MODE_THREE_ROUND_BURST  2
#define MODE_FULLY_AUTOMATIC    3

#define TIMER_INTERVAL_MILLISECONDS   4
#define CPU_MHZ                       16
#define TIMER_PRESCALAR               1024

#define TIMER_TRIGGER_DEBOUNCE_MAX_COUNT  10
#define TIMER_TRIGGER_RESET_MAX_COUNT     10
#define TIMER_LASER_RESET_MAX_COUNT       1


// TYPE DEFINITIONS
typedef struct timer_t timer_t;
struct timer_t
{
  uint16_t flag_isEnabled : 1;
  uint16_t count          : 15;
  uint16_t maxCount       : 15; 
  uint16_t flag_doEvent   : 1; 
};


// GLOBAL CONSTANTS
const double MAX_TIMER_ISR_COUNT = ((CPU_MHZ * 1000) / TIMER_PRESCALAR * TIMER_INTERVAL_MILLISECONDS);


// GLOBAL VARIABLES
// NOTE:  SAFETY MODE IS IMPLEMENTED BY DISABLING THE TRIGGER AT SYSTEM START
uint8_t firingMode = MODE_SEMI_AUTOMATIC;


// ISR VARIABLES
// NOTE: ISR VARIABLES MUST BE DECLARED VOLATILE
volatile timer_t timer_triggerDebounce =  {0, 0, TIMER_TRIGGER_DEBOUNCE_MAX_COUNT, 0};
volatile timer_t timer_triggerReset =     {0, 0, TIMER_TRIGGER_RESET_MAX_COUNT, 0};
volatile timer_t timer_laserReset =       {0, 0, TIMER_LASER_RESET_MAX_COUNT, 0};

volatile uint8_t flag_isTriggerEnabled = 0;
volatile uint8_t flag_isSlideEnabled = 1;
volatile uint8_t flag_doFireLaser = 0;
volatile uint8_t flag_doRackSlide = 0;


// PERFORM INITIALIZATION
void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  pinMode(PIN_TRIGGER_IN, INPUT_PULLUP);
  pinMode(PIN_SLIDE_IN, INPUT_PULLUP);

  pinMode(PIN_TRIGGER_OUT, OUTPUT);
  pinMode(PIN_SLIDE_OUT, OUTPUT);

  pinMode(PIN_LASER_OUT, OUTPUT);


  // DISABLE INTERRUPTS
  cli();

  // ATTACH ISR'S FOR PIN INTERRUPTS
  attachInterrupt(digitalPinToInterrupt(PIN_TRIGGER_IN), ISR_pin_trigger_in, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_SLIDE_IN), ISR_pin_slide_in, FALLING);

  // CLEAR TIMER/COUNTER REGISTERS
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  // SET MAX TIMER COUNTER
  OCR1A = MAX_TIMER_ISR_COUNT;
  
  // CLEAR TIMER ON COMPARE MATCH
  TCCR1B |= (1 << WGM12);

  // SET CS12 AND CS10 TO USE PRESCALAR 1024
  TCCR1B |= (1 << CS12);
  TCCR1B |= (1 << CS10);  
  
  // ENBABLE TIMER COMPARE INTERRUPT
  TIMSK1 |= (1 << OCIE1A);

  // ENABLE INTERRUPTS
  sei();

}


// MAIN LOOP
void loop() 
{

  // ON FIRE LASER EVENT
  if (flag_doFireLaser)
  {

    // FIRE LASER
    digitalWrite(PIN_LASER_OUT, HIGH);

//      digitalWrite(PIN_TRIGGER_OUT, HIGH);

    // CLEAR FIRE LASER FLAG
    flag_doFireLaser = 0;

  }


  // ON LASER RESET EVENT
  if (timer_laserReset.flag_doEvent)
  {  
    // STOP FIRING LASER
    digitalWrite(PIN_LASER_OUT, LOW);

//    digitalWrite(PIN_TRIGGER_OUT, LOW);

    // CLEAR RESET LASER FLAG
    timer_laserReset.flag_doEvent = 0;
  }


  // ON TRIGGER DEBOUNCE TIMER EVENT
    // IF TRIGGER PIN HIGH
      // CLEAR TRIGGER DEBOUNCE DO EVENT FLAG
      // SET TRIGGER RESET TIMER COUNT AS MAX COUNT
      // ENABLE TRIGGER RESET TIMER
    // ELSE
      // ENABLE TRIGGER DEBOUNCE TIMER


  // ON TRIGGER DEBOUNCE TIMER EVENT
  if (timer_triggerDebounce.flag_doEvent)
  {  
    // IF TRIGGER PIN HIGH
    if (digitalRead(PIN_TRIGGER_IN) == HIGH)
    {
      // CLEAR TRIGGER DEBOUNCE DO EVENT FLAG
      timer_triggerDebounce.flag_doEvent = 0;
      
      // SET TRIGGER RESET TIMER COUNT AS MAX COUNT
      timer_triggerReset.count = timer_triggerReset.maxCount;

      // ENABLE TRIGGER RESET TIMER
      ++timer_triggerReset.flag_isEnabled;      
    }
    // ELSE
    else
    {
      // RESET TRIGGER DEBOUNCE TIMER COUNT AS MAX COUNT
      timer_triggerDebounce.count = timer_triggerDebounce.maxCount;
      
      // ENABLE TRIGGER DEBOUNCE TIMER
      ++timer_triggerDebounce.flag_isEnabled;      
    }
  }


  // ON TRIGGER RESET TIMER EVENT
    // CLEAR TRIGGER RESET DO EVENT FLAG
    // ENABLE TRIGGER INPUT

  // ON TRIGGER RESET TIMER EVENT
  if (timer_triggerReset.flag_doEvent)
  {
    // CLEAR TRIGGER RESET DO EVENT FLAG
    timer_triggerReset.flag_doEvent = 0;
    
    // ENABLE TRIGGER INPUT
    ++flag_isTriggerEnabled;
  }


/*
  if (flag_doRackSlide)
  {
    // GENERATE RACK SLIDE SOUND
    {
      digitalWrite(PIN_SLIDE_OUT, HIGH);
      delay(DELAY_RACK_SLIDE);
    }
    
    // STOP RACKING SLIDE
    {
      digitalWrite(PIN_SLIDE_OUT, LOW);
      flag_doRackSlide = 0;

      // TEMPORARY CODE TO DEBOUNCE SLIDE
      delay(DELAY_SLIDE_DEBOUNCE);    
    }
  }
*/
  
}


// INTERRUPT SERVICE ROUTINE FOR TRIGGER SWITCH
void ISR_pin_trigger_in() 
{ 

  // IF TRIGGER INPUT ENABLED
    // DISABLE TRIGGER INPUT
    // SET FIRE LASER FLAG
    // SET TRIGGER DEBOUNCE TIMER COUNT AS MAX COUNT
    // ENABLE TRIGGER DEBOUNCE TIMER
    // SET LASER RESET TIMER COUNT AS MAX COUNT
    // ENABLE LASER RESET TIMER


  // IF TRIGGER INPUT ENABLED
  if (flag_isTriggerEnabled)
  {
    // DISABLE TRIGGER INPUT
    flag_isTriggerEnabled = 0;
    
    // SET FIRE LASER FLAG
    ++flag_doFireLaser;
    
    // SET TRIGGER DEBOUNCE TIMER COUNT AS MAX COUNT
    timer_triggerDebounce.count = timer_triggerDebounce.maxCount;

    // ENABLE TRIGGER DEBOUNCE TIMER
    ++timer_triggerDebounce.flag_isEnabled;

    // SET LASER RESET TIMER COUNT AS MAX COUNT
    timer_laserReset.count = timer_laserReset.maxCount;
    
    // ENABLE LASER RESET TIMER
    ++timer_laserReset.flag_isEnabled;
  }
}


// INTERRUPT SERVICE ROUTINE FOR SLIDE SWITCH
void ISR_pin_slide_in()
{
  if (flag_isSlideEnabled)
  {
    // GENERATE RACK SLIDE SOUND ON NEXT LOOP ITERATION
    flag_doRackSlide = 1;

    // RESET TRIGGER (LEAVE SAFETY MODE)
    flag_isTriggerEnabled = 1;
  }
}


// INTERRUPT SERVICE ROUTINE FOR TIMER1
ISR(TIMER1_COMPA_vect) 
{

  // IF TRIGGER DEBOUNCE TIMER ENABLED
    // IF COUNT <= 0 
      // DISABLE TIMER
      // SET DO EVENT FLAG
    // ELSE 
      // DECREMENT COUNT
      
  // IF TRIGGER RESET TIMER ENABLED
    // IF COUNT <= 0 
      // DISABLE TIMER
      // SET DO EVENT FLAG
    // ELSE 
      // DECREMENT COUNT
      
  // IF TRIGGER RESET TIMER ENABLED
    // IF COUNT <= 0 
      // DISABLE TIMER
      // SET DO EVENT FLAG
    // ELSE 
      // DECREMENT COUNT

  // IF LASER RESET TIMER ENABLED
    // IF COUNT <= 0 
      // DISABLE TIMER
      // SET DO EVENT FLAG
    // ELSE 
      // DECREMENT COUNT


  // IF TRIGGER DEBOUNCE TIMER ENABLED
  if (timer_triggerDebounce.flag_isEnabled)
  {
    // IF COUNT <= 0 
    if (timer_triggerDebounce.count <= 0)
    {
      // DISABLE TIMER
      timer_triggerDebounce.flag_isEnabled = 0;
      
      // SET DO EVENT FLAG
      ++timer_triggerDebounce.flag_doEvent;
    }
    // ELSE 
    else
    {
      // DECREMENT COUNT
      --timer_triggerDebounce.count;
    }
  }
  
  // IF TRIGGER RESET TIMER ENABLED
  if (timer_triggerReset.flag_isEnabled)
  {
    // IF COUNT <= 0 
    if (timer_triggerReset.count <= 0)
    {
      // DISABLE TIMER
      timer_triggerReset.flag_isEnabled = 0;
      
      // SET DO EVENT FLAG
      ++timer_triggerReset.flag_doEvent;
    }
    // ELSE 
    else
    {
      // DECREMENT COUNT
      --timer_triggerReset.count;
    }
  }
  
  // IF LASER RESET TIMER ENABLED
  if (timer_laserReset.flag_isEnabled)
  {
    // IF COUNT <= 0 
    if (timer_laserReset.count <= 0)
    {
      // DISABLE TIMER
      timer_laserReset.flag_isEnabled = 0;
      
      // SET DO EVENT FLAG
      ++timer_laserReset.flag_doEvent;
    }
    // ELSE 
    else
    {
      // DECREMENT COUNT
      --timer_laserReset.count;
    }
  }

}
