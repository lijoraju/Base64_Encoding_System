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

    // accepting incoming connection
    clientlen = sizeof(clientAddr);
    newsockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientlen);
    if (newsockfd < 0)
    {
        cout << "Error accepting incoming connection" << endl;
        exit(1);
    }

    cout << "Accepted incoming connection from IP:" << inet_ntoa(clientAddr.sin_addr) << " Port:" << ntohs(clientAddr.sin_port) << endl;

    // sending msg from server to client
    send(newsockfd, "Hi I'm Server", 13, 0);

    // receiving msg from client
    bzero(buffer, sizeof(buffer));
    n = read(newsockfd, buffer, BUFFERSIZE);
    if (n < 0)
    {
        cout << "Error reading msg from client" << endl;
        exit(1);
    }

    cout << "Message Received: " << buffer << endl;
    doBase64Decoding(buffer);

    close(newsockfd);
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

    for (int i = 0; i < n; i += 4)
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

    cout << "Decoded Message: " << decodedMsg << endl;
}