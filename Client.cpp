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

void doBase64Encoding(char *, char *);

int main(int argc, char **argv)
{

    int sockfd, portno, n;
    struct sockaddr_in serverAddr;
    struct hostent *server;
    char *buffer = (char *)malloc(BUFFERSIZE * sizeof(char));
    char *msg = (char *)malloc(2 * BUFFERSIZE * sizeof(char));
    bool isConnectionClosing = true;
    char option;

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

    isConnectionClosing = false;
    cout << "Connection successful" << endl;
    msg[0] = '1';

    do
    {
        cout << "Type your Message and press enter to send: ";
        memset(buffer, 0, BUFFERSIZE * sizeof(char)); // clearing buffer
        fgets(buffer, BUFFERSIZE, stdin);             // getting msg from user
        buffer[strcspn(buffer, "\n")] = 0;            // removing newline character from input
        doBase64Encoding(buffer, msg);                // base 64 encoding

        // sending encoded msg(Type 1) to server
        n = write(sockfd, msg, strlen(msg));
        if (n < 0)
        {
            cout << "Msg sending failed" << endl;
            exit(1);
        }

        // receiving Ack(Type 2) from server
        memset(buffer, 0, BUFFERSIZE * sizeof(char)); // clearing buffer
        n = read(sockfd, buffer, BUFFERSIZE);
        if (n < 0)
        {
            cout << "Error reading msg from server" << endl;
            exit(1);
        }

        cout << "New message from server: " << buffer << endl;

        cout << "Do you want to close the connection now(Y/N)?: ";
        cin >> option;
        cin.clear();
        cin.ignore();
        memset(msg, 0, 2 * BUFFERSIZE * sizeof(char)); // clearing msg
        if (toupper(option) == 'Y')
        {
            msg[0] = '3';
            isConnectionClosing = true;
        }
        else
        {
            msg[0] = '1';
        }
    } while (!isConnectionClosing);

    // sending Type 3 msg to server
    strcat(msg, "Closing connection");
    n = write(sockfd, msg, strlen(msg));
    if (n < 0)
    {
        cout << "Type 3 Msg sending failed" << endl;
        exit(1);
    }

    close(sockfd);
    cout << "Connection closed." << endl;

    return 0;
}

// base64 encoding
void doBase64Encoding(char *msg, char *encodedMsg)
{
    char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int n = strlen(msg);
    int index, characterCount, numberOfBits, asciiValue, padding;
    int j, p, k = 1;

    for (int i = 0; i < n; i += 3)
    {
        j = i;
        characterCount = 0;
        asciiValue = 0;

        while (j < (i + 3) && j < n)
        {
            asciiValue = asciiValue << 8;
            asciiValue = asciiValue | msg[j];
            characterCount++;
            j++;
        }

        numberOfBits = characterCount * 8;
        padding = numberOfBits % 3;

        while (numberOfBits > 0)
        {
            if (numberOfBits > 6)
            {
                p = numberOfBits - 6;
                index = (asciiValue >> p) & 63;
                numberOfBits -= 6;
            }
            else
            {
                p = 6 - numberOfBits;
                index = (asciiValue << p) & 63;
                numberOfBits = 0;
            }
            encodedMsg[k++] = base64Chars[index];
        }
    }

    for (j = 0; j < padding; j++)
    {
        encodedMsg[k++] = '=';
    }

    encodedMsg[k] = '\0';
}