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

using namespace std;
// resources used:
// https://www.w3schools.com/c/c_strings.php - for string functions 
// https://www.w3schools.com/c/ref_stdio_sprintf.php 
// https://en.cppreference.com/w/cpp/language/reinterpret_cast 
/****************************************************************************
START OF MAIN PROCEDURE
This server creates a message queue and waits for a message requesting a token
The message received also has the name of the client mailbox.  The server uses
that name to reply.
*****************************************************************************/
struct send_to_server{
    char client_queue_name[64];
    float client_temp; 
    
};
int main (int argc, char** argv) // to include cmd line arguments 
{
    mqd_t qd_server, qd_client;   // queue descriptors
    send_to_server msg; 

    string str_client_queue_name = "/kellyclient-" + to_string(getpid ());
	strcpy(client_queue_name, str_client_queue_name.c_str());
    
    send_to_server.client_temp = atof(argv[1]); // turns string to float 
    
    // Build message queue attribute structure passed to the mq open
    struct mq_attr attr;
		attr.mq_flags = 0;
		attr.mq_maxmsg = MAX_MESSAGES;
		attr.mq_msgsize = sizeof(send_to_server);
		attr.mq_curmsgs = 0;

	//char in_buffer [MSG_BUFFER_SIZE];   // Build input buffer
	bool up = true; 

	// Create and open client message queue
    if ((qd_client = mq_open (msg.client_queue_name, O_CREAT | O_RDONLY, QUEUE_PERMISSIONS, &attr)) == -1) {
        cerr<<"Client: mq_open (client)";
        exit (1);
    }
	cout << "Client created queue: " << msg.client_queue_name << endl;

	// Open server message queue. The name is shared. It is opend write and read only
    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
         cerr<<"Client: mq_open (server)";
        exit (1);
    }
    
    while (up) {
        // sprintf(in_buffer, "%s %.2f", client_queue_name, client_temp); // stores my_temp into in_buffer
        // Send message to server
		//  Data sent is the client's message queue name
        if (mq_send (qd_server, reinterpret_cast<char*>(&msg), sizeof(msg), 0) == -1) {
             cerr<<"Client: Not able to send message to server";
            continue;
        }
        printf("client %d sent temp: %.2f\n", getpid(), msg.client_temp);
        
        // Receive response from server
        if (mq_receive (qd_client, reinterpret_cast<char*>(&msg), sizeof(msg), NULL) == -1) {
             cerr<<"Client: mq_receive";
            exit (1);
        }
        // check if client needs to quit 
        if(msg.client_temp == -1){
            cout << "client quitting./n";
            up = false;
            break;
        }
        // calculate new cent temp after server calculates new temp 
        //float central_temp = atof(in_buffer); 
        msg.client_temp = (msg.client_temp * 3 + 2 * msg.client_temp) / 5;
        // printf("client %d updated temperature: %.2f\n", getpid(), client_temp); removing getpid for testing
        printf("client updated temperature: %.2f\n", msg.client_temp); 
    }

    // close mq, unlink 
    mq_close(qd_client);
    mq_unlink(msg.client_queue_name);
    printf("Exiting.");
    exit (0);
}