# Go-Back-N Project
# Simple Reliable Data Transfer Protocol Implementation

## Project Description
This project is an implementation of the Go-Back-N protocol, a data link layer protocol used for reliable data transfer resembling real-world network connection systems. It is built around a network simulator that includes packet loss and corruption. This is done in C Programming Language.

The implementation is done in two ways:
1. Establish a basic communcation without any loss or corruption.
2. Use the Go-Back-N Protocol for handling loss and corruption effectively.

Go-Back-N Protocol
This provides a way for the sender to transmit multiple frames before needing an acknowledgment for the first frame. It has a window size N, which is the maximum number of outstanding frames that the protocol allows. 

## Running the Project
The following command with the respectie arguments can be used:

''' ./GoBackN -n <number_of_messages> -l <loss_probability> -c <corruption_probability> -t <message_delay> -d <debug_level> '''

For example, to simulate 10,000 messages with 1% probability of loss, a 1% probability of corruption, an average delay of 1 second between messages, and the maximum debugging level, we can use:

''' ./GoBackN -n 10000 -l 0.01 -c 0.01 -t 100 -d 5 '''


## Project Requirement
1. Must be able to handle any combination of input values.
2. The sender is limited to caching 10 messages at any given time.
