/*  HC_SR04 ULTRASONIC SENSOR using Atmega2560
 *  Trigger pins = PC4  and PC5/ Digital pin 33 and 34
 *  Echo pin    = ICP4 and ICP5/ Digital pin 49 and 50
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

long int risingr, fallingr,cr=0,risingl,fallingl,cl;
volatile long int countsr,countsl;
volatile int distr,distl;
double us_per_count;
int i=0,j=0,k=0;



ISR (TIMER4_CAPT_vect)
{
  
  if (TCCR4B & (1<<ICES4)) 
  {
    TCCR4B &= ~(1<<ICES4); 
    risingr =  ICR4+i*2000; 
  }
  else
  {
    TCCR4B |= (1<<ICES4); 
    fallingr = ICR4+i*2000; 
    countsr = fallingr - risingr;
    distr =  countsr *340/200000;
  }
}

ISR (TIMER5_CAPT_vect)
{
  
  if (TCCR5B & (1<<ICES5)) 
  {
    TCCR5B &= ~(1<<ICES5); 
    risingl =  ICR5+k*2000; 
  }
  else
  {
    TCCR5B |= (1<<ICES5); 
    fallingl = ICR5+k*2000; 
    countsl = fallingl - risingl;
    distl =  countsl *340/200000;
  }
}
ISR (TIMER4_COMPA_vect)
{
    if(i==1400){
    PORTC |= 1 << PINC4;
    _delay_us(12);
    PORTC &= ~(1 << PINC4);
    i=0;
    }
    
    else{
    i++;}
}
ISR (TIMER5_COMPA_vect)
{
    if(k==1400){
    PORTC |= 1 << PINC5;
    _delay_us(12);
    PORTC &= ~(1 << PINC5);
    k=0;
    }
    
    else{
    k++;}
}
void init_timer4()
{
  TCCR4A |= (1 << WGM41);
   
  TCCR4B |=  1 << CS41;

  TIMSK4 |= 1 << OCIE4A | 1 << ICIE4;
  
  TCCR4B |= 1 << ICES4; 
  
  OCR4A = 200;
  
  sei();
  us_per_count = .5; 
}
void init_timer5()
{
  TCCR5A |= (1 << WGM51);
   
  TCCR5B |=  1 << CS51;

  TIMSK5 |= 1 << OCIE5A | 1 << ICIE5;
  
  TCCR5B |= 1 << ICES5; 
  
  OCR5A = 200; 
}

int main()
{
  DDRC |= 1 << PINC4 | 1<<PINC5;
  PORTC &= ~(1 << PINC4);
  PORTC &= ~(1 << PINC5);
  init_timer4();
  init_timer5();
  _delay_ms(965);
  Serial.begin(9600);
  while(1)
  {

    Serial.print(distl);
    Serial.print("       ");
    Serial.println(distr);
  }
}
