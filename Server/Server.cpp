#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#define PORT 8080
#define BUFFER_SIZE 1024

void sendFile(SOCKET clientSocket, const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Ошибка: файл не найден!" << std::endl;
        return;
    }
    char buffer[BUFFER_SIZE];
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        send(clientSocket, buffer, file.gcount(), 0);
    }
    std::cout << "Файл отправлен!" << std::endl;
    file.close();
}

void receiveFile(SOCKET clientSocket, const std::string& fileName) {
    std::ofstream file(fileName, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Ошибка при создании файла!" << std::endl;
        return;
    }
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) { file.write(buffer, bytesReceived); }
    std::cout << "Файл получен!" << std::endl;
    file.close();
}

int main() {
    setlocale(LC_ALL, "Ru");

    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка инициализации Winsock!" << std::endl;
        return 1;
    }
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета!" << std::endl;
        WSACleanup();
        return 1;
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка связывания сокета!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    if (listen(serverSocket, 3) == SOCKET_ERROR) {
        std::cerr << "Ошибка прослушивания!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Ожидание клиента..." << std::endl;
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка подключения клиента!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    char command[BUFFER_SIZE] = { 0 };
    recv(clientSocket, command, sizeof(command), 0);
    if (std::string(command) == "SEND") {
        char fileName[BUFFER_SIZE] = { 0 };
        recv(clientSocket, fileName, sizeof(fileName), 0);
        receiveFile(clientSocket, fileName);
    }
    else if (std::string(command) == "GET") {
        char fileName[BUFFER_SIZE] = { 0 };
        recv(clientSocket, fileName, sizeof(fileName), 0);
        sendFile(clientSocket, fileName);
    }
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
