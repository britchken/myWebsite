/*------------------------------------------------------------
 * Chat application!
 *   Chat with friends!
 *----------------------------------------------------------*/
#include "bsp.h"
#include "mrfi.h"
#include "radios/family1/mrfi_spi.h"
#include <string.h>

#define RED_LED 0x01
#define GREEN_LED 0x02

#define addr 5
#define addrs "5"


char message[] = "";
int friend;
char *sadd;
int packetready;
mrfiPacket_t	packet;


char encode() {
	return '0' + addr;
}


void bloop(char *mess, uint8_t dest) {
	mrfiPacket_t 	tpacket;


	packet.frame[0] = strlen(mess) + 9;


	packet.frame[1] = 192;  //Destination
	packet.frame[2] = 168;
	packet.frame[3] = 41;
	packet.frame[4] = dest;

	packet.frame[5] = 192;  //Source
	packet.frame[6] = 168;
	packet.frame[7] = 41;
	packet.frame[8] = encode();


	strcpy( (char *) &tpacket.frame[9] , mess);


	MRFI_Transmit(&tpacket , MRFI_TX_TYPE_FORCED);
}


void sleep(unsigned int count);


void init_uart(void) {
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL  = CALDCO_1MHZ;
	P3SEL = 0x30;
	UCA0CTL1 |= UCSSEL_2;
	UCA0BR0 = 104;
	UCA0BR1 = 0;
	UCA0MCTL = UCBRS0;
	UCA0CTL1 &= ~UCSWRST;
	IE2  |=  UCA0RXIE;


}


void uart_putc(char c) {
	while(!(IFG2 & UCA0TXIFG));
	UCA0TXBUF = c;
}


void uart_puts(char *str) {
	while (*str) {
		/* Replace newlines with \r\n carriage return */
		if(*str == '\n') { uart_putc('\r'); }
		uart_putc(*str++);
	}
}
void uart_clear_screen(void) {
	uart_putc(0x1B);
	uart_puts("[2J");
	uart_putc(0x1B);
	uart_puts("[0;0H");
}


void MRFI_RxCompleteISR(void) {
	/* Read the received data packet */
	MRFI_Receive(&packet);
	packetready = 1;
}


void main(void) {
	packetready=0;
	WDTCTL = WDTPW + WDTHOLD;


	sadd = addrs; 		//String of your own address
	friend = 0;
	uint8_t address[] = {192,168,41,addr}; //your unique MATCHSP430 ID
	__enable_interrupt();
	init_uart();


	unsigned char status;


	BSP_Init();
	P1DIR = RED_LED + GREEN_LED;
	P1OUT = GREEN_LED;


	MRFI_Init();
	MRFI_WakeUp();


	status = MRFI_SetRxAddrFilter(address);
	MRFI_EnableRxAddrFilter();
	if( status != 0) {
		P1OUT = RED_LED | GREEN_LED;
		while(1);
	}


	MRFI_RxOn();


	__bis_SR_register(GIE);


	while(1){
		if (packetready) {
			char *packetmessage = (char *) &packet.frame[8];
			int sender = packetmessage[0]-'0';


			if (strcmp(&packetmessage[1], "searchingforfriends")==0) {    //If you just got your own address, you’re matched!
				uart_puts("Someone just found you!\n");
				friend = sender;
				sleep(50000);
				bloop("youfoundsomeone", friend);
			} else if (strcmp(&packetmessage[1], "youfoundsomeone")==0) {
				friend = sender;
				uart_puts("You found someone and you can now chat!!!");
			} else {
				uart_puts(&packetmessage[1]);  //Display the message
			}
			packetready = 0;
		}
	}
}


void sleep(unsigned int count) {
	int i;
	for (i = 0; i < 10; i++) {
		while(count > 0) {
			count--;
			__no_operation();
		}
	}
}


void new_line(void){
	uart_putc(0x1B);		/* Escape character */
	uart_puts("[u");		/* Move cursor to 0,0 */
	uart_putc(0x1B);
	uart_puts("[1B");


	uart_putc(0x1B);		/* Escape character */
	uart_puts("[s");
}


void backup(void){
	if(strlen(message) == 0){
		return;
	}
	char *str = &message[0];
	while (*str){
		str++;
	}
	str--;
	*str = '\0';
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR (void)
{
	int i;
	char b = UCA0RXBUF;
	P1OUT ^= RED_LED;


	if (b == 8 || b == 0x7f) {
		uart_putc(0x7f);
		backup();
	}
	else if (b == 13) { //If the key pressed is the enter key
		uart_putc(b);
		if (strcmp(message,"#clear") == 0) {
			uart_clear_screen();
			uart_putc(0x1B);
			uart_puts("[s");
		} else if (strcmp(message,"#search") == 0) {
			uart_puts("searching"); //add handshake*
			for (i = 0; i<10; i++) {
				if (i != addr) {
					bloop("searchingforfriends",i);
					uart_putc('.');
				}
			}
			new_line();
		} else {
			strcat(message, "    ");
			bloop(message, friend);
			new_line();
		}
		P1OUT ^= RED_LED;
		message[0] = '\0';
	} else {
		uart_putc(b);
		int leng = strlen(message);
		message[leng] = b;
		message[leng+1] = 0;
	}
}