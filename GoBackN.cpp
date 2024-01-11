#include "includes.h"


#define MAX_SEQUENCE_NUM 7
#define MAX_WINDOW_SIZE 10


// ***************************************************************************
// * ALTERNATING BIT AND GO-BACK-N NETWORK EMULATOR: VERSION 1.1  J.F.Kurose
// *
// * These are the functions you need to fill in.
// ***************************************************************************

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity A routines are called. You can use it to do any initialization
// ***************************************************************************

//global variables
int windowSize = 10; //space to cahce 10 messageson the sender
int base = 1; //beginning of sender window
int nextSequenceNum = 0;
int N = 10;
int timerValue = 0;
const int ACK = 1;
struct pkt ackPacket;

struct pkt sentPackets[MAX_WINDOW_SIZE];
struct pkt unAckPacks[MAX_WINDOW_SIZE]; //store unacknowledged packets

float packetStartTimes[MAX_WINDOW_SIZE];

void A_init() {
	base = 1;
	nextSequenceNum = 1;
	timerValue = 100;
	windowSize = 10;

	for (int i = 0; i < MAX_WINDOW_SIZE; i++) {
		unAckPacks[i].seqnum = -1; //invalid seq num
		unAckPacks[i].acknum = -1; //invalid ack num
		unAckPacks[i].checksum = 0;
		memset(unAckPacks[i].payload, 0, sizeof(unAckPacks[i].payload));
	}
}

// ***************************************************************************
// * The following routine will be called once (only) before any other
// * entity B routines are called. You can use it to do any initialization
// ***************************************************************************
int expectedSequenceNum = 1;
int receivedPackets = 0;
int acknowledgementFlag = 0;
int maxPacketBufferSize = 0;
int senderWindow = 10;

void B_init() {
	expectedSequenceNum = 1;
}

int inputChecksum(struct pkt packet) {
	int checksum = 0;

	checksum += packet.seqnum;
	checksum += packet.acknum;

	for (int i = 0; i < sizeof(packet.payload); i++) {
		checksum += static_cast<unsigned char>(packet.payload[i]);
	}

	return checksum;
}

struct pkt make_pkt(int sequenceNumber, const char data[20], int ackNumber, int checksum) {
	struct pkt packet;
	packet.seqnum = sequenceNumber;
	packet.acknum = ackNumber;
	if (data != nullptr) {
		strncpy(packet.payload, data, sizeof(packet.payload));
		packet.checksum = inputChecksum(packet);
	} else {
		memset(packet.payload, 0, sizeof(packet.payload));
		packet.checksum = checksum;
	}
	
	return packet;
}

bool is_corrupt(struct pkt packet) {

	int calculated_checksum = inputChecksum(packet);
	return packet.checksum != calculated_checksum;
}

bool has_seqnum(struct pkt packet, int seqnum) {
	return packet.seqnum == seqnum;
}

int get_acknum(struct pkt packet) {
	return packet.acknum;
}

bool if_corrupt(struct pkt packet) {
	return is_corrupt(packet);
}

void extract(const struct pkt& packet, struct msg& message) {
	strncpy(message.data, packet.payload, sizeof(message.data));
}

float EstimatedRTT = 0.0;
const float alpha = 0.125;
float SampleRTT = 0.0;

void refuse_data(const char data[20]) {
	INFO << "Window is full. Can't send more Data: " << data << ENDL;
}

// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side 
// ***************************************************************************
bool rdt_sendA(struct msg message) {
	
	INFO << "INFO: RTD_SEND_A: Layer 4 on side A has received a message from the application that sound be sent to side B:" << " (seq = " << nextSequenceNum << ", ack = " << nextSequenceNum - 1 << ", chk = " << inputChecksum(sentPackets[nextSequenceNum % MAX_WINDOW_SIZE]) << ") " << message.data << ENDL;
		
	INFO << "INFO: TOLAYER3 (" << simulation->getSimulatorClock() << "): " << "1 packets in flight to side B (" << nextSequenceNum << ", " << nextSequenceNum << ", " << inputChecksum(sentPackets[nextSequenceNum % MAX_WINDOW_SIZE]) << ") " << message.data << ENDL;

	if (nextSequenceNum < base + N) {
		//create a packet
		struct pkt packet = make_pkt(nextSequenceNum, message.data, 0, 0);
		
		int index = nextSequenceNum % MAX_WINDOW_SIZE;
		
		sentPackets[index] = packet;

		packetStartTimes[index] = simulation->getSimulatorClock();

		packet.checksum = inputChecksum(packet);

		simulation->udt_send(A,packet);

		if (base == nextSequenceNum) {
			simulation->start_timer(A, static_cast<float> (timerValue));
		}

		nextSequenceNum++;
		return true;
	} else {
		refuse_data(message.data);
		return false;
		
	}
}



// ***************************************************************************
// * Called from layer 3, when a packet arrives for layer 4 on side A
// ***************************************************************************
void rdt_rcvA(struct pkt packet) {
	if (!is_corrupt(packet)) {

		base = get_acknum(packet) + 1;

		if (base == nextSequenceNum) {
			simulation->stop_timer(A);
		} else {
			simulation->start_timer(A, EstimatedRTT);
		}
		
		int ackPacketIndex = packet.acknum % MAX_WINDOW_SIZE;
		float finalTime = simulation->getSimulatorClock();
		float start_time = packetStartTimes[ackPacketIndex];
		SampleRTT = finalTime - start_time;

		EstimatedRTT = (1 - alpha) * EstimatedRTT + alpha * SampleRTT;
	}
}

// ***************************************************************************
// * Called from layer 5, passed the data to be sent to other side
// ***************************************************************************
bool rdt_sendB(struct msg message) {
    INFO<< "RDT_SEND_B: Layer 4 on side B has received a message from the application that should be sent to side A: "
              << message << ENDL;
    
    bool accepted = false;
    
    return (accepted);
}


// ***************************************************************************
// // called from layer 3, when a packet arrives for layer 4 on side B 
// ***************************************************************************
void rdt_rcvB(struct pkt packet) {
	INFO << "INFO: RTD_RCV_B: Layer 4 on side B has received a packet from layer 3 sent over the network from side A:" << " (seq = " << packet.seqnum << ". ack = " << packet.acknum << ", chk =" << packet.checksum << ") " << packet.payload << ENDL;
	if (!is_corrupt(packet) && has_seqnum(packet, expectedSequenceNum)){
		struct msg message;
		extract(packet, message);
		simulation->deliver_data(B, message);
    
		struct pkt ackPacket = make_pkt(0, "ACK", packet.seqnum, 0);
		simulation->udt_send(B, ackPacket);

		expectedSequenceNum++;
    } else {
	    struct pkt ackPacket = make_pkt(0, "ACK", expectedSequenceNum - 1, 0);
	    simulation->udt_send(B, ackPacket);
    }

}


// ***************************************************************************
// * Called when A's timer goes off 
// ***************************************************************************
void A_timeout() {
    INFO << "A_TIMEOUT: Side A's timer has gone off." << ENDL;

    int current = base;

    while (current < nextSequenceNum) {
	    int index = current % MAX_WINDOW_SIZE;
	    if (sentPackets[index].seqnum != -1) {
		simulation->udt_send(A, sentPackets[index]);
		packetStartTimes[index] = simulation->getSimulatorClock();
	    }
	    current++;	    
    }
    if (base != nextSequenceNum) {
	    simulation->start_timer(A, EstimatedRTT);
    }
}

// ***************************************************************************
// * Called when B's timer goes off 
// ***************************************************************************
void B_timeout() {
    INFO << "B_TIMEOUT: Side B's timer has gone off." << ENDL;
}

//Note: I received assistance for the above functions from ChatGPT created by Open AI for code-related questions for this project.
//Reference: https://openai.com/chatgpt
