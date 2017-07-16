/*  HC_SR04 ULTRASONIC SENSOR using Atmega2560
 *  Trigger pin = PC4 / Digital pin 33
 *  Echo pin    = ICP4 / Digital pin 49
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#define LJ 7
#define RJ 6
#define FJ 5
#define LE 4
#define RE 3
#define FE 2
#define BE 1
#define EF 0

long long int risingr, fallingr,cr=0,risingl,fallingl,cl,flag;
volatile long long int countsr,countsl;
volatile long int distr,distro=0,distl;
int i=0,j=0,k=0,check=0,l30;
struct node
{
    char data;
    struct node *next;
};
struct node *top = NULL;
ISR (TIMER1_CAPT_vect)
{
  
  if (TCCR1B & (1<<ICES1)) 
  {
    TCCR1B &= ~(1<<ICES1); 
    risingr =  ICR1; 
    check=0;
  }
  else
  {
    check=1;
    TCCR1B |= (1<<ICES1); 
    fallingr = ICR1; 
    countsr = fallingr - risingr;
    distr =  (countsr*686)/10000;
    if(distr<0){
      distr=distro;
    }
    else{
      if(distro-distr>30){
      flag=1;
    }
    else{
      flag=0;
    }
    }
    distro=distr;
  }
}
ISR (TIMER1_COMPB_vect){
  if(check==1){
    l30=1;
  }
  else if(check==0){
    l30=0;
  }
}
ISR (TIMER1_COMPA_vect)
{
    PORTB |= 1 << PINB4;
    _delay_us(8);
    PORTB &= ~(1 << PINB4);
}
void init_timer1()
{
  TCCR1A |= (1 << WGM11);
   
  TCCR1B |=  1 << CS11 | 1 << CS10;

  TIMSK1 |= 1 << OCIE1A | 1 << ICIE1 | 1<<OCIE1B;
  
  TCCR1B |= 1 << ICES1; 
  
  OCR1A = 17500;
  OCR1B = 490;
  
  sei();
}
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
int main()
{
  DDRB |= 1 << PINB4;
  PORTB &= ~(1 << PINB4);
  init_timer1();
  _delay_ms(965);
  Serial.begin(9600);
  while(1)
  {
    if(l30==1){
    Serial.println(distr);
    }
    else{
     Serial.println('a');
    }
  }
}
