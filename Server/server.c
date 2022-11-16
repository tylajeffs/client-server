#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

void handle_request(int socketfd);

int main(int argc, char *argv[]) {

    int server_sock, client_sock, len;
    short port;
    struct sockaddr_in server_addr, client_addr;

    //get the listening port number from the command line
    if(argc < 2) {
        printf("Usage: %s PORT\n", argv[0]);
        exit(0);
    }

    //set up the port
    port = (short)atoi(argv[1]);
    printf("Launching Server on Port: %d\n", port);

    //create a new socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock == -1) {
        printf("Could not create socket\n");
        exit(0);
    }

    //zero out the server address
    bzero(&server_addr, sizeof(server_addr));

    //launch thread pool






    //set structs to contain internet addresses we might want to connect to
    server_addr.sin_family = AF_INET; //IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY; //willing to connect to any IP address
    server_addr.sin_port = htons(port); //set the port, convert to big endian

    //ask OS for permission to listen on this port number
    if((bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr))) < 0) {
        printf("Error binding socket\n");
        perror("Error");
        exit(0);
    }
    printf("Socket Bound\n");

    //turn socket into listening mode
    if((listen(server_sock, 3)) != 0) {
        printf("Listen Failed\n");
        exit(0);
    }

    len = sizeof(client_addr);

    //loop to continuously recieve connections
    while(1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &len);

        if(client_sock < 0) {
            printf("Error accepting connection\n");
            exit(0);
        }

        printf("New connection\n");

        //handle the request
        handle_request(client_sock);
    }

}


void handle_request(int socketfd) {
    char message[200];
    char buffer[200];
    int bytes_read = 0;
    long int file_bytes = 0;


    bytes_read = read(socketfd, buffer, 200); //max bytes to read is 200
    buffer[bytes_read] = 0;
    printf("From Client (%d bytes): %s\n", bytes_read, buffer);


    //open the file
    FILE *ptr = fopen(buffer,"r");
    if (NULL == ptr) {
        printf("file can't be opened \n");
        sprintf(message, "File cannot be opened");
        exit(0);
    } else {
        printf("file opened\n");
    }

    //get the size of the file (in bytes)
    fseek(ptr, 0L, SEEK_END);
    file_bytes = ftell(ptr);

    //read the contents of the file
    if(fgets(message, 200, ptr) != NULL) {
        printf("File contents: %s\n", message);
    }

    //close the file
    fclose(ptr);


    //send message back to the client (bytes & file contents)
    write(socketfd, file_bytes, strlen(file_bytes));
    write(socketfd, message, strlen(message));
}