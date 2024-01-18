#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <conio.h>


#pragma comment(lib, "ws2_32.lib")

const int PORT = 10155;
const char* SERVER_IP = "192.168.3.40"; // Adres IP serwera

void receive_messages(SOCKET sock) {
    char buffer[4096];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int read = recv(sock, buffer, sizeof(buffer), 0);
        if (read <= 0) break;

        // Wyświetlanie aktualnego stanu gry
        
        if (std::string(buffer).find(
            "Game Over") != std::string::npos) {
            std::cout << buffer << std::endl; // Wyświetlamy komunikat o przegranej
        break; // Kończymy pętlę odbierania wiadomości
    }
        system("cls");
        std::cout << buffer << std::endl;
    }
}

int main() {
    WSADATA wsaData;
    SOCKET sock = 0;
    struct sockaddr_in serv_addr;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "Oczekiwanie na gracza";
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    // Łączenie z serwerem
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Połączenie nie powiodło się." << std::endl;
        return 1;
    }

    std::thread receiver_thread(receive_messages, sock);
    receiver_thread.detach();

    while (true) {
        if (_kbhit()) {
            char c = _getch();
            char dir;
            switch (c) {
            case 'w': dir = 'U'; break;
            case 's': dir = 'D'; break;
            case 'a': dir = 'L'; break;
            case 'd': dir = 'R'; break;
            default: continue;
            }
            send(sock, &dir, sizeof(dir), 0);
        }
    }

    closesocket(sock);
    WSACleanup();

    return 0;
}