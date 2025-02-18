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
#include <iostream>
#include <unistd.h>
#include <cmath>

// DEFINE THE SERVER NAME AN DEFAULT VALUES FOR THE MESSAGE QUEUE
#define SERVER_QUEUE_NAME   "/kellyserver"
#define QUEUE_PERMISSIONS 0660  // like chmod values, user and owner can read and write to queue
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define CLIENT_COUNT 1 

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
// https://www.w3schools.com/c/ref_stdio_sscanf.php 

struct send_to_server{
    char client_queue_name[64];
    float client_temp; 
};
int main ()
{
    mqd_t qd_server, qd_client;   // queue descriptors
    
    send_to_server msg; 
    float prev_temp = 0.0;
    float central_temp = 0.0;


	// Build message queue attribute structure passed to the mq open
    struct mq_attr attr;

		attr.mq_flags = 0;
		attr.mq_maxmsg = MAX_MESSAGES;
		attr.mq_msgsize = sizeof(send_to_server);
		attr.mq_curmsgs = 0;
	
    
        // Open and create the server message queue
	if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Server: mq_open (server)");
        exit (1);
    }
    cout << "Server: message queue opened!" << endl; 

    bool stabilize = false; // bool for evaluating if the server is stabalized 

    while(!stabilize){
         // get temps from single client 
            if (mq_receive (qd_server, reinterpret_cast<char*>(&msg), sizeof(msg), NULL) == -1) {
                cerr << "Server: mq_receive";
                exit (1);
            }
            // inputs the temp recieved from client into the buffer and stored in client_temps[0]; then prints out the temp from client
            printf("Temperature recieved from client %s: %.2f\n", msg.client_queue_name, msg.client_temp); // taking out current array form to test single value 

            // calculates the new central temperature then prints. 
            float new_cen_temp = (2 * central_temp + msg.client_temp) / 3.0; // changed from total to client temp
            printf("New Central Temperature: %.2f\n", new_cen_temp); 

            // stabalization check
            cout << "attempting to stabilize\n";
            if (fabs(prev_temp - new_cen_temp) < 0.01){
            stabilize = true;
            }
            cout << "successful\n";
            // send the new central temp back to client 
            
            // open mq then checks if mq_open was successful 
            cout << "attempting to open mq\n";
            if((qd_client = mq_open(msg.client_queue_name, O_WRONLY))== -1){
                cerr << "Server: mq_open" << msg.client_queue_name << "failed \n";
                continue; // might have to fix to continue
            }
            cout << "Server: Client MQ opened!" << endl;
            // format new_central_temp to string, then stores it in inbuffer, then send it to client mq
            if(stabilize){
                msg.client_temp = -1;
            } else{
                msg.client_temp = new_cen_temp;
            }
            mq_send(qd_client, reinterpret_cast<char*>(&msg), sizeof(msg), 0);
            mq_close(qd_client); //close mq
    
            central_temp = new_cen_temp;   // updates central temperature with the new one
            prev_temp = new_cen_temp; // prev temp is updarted with new one
    } 
	
    printf("The System is now stablizied. Quitting.\n");

    // close server, unlink 
    mq_close(qd_server);
    mq_unlink(SERVER_QUEUE_NAME);
    printf("Exiting.\n");
    exit (0);
}
