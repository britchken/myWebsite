#include "matchSP430.h"


#define PACKET_FIRST 0x01
#define PACKET_LAST 0x02
#define PACKET_BIO 0x03
#define PACKET_GENDER 0x04
#define PACKET_INTEREST 0x05
#define PACKET_MESSAGE 0x10
#define PACKET_MATCH 0x20


mrfiPacket_t *main_packet = NULL;


void init_bloop() {
	uint8_t address[] = {192,168,41,my_profile->address};
	DCOCTL = 0;
	BSP_Init();
	MRFI_Init();
	MRFI_WakeUp();
	MRFI_SetRxAddrFilter(address);
	MRFI_EnableRxAddrFilter();
	MRFI_RxOn();


	main_packet = (mrfiPacket_t *) malloc(sizeof(mrfiPacket_t));
}


char alreadyConnected (char a){
	if (connections == NULL)
		return 0;


	matchQ_t *temp = connections;
	while (temp != NULL){
		if (temp -> value -> address == a)
			return 1;
		temp = temp -> next;
	}
	return 0;
}


void bloop(char *mess, uint8_t dest) {
	mrfiPacket_t tpacket;


	tpacket.frame[0] = strlen(mess) + 8;


	tpacket.frame[1] = 192;  //Destination
	tpacket.frame[2] = 168;
	tpacket.frame[3] = 41;
	tpacket.frame[4] = dest;


	tpacket.frame[5] = 192;  //Source
	tpacket.frame[6] = 168;
	tpacket.frame[7] = 41;
	tpacket.frame[8] = my_profile->address;


	strcpy( (char *) &tpacket.frame[9] , mess);


	MRFI_Transmit(&tpacket , MRFI_TX_TYPE_FORCED);
}


int isMatched(int addre) {
	while (matches->next != NULL) {
			if (matches->value->address == addre) {  //If you are matched with them
				return 1;
			}
	}
	return 0;
}


int isConnected(int addre) {
	while (connections->next != NULL) {
			if (connections->value->address == addre) {  //If you are matched with them
				return 1;
			}
	}
	return 0;
}


void addMatch(int addr) {  //This function moves a matchQ_t from connections to matches
	matchQ_t *searcher = *connections;
	matchQ_t *onebehindsearcher;
	while (searcher->value->address!=addr) { //find the right node
		*onebehindsearcher = searcher;
		*searcher = searcher->next;
	}


	*onebehindsearcher->next = *searcher->next;   //Delete it from connections


	new->next = matches;  //Add it to the front of matches
	matches = *searcher;
}


void addConnected(void) {
	matchQ_t *new = (matchQ_t *) malloc(sizeof(matchQ_t));
	new->next = connections;
	connections = *new;
}


void addInfo(char *data, int type) {
	if (type == PACKET_FIRST) {
		addConnected();
		connections->value->first_name = *data;
	} else if (type == PACKET_LAST) {
		connections->value->last_name = *data;
	} else if (type == PACKET_BIO) {
		connections->value->bio = *data;
	} else if (type == PACKET_GENDER) {
		connections->value->gender = *data;
	} else if (type == PACKET_INTEREST) {
		connections->value->interest = *data;
	}
}


void recievedBloop() {
	int sender = main_packet->frame[8]-'0';
	int datatype = main_packet->frame[9];


	if(datatype < 0x06) {                      //If adding info
		char *packetdata = (char *) &packet.frame[10];
		addInfo(*packetdata, datatype);
	} else if(datatype == PACKET_MESSAGE) {    //If sending message
		char *packetdata = (char *) &packet.frame[10];
		if (isMatched(sender)){
			uart_puts(&packetmessage[9]);
			new_line();
		}
	} else if(datatype == PACKET_MATCH) {      //If sending match
		if (isConnected(sender)) {
			addMatch(sender);
		}
	}
}
