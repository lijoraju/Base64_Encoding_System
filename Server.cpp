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

#define BUFFERSIZE 1024
#define ACK "Message Received"
#define FIN "Finished"

enum ErrorCode
{
    PORT_NO_INVALID,
    SOCKET_CREATION_FAILED,
    SOCKET_BINDING_FAILED,
    CONNECTION_ACCEPTING_FAILED,
    ERROR_READING_MSG,
};

void serverProcess(int);
void listenClientProcesses(int);
void closeConnectionToClient(int, string);
void doClientServerCommunications(int, string);
void doBase64Decoding(char *);
void error(ErrorCode);
void printToConsole(string);

int main(int argc, char **argv)
{
    int portno;
    if (argc < 2)
    {
        error(PORT_NO_INVALID);
    }

    portno = atoi(argv[1]); // getting port no as CLI argument

    if (portno < 0 || portno > 65534)
    {
        error(PORT_NO_INVALID);
    }

    serverProcess(portno);

    return 0;
}

/**
 * @brief Error handling utility fuction
 * @param errCode the error code for which exception need to be handled
 */
void error(ErrorCode errCode)
{
    string errorMsg;

    switch (errCode)
    {
    case PORT_NO_INVALID:
        errorMsg = "ERROR: Invalid Port No.";
        break;
    case SOCKET_CREATION_FAILED:
        errorMsg = "ERROR: Socket Creation Failed.";
        break;
    case SOCKET_BINDING_FAILED:
        errorMsg = "ERROR: Socket Binding Failed.";
        break;
    case CONNECTION_ACCEPTING_FAILED:
        errorMsg = "ERROR: Acccepting Conncetion Failed.";
        break;
    case ERROR_READING_MSG:
        errorMsg = "ERROR: Reading Message From Client.";
        break;

    default:
        break;
    }

    printToConsole(errorMsg);

    exit(1);
}

/**
 * @brief Printing console output
 * @param msg the output to print
 */
void printToConsole(string msg)
{
    cout << msg << endl;
}

/**
 * @brief Starting server process
 * @param portno the port no to which server socket to bind
 */
void serverProcess(int portno)
{
    int sockfd;
    struct sockaddr_in serverAddr;

    // creating the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error(SOCKET_CREATION_FAILED);
    }

    printToConsole("COMPLETED: Server Socket Creation.");

    // initializing serverAddr to null value
    bzero((char *)&serverAddr, sizeof(serverAddr));

    // setting serverAddr parameters
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(portno);

    // binding the socket
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        error(SOCKET_BINDING_FAILED);
    }

    printToConsole("COMPLETED: Server Socket Binding.");

    listen(sockfd, 5); // No of concurrent connections is restricted to 10

    printToConsole(("\nServer Is Up And Running.\nListening Incoming Connections On Port No " + to_string(portno)));

    listenClientProcesses(sockfd);
}

/**
 * @brief Listening incoming connections
 * @param sockfd the server socket which is listening the client connections
 */
void listenClientProcesses(int sockfd)
{
    int newsockfd;
    socklen_t clientlen;
    struct sockaddr_in clientAddr;
    string clientIP;
    pid_t pid;

    do
    {
        // accepting new incoming connection
        clientlen = sizeof(clientAddr);
        newsockfd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientlen);
        if (newsockfd < 0)
        {
            error(CONNECTION_ACCEPTING_FAILED);
        }
        clientIP = inet_ntoa(clientAddr.sin_addr);
        clientIP = clientIP + '/';
        clientIP = clientIP + to_string(ntohs(clientAddr.sin_port));

        printToConsole(("\nAccepted New Incoming Connection From Client IP/PortNo: " + clientIP));

        // creating child process for handling client communications
        if ((pid = fork()) == 0)
        {
            // child process
            close(sockfd); // closing its listening socket
            doClientServerCommunications(newsockfd, clientIP);
            exit(0);
        }
        close(newsockfd); // parent process closes its connected client socket

    } while (true);
}

/**
 * @brief Client-Server communications
 * @param newsockfd the new socket created for client communications
 * @param clientIP  the client IP/PortNo
 */
void doClientServerCommunications(int newsockfd, string clientIP)
{
    char buffer[BUFFERSIZE];

    do
    {
        bzero(buffer, sizeof(buffer)); // clearing buffer before reading msg

        if (read(newsockfd, buffer, BUFFERSIZE) <= 0)
        {
            error(ERROR_READING_MSG);
        }

        if (buffer[0] == '3')
        {
            // client has send Type 3 msg
            printToConsole(("\nClient " + clientIP + " Requested To Close Connection."));
            break;
        }

        printToConsole(("\nNew Message Received From Client " + clientIP + "\n\nReceived Message:\n"));

        printToConsole(buffer); // received msg from client

        doBase64Decoding(buffer);

        if (buffer[0] == '1')
        {
            // sending ACK to client on recieving Type 1 Msg
            send(newsockfd, ACK, strlen(ACK), 0);
        }

    } while (true);

    closeConnectionToClient(newsockfd, clientIP);
}

/**
 * @brief Closing conncetion gracefully
 * @param newsockfd the new socket created for client communications
 */
void closeConnectionToClient(int newsockfd, string clientIP)
{
    clock_t time;
    char buffer[BUFFERSIZE];
    bool receivedAck = false;

    bzero(buffer, sizeof(buffer));        // clearing buffer before reading msg
    send(newsockfd, ACK, strlen(ACK), 0); // send ACK to client
    printToConsole("\nCONNECTION CLOSING: ACK Signal Send To Client " + clientIP);
    sleep(5);
    send(newsockfd, FIN, strlen(FIN), 0); // send FIN to client
    printToConsole("\nCONNECTION CLOSING: FIN Signal Send To Client " + clientIP);
    time = clock();

    while ((clock() - time) < 3000)
    {
        if (read(newsockfd, buffer, BUFFERSIZE) != -1)
        {
            printToConsole("\nCONNECTION CLOSING: Last ACK Received From Client " + clientIP);
            receivedAck = true;
            break;
        }
    }

    if (!receivedAck)
        printToConsole("\nACK TIMEOUT: Last ACK From Client " + clientIP + " Not Received.");

    close(newsockfd);

    printToConsole("\nConnection To Client " + clientIP + " Closed.");
}

/**
 * @brief Base64 decoding
 * @param encodedMsg the received msg from client
 */
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
                    printToConsole(("ERROR: Unable To Decode " + encodedMsg[j]));
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

    printToConsole("\nOriginal Message:\n");

    printToConsole(decodedMsg); // original msg send by client
}