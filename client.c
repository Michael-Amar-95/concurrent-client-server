#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <time.h>

#include <errno.h>

#include <dirent.h>

#ifndef MAX_BUF
#define MAX_BUF 256
#endif

/**
 * Send data and signal to server.
 * @param serverPid - sever pid
 * @param argv - the argument to calculates.
 */
void sendSignal(int proc_pid, char **argv) {

//    char buf[256];
//    memset(buf,0,256);

    // open "to_srv" file
    int fd;
    fd = open("to_srv", O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, S_IRWXU);
    if (fd < 0) {
        write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
        exit(-1);
    }

    // write into the file the pid
    pid_t pid = getpid(); //get the pid of client

    char tempBuf[MAX_BUF];
    int bytes = sprintf(tempBuf, "%d\n", pid); //get the size of tempBuf

    //write the client pid to file
    write(fd, tempBuf, bytes);

//    printf("write pid to_client\n");

    int i;
    for (i = 2; i < 5; i++) {

//        printf("write to_client: %s\n", argv[i]);

        //write into the file the arguments
        write(fd, argv[i], strlen(argv[i]));
        write(fd, "\n", strlen("\n"));
    }


    //close the file
    if (close(fd) < 0) {
        write(1, "ERROR_FROM_PROJECT\n", strlen("ERROR_FROM_PROJECT\n"));
        exit(-1);
    }

    //send signal to the server
    kill(proc_pid, SIGUSR1);
}

/**
 * This function handle with the signals.
 * @param signum - the signal.
 */
void sig_handler(int signum) {

    if (signum == SIGALRM) {         //signal handler for SIGALRM
        //print that no signal has received from server
        write(1, "Client closed because no response was received from the server for 30 seconds\n",
              strlen("Client closed because no response was received from the server for 30 seconds\n"));
        exit(EXIT_FAILURE);
    }

    if (signum == SIGUSR2) {  //signal handler from server

//        printf("client received data from server \n");

        alarm(0);

        //save the client pid
        char clientPid[MAX_BUF];
        memset(clientPid, 0, MAX_BUF);
        sprintf(clientPid, "%d", getpid());

        //try to read the answer from the server file
        char to_client[MAX_BUF];
        memset(to_client, 0, MAX_BUF);
        strcpy(to_client, "to_client_");
//        sprintf(to_client,"to_client_");

        //the name of the file that the server create
        strcat(to_client, clientPid);

//        printf("try open to_client: %s\n", to_client);

        char buffer[MAX_BUF];
        //try to open the file
        int fd = open(to_client, O_RDONLY);
        if (fd < 0) {

//            printf("cant open to_client\n");

            printf("ERROR_FROM_PROJECT\n");
            exit(-1);
        }

//        printf("can open to_client\n");


        //try to read from the file
        if (read(fd, buffer, MAX_BUF) < 0) {
            printf("ERROR_FROM_PROJECT\n");
            exit(-1);
        }

        //check if we made divide in zero
        if (buffer[0] == 'C') {
            printf("%s\n", "CANNOT_DIVIDE_BY_ZERO");
        } else {
            printf("%d\n", atoi(buffer));
        }

        //try to close the file
        if (close(fd) < 0) {
            printf("ERROR_FROM_PROJECT\n");
            exit(-1);
        }

        //try to delete the file
        if (remove(to_client) < 0) {
            printf("ERROR_FROM_PROJECT\n");
            exit(-1);
        }
        exit(0);
    }
}

/**
 * This is the main function.
 * @param argc = count how many args receive.
 * @param argv - the args.
 * @return
 */
int main(int argc, char **argv) {

    //check if I got exactly 5 arguments
    if (argc != 5) {
        printf("ERROR_FROM_PROJECT\n");
        exit(-1);
    }

    // Register signal handler for SIGALRM
    signal(SIGALRM, sig_handler);
    // Register signal handler for SIGINT
    signal(SIGUSR2, sig_handler); /* handles signal from server */

    // for random time
    srand(time(NULL));

    // get the sever pid
    int proc_pid = atoi(argv[1]);;
    //get the action
    int action = atoi(argv[3]);

//    printf("in server \n");


    //check if the action is valid
    if (action < 1 || action > 4) {

//        printf("check action\n");

        printf("ERROR_FROM_PROJECT\n");
        exit(-1);
    }

    //count the attempts of getting to srv file
    int counterAttempts = 0;
    while (counterAttempts < 10) {

//        printf("will loop \n");
//        printf("client pid: %d\n", getpid());


        // check if I can access to srv file.
        if (!access("to_srv", F_OK)) { // if cant I will sleep for 1-5 second

//            printf("cant accesses to_srv\n");

            int s = rand() % 6;
            sleep(s);
        } else {

//            printf("server pid: %d\n", proc_pid);

            sendSignal(proc_pid, argv);
            alarm(30);
            break;
        }
        counterAttempts++;
    }

//    printf("out of loop\n");

    //check how many attempts we madeS
    if (counterAttempts == 9) {
        printf("ERROR_FROM_PROJECT\n");
        exit(-1);
    }

    unsigned int ret = alarm(30);
    pause();

    return 0;
}

