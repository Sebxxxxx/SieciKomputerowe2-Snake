# Gra Wąż - Projekt Serwerowy

## Opis

Projekt ten jest serwerem do prostej gry sieciowej w węża, zaprojektowanej dla dwóch graczy. Każdy gracz kontroluje węża, który porusza się po planszy. Celem gry jest unikanie kolizji z własnym ogonem oraz wężem przeciwnika. Gracze mogą zmieniać kierunek ruchu swoich węży za pomocą klawiszy kierunkowych na klawiaturze. Gra kończy się, gdy któryś z graczy przegra, uderzając w przeszkodę lub w swojego węża.

## Wymagania środowiskowe

Aby uruchomić serwer, potrzebujesz następujących środowisk i narzędzi:
- **System operacyjny:** Linux / Unix / Windows
- **Kompilator C++** obsługujący standard C++11 lub nowszy (np. g++, clang)
- **Biblioteka Winsock2** dla Windows (dla Linuxa/Unixa nie jest potrzebna)
- **Zainstalowane i skonfigurowane środowisko deweloperskie** (IDE lub kompilator)

## Instrukcja uruchomienia

1. **Kompilacja kodu źródłowego:** Najpierw należy ustawic odpowiedni adres ip oraz port poączenia na serwerze oraz kliencie, następnie skompilować kod serwera i klienta. Można to zrobić za pomocą kompilatora C++ w wierszu poleceń. Przykład kompilacji dla systemu Linux/Unix:
    ```bash
    g++ -std=c++11 -pthread server.cpp -o server
    g++ -std=c++11 klient.cpp -o klient
    ```

    Dla systemu Windows, upewnij się, że używasz kompilatora obsługującego bibliotekę Winsock2.

2. **Uruchomienie serwera:** Po skompilowaniu, uruchom skompilowany plik serwera. Na przykład:
    ```bash
    ./server
    ```

3. **Połączenie klientów:** Po uruchomieniu serwera, dwóch graczy może uruchomić klienta na swoich komputerach i połączyć się z serwerem. Klient zostanie uruchomiony za pomocą:
    ```bash
    ./client
    ```

4. **Rozpoczęcie gry:** Po pomyślnym połączeniu obu graczy, gra rozpocznie się automatycznie.

# Komunikacja siecowa
Serwer i klient komunikują się za pomocą gniazd TCP/IP. 
Klient wysyła ruchy gracza do serwera, a serwer wysyła stan gry do klientów. 
Gdy gra się zakończy, serwer wysyła komunikat "Game Over"

## Serwer
- Serwer jest uruchamiany na adresie IP "192.168.3.40" i porcie 10155.
- Serwer nasłuchuje na tym porcie na przychodzące połączenia od klientów.
- Gdy klient nawiąże połączenie, serwer przyjmuje je i tworzy nowy wątek, który obsługuje tego klienta.
- Każdy klient jest reprezentowany przez wątek handle_client.
- Wątek handle_client początkowo wysyła komunikat do klienta, informując go, że oczekuje na drugiego gracza, jeśli jest to pierwszy klient.
- Następnie wątek handle_client czeka na połączenie drugiego gracza i inicjalizuje grę, wysyłając komunikat o rozpoczęciu gry.
- Wątek handle_client nasłuchuje na ruch gracza (strzałki w górę, w dół, w lewo, w prawo) i przekazuje te ruchy do serwera za pomocą funkcji send.
- Wątek handle_client również odbiera stan gry od serwera i wyświetla go na ekranie klienta.
- Jeśli któryś z graczy przegra, serwer wysyła komunikat "Game Over" do klientów, a gra zostaje zakończona.


```bash
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

    std::string waiting_message = "Waiting for the other player to join...";
    send(client_socket, waiting_message.c_str(), waiting_message.size(), 0);

  
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

```


## Klient
- Klient łączy się z serwerem na adresie IP "192.168.3.40" i porcie 10155.
- Klient tworzy wątek receive_messages, który jest odpowiedzialny za odbieranie wiadomości od serwera.
- Wątek receive_messages odbiera dane od serwera za pomocą funkcji recv. Jeśli dane zawierają komunikat "Game Over", to wyświetla go na ekranie i kończy odbieranie wiadomości.
- W przeciwnym przypadku wątek receive_messages czyszczy ekran i wyświetla aktualny stan gry.
- W głównym wątku klienta oczekuje na naciśnięcie klawiszy przez gracza (_kbhit). Jeśli gracz naciśnie klawisze WSAD, to klient wysyła odpowiednią komendę do serwera za pomocą funkcji send.

## Znane problemy i ograniczenia

- Obecnie nie ma wsparcia dla graficznego interfejsu użytkownika; gra jest prezentowana w konsoli.
- Gra obsługuje tylko dwa połączenia jednocześnie.
- Brak zabezpieczeń sieciowych i obsługi błędów w przypadku nieoczekiwanego rozłączenia.

## Autorzy

*Sebastian Piter*  

