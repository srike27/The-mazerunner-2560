#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include<avr/interrupt.h>
int lcount=0,rcount=0;
double tim=0;
int counter=0,i=0,lrevs=0,rrevs=0;
int ovf=0,j=0;
void timer0init(){
  TCCR0A|=(1<<WGM00)|(1<<WGM01)|(1<<COM0A1)|(1<<COM0B1);
  TCCR0B|=(1<<CS01);
  TIMSK0|=(1<<OCIE0A)|(1<<OCIE0B);
}
void setduty(int l,int r){
  OCR0A=255*(l)/100;
  OCR0B=255*(r)/100;
}
float distst(){
  return (float(rrevs)+(float(rcount))/40)*(8*34)*.8;
}
void odoreset(){
  cli();
  lrevs=0;
  lcount=0;
  rrevs=0;
  rcount=0;
  sei();
}
float angturn(){
  return distst()*180.0/(3.1*80);
}
void revtrans(int dis){
  odoreset();
  PORTA=0;
  double e,eo,sum,kp,ki,kd;
  while((e=dis-distst())>=0){
    PORTA|=(1<<1)|(1<<2);//FRONT
  }
  PORTA=0;
  odoreset();
}
void fronttrans(int dis){
  odoreset();
  PORTA=0;
  double e,eo,sum,kp,ki,kd;
  while((e=dis-distst())>=0){
    PORTA|=(1<<0)|(1<<3);//REVERSE
  }
  PORTA=0;
  odoreset();
}
void leftrot(int ang){
  odoreset();
  PORTA=0;
  while(angturn()<=ang){
     PORTA|=(1<<0)|(1<<2);//RIGHT
  }
  PORTA=0;
  odoreset();
}
void rightrot(int ang){
  odoreset();
  PORTA=0;
  while(angturn()<=ang){
     PORTA|=(1<<1)|(1<<3);//LEFT
  }
  PORTA=0;
  odoreset();
}
int main(void){
    DDRB|=1<<PB7;
  DDRG|=1<<PG5;
  timer0init();
  setduty(75,75);
  DDRA=0b11111111;
  sei();
  EICRA|=(1<<ISC00);
  EIMSK|=(1<<INT0);
  EICRA|=(1<<ISC10);
  EIMSK|=(1<<INT1);
  while(1){
    fronttrans(100);
    rightrot(90);
    }
  
  
}
ISR(INT0_vect){
  lcount++;
  if(lcount>=40){
    lcount=0;
    //PORTB&=(0<<1);
    lrevs++;
  }
}
ISR(INT1_vect){
  rcount++;
  if(rcount>=40){
    rcount=0;
    //PORTB&=(0<<2);
    rrevs++;
  }
}
ISR(TIMER0_COMPA_vect){
  
}
ISR(TIMER0_COMPB_vect){
  
}
