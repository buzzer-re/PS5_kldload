#include "../include/server.h"


int start_server(int port, void(*callback)(int fd, void* data, ssize_t data_size))
{
    int sock_fd;
    int conn;
    struct sockaddr_in addr;
    int socklen = sizeof(addr);
    int status = true;

    uint8_t buff[SOCK_BUFF_SIZE] = {0};
    uint8_t* read_data = NULL;
    ssize_t bytes_read = 0;
    ssize_t total_len = 0;



    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0) ) == 0)
    {
        return -1;
    }
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
        return -1;
    }
    
    

    if (listen(sock_fd, 1) < 0)
    {
        return -1;
    }


    while (1)
    {
        printf("Waiting connections...\n");
        conn = accept(sock_fd, (struct sockaddr*) &addr, (socklen_t*) &socklen);

        if (conn < 0)
        {
            puts("kldload server: Accept failed!");
            status = false;
            continue;
            // break;
        }

        //
        // This is not a very good method to know the incoming data, but this is not a server that will receive 
        // a lot amount of it 
        //
        while ((bytes_read = recv(conn, buff, SOCK_BUFF_SIZE, 0)) > 0)
        {
            read_data = realloc(read_data, bytes_read + total_len);
            memcpy(read_data + total_len, buff, bytes_read);

            total_len += bytes_read;
        }

        printf("Read %zd bytes!, calling callback...\n", total_len);
        
        //
        // Call the callback to process what was received
        //
        callback(conn, read_data, total_len);
        
        close(conn);
        free(read_data);
        read_data = NULL;
    }

    close(sock_fd);
    return status;
}