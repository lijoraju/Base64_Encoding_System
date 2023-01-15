// Lijo Raju 224101033
// 14 Jan 2023
// Assignment 1 Base 64 Encoding Client.cpp

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

#define BUFFERSIZE 255

int main(int argc, char **argv)
{

    int sockfd, portno, n;
    struct sockaddr_in serverAddr;
    struct hostent *server;
    char buffer[256];

    if (argc < 3)
    {
        cout << "Required hostname and port no" << endl;
        exit(1);
    }

    portno = atoi(argv[2]); // getting port no from CLI argument

    // creating socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cout << "Error creating socket" << endl;
        exit(1);
    }

    cout << "Socket creation:SUCCESS" << endl;

    // getting server hostname
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        cout << "Invalid host" << endl;
        exit(1);
    }

    // intializing serverAddr to NULL
    bzero((char *)&serverAddr, sizeof(serverAddr));

    // setting serverAddr parameters
    serverAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
    serverAddr.sin_port = htons(portno);

    // connecting to server
    cout << "Connecting to server IP:" << server->h_name << " Port:" << portno << endl;
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        cout << "Error connecting to server" << endl;
        exit(1);
    }

    cout << "Connection successful" << endl;

    // sending msg to server
    cout << "Type your Message and press enter to send: ";
    bzero(buffer, 256);
    fgets(buffer, BUFFERSIZE, stdin);
    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0)
    {
        cout << "Msg sending failed" << endl;
        exit(1);
    }

    // receiving msg from server
    bzero(buffer, 256);
    n = read(sockfd, buffer, BUFFERSIZE);
    if (n < 0)
    {
        cout << "Error reading msg from server" << endl;
        exit(1);
    }
    cout << "New message received: " << buffer << endl;
    close(sockfd);

    return 0;
}