/*Shuzhan Yang
2026/3/11*/
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#include "cJSON.h"

void format_message(char *json_string) {
    // 1. Handle Error (invalid format or non-JSON data)
    cJSON *root = cJSON_Parse(json_string); 
    if (!root) {
        const char *error_ptr = cJSON_GetErrorPtr(); // Get the specific location where the error occurred
        if (error_ptr != NULL) {
            printf("Error: Invalid JSON format. Error before: %s\n", error_ptr);
        } else {
            printf("Error: Failed to parse JSON data.\n");
        }
        return; // Exit directly on error to prevent program crash
    }

    // 2. Handle JSON Array of Objects (e.g.: [{...}, {...}])
    if (cJSON_IsArray(root)) {
        printf("Received JSON Array of Objects (Formatted):\n");
        char *formatted_json = cJSON_Print(root);
        if (formatted_json) {
            printf("%s\n", formatted_json);
            free(formatted_json); // Remember to free memory
        }
        
    } 
    // 3. Handle a single JSON Object (e.g.: {...})
    else if (cJSON_IsObject(root)) {
        printf("Received JSON Object (Formatted):\n");
        char *formatted_json = cJSON_Print(root);
        if (formatted_json) {
            printf("%s\n", formatted_json);
            free(formatted_json); // Remember to free memory
        }
    } 
    // 4. Handle other rare cases (e.g., received a string or number directly)
    else {
        printf("Warning: Received JSON is valid but is neither an Object nor an Array.\n");
    }

    // 5. Free the memory occupied by the cJSON structure
    cJSON_Delete(root); 
}

int main(){
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    int reuse = 1; // 1 means on
    struct sockaddr_in src;
    socklen_t len = sizeof(src);

    struct sockaddr_in addr;
    struct ip_mreq mreq;
    memset(&addr, 0, sizeof(addr));      // initialize
    addr.sin_family = AF_INET;            // IPv4
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // address

    char mcast_ip[32];
    int port;

    printf("Enter multicast IP+port: ");
    int result = scanf("%31s%d", mcast_ip, &port);
    if (result != 2) { // handle exception
        printf("Error: Invalid input format.\n");
    } else {
        printf("Success: %s:%d\n", mcast_ip, port);
    }

    mreq.imr_multiaddr.s_addr = inet_addr(mcast_ip); // multicast address 
    addr.sin_port = htons(port);         // port number

    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    bind(socketfd, (struct sockaddr*)&addr, sizeof(addr));
    setsockopt(socketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)); // join
    char buf[65536];  // 64 KB cache

    while (1) {
        int n = recvfrom(socketfd, buf, sizeof(buf) - 1, 0,
                         (struct sockaddr*)&src, &len);
        if (n < 0) {
            perror("recvfrom");
            break;
        }

        buf[n] = 0; 
        printf("Raw JSON: %s\n", buf);
        format_message(buf);
    }
}