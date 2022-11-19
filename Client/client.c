#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>


int main(int argc, char *argv[]) {

    int sockfd, bytes_read = 0, file_size;
    short port;
    struct sockaddr_in servaddr;
    char buffer[2000];
    char file_to_read[50];

    //get the listening port number from the command line
    if(argc < 2) {
        printf("Usage: %s PORT\n", argv[0]);
        exit(0);
    }

    //get the file the client wants to retrieve
    printf("Enter file to retrieve: ");
    scanf("%s",file_to_read);

    //set up the port
    port = (short)atoi(argv[1]);
    printf("Launching Client on Port: %d\n", port);

    //create a new socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        printf("Could not create socket\n");
        exit(0);
    }

    //zero out the server address
    bzero(&servaddr, sizeof(servaddr));

    //set variables of the structs
    servaddr.sin_family = AF_INET; //IPv4
    servaddr.sin_port = htons(port);

    //connect the client to their IP address (localhost)
    if(inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <= 0) {
        printf("Invalid address, not supported\n");
        exit(0);
    }

    //try to connect (wakes up server)
    if(connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        perror("Error");
        printf("Connection Failed\n");
        exit(0);
    }

    //message to send to the server
    sprintf(buffer, "%s",file_to_read);

    //send message to the server
    write(sockfd, buffer, strlen(buffer));
    //erase buffer
    bzero(&buffer,sizeof(buffer)); 

    //read the message from the server
    //bytes_read = read(sockfd, buffer, sizeof(buffer)); 
    //buffer[bytes_read] = 0;

    //read the first 3 digits (this is the file size)
    read(sockfd, buffer, 3); 
    file_size = atoi(buffer);

    //erase buffer
    bzero(&buffer,sizeof(buffer)); 

    //set bytes read
    bytes_read = 0;
    
    //read the rest of the message until it matches the file size sent
    while(bytes_read < file_size) {
        //read another byte
        bytes_read = read(sockfd, &buffer[bytes_read], (file_size - bytes_read)); 
    }

    //print it
    printf("From the server: %s\n", buffer);

    


}



