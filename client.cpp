/*
 * Adapted from  Softpryaog 
  *    https://www.softprayog.in/programming/interprocess-communication-using-posix-message-queues-in-linux
  * by MA Doman 2018
 * client.c: Client program
 *           to demonstrate interprocess communication
 *           with POSIX message queues
 */

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <cstring>
#include <iostream>

// DEFINE THE SERVER NAME AN DEFAULT VALUES FOR THE MESSAGE QUEUE
#define SERVER_QUEUE_NAME   "/kellyserver"
#define QUEUE_PERMISSIONS 0660  // like chmod values, user and owner can read and write to queue
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10   // leave some extra space after message
#define CLIENT_QUEUE_NAME "/kellyclient"
using namespace std;
// resources used:
// https://www.w3schools.com/c/c_strings.php - for string functions 
// https://www.w3schools.com/c/ref_stdio_sprintf.php 
/****************************************************************************
START OF MAIN PROCEDURE
This server creates a message queue and waits for a message requesting a token
The message received also has the name of the client mailbox.  The server uses
that name to reply.
*****************************************************************************/
int main (int argc, char** argv) // to include cmd line arguments 
{
    mqd_t qd_server, qd_client;   // queue descriptors

    // create the client queue for receiving messages from server
	// use the client PID to help differentiate it from other queues with similar names
	// the queue name must be a null-terminated c-string.
	// strcpy makes that happen
    char client_queue_name [64];
    string str_client_queue_name = "/kellyclient-" + to_string(getpid ());
	strcpy(client_queue_name, str_client_queue_name.c_str());
    float client_temp = atof(argv[1]); // turns string to float 
    	// Build message queue attribute structure passed to the mq open
    struct mq_attr attr;
		attr.mq_flags = 0;
		attr.mq_maxmsg = MAX_MESSAGES;
		attr.mq_msgsize = MAX_MSG_SIZE;
		attr.mq_curmsgs = 0;

	char in_buffer [MSG_BUFFER_SIZE];   // Build input buffer
	bool up = true; 


	// Create and open client message queue
    if ((qd_client = mq_open (CLIENT_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        cerr<<"Client: mq_open (client)";
        exit (1);
    }
	
	// Open server message queue. The name is shared. It is opend write and read only
    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
         cerr<<"Client: mq_open (server)";
        exit (1);
    }
    
    while (up) {
        sprintf(in_buffer, "%s %.2f", client_queue_name, client_temp); // stores my_temp into in_buffer
        // Send message to server
		//  Data sent is the client's message queue name
        if (mq_send (qd_server, in_buffer , strlen(in_buffer) + 1, 0) == -1) {
             cerr<<"Client: Not able to send message to server";
            continue;
        }
        printf("client temperature sent: %.2f\n", client_temp);
        // Receive response from server
        if (mq_receive (qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
             cerr<<"Client: mq_receive";
            exit (1);
        }
        
        if (strcmp(in_buffer, "quit") == 0){ // clients will stop when the temp stabilizes then exits loop. 
            up = false; 
            break;
        }
        // calculate new cent temp after server calculates new temp 
        float central_temp = atof(in_buffer); 
        client_temp = (client_temp * 3 + 3 * central_temp) / 5;
        // printf("client %d updated temperature: %.2f\n", getpid(), client_temp); removing getpid for testing
        printf("client updated temperature: %.2f\n", client_temp); 
    }

    // close mq, unlink 
    mq_close(qd_client);
    mq_unlink(client_queue_name);
    printf("Exiting.");
    exit (0);
}