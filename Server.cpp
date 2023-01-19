// Lijo Raju 224101033
// 14 Jan 2023
// Assignment 1 Base 64 Encoding Server.cpp

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

using namespace std;

#define BUFFERSIZE 255

void doBase64Decoding(char *);

int main(int argc, char **argv)
{
    int sockfd, newsockfd, portno;
    socklen_t clientlen;
    char buffer[256];
    struct sockaddr_in serverAddr, clientAddr;
    int n;
    pid_t pid;
    string clientIP;

    if (argc < 2)
    {
        cout << "Port No Invalid" << endl;
        exit(1);
    }

    // creating socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cout << "Error Creating Socket" << endl;
        exit(1);
    }

    cout << "Server socket creation:SUCCESS" << endl;

    // initializing serverAddr to null value
    bzero((char *)&serverAddr, sizeof(serverAddr));
    portno = atoi(argv[1]); // getting port no as CLI argument

    // setting serverAddr parameters
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(portno);

    // binding the socket to portno
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        cout << "Error on binding" << endl;
        exit(1);
    }

    cout << "Server socket binding:SUCCESS" << endl;

    // listening incoming connections
    listen(sockfd, 10);

    cout << "Server is up\nListening for incoming connections on portno " << portno << endl;

    do
    {
        // accepting new incoming connection
        clientlen = sizeof(clientAddr);
        newsockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientlen);
        if (newsockfd < 0)
        {
            cout << "Error accepting incoming connection" << endl;
            exit(1);
        }
        clientIP = inet_ntoa(clientAddr.sin_addr);
        clientIP = clientIP + '/';
        clientIP = clientIP + to_string(ntohs(clientAddr.sin_port));
        cout << "Accepted new incoming connection from IP/PortNo:" << clientIP << endl;

        // creating child process to handle the connected client
        if ((pid = fork()) == 0)
        {
            close(sockfd); // child server process closing its listening socket

            do
            {
                // receiving msg from client
                bzero(buffer, sizeof(buffer));
                n = read(newsockfd, buffer, BUFFERSIZE);
                if (n < 0)
                {
                    cout << "Error reading msg from client" << endl;
                    exit(1);
                }

                if (buffer[0] == '3')
                {
                    cout << "Client " << clientIP << " requested to close connection." << endl;
                    break;
                }

                cout << "New message from client " << clientIP << ": " << buffer << endl;
                doBase64Decoding(buffer);

                // sending ACK to client on recieving Type 1 Msg
                if (buffer[0] == '1')
                    send(newsockfd, "Message recieved", 20, 0);
            } while (true);

            close(newsockfd);
            cout << "Connection closed." << endl;
            exit(0);
        }
        close(newsockfd); // parent server process closes connected client socket
    } while (true);

    close(sockfd);

    return 0;
}

// base 64 decoding
void doBase64Decoding(char *encodedMsg)
{
    int n = strlen(encodedMsg);
    char *decodedMsg = (char *)malloc(BUFFERSIZE * sizeof(char));
    int asciiValue, numberOfBits;
    int j, p, k = 0;

    for (int i = 1; i < n; i += 4)
    {
        j = i;
        asciiValue = 0;
        numberOfBits = 0;

        while (j < (i + 4) && j < n)
        {
            if (encodedMsg[j] != '=')
            {
                asciiValue = asciiValue << 6;
                numberOfBits += 6;

                if (encodedMsg[j] >= 'A' && encodedMsg[j] <= 'Z')
                {
                    asciiValue = asciiValue | (encodedMsg[j] - 'A');
                }
                else if (encodedMsg[j] >= 'a' && encodedMsg[j] <= 'z')
                {
                    asciiValue = asciiValue | (encodedMsg[j] - 'a' + 26);
                }
                else if (encodedMsg[j] >= '0' && encodedMsg[j] <= '9')
                {
                    asciiValue = asciiValue | (encodedMsg[j] - '0' + 52);
                }
                else if (encodedMsg[j] == '+')
                {
                    asciiValue = asciiValue | 62;
                }
                else if (encodedMsg[j] == '/')
                {
                    asciiValue = asciiValue | 63;
                }
                else
                {
                    cout << "Unknown character encountered:" << encodedMsg[j] << endl;
                }
            }
            else
            {
                asciiValue = asciiValue >> 2;
                numberOfBits -= 2;
            }
            j++;
        }

        while (numberOfBits > 0)
        {
            p = numberOfBits - 8;
            decodedMsg[k++] = (asciiValue >> p) & 255;
            numberOfBits -= 8;
        }
    }
    decodedMsg[k] = '\0';

    cout << "Decoded message: " << decodedMsg << endl;
}