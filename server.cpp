/*
 * server.cpp: Server program
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
#define CLIENT_COUNT 4 // update to 4 clients 

using namespace std;
// https://www.w3schools.com/c/c_strings.php - for string functions 
// https://www.w3schools.com/c/ref_stdio_sprintf.php 
// https://en.cppreference.com/w/cpp/language/reinterpret_cast 
// https://www.softprayog.in/programming/interprocess-communication-using-posix-message-queues-in-linux
// https://man7.org/linux/man-pages/man7/mq_overview.7.html  
// https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/MQueues.html ** helped understandning with struct passing!
struct send_to_server{
    char client_queue_name[64];
    float client_temp; 
};
int main ()
{
    mqd_t qd_server, qd_client[CLIENT_COUNT];   // queue descriptors
    send_to_server msg; // stores messages between server and client 

    float client_temps[CLIENT_COUNT] = {0}; // array that will store recieved client temps 
    float central_temp = 0.0;
    float prev_temp[CLIENT_COUNT]={0};

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
        float sum_temps = 0.0;
         // get temps from all clients client 
         for(int i = 0; i < CLIENT_COUNT; i++){
            if (mq_receive (qd_server, reinterpret_cast<char*>(&msg), sizeof(msg), NULL) == -1) {
                cerr << "Server: mq_receive";
                exit (1);
            }

            // store the client temps 
            client_temps[i] = msg.client_temp;
            sum_temps += msg.client_temp;

            printf("temperature recieved from client %s: %.2f\n", msg.client_queue_name, msg.client_temp);
         }

            // calculates the new central temperature then prints. 
            float new_cen_temp = (2 * central_temp + sum_temps) / 6.0; // changed back to 6 for original formula
            printf("new central temperature: %.2f\n", new_cen_temp); 

            // stabalization check
            stabilize = true; 
            for(int i = 0; i < CLIENT_COUNT; i++ ){
                if (fabs(prev_temp - new_cen_temp) < 0.01){
                    stabilize = true;
                    break;
                }
            }
           
            // send the new central temp back to clients
            for (int i = 0; i < CLIENT_COUNT; i++) {
                string client_queue_name = "/kellyclient-" + to_string(i + 1);
                strcpy(msg.client_queue_name, client_queue_name.c_str());
             // format new_central_temp to string, then stores it in inbuffer, then send it to client mq  
                if(stabilize){
                    msg.client_temp = -1;
                } else{
                    msg.client_temp = new_cen_temp;
                }
                // open mq then checks if mq_open was successful
                if((qd_client[i] = mq_open(client_queue_name.c_str(), O_WRONLY))== -1){
                    cerr << "Server: mq_open" << client_queue_name << "failed \n";
                    continue; 
                }
                cout << "Server: Client MQ opened!" << endl;
                mq_send(qd_client[i], reinterpret_cast<char*>(&msg), sizeof(msg), 0);
                mq_close(qd_client[i]); //close mq
            }

        
            central_temp = new_cen_temp;   // updates central temperature with the new one
            for (int i = 0; i < CLIENT_COUNT; i++){
                prev_temp[i] = client_temps[i]; // prev temp is updarted with new one in array
        } 
            }
            
	
    printf("system stablized");

    // close server, unlink 
    mq_close(qd_server);
    mq_unlink(SERVER_QUEUE_NAME);
    printf("exiting.\n");
    exit (0);
}
