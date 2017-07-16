////////////////////////////////////////////////////////////////////////////
//HEADERS///////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <math.h>
#define LJ 7
#define RJ 6
#define FJ 5
#define LE 4
#define RE 3
#define FE 2
#define BE 1
#define EF 0
#define lthresh 7
#define rthresh 7

////////////////////////////////////////////////////////////////////////////
//GLOBAL VARIABLES//////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

int lcount=0,rcount=0,frontdet=0,dflag;                     //odometric counts
int lrevs=0,rrevs=0;
long long int risingr, fallingr,cr=0,risingl,fallingl,cl;
volatile long long int countsr,countsl;                    //timer counts
volatile long int distr,distro=0,distl,distlo=0;
int i=0,j=0,k=0,lr30=1,ll30=1,checkl,checkr,ll30o,lr30o;

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
  TCCR4B |=  (1 << CS41)|(1 << CS40)|(1 << ICES4);
  TIMSK4 |= (1 << OCIE4A) |(1 << OCIE4B) | (1 << ICIE4);
  OCR4A = 17500;
  OCR4B = 490;
}
void timer5init()
{
  TCCR5A |= (1 << WGM51);
  TCCR5B |=  (1 << CS51)|(1 << CS50) | (1 << ICES5);
  TIMSK5 |= (1 << OCIE5A) | (1<<OCIE5B) | (1 << ICIE5);
  OCR5A = 17500;
  OCR5B = 490;
}
////////////////////////////////////////////////////////////////////////////
//STACK DEFINITION AND FUNCTIONS////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

struct node
{
    char data;
    struct node *next;
};

struct node *top = NULL;

void push(char item)
{
    struct node *nptr = malloc(sizeof(struct node));
    nptr->data = item;
    nptr->next = top;
    top = nptr;
}

void traverse()
{
    struct node *temp;
    temp = top;
    while (temp != NULL)
    {
        temp = temp->next;
    }
}

void pop()
{
    if (top == NULL)
    {
        
    }
    else
    {
        struct node *temp;
        temp = top;
        top = top->next;
        
        free(temp);
    }
}
void revtaken(){
  struct node *temp;
    temp = top;
    while (temp != NULL)
    {
        temp->data ^= (1<<LJ)|(1<<RJ)|(1<<FJ);
        temp = temp->next;
    }
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
/*void freeforward()
{
  while(!frontdet)
  {
    if((distl>=7 || distr<=6) && distl<20)
    {
      setduty(70,100);
    }
    else if((distr>=7 || distl<=6 )&& distr<20)
    {
      setduty(100,70);
    } 
    else setduty(100,100);
    PORTA|=(1<<0)|(1<<3);
  }
  PORTA=0;
}*/
void freeforward()
{
  while(!frontdet)
  {
    int el,er;
    el= abs(lthresh-distl);
    er= abs(rthresh-distr);
    if(lr30 && ll30){
      if(distr<=rthresh-1 && distl > lthresh){
        setduty(100-el*6,100);
      }
      if(distr>rthresh && distl< lthresh-1){
        setduty(100,100-er*6);
      }
      else setduty(100,100);
    }
    if(lr30 && !ll30){
      if(distr<rthresh-1){
        setduty(100-er*3,100);
      }
      else setduty(100,100);
    }
    if(!lr30 && ll30){
      if(distl<lthresh-1){
        setduty(100,100-el*3);
      }
      else setduty(100,100);
    }
    if(!ll30 && !lr30){
      setduty(100,100);
    }

    if(dflag)//if sudden discontinuity occurs
    {
      push(junctiondet());
      //create node to describe the paths available
      //push into stack
      dflag=0;
    }
    PORTA|=(1<<0)|(1<<3);
  }
  push(junctiondet());
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
//DECISION MAKING////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void dec()
{
  if(!ll30)
  {
    leftrot(90);
  }
  if(!lr30)
  {
    rightrot(90);
  }
  if(ll30 && lr30)
  {
    revtaken();
    PORTA=0;
    _delay_ms(5000);
  }
}
char junctiondet(){
  char ch=0;
  if(ll30){
    ch |= (1<<LJ);
  }
  if(lr30){
    ch |= (1<<RJ);
  }
  if(frontdet){
    ch |= (1<<FJ);
  }
  ch |= 1<<BE;
  return ch;
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
   dec();
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
    checkr=0;
    TCCR4B &= ~(1<<ICES4); 
    risingr =  ICR4; 
  }
  else
  {
    checkr=1;
    TCCR4B |= (1<<ICES4); 
    fallingr = ICR4; 
    countsr = fallingr - risingr;
    distr =  (countsr *686)/20000;
  if(distr<0){
      distr=distro;
  }
  else
  {
      
  }
  distro=distr;
}
}

ISR (TIMER5_CAPT_vect)
{
  
  if (TCCR5B & (1<<ICES5)) 
  {
    checkl=0;
    TCCR5B &= ~(1<<ICES5); 
    risingl =  ICR5; 
  }
  else
  {
    checkl=1;
    TCCR5B |= (1<<ICES5); 
    fallingl = ICR5; 
    countsl = fallingl - risingl;
    distl =  (countsl *686)/20000;
  }
}
ISR (TIMER4_COMPA_vect)
{
    PORTC |= 1 << PINC4;
    _delay_us(8);
    PORTC &= ~(1 << PINC4);
}
ISR (TIMER5_COMPA_vect)
{
    PORTC |= 1 << PINC5;
    _delay_us(8);
    PORTC &= ~(1 << PINC5);
}

ISR (TIMER4_COMPB_vect){
  if(checkr==1){
    lr30=1;
  }
  else if(checkr==0){
    lr30=0;
    if(lr30o)
      dflag=1;
    lr30o=lr30;
  }
}
  
ISR (TIMER5_COMPB_vect){
  if(checkl==1){
    ll30=1;
  }
  else if(checkl==0){
    ll30=0;
    if(ll30o)
      dflag=1;
    ll30o=ll30;
  }
}
