/*
 * server.c: Server program
 *           to demonstrate interprocess commnuication
 *           with POSIX message queues
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

// DEFINE THE SERVER NAME AN DEFAULT VALUES FOR THE MESSAGE QUEUE
#define SERVER_QUEUE_NAME   "/kellyserver"
#define QUEUE_PERMISSIONS 0660  // like chmod values, user and owner can read and write to queue
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10   // leave some extra space after message
#define CLIENT_COUNT 1 
#define CLIENT_QUEUE_NAMES "/kelly_client"
using namespace std;
/****************************************************************************
START OF MAIN PROCEDURE
This server creates a message queue and waits for a message requesting a token
The message received also has the name of the client mailbox.  The server uses
that name to reply.
*****************************************************************************/
// resources used:
// https://www.w3schools.com/c/c_strings.php - for string functions 
// https://www.w3schools.com/c/ref_stdio_sprintf.php 
int main ()
{
    mqd_t qd_server, qd_client;   // queue descriptors
    
	// Build message queue attribute structure passed to the mq open
    struct mq_attr attr;

		attr.mq_flags = 0;
		attr.mq_maxmsg = MAX_MESSAGES;
		attr.mq_msgsize = MAX_MSG_SIZE;
		attr.mq_curmsgs = 0;
	
    
        // Open and create the server message queue
	if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Server: mq_open (server)");
        exit (1);
    }

    cout << "mq opened" << endl; // testing to see if it opens. 
    
	// Declare (create) the buffers to hold message received and sent
    char in_buffer [MSG_BUFFER_SIZE];
    // char out_buffer [MSG_BUFFER_SIZE];
	float client_temps[CLIENT_COUNT];  // array that holds the client temps (currently a single client)
    float total_client_temps = 0.0;  // initalized to 0
    float central_temp = 0.0;  // initalized to 0 
    bool stabilize = false; // bool for evaluating if the server is stabalized 
    char client_queue_name[64];
	// Initialize the token to be given to client
	// int token_number = 1; 
	

    while(!stabilize){
         // get temps from single client 
            if (mq_receive (qd_server, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
                cerr << "Server: mq_receive";
                exit (1);
            }
            // inputs the temp recieved from client into the buffer and stored in client_temps[0]; then prints out the temp from client
            client_temps[0] = atof(in_buffer);
            total_client_temps += client_temps[0];
            printf("Temperature recieved from client: %.2f\n", client_temps[0]);
         }
            // calculates the new central temperature then prints. 
            float new_cen_temp = (2 * central_temp + total_client_temps) / 3.0;
            printf("New Central Temperature: %.2f\n", new_cen_temp); 

            // stabalization check
            stabilize = (client_temps[0] == new_cen_temp); 

            // send the new central temp back to client 
            sprintf(client_queue_name, "%s0", CLIENT_QUEUE_NAMES);
            // open mq then checks if mq_open was successful 
            qd_client = mq_open(client_queue_name, O_WRONLY);
            if (qd_client == -1) {
                cerr << "Server: mq_open (client queue)";
                exit(1);

            // format new_central_temp to string, then stores it in inbuffer, then send it to client mq
            sprintf(in_buffer, "%.2f", new_cen_temp);
            mq_send(qd_client, in_buffer, strlen(in_buffer) + 1, 0);
            mq_close(qd_client); //close mq
    
            central_temp = new_cen_temp;   // updates central temperature with the new one
    } 
	
    printf("The System is now stablizied. Quitting.");

    // client terminates
    sprintf(client_queue_name, "%s0", CLIENT_QUEUE_NAMES);
    qd_client = mq_open(client_queue_name, O_WRONLY);
    // open mq, if fails quit!
    if (qd_client != -1) {
        mq_send(qd_client, "quit", 10, 0);
        mq_close(qd_client);
    }

    // close server, unlink 
    mq_close(qd_server);
    mq_unlink(SERVER_QUEUE_NAME);
    printf("Exiting.");
    exit (0);
}
