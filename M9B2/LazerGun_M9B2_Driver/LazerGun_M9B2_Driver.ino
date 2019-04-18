// BRIAN SUMNER
// IN AFFILIATION WITH KAMIL ADYLOV AND PHI HUYNH
// UCDENVER CSCI 4287
// SPRING 2019
// LAZERBOY ENTERTAINMENT SYSTEM:
// LAZERGUN DRIVER
// MODEL M9B2
// VERSION: BETA_05


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
typedef struct timer16_t timer16_t;
struct timer16_t
{
  uint16_t flag_isEnabled : 1;
  uint16_t count          : 15;
  uint16_t maxCount       : 15; 
  uint16_t flag_doEvent   : 1; 
};


// GLOBAL CONSTANTS
const double MAX_TIMER_ISR_COUNT = ((CPU_MHZ * 1000.0) / TIMER_PRESCALAR * TIMER_INTERVAL_MILLISECONDS);


// GLOBAL VARIABLES
// NOTE:  SAFETY MODE IS IMPLEMENTED BY DISABLING THE TRIGGER AT SYSTEM START
uint8_t firingMode = MODE_SEMI_AUTOMATIC;


// ISR VARIABLES
// NOTE: ISR VARIABLES MUST BE DECLARED VOLATILE
volatile timer16_t timer_triggerDebounce =  {0, 0, TIMER_TRIGGER_DEBOUNCE_MAX_COUNT, 0};
volatile timer16_t timer_triggerReset =     {0, 0, TIMER_TRIGGER_RESET_MAX_COUNT, 0};
volatile timer16_t timer_laserReset =       {0, 0, TIMER_LASER_RESET_MAX_COUNT, 0};

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

// END SETUP
}


// MAIN LOOP
void loop() 
{

  // ON FIRE LASER EVENT
    // FIRE LASER
    // CLEAR DO FIRE LASER FLAG

  if (flag_doFireLaser)
  {
    digitalWrite(PIN_LASER_OUT, HIGH);
//    digitalWrite(PIN_TRIGGER_OUT, HIGH);
    flag_doFireLaser = 0;
  }


  // ON LASER RESET EVENT
    // STOP FIRING LASER
    // CLEAR RESET LASER DO EVENT FLAG

  if (timer_laserReset.flag_doEvent)
  {  
    digitalWrite(PIN_LASER_OUT, LOW);
//    digitalWrite(PIN_TRIGGER_OUT, LOW);
    timer_laserReset.flag_doEvent = 0;
  }


  // ON TRIGGER DEBOUNCE TIMER EVENT
    // IF TRIGGER PIN HIGH
      // CLEAR TRIGGER DEBOUNCE DO EVENT FLAG
      // SET TRIGGER RESET TIMER COUNT AS MAX COUNT
      // ENABLE TRIGGER RESET TIMER
    // ELSE
      // RESET TRIGGER DEBOUNCE TIMER COUNT AS MAX COUNT
      // ENABLE TRIGGER DEBOUNCE TIMER

  if (timer_triggerDebounce.flag_doEvent)
  {  
    if (digitalRead(PIN_TRIGGER_IN) == HIGH)
    {
      timer_triggerDebounce.flag_doEvent = 0;
      timer_triggerReset.count = timer_triggerReset.maxCount;
      ++timer_triggerReset.flag_isEnabled;      
    }
    else
    {
      timer_triggerDebounce.count = timer_triggerDebounce.maxCount;
      ++timer_triggerDebounce.flag_isEnabled;      
    }
  }


  // ON TRIGGER RESET TIMER EVENT
    // CLEAR TRIGGER RESET DO EVENT FLAG
    // ENABLE TRIGGER INPUT

  if (timer_triggerReset.flag_doEvent)
  {
    timer_triggerReset.flag_doEvent = 0;
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
  
// END LOOP
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

  if (flag_isTriggerEnabled)
  {
    flag_isTriggerEnabled = 0;
    ++flag_doFireLaser;
    timer_triggerDebounce.count = timer_triggerDebounce.maxCount;
    ++timer_triggerDebounce.flag_isEnabled;
    timer_laserReset.count = timer_laserReset.maxCount;
    ++timer_laserReset.flag_isEnabled;
  }
  
// END TRIGGER SWITCH ISR
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

// END SLIDE SWITCH ISR
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
      
  if (timer_triggerDebounce.flag_isEnabled)
  {
    if (timer_triggerDebounce.count <= 0)
    {
      timer_triggerDebounce.flag_isEnabled = 0;
      ++timer_triggerDebounce.flag_doEvent;
    }
    else
    {
      --timer_triggerDebounce.count;
    }
  }
  

  // IF TRIGGER RESET TIMER ENABLED
    // IF COUNT <= 0 
      // DISABLE TIMER
      // SET DO EVENT FLAG
    // ELSE 
      // DECREMENT COUNT

  if (timer_triggerReset.flag_isEnabled)
  {
    if (timer_triggerReset.count <= 0)
    {
      timer_triggerReset.flag_isEnabled = 0;
      ++timer_triggerReset.flag_doEvent;
    }
    else
    {
      --timer_triggerReset.count;
    }
  }
  

  // IF LASER RESET TIMER ENABLED
    // IF COUNT <= 0 
      // DISABLE TIMER
      // SET DO EVENT FLAG
    // ELSE 
      // DECREMENT COUNT

  if (timer_laserReset.flag_isEnabled)
  {
    if (timer_laserReset.count <= 0)
    {
      timer_laserReset.flag_isEnabled = 0;
      ++timer_laserReset.flag_doEvent;
    }
    else
    {
      --timer_laserReset.count;
    }
  }

// END TIMER1 ISR
}
