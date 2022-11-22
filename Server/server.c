#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

#define SOCKET_ARRAY_LENGTH 4

int socketfd;
int socket_buffer[SOCKET_ARRAY_LENGTH];
sem_t buf_sem;
sem_t items_in_buf;
sem_t empty_slots;
int index_to_write = 0;
int index_to_take = 0;

void *handle_request(void *arg);

//main is the main thread
int main(int argc, char *argv[]) {

    int server_sock, client_sock, len, numThreads=3;
    short port;
    struct sockaddr_in server_addr, client_addr; 
    //array of thread structs to act as headers
    pthread_t *threads; 


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
    printf("Launching %d threads... \n\n",numThreads);
    //create thread structs and put on the heap
    threads = (pthread_t*) calloc(numThreads,sizeof(pthread_t));

    //initialize semaphores
    if(sem_init(&buf_sem,0,1) < 0) {
        perror("Error initializing buf_sem");
    }
    if(sem_init(&items_in_buf,0,0) < 0) {
        perror("Error initializing items_in_buf");
    }
    if(sem_init(&empty_slots,0,SOCKET_ARRAY_LENGTH) < 0) {
        perror("Error initializing empty_slots");
    }

    //go through and launch all the threads
    for(int i=0; i<numThreads; i++) {
        
        long thread_id = (long)i;
        
        //launch the thread
        if(pthread_create(&threads[i],NULL,&handle_request,(void *)thread_id) != 0) {
            perror("Error");
        }
    }



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
        
        
        //accept the connection and create a new socket
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &len);
        if(client_sock < 0) {
            printf("Error accepting connection\n");
            exit(0);
        }
        printf("New connection\n");

        //wait until there is an empty slot in the socket buffer
        sem_wait(&empty_slots);
        
        //wait until we can read/write to the buffer
        sem_wait(&buf_sem);

        //printf("I got in the buffer\n");

        

        //slot is empty, fill it with the new socket
        socket_buffer[index_to_write] = client_sock;
        index_to_write = (index_to_write + 1)%SOCKET_ARRAY_LENGTH; //increment the index


        //signal that the socket buffer is open to be read/modified now
        sem_post(&buf_sem);
        //signal that the number of items in the socket buffer has increased
        sem_post(&items_in_buf);
        //printf("theres something in the buffer\n");


    }

    //release the threads
    free(threads);

}


void *handle_request(void *arg) {

    char message[2000];
    char buffer[2000];
    char bytes_size[20];
    int bytes_read = 0;
    long int file_bytes;

    //get the thread id
    int t_id = (int)(long)arg;
    printf("Thread %d launched... \n",t_id);

    
    //wait until there are items in the socket buffer
    sem_wait(&items_in_buf);
    //printf("now i'm seeing if theres stuff in the buffer\n");
    
    //wait until the socket buffer is open to read/write
    sem_wait(&buf_sem);
    //printf("now the buffer is unlocked\n");
    
    //take the first item
    int socketfd = socket_buffer[index_to_take];
    index_to_take = (index_to_take + 1) % SOCKET_ARRAY_LENGTH; //increment the index to take
    
    //signal that the buffer is open to be read/modified now
    sem_post(&buf_sem);
    //printf("now the buffer is open\n");

    //signal that the buffer now has an empty slot
    sem_post(&empty_slots);
    //printf("now it has an empty slot\n");
    

    //get the message from the client
    bytes_read = read(socketfd, buffer, 200); //max bytes to read is 200
    printf("From Client (%d bytes): %s\n", bytes_read, buffer);
    buffer[bytes_read] = 0;


    //open the file
    FILE *f = fopen(buffer,"r");
    if (NULL == f) {
        printf("file can't be opened \n");
        sprintf(message, "File cannot be opened");
        write(socketfd, message, strlen(message));
        exit(0);
    } else {
        printf("file opened\n");
    }

    
    //get the size of the file (in bytes)
    fseek(f, 0, SEEK_END);
    file_bytes = ftell(f);

    //make sure the size is at least 3 bytes
    if((file_bytes/100) < 1) {
        //add a byte to the front
        char zero[20] = "0";
        sprintf(bytes_size, "%ld", file_bytes);
        strcat(zero, bytes_size);

        //erase the bytes and replace with the updated size with correct bytes
        bzero(&bytes_size,sizeof(bytes_size));
        strcat(bytes_size,zero);

    } else {
        //leave it 
        sprintf(bytes_size, "%ld", file_bytes);
        printf("this is the size: %s\n", bytes_size);
    }

    //set the pointer back to the front of the file
    fseek(f, 0, SEEK_SET);
    //read the contents of the file
    int count = fread(&message, 1, file_bytes, f);

    //error checking
    printf("File contents: %s\n", message);

    //close the file
    fclose(f);


    //send message back to the client (bytes & file contents)
    write(socketfd, bytes_size, strlen(bytes_size));
    write(socketfd, message, strlen(message));

    
}