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

## Znane problemy i ograniczenia

- Obecnie nie ma wsparcia dla graficznego interfejsu użytkownika; gra jest prezentowana w konsoli.
- Gra obsługuje tylko dwa połączenia jednocześnie.
- Brak zabezpieczeń sieciowych i obsługi błędów w przypadku nieoczekiwanego rozłączenia.

## Autorzy

*Sebastian Piter*  

