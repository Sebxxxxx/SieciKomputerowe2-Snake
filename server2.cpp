#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <chrono>

const int BOARD_WIDTH = 40;
const int BOARD_HEIGHT = 25;
enum Cell { EMPTY, SNAKE, FOOD };

std::vector<int> clients;
std::mutex clients_mutex;

struct Snake {
    std::vector<std::pair<int, int>> body;
    char direction;
    int length;
};
bool game_over = false;
std::vector<Snake> snakes;
bool game_ready = false; 
std::mutex board_mutex;
std::pair<int, int> food_position;

char buffer[1024];

int server_fd;
struct sockaddr_in address;
int addrlen = sizeof(address);

const char EMPTY_CHAR = ' ';
const char WALL_CHAR = '#';
const char SNAKE_CHAR = 'o';
const char FOOD_CHAR = 'F';

Cell board[BOARD_HEIGHT][BOARD_WIDTH] = {};

std::pair<int, int> generate_food_position() {
    int x, y;
    do {
        x = rand() % (BOARD_WIDTH - 2) + 1;  // Generuj x w zakresie [1, BOARD_WIDTH - 2]
        y = rand() % (BOARD_HEIGHT - 2) + 1; // Generuj y w zakresie [1, BOARD_HEIGHT - 2]
    } while (std::any_of(snakes.begin(), snakes.end(),
             [&](const Snake& snake) {
                 return std::any_of(snake.body.begin(), snake.body.end(),
                     [&](const std::pair<int, int>& segment) {
                         return segment.first == x && segment.second == y;
                     });
             }));
    return std::make_pair(x, y);
}
void place_food() {
    std::lock_guard<std::mutex> lock(board_mutex);
    if (board[food_position.second][food_position.first] == EMPTY) {
        board[food_position.second][food_position.first] = FOOD;
    }
}

std::string serialize_game_state() {
    std::string game_state = "";
    game_state += "Score: " + std::to_string(snakes[0].length) + " - " + std::to_string(snakes[1].length) + "\n";
    
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (i == 0 || i == BOARD_HEIGHT - 1 || j == 0 || j == BOARD_WIDTH - 1) {
                game_state += WALL_CHAR;
            } else if (board[i][j] == EMPTY) {
                game_state += EMPTY_CHAR;
            } else if (board[i][j] == SNAKE) {
                game_state += SNAKE_CHAR;
            } else if (board[i][j] == FOOD) {
                game_state += FOOD_CHAR;
            }
        }
        game_state += "\n";
    }
    
    return game_state;
}

void initialize_game(int player1_socket, int player2_socket) {
    srand(time(NULL));
    for (auto &row : board) {
        std::fill(row, row + BOARD_WIDTH, EMPTY);
    }

    snakes.clear();
    snakes.push_back({{std::make_pair(BOARD_WIDTH / 2, BOARD_HEIGHT / 2)}, 'R', 1});
    snakes.push_back({{std::make_pair(BOARD_WIDTH / 2 - 1, BOARD_HEIGHT / 2)}, 'L', 1});

    for (auto &snake : snakes) {
        for (auto &segment : snake.body) {
            board[segment.second][segment.first] = SNAKE;
        }
    }
    food_position = generate_food_position();
    board[food_position.second][food_position.first] = FOOD;

    std::string start_message = "Game starts now!";
    send(player1_socket, start_message.c_str(), start_message.size(), 0);
    send(player2_socket, start_message.c_str(), start_message.size(), 0);
}

void update_snake_position(Snake& snake) {
    std::pair<int, int> newHead = snake.body.front();
    if (snake.direction == 'L') newHead.first--;
    if (snake.direction == 'R') newHead.first++;
    if (snake.direction == 'U') newHead.second--;
    if (snake.direction == 'D') newHead.second++;


    bool ateFood = false;
    // Ograniczenia planszy, zapobieganie wyjściu poza nią
    if (newHead.first >= BOARD_WIDTH - 1) newHead.first = 1;
    else if (newHead.first <= 0) newHead.first = BOARD_WIDTH - 2;

    if (newHead.second >= BOARD_HEIGHT - 1) newHead.second = 1;
    else if (newHead.second <= 0) newHead.second = BOARD_HEIGHT - 2;

    // Sprawdzanie, czy wąż zjadł jedzenie
    if (newHead == food_position) {
    
          snake.length++;
        
        snake.body.insert(snake.body.begin(), newHead);
        food_position = generate_food_position(); 
    } else {
        std::pair<int, int> tail = snake.body.back();
        snake.body.pop_back();
        board[tail.second][tail.first] = EMPTY;
        snake.body.insert(snake.body.begin(), newHead);
    }

    // Dodaj nową głowę węża
    std::lock_guard<std::mutex> lock(board_mutex);
    board[newHead.second][newHead.first] = SNAKE;
    for (auto &segment : snake.body) {
        board[segment.second][segment.first] = SNAKE;
        }
}


bool check_collision(const Snake& snake) {
    const std::pair<int, int>& head = snake.body.front();

  
    for (size_t i = 1; i < snake.body.size(); ++i) {
        if (head == snake.body[i]) {
            return true;
        }
    }

    
    for (const auto& other_snake : snakes) {
        if (&other_snake == &snake) continue; 

        for (const auto& segment : other_snake.body) {
            if (head == segment) {
                return true;
            }
        }
    }

    return false;
}

void update_game_state() {
    for (auto &row : board) {
        std::fill(row, row + BOARD_WIDTH, EMPTY);
    }

    for (int i = 0; i < snakes.size(); i++) {
        update_snake_position(snakes[i]);
        if (check_collision(snakes[i])) {
            std::cout << "Player " << i + 1 << " lost." << std::endl;
            std::string game_over_message = "Game Over. Player " + std::to_string(i + 1) + " lost.";
            for (int client : clients) {
                send(client, game_over_message.c_str(), game_over_message.size(), 0);
            }
            return;
        }
    }

    place_food();
    std::string game_state = serialize_game_state();
    for (int client : clients) {
        send(client, game_state.c_str(), game_state.size(), 0);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Opóźnienie gry
}

void handle_client(int client_socket, int player_id) {
    if (player_id % 2 == 1 ) {
        std::string start_message = "Waiting for the other player to join...";
        send(client_socket, start_message.c_str(), start_message.size(), 0);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    for (int i = 3; i > 0; i--) {
        std::string count_message = std::to_string(i);
        send(client_socket, count_message.c_str(), count_message.size(), 0);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::thread snake_thread([&]() {
        while (true) {
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                update_snake_position(snakes[player_id]);
                if (check_collision(snakes[player_id])) {
                    std::cout << "Player " << player_id + 1 << " lost." << std::endl;
                    std::string game_over_message = "Game Over. Player " + std::to_string(player_id + 1) + " lost.";
                    for (int client : clients) {
                        send(client, game_over_message.c_str(), game_over_message.size(), 0);
                    }
                    return;
                }
                update_game_state();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });
    snake_thread.detach();

    

  
    initialize_game(client_socket, -1);

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int read = recv(client_socket, buffer, sizeof(buffer), 0);
        if (read <= 0) break;

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            if (buffer[0] == 'U' || buffer[0] == 'D' || buffer[0] == 'L' || buffer[0] == 'R') {
                snakes[player_id].direction = buffer[0];
            }
        }
    }
    close(client_socket);
}

int main() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(10155);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    while (true) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            continue;
        }

        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(new_socket);

        
        if (clients.size() % 2 == 0) {
            
            initialize_game(clients[clients.size() - 2], clients[clients.size() - 1]);

            std::thread client_thread1(handle_client, clients[clients.size() - 2], 0);
            std::thread client_thread2(handle_client, clients[clients.size() - 1], 1);

            
            client_thread1.detach();
            client_thread2.detach();
        }
    }

    
    close(server_fd);
    return 0;
}
