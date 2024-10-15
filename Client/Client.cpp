#include <iostream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

void sendFile(SOCKET socket, const std::string& fileName) {
    std::ifstream file(fileName, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Ошибка: файл не найден!" << std::endl;
        return;
    }

    char buffer[BUFFER_SIZE];
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        send(socket, buffer, file.gcount(), 0);
    }
    std::cout << "Файл отправлен!" << std::endl;
    file.close();
}

void receiveFile(SOCKET socket, const std::string& fileName) {
    std::ofstream file(fileName, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Ошибка при создании файла!" << std::endl;
        return;
    }

    char buffer[BUFFER_SIZE];
    int bytesReceived;
    while ((bytesReceived = recv(socket, buffer, sizeof(buffer), 0)) > 0) {
        file.write(buffer, bytesReceived);
    }

    std::cout << "Файл получен!" << std::endl;
    file.close();

    std::ofstream out;
    out.open(fileName);
    if (out.is_open())
    {
        out << "Hello World!";
    }
    out.close();
}

int main() {
    setlocale(LC_ALL, "Ru");

    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Ошибка инициализации Winsock!" << std::endl;
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета!" << std::endl;
        WSACleanup();
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        std::cerr << "Неверный адрес!" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Ошибка подключения!" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::string command;
    std::cout << "Введите команду (SEND или GET): ";
    std::cin >> command;

    send(clientSocket, command.c_str(), command.length(), 0);

    if (command == "SEND") {
        std::string fileName;
        std::cout << "Введите имя файла для отправки: ";
        std::cin >> fileName;
        send(clientSocket, fileName.c_str(), fileName.length(), 0);
        sendFile(clientSocket, fileName);
    }
    else if (command == "GET") {
        std::string fileName;
        std::cout << "Введите имя файла для получения: ";
        std::cin >> fileName;
        send(clientSocket, fileName.c_str(), fileName.length(), 0);
        receiveFile(clientSocket, fileName);
    }
    else {
        std::cerr << "Неверная команда!" << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
