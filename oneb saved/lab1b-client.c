#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 


int sockfd, portno, n, logfd;


struct option getOptStruct[] =
{
    {"port", 1, 0, 1},
    {"log", 1, 0, 2},
    {0, 0, 0, 0}
};

//taken from TA slides
struct termios saved_attributes;

void reset_input_mode (void) //saves terminal settings here
{
    tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
    close(sockfd);
    close(logfd);
}

int main(int argc, char* argv[])
{
    int returnValue = getopt_long(argc, argv, "", getOptStruct, NULL); 
    int port = 0;
    int log = 0;
    char* logarg;
    //parses through command line arguments
    while (1)
    {
        if (returnValue < 0)
        {
            break;
        }
        if (returnValue == 1)
        {
            port = 1;
            portno = atoi(optarg);
        }
        else if (returnValue == 2)
        {
            log = 1;
            logarg = optarg;
            fprintf(stderr, "%d", portno);
        }
        else
        {
            fprintf(stderr, "Invalid inputs formatting. --port is a required argument. Arguments should be formatted as such: ./lab1b --port \n");
            exit(EXIT_FAILURE);
        }
        returnValue = getopt_long(argc, argv, "", getOptStruct, NULL);
    }

    if (port == 0)
    {
        fprintf(stderr, "Invalid inputs formatting. --port is a required argument. Arguments should be formatted as such: ./lab1b --port \n");
        exit(EXIT_FAILURE);

    }

    if (log >= 0)
    {
        logfd = creat(logarg, 0666);    
        if (logfd < 0)
        {
            fprintf(stderr, "Unable to create log file");
        }

    }
    





    //copied from a resource posted on the spec

    //reset code taken from TA slides
    if (!isatty (STDIN_FILENO))
    {
        fprintf (stderr, "Not a terminal.\n");
        exit(EXIT_FAILURE);
    }

    int sockfd;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        fprintf(stderr, "Unable to open socket\n");
        exit(1);
    }
    server = gethostbyname("localhost");

    if (server == NULL)
    {
        fprintf(stderr, "Error: no such host\n");
        exit(1);
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);

    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        fprintf(stderr, "Error: Unable to connect socket\n");
        exit(1);
    }







    // struct sockaddr_in serv_addr;
    // struct hostent *server;



    // sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sockfd < 0)
    // { 
    //     fprintf(stderr,"ERROR opening socket\n");
    //     exit(EXIT_FAILURE);
    // }
    // server = gethostbyname("localhost");
    // if (server == NULL) {
    //     fprintf(stderr,"ERROR, no such host\n");
    //     exit(EXIT_FAILURE);
    // }

    // bzero((char *) &serv_addr, sizeof(serv_addr));
    // serv_addr.sin_family = AF_INET;
    // bcopy((char *)server->h_addr, 
    //      (char *)&serv_addr.sin_addr.s_addr,
    //      server->h_length);
    // serv_addr.sin_port = htons(portno);
    // if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    // { 
    //     fprintf(stderr, "%d", portno);
    //     fprintf(stderr, "ERROR connecting");
    //     exit(EXIT_FAILURE);
    // }

    // printf("Please enter the message: ");
    // bzero(buffer,256);
    // fgets(buffer,255,stdin);
    // n = write(sockfd,buffer,strlen(buffer));
    // if (n < 0) 
    //      error("ERROR writing to socket");
    // bzero(buffer,256);
    // n = read(sockfd,buffer,255);
    // if (n < 0) 
    //      error("ERROR reading from socket");
    // printf("%s\n",buffer);
    // return 0;


    tcgetattr (STDIN_FILENO, &saved_attributes);

    //when the program exits, we reset terminal to default settings
    atexit(reset_input_mode);

    struct termios current_attributes;
    //takes away terminal default settings so we can rebuild them later in code
    //taken from TA slides
    current_attributes.c_iflag = ISTRIP;
    current_attributes.c_oflag = 0;
    current_attributes.c_lflag = 0;
    current_attributes.c_cc[VMIN] = 1;
    current_attributes.c_cc[VTIME] = 0;

    //implements the new settings
    tcsetattr (STDIN_FILENO, TCSANOW, &current_attributes);

    struct pollfd eventDescriptors[] = { 
        {STDIN_FILENO, POLLIN | POLLERR | POLLHUP, 0}, 
        {sockfd, POLLIN | POLLERR | POLLHUP, 0}
    };

    int breakFlag = 0;

    int readConstant = 0;
    int writeConstant = 1;

    while (breakFlag == 0)
    {
        int checker = poll(eventDescriptors, 2, 0);

        if (checker == -1)
        {
            fprintf(stderr, "Unable to poll.");
        }

        short stdinRevent = eventDescriptors[readConstant].revents;
        short socketRevent = eventDescriptors[writeConstant].revents;
        
        if (POLLIN & socketRevent) //send output from socket to terminal
        {
            int stdin = STDIN_FILENO;
            int stdout = STDOUT_FILENO;

            char inputBuf[256];
            int size = read(sockfd, inputBuf, 256);
            //read in large chunk
            if (size < 0)
            {
                //print error
                fprintf(stderr, "Could not read from socket");
                exit(EXIT_FAILURE);
            }

            if (size == 0)
            {
                exit(0);
            }

            //write(STDOUT_FILENO, inputBuf, size);
            char inputchar;
            for (int x = 0; x < size; x++)
            {
                //write back to terminal to show user what command they typed
                //send it to the shell so it will be executed
                inputchar = inputBuf[x];
                if (inputchar == '\012')
                {
                    inputchar = '\015';
                    //write(STDOUT_FILENO, &inputchar, 1);
                    //only print line feed to shell, not carraige return
                    inputchar = '\012';
                    write(STDOUT_FILENO, "\012\015", 2);
                }
                else
                {
                    write(STDOUT_FILENO, &inputchar, 1);
                }

            }

            if (log > 0)
            {
                char printBuffer[20];
                write(logfd, "RECIEVED ", 9);
                sprintf(printBuffer, "%d", size);
                write(logfd, printBuffer, strlen(printBuffer));
                write(logfd, " BYTES: ", 8);
                write(logfd, inputBuf, size);
                char linefeed = '\012';
                write(logfd, &linefeed, 1);

            }
        }

        if (POLLIN & stdinRevent) //send text from terminal to shell
        {
            int stdin = STDIN_FILENO;
            int stdout = STDOUT_FILENO;
            int size;

            char inputBuf[256];

            size = read(0, &inputBuf, 256);
            //read input into a large buffer
            if (size < 0)
            {
                //print error
                fprintf(stderr, "Could not read from child to parent pipe");
                exit(EXIT_FAILURE);
            }

            for (int x = 0; x < size; x++)
            {
                //write back to terminal to show user what command they typed
                //send it to the shell so it will be executed
                char inputchar = inputBuf[x];
                if (inputchar == '\015' || inputchar == '\012')
                {
                    inputchar = '\015';
                    write(STDOUT_FILENO, &inputchar, 1);
                    //only print line feed to shell, not carraige return
                    inputchar = '\012';
                    write(STDOUT_FILENO, &inputchar, 1);
                }
                else
                {
                    write(STDOUT_FILENO, &inputchar, 1);
                }
            }

            if (log > 0)
            {
                char printBuffer[20];
                write(logfd, "SENT ", 5);
                sprintf(printBuffer, "%d", size);
                write(logfd, printBuffer, strlen(printBuffer));
                write(logfd, " BYTES: ", 8);
                write(logfd, inputBuf, size);
                char linefeed = '\012';
                write(logfd, &linefeed, 1);

            }

            write(sockfd, inputBuf, size);
            
        }

        //if we find errors, quit the program
        if (POLLERR & socketRevent || POLLHUP &socketRevent)
        {
            exit(0);

        }
        if (POLLERR & stdinRevent || POLLHUP &stdinRevent)
        {
            fprintf(stderr, "Recieved POLLERR or POLLHUP from stdin");
            exit(1);
        }
    }

    exit(0);
    
}