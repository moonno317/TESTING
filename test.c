#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ncurses.h>

#define SERVER_IP "127.0.0.1"
#define PORT 9999
#define MAX_MESSAGE_LENGTH 100

WINDOW *inputWin, *msgWin;

void init_windows();
void destroy_windows();
void print_chat_message(const char* message);
void print_input_message(const char* message);
void client(const char* serverIP, const char* mess);

int main() {
    init_windows();

    char mess[MAX_MESSAGE_LENGTH];
    while (1) {
        print_input_message("Enter your message: ");
        fgets(mess, sizeof(mess), stdin);
        mess[strcspn(mess, "\n")] = '\0';  // Remove trailing newline character

        client(SERVER_IP, mess); // Send the message to the server with the server IP address

        if (strcmp(mess, "~") == 0) {
            break;
        }
    }

    destroy_windows();
    return 0;
}

void init_windows() {
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();

    // Calculate window dimensions
    int inputBoxHeight = 3;
    int inputBoxWidth = COLS;
    int inputBoxStartY = LINES - inputBoxHeight;
    int inputBoxStartX = 0;

    // Create a box around the chat window
    msgWin = newwin(LINES - inputBoxHeight - 1, COLS, 0, 0);
    box(msgWin, 0, 0);

    // Create a box around the input window
    inputWin = newwin(inputBoxHeight, inputBoxWidth, inputBoxStartY, inputBoxStartX);
    box(inputWin, 0, 0);

    // Enable scrolling for the chat window
    scrollok(msgWin, TRUE);

    // Refresh the initial state of windows
    refresh();
    wrefresh(msgWin);
    wrefresh(inputWin);
}

void destroy_windows() {
    // Clean up and exit ncurses
    delwin(inputWin);
    delwin(msgWin);
    endwin();
}

void print_chat_message(const char* message) {
    wprintw(msgWin, "User: %s\n", message);
    wrefresh(msgWin);
}

void print_input_message(const char* message) {
    werase(inputWin);
    box(inputWin, 0, 0); // Add a box to the input window

    // Calculate the maximum width of the input message
    int max_width = getmaxx(inputWin) - 2;

    // Truncate the message if it exceeds the maximum width
    char truncated_message[MAX_MESSAGE_LENGTH];
    strncpy(truncated_message, message, max_width - 1);
    truncated_message[max_width - 1] = '\0';

    mvwprintw(inputWin, 1, 1, "%s", truncated_message); // Print the message inside the box
    wrefresh(inputWin);
}

void client(const char* serverIP, const char* mess) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, serverIP, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("connect error");
        exit(EXIT_FAILURE);
    }

    size_t message_len = strlen(mess);

    ssize_t total_sent = 0;
    ssize_t bytes_sent;
    while (total_sent < message_len) {
        bytes_sent = send(sock_fd, mess + total_sent, message_len - total_sent, 0);
        if (bytes_sent == -1) {
            perror("send error");
            exit(EXIT_FAILURE);
        }
        total_sent += bytes_sent;
    }

    // Receive the response from the server
    char response[MAX_MESSAGE_LENGTH];
    ssize_t bytes_received = recv(sock_fd, response, sizeof(response) - 1, 0);
    if (bytes_received == -1) {
        perror("receive error");
        exit(EXIT_FAILURE);
    }
    response[bytes_received] = '\0';

    print_chat_message(response);

    close(sock_fd);
}