////////////////////////////////////////////////////////////////////////////
//HEADERS///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>

////////////////////////////////////////////////////////////////////////////
//GLOBAL VARIABLES//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

int lcount=0,rcount=0,frontdet=0;                     //odometric counts
int lrevs=0,rrevs=0;
long int risingr, fallingr,cr=0,risingl,fallingl,cl;
volatile long int countsr,countsl;                    //timer counts
volatile int distr,distl;
int i=0,j=0,k=0;

////////////////////////////////////////////////////////////////////////////
//INITIALIZATIONS///////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void ddrinit()
{
  DDRB|=1<<PB7;
  DDRG|=1<<PG5;
  DDRC |= 1 << PINC4 | 1<<PINC5;
  PORTC &= ~(1 << PINC4);
  PORTC &= ~(1 << PINC5);
  DDRA=0b11111111;
}
void exintinit()
{
  EICRA|=(1<<ISC00);
  EIMSK|=(1<<INT0);
  EICRA|=(1<<ISC10);
  EIMSK|=(1<<INT1);
  EICRA|=(1<<ISC20)|(1<<ISC21);
  EIMSK|=(1<<INT2);
}
void timer0init()
{
  TCCR0A|=(1<<WGM00)|(1<<WGM01)|(1<<COM0A1)|(1<<COM0B1);
  TCCR0B|=(1<<CS01);
  TIMSK0|=(1<<OCIE0A)|(1<<OCIE0B);
}
void timer4init()
{
  TCCR4A |= (1 << WGM41);
  TCCR4B |=  (1 << CS41)|(1 << ICES4);
  TIMSK4 |= (1 << OCIE4A) | (1 << ICIE4);
  OCR4A = 200;
}
void timer5init()
{
  TCCR5A |= (1 << WGM51);
  TCCR5B |=  (1 << CS51) | (1 << ICES5);
  TIMSK5 |= (1 << OCIE5A) | (1 << ICIE5);
  OCR5A = 200; 
}

////////////////////////////////////////////////////////////////////////////
//SENSOR AND MOTOR CONTROL FUNCTIONS////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void setduty(int l,int r)
{
  OCR0A=255*(l)/100;
  OCR0B=255*(r)/100;
}
float distst()
{
  return (float(rrevs+lrevs)/2+(float(rcount+lcount)/2)/40)*(8*34)*.8;
}
void odoreset()
{
  cli();
  lrevs=0;
  lcount=0;
  rrevs=0;
  rcount=0;
  sei();
}
float angturn()
{
  return distst()*180.0/(3.1*80);
}
void freeforward()
{
  while(!frontdet)
  {
    if((distl>=7 || distr<=6) && distl<30)
    {
      setduty(70,100);
    }
    else if((distr>=7 || distl<=6 )&& distr<30)
    {
      setduty(100,70);
    } 
    else setduty(100,100);
    PORTA|=(1<<0)|(1<<3);
  }
  PORTA=0;
}
void revtrans(int dis)
{
  setduty(100,100);
  odoreset();
  PORTA=0;
  while((dis-distst())>=0)
  {
    PORTA|=(1<<1)|(1<<2);
  }
  PORTA=0;
  odoreset();
}
void fronttrans(int dis)
{
  setduty(100,100);
  odoreset();
  PORTA=0;
  while((dis-distst())>=0)
  {
    PORTA|=(1<<0)|(1<<3);
  }
  PORTA=0;
  odoreset();
}
void leftrot(int ang)
{
  setduty(100,100);
  odoreset();
  PORTA=0;
  while(angturn()<=ang)
  {
     PORTA|=(1<<0)|(1<<2);
  }
  PORTA=0;
  odoreset();
}
void rightrot(int ang)
{
  setduty(100,100);
  odoreset();
  PORTA=0;
  while(angturn()<=ang)
  {
     PORTA|=(1<<1)|(1<<3);
  }
  PORTA=0;
  odoreset();
}

/////////////////////////////////////////////////////////////////////////
//MAIN///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

int main(void)
{
  ddrinit();
  timer0init();
  timer4init();
  timer5init();
  exintinit();
  setduty(100,100);
  sei();
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  while(1)
  {
   freeforward();
   rightrot(180);
   frontdet=0; 
   PORTA=0;
  }
  while(1){
  }
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}  
///////////////////////////////////////////////////////////////////////////
//SERVICE ROUTINES/////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////


ISR(INT0_vect)
{
  lcount++;
  if(lcount>=40)
  {
    lcount=0;
    lrevs++;
  }
}
ISR(INT1_vect)
{
  rcount++;
  if(rcount>=40){
    rcount=0;
    rrevs++;
  }
}
ISR(INT2_vect)
{
  frontdet=1;
}
ISR(TIMER0_COMPA_vect)
{
  
}
ISR(TIMER0_COMPB_vect)
{
  
}
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
  if(i==1400)
  {
    PORTC |= 1 << PINC4;
    _delay_us(12);
    PORTC &= ~(1 << PINC4);
    i=0;
  }
    
  else
  {
    i++;
  }
}
ISR (TIMER5_COMPA_vect)
{
  if(k==1400)
  {
    PORTC |= 1 << PINC5;
    _delay_us(12);
    PORTC &= ~(1 << PINC5);
    k=0;
  }
    
  else
  {
    k++;
  }
}
