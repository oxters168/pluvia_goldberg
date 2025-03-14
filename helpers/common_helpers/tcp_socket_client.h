#ifndef TCP_SOCKET_CLIENT
#define TCP_SOCKET_CLIENT

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <common_helpers/logger.h>
#include <cstdint>

#define DEFAULT_TCP_SOCKET_PORT 34865
#define DEFAULT_TCP_ADDRESS "127.0.0.1"
#define SOCKET_TIMEOUT_MS 30000

enum SteamMessageType {
    MSG_INIT = 1,
    MSG_SHUTDOWN = 2,
    MSG_RESTART_APP = 3,
    MSG_IS_RUNNING = 4,
    MSG_REGISTER_CALLBACK = 5,
    MSG_UNREGISTER_CALLBACK = 6,
    MSG_RUN_CALLBACKS = 7,
};

class TCPSocketClient {
private:
    SOCKET sockfd;
    bool initialized;

    bool tryConnect() {
        struct sockaddr_in server_addr;
        ZeroMemory(&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(DEFAULT_TCP_SOCKET_PORT);
        // server_addr.sin_port = DEFAULT_PORT;
        inet_pton(AF_INET, DEFAULT_TCP_ADDRESS, &server_addr.sin_addr);

        DWORD timeout = SOCKET_TIMEOUT_MS;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            LogError("Connection");
            return false;
        }
        return true;
    }

public:
    TCPSocketClient() : initialized(false), sockfd(INVALID_SOCKET) {
        LogMessage("Creating TCPSocketClient");
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            LogError("WSAStartup");
            return;
        }

        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd == INVALID_SOCKET) {
            LogError("socket");
            WSACleanup();
            return;
        }

        if (!tryConnect()) {
            closesocket(sockfd);
            WSACleanup();
            return;
        }

        LogMessage("TCPSocketClient initialized");
        initialized = true;
    }

    ~TCPSocketClient() {
        if (initialized) {
            closesocket(sockfd);
            WSACleanup();
        }
    }

    bool isInitialized() const { return initialized; }

    bool sendMessage(SteamMessageType type) {
        if (!initialized) return false;

        uint32_t networkType = type;
        LogMessage("Sending message type: %u", type);

        return send(sockfd, (const char*)&networkType, sizeof(networkType), 0) == sizeof(networkType);
    }

    bool sendMessage(SteamMessageType type, const void* data, size_t dataSize) {
        if (!initialized) return false;

        uint32_t messageType = type;
        // uint32_t messageSize = dataSize;

        if (send(sockfd, (const char*)&messageType, sizeof(messageType), 0) != sizeof(messageType)) {
            return false;
        }

        if (dataSize > 0 && data != nullptr) {
            if (send(sockfd, (const char*)data, dataSize, 0) != dataSize) {
                return false;
            }
        }
        return true;
    }

    template<typename T>
    bool receiveResponse(T* response) {
        if (!initialized) return false;

        LogMessage("Waiting for response of size %zu", sizeof(T));

        int bytesReceived = recv(sockfd, (char*)response, sizeof(T), 0);
        if (bytesReceived == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAETIMEDOUT) {
                LogMessage("Receive timeout");
            }
            else {
                LogError("receiveResponse");
            }
            return false;
        }

        LogMessage("Received %d bytes", bytesReceived);

        return bytesReceived == sizeof(T);
    }
};

#endif
