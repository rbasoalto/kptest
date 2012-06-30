#include <msp430.h>
#include <stdio.h>
#define interrupt(x) void __attribute__((interrupt (x)))
#define uint unsigned int
/**************************
 * KEYPAD CONNECTIONS
 * A0-A3 -> P1.4 - P1.7
 * B0-B3 -> P2.0 - P2.3
 **************************/

static const char keys[][4] = {{'1','2','3','A'},{'4','5','6','B'},{'7','8','9','C'},{'*','0','#','D'}};

 inline void uart_init() {
  UCA0CTL1 |= UCSWRST;
  //initialize module
  UCA0CTL0 = 0;
  UCA0CTL1 = UCSWRST | UCSSEL_3; // source from smclk at 1mhz
  
  UCA0BR0 = 104u;
  //UCA0BR1 = 0u;
  // UCA0MCTL = 2u<<1;
  
  UCA0STAT = 0;
  
  UCA0IRTCTL = 0;
  UCA0IRRCTL = 0;
  
  UCA0ABCTL = 0;  
  //configure ports
  
  P1SEL |= BIT1 | BIT2;
  P1SEL2 |= BIT1 | BIT2;
  
  UCA0CTL1 &= ~UCSWRST;
  //enable interrupts?
  
  IE2 &= ~(UCA0TXIE | UCA0RXIE);
  IFG2 &= ~(UCA0TXIFG | UCA0RXIFG);  
  
}

void uart_putc(char c) {
  while(UCA0STAT & UCBUSY);
  UCA0TXBUF = c;
}

char kp_getc() {
  uint a;
  uint read = 0;
  char ret = 0;
  for(a = 0; a < 4; a++) {
    P1OUT = (0x10 << a) | (P1OUT & 0x0F);
    read = P2IN & 0x0F;
    if(read) {
      int nread = 0;
      while(!((1<<nread) & read)) nread++;
      P1OUT ^= BIT0;
      ret = keys[nread][a];
      while(P2IN & read);
    }
  }
  P1OUT &= 0x0F;
  return ret;
}

int main() {
  
  WDTCTL = WDTPW | WDTHOLD;
  DCOCTL = 0;
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;
  
  uart_init();
  
  _BIS_SR(GIE);
  
  P1DIR |= 0xF0; // bits 4-7 output
  P1DIR |= BIT0; // bit0 (led1) output
  P1OUT = 0;
  
  P2DIR &= ~0x0F; // bits 0-3 input
  P2REN |= 0x0F; // resistors enabled
  P2OUT &= ~0x0F; // pulldown resistors
  
  while(23) {
    char read = kp_getc();
    if(read) putchar(read);
  } 
  
  LPM4;
  
  return 0;
  
}
