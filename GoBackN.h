
// ***********************************************************
// * Any additional include files should be added here.
// ***********************************************************

// ***********************************************************
// * Any functions you want to add should be included here.
// ***********************************************************

//extern simulator sim;

struct pkt make_pkt(int sequenceNumber, const char data[20] = "", int ackNumber = 0, int checksum = 0);
int computeChecksum(struct pkt packet);

//added functions
bool is_corrupt(struct pkt packet);
bool has_seqnum(struct pkt packet, int seqnum);
int get_acknum(struct pkt packet);
bool if_corrupt(struct pkt packet);
void extract(const struct pkt& packet, struct msg& message);
void refuse_data(const char data[20]);





