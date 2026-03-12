#include "../include/server.h"
#include <ps5/klog.h>


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



    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        klog_printf("socket() failed: %d\n", sock_fd);
        return -1;
    }
//    printf("socket fd: %d\n", sock_fd);

    int optval = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
        klog_perror("bind failed");
        return -1;
    }
    // puts("bind ok");

    if (listen(sock_fd, 1) < 0)
    {
        klog_perror("listen failed");
        return -1;
    }
    // puts("listen ok");


    while (1)
    {
        // printf("Waiting connections...\n");
        conn = accept(sock_fd, (struct sockaddr*) &addr, (socklen_t*) &socklen);

        if (conn < 0)
        {
            klog_puts("kldload server: Accept failed!");
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
            // a lot of data in a row could break this
            uint8_t* tmp = realloc(read_data, bytes_read + total_len);
            if (!tmp)
            {
                klog_puts("kldload server: realloc failed");
                free(read_data);
                read_data = NULL;
                total_len = 0;
                break;
            }
            read_data = tmp;
            memcpy(read_data + total_len, buff, bytes_read);

            total_len += bytes_read;
        }

        if (!read_data || total_len <= 0)
        {
            klog_puts("kldload server: empty or failed recv, skipping");
            close(conn);
            free(read_data);
            read_data = NULL;
            total_len = 0;
            continue;
        }

        klog_printf("Read %zd bytes!, calling callback...\n", total_len);
        callback(conn, read_data, total_len);
        
        close(conn);
        free(read_data);
        read_data = NULL;
        total_len = 0;
    }

    close(sock_fd);
    return status;
}