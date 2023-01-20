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

#define BUFFERSIZE 1024
#define ACK "Message Received"

enum ErrorCode
{
    HOST_NAME_AND_PORT_NO_INVALID,
    SOCKET_CREATION_FAILED,
    CONNECTION_FAILED,
    HOST_NOT_VALID,
    ERROR_SENDING_MSG,
    ERROR_READING_MSG,
    PORT_NO_INVALID
};

enum ClosingState
{
    FIN_WAIT_1,
    FIN_WAIT_2
};

void clientProcess(int, struct hostent *);
void doClientServerCommunications(int);
void closeConnectionToServer(int);
void doBase64Encoding(char *, char *);
void error(ErrorCode);
void printToConsole(string);

int main(int argc, char **argv)
{
    int portno;
    struct hostent *server;

    if (argc < 3)
    {
        error(HOST_NAME_AND_PORT_NO_INVALID);
    }

    portno = atoi(argv[2]);          // getting port no from CLI argument
    server = gethostbyname(argv[1]); // getting server hostname

    if (portno < 0 || portno > 65534)
    {
        error(PORT_NO_INVALID);
    }

    if (server == NULL)
    {
        error(HOST_NOT_VALID);
    }

    clientProcess(portno, server);

    return 0;
}

/**
 * @brief Starting client process
 * @param portno the server port no
 * @param server the server ip
 */
void clientProcess(int portno, struct hostent *server)
{
    int sockfd;
    struct sockaddr_in serverAddr;

    // creating socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error(SOCKET_CREATION_FAILED);
    }

    printToConsole("COMPLETED: Server Socket Creation.");

    // intializing serverAddr to NULL
    bzero((char *)&serverAddr, sizeof(serverAddr));

    // setting serverAddr parameters
    serverAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
    serverAddr.sin_port = htons(portno);

    // connecting to server
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        error(CONNECTION_FAILED);
    }

    printToConsole("COMPLETED: Connecting To Server.");

    doClientServerCommunications(sockfd);
}

/**
 * @brief Client-Server communication
 * @param sockfd the client socket created for communicating with server
 */
void doClientServerCommunications(int sockfd)
{
    char *buffer = (char *)malloc(BUFFERSIZE * sizeof(char));
    char *msg = (char *)malloc(2 * BUFFERSIZE * sizeof(char));
    char option;
    char type1 = '1', type3 = '3';

    msg[0] = type1;

    do
    {
        memset(buffer, 0, BUFFERSIZE * sizeof(char)); // clearing buffer

        printToConsole("\nType your Message and press enter to send: ");

        fgets(buffer, BUFFERSIZE, stdin); // getting msg from user

        buffer[strcspn(buffer, "\n")] = 0; // removing newline character from input

        doBase64Encoding(buffer, msg); // base 64 encoding

        // sending encoded msg(Type 1) to server
        if (write(sockfd, msg, strlen(msg)) <= 0)
        {
            error(ERROR_SENDING_MSG);
        }

        memset(buffer, 0, BUFFERSIZE * sizeof(char)); // clearing buffer before reading msg

        // receiving Ack(Type 2) from server
        if (read(sockfd, buffer, BUFFERSIZE) <= 0)
        {
            error(ERROR_READING_MSG);
        }

        printToConsole("\nNew message from server:");

        printToConsole(buffer);

        printToConsole("\nDo you want to close the connection now?.(Y/N)");

        cin >> option;
        cin.clear();
        cin.ignore();

        memset(msg, 0, 2 * BUFFERSIZE * sizeof(char)); // clearing msg

        if (toupper(option) == 'Y')
        {
            msg[0] = type3;
            break;
        }

        msg[0] = type1;
    } while (true);

    closeConnectionToServer(sockfd);
}

/**
 * @brief Closing connection gracefully
 * @param sockfd the client socket created for communicating with server
 */
void closeConnectionToServer(int sockfd)
{
    char type3 = '3';
    char *buffer = (char *)malloc(BUFFERSIZE * sizeof(char));
    char *msg = (char *)malloc(2 * BUFFERSIZE * sizeof(char));
    clock_t time;
    ClosingState state;

    memset(msg, 0, 2 * BUFFERSIZE * sizeof(char)); // clearing msg
    msg[0] = type3;

    // sending Type 3 msg to server
    strcat(msg, "Finished");
    if (write(sockfd, msg, strlen(msg)) < 0)
    {
        error(ERROR_SENDING_MSG);
    }

    printToConsole("\nType 3 Msg Send To Server");

    state = FIN_WAIT_1;
    memset(buffer, 0, BUFFERSIZE * sizeof(char)); // clearing buffer before reading msg

    time = clock();
    while (state != FIN_WAIT_2)
    {
        if (read(sockfd, buffer, BUFFERSIZE) > 0)
        {
            // receiving Ack for Type 3 msg from server
            state = FIN_WAIT_2;
            printToConsole("CONNECTION CLOSING: ACK Received From Server.");
        }
        else if ((clock() - time) > 3000)
        {
            // timeout not received ACK from server
            state = FIN_WAIT_2;
            printToConsole("ACK TIMEOUT: Not Received ACK From Server. Entered FIN_WAIT_2 State.");
        }
    }

    memset(buffer, 0, BUFFERSIZE * sizeof(char)); // clearing buffer before reading next msg
    time = clock();

    while ((clock() - time) < 3000)
    {
        if (read(sockfd, buffer, BUFFERSIZE) > 0)
        {
            printToConsole("CONNECTION CLOSING: FIN Signal Received From Server.");
            printToConsole("CONNECTION CLOSING: Last ACK Send To Server.");
            send(sockfd, ACK, strlen(ACK), 0); // send ACK to server
            break;
        }
    }

    sleep(5);
    close(sockfd);

    printToConsole("Connection Closed.");
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
    case HOST_NAME_AND_PORT_NO_INVALID:
        errorMsg = "ERROR: Invalid Host Name and Port No.";
        break;
    case SOCKET_CREATION_FAILED:
        errorMsg = "ERROR: Socket Creation Failed.";
        break;
    case CONNECTION_FAILED:
        errorMsg = "ERROR: Server Connection Failed.";
        break;
    case ERROR_SENDING_MSG:
        errorMsg = "ERROR: Sending Message To Server.";
        break;
    case ERROR_READING_MSG:
        errorMsg = "ERROR: Reading Message From Server.";
        break;
    case PORT_NO_INVALID:
        errorMsg = "ERROR: Invalid Port No.";
        break;
    case HOST_NOT_VALID:
        errorMsg = "ERROR: Host Not Valid.";
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
 * @brief Base64 encoding
 * @param msg original message
 * @param encodedMsg encoded msg to send
 */
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