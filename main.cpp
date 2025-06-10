#include <iostream>
#include <chrono> // Do pomiaru czasu operacji
#include <random> // Do generowania losowych danych
#include <vector> // Do przechowywania danych
#include <algorithm> // Do tasowania (shuffle)
#include <memory> // Do zarzadzania pamiecia (unique_ptr)
#include <fstream> // Do zapisu wynikow do pliku
#include <iomanip> // Do formatowania wyjscia
#include <limits>  // Do std::numeric_limits

#include "hash_table_base.h" // Bazowa klasa dla tabeli hashujacej
#include "chaining_hash_table.h" // Implementacja z lancuchowaniem
#include "open_addressing_hash_table.h" // Implementacja z adresowaniem otwartym
#include "avl_hash_table.h" // Implementacja z lancuchowaniem i drzewami AVL

// Uwaga: Upewnij sie, ze klasy HashTableBase, ChainingHashTable, OpenAddressingHashTable,
// i AVLHashTable sa poprawnie zdefiniowane w odpowiednich plikach .h
// i implementuja metody insert, find, remove, size, clear oraz get_name.
// HashTableBase powinna byc abstrakcyjna klasa bazowa z wirtualnymi funkcjami.
// AVLHashTable powinna implementowac lancuchowanie z wykorzystaniem drzew AVL do rozwiazywania kolizji.

class PerformanceTester {
private:
    // Ten tester bedzie generowal klucze/wartosci dla konkretnego przebiegu testu.
    // Jest tworzony dla kazdego testu, wiec zestaw danych jest swiezy.

public:
    // Ta metoda przyjmuje teraz parametry dla przebiegu testu
    void run_tests(
        const std::vector<int>& sizes, // Rozmiary tabel do testowania
        int num_data_sets, // Liczba zestawow danych dla kazdego rozmiaru
        int repetitions, // Liczba powtorzen dla kazdego zestawu danych
        const std::string& output_filename = "wyniki.xlsx" // Nazwa pliku wyjsciowego
    ) {
        std::cout << "\n=== STARTING PERFORMANCE TESTS ===" << std::endl;
        auto full_time_start = std::chrono::high_resolution_clock::now(); // Czas rozpoczecia calego testu

        std::ofstream outFile(output_filename); // Otworz plik do zapisu wynikow
        // Zaktualizowany naglowek pliku wyjsciowego, bez kolumn wyszukiwania
        outFile << "Rozmiar\tAdresowanie otwarte Wstawianie (ns)\tLancuchowanie Wstawianie (ns)\tAVL Wstawianie (ns)\t"
            << "Adresowanie otwarte Usuwanie (ns)\tLancuchowanie Usuwanie (ns)\tAVL Usuwanie (ns)\n";

        for (int size : sizes) { // Petla po roznych rozmiarach tabel
            std::cout << "Testing for size: " << size << std::endl;

            // Zmienne do akumulowania srednich czasow
            double avg_open_insert = 0;
            double avg_chaining_insert = 0;
            double avg_avl_insert = 0;
            double avg_open_remove = 0;
            double avg_chaining_remove = 0;
            double avg_avl_remove = 0;
            // Usunieto deklaracje zmiennych dla czasow wyszukiwania

            for (int data_set_idx = 0; data_set_idx < num_data_sets; ++data_set_idx) { // Petla po zestawach danych
                std::cout << "  Data Set " << data_set_idx + 1 << " of " << num_data_sets << std::endl;

                std::random_device rd; // Uzyj sprzetowego generatora liczb losowych
                std::mt19937 gen(rd()); // Mersenne Twister jako generator
                std::uniform_int_distribution<> dis_keys(1, size * 10); // Dystrybucja dla kluczy
                std::uniform_int_distribution<> dis_values(1, 1000); // Dystrybucja dla wartosci

                std::vector<int> keys(size); // Wektor na klucze
                std::vector<int> values(size); // Wektor na wartosci

                for (int i = 0; i < size; ++i) { // Generuj klucze i wartosci
                    keys[i] = dis_keys(gen);
                    values[i] = dis_values(gen);
                }

                for (int rep_idx = 0; rep_idx < repetitions; ++rep_idx) { // Petla po powtorzeniach
                    // Utworz nowe instancje dla kazdego powtorzenia, aby zapewnic czysty stan
                    // Uzyj unikalnego seeda dla kazdego powtorzenia, aby zapewnic rozne dane dla kazdego przebiegu
                    std::mt19937 rep_gen(rd() + rep_idx);
                    std::uniform_int_distribution<> rep_dis_keys(1, size * 10);

                    // Ponownie wygeneruj klucze dla kazdego powtorzenia, aby uniknac efektow buforowania z poprzednich przebiegow
                    std::vector<int> current_keys(size);
                    for (int i = 0; i < size; ++i) {
                        current_keys[i] = rep_dis_keys(rep_gen);
                    }

                    ChainingHashTable chaining_ht(size); // Inicjalizuj tabele z lancuchowaniem (pojemnosc)
                    OpenAddressingHashTable open_ht(size); // Inicjalizuj tabele z adresowaniem otwartym
                    AVLHashTable avl_ht(size); // Inicjalizuj tabele z drzewami AVL

                    // --- TESTY WSTAWIANIA ---
                    auto start_time = std::chrono::high_resolution_clock::now(); // Czas rozpoczecia
                    for (int key : current_keys) {
                        chaining_ht.insert(key, 0); // Wartosc nie ma znaczenia dla pomiaru czasu
                    }
                    auto end_time = std::chrono::high_resolution_clock::now(); // Czas zakonczenia
                    avg_chaining_insert += std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count() / (double)size; // Dodaj czas do sumy

                    start_time = std::chrono::high_resolution_clock::now();
                    for (int key : current_keys) {
                        open_ht.insert(key, 0);
                    }
                    end_time = std::chrono::high_resolution_clock::now();
                    avg_open_insert += std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count() / (double)size;

                    start_time = std::chrono::high_resolution_clock::now();
                    for (int key : current_keys) {
                        avl_ht.insert(key, 0);
                    }
                    end_time = std::chrono::high_resolution_clock::now();
                    avg_avl_insert += std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count() / (double)size;


                    // --- TESTY USUWANIA ---
                    // Utworz kopie kluczy do usuniecia, aby nie zaklocac danych wstawiania
                    std::vector<int> keys_to_remove = current_keys;
                    std::shuffle(keys_to_remove.begin(), keys_to_remove.end(), rep_gen); // Tasuj dla losowej kolejnosci usuwania

                    start_time = std::chrono::high_resolution_clock::now();
                    for (size_t i = 0; i < size / 2; ++i) { // Usun polowe elementow
                        chaining_ht.remove(keys_to_remove[i]);
                    }
                    end_time = std::chrono::high_resolution_clock::now();
                    avg_chaining_remove += std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count() / (double)(size / 2);

                    start_time = std::chrono::high_resolution_clock::now();
                    for (size_t i = 0; i < size / 2; ++i) {
                        open_ht.remove(keys_to_remove[i]);
                    }
                    end_time = std::chrono::high_resolution_clock::now();
                    avg_open_remove += std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count() / (double)(size / 2);

                    start_time = std::chrono::high_resolution_clock::now();
                    for (size_t i = 0; i < size / 2; ++i) {
                        avl_ht.remove(keys_to_remove[i]);
                    }
                    end_time = std::chrono::high_resolution_clock::now();
                    avg_avl_remove += std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time).count() / (double)(size / 2);
                }
            }

            // Oblicz ogolne srednie
            double divisor = (double)num_data_sets * repetitions;
            avg_open_insert /= divisor;
            avg_chaining_insert /= divisor;
            avg_avl_insert /= divisor;
            avg_open_remove /= divisor;
            avg_chaining_remove /= divisor;
            avg_avl_remove /= divisor;
            // Usunieto obliczenia dla srednich czasow wyszukiwania

            // Zapisz wyniki do pliku
            // Zaktualizowany zapis wynikow, bez kolumn wyszukiwania
            outFile << size << "\t"
                << avg_open_insert << "\t"
                << avg_chaining_insert << "\t"
                << avg_avl_insert << "\t"
                << avg_open_remove << "\t"
                << avg_chaining_remove << "\t"
                << avg_avl_remove << "\n";

            // Wyswietl wyniki w konsoli
            std::cout << "  Results for size " << size << ":" << std::endl;
            std::cout << std::fixed << std::setprecision(2); // Formatuj wyjscie do 2 miejsc po przecinku
            std::cout << "    Open Addressing Insert: " << avg_open_insert << " ns" << std::endl;
            std::cout << "    Chaining Insert:        " << avg_chaining_insert << " ns" << std::endl;
            std::cout << "    AVL Insert:             " << avg_avl_insert << " ns" << std::endl;
            // Usunieto wyswietlanie wynikow wyszukiwania w konsoli
            std::cout << "    Open Addressing Remove: " << avg_open_remove << " ns" << std::endl;
            std::cout << "    Chaining Remove:        " << avg_chaining_remove << " ns" << std::endl;
            std::cout << "    AVL Remove:             " << avg_avl_remove << " ns" << std::endl;
        }

        outFile.close(); // Zamknij plik

        auto full_time_end = std::chrono::high_resolution_clock::now(); // Czas zakonczenia calego testu
        auto full_time_duration = std::chrono::duration_cast<std::chrono::minutes>(full_time_end - full_time_start).count(); // Czas trwania w minutach
        std::cout << "\nTotal measurement time: " << full_time_duration << " minutes" << std::endl;
        std::cout << "=== PERFORMANCE TESTS COMPLETE ===" << std::endl;
    }
};

void demonstration() {
    std::cout << "=== DEMONSTRATION OF HASH TABLE OPERATIONS ===" << std::endl;

    // Przykładowe dane do demonstracji
    std::vector<std::pair<int, int>> sample_data = {
        {10, 100}, {22, 220}, {31, 310}, {4, 40}, {15, 150},
        {28, 280}, {17, 170}, {88, 880}, {59, 590}
    };

    // Testuj kazda implementacje
    // Inicjalizuj z rozsadna mala pojemnoscia dla demonstracji
    std::vector<std::unique_ptr<HashTableBase>> tables;
    tables.push_back(std::make_unique<ChainingHashTable>(8)); // Tabela z lancuchowaniem
    tables.push_back(std::make_unique<OpenAddressingHashTable>(8)); // Tabela z adresowaniem otwartym
    tables.push_back(std::make_unique<AVLHashTable>(8)); // Tabela z drzewami AVL

    for (auto& table : tables) { // Petla po kazdej tabeli hashujacej
        // Wyczysc poprzednie dane jesli istnieja (dla bezpieczenstwa, choc unique_ptr zapewnia swiezy start)
        table->clear();

        // Wstaw dane
        std::cout << "\n--- Inserting data into " << table->get_name() << " ---" << std::endl;
        for (const auto& pair : sample_data) {
            table->insert(pair.first, pair.second);
        }

        // Wyswietl strukture
        table->display();

        // Test wyszukiwania (ta czesc pozostaje w demonstracji, poniewaz jest przydatna do pokazania funkcjonalnosci)
        int value;
        std::cout << "\nTesting search operations:" << std::endl;
        for (int key : {10, 22, 99, 4}) { // Dodaj 4, aby przetestowac istniejacy klucz
            if (table->find(key, value)) {
                std::cout << "Key " << key << " -> value " << value << std::endl;
            }
            else {
                std::cout << "Key " << key << " not found" << std::endl;
            }
        }

        // Test usuwania
        std::cout << "\nRemoving keys 22 and 31..." << std::endl;
        if (table->remove(22)) {
            std::cout << "Key 22 removed successfully." << std::endl;
        }
        else {
            std::cout << "Key 22 not found for removal." << std::endl;
        }
        if (table->remove(31)) {
            std::cout << "Key 31 removed successfully." << std::endl;
        }
        else {
            std::cout << "Key 31 not found for removal." << std::endl;
        }
        if (table->remove(100)) { // Test usuniecia nieistniejacego klucza
            std::cout << "Key 100 removed successfully." << std::endl;
        }
        else {
            std::cout << "Key 100 not found for removal (expected)." << std::endl;
        }


        std::cout << "Size after removals: " << table->size() << std::endl; // Wyswietl rozmiar po usunieciach
        std::cout << "\n" << std::string(60, '=') << std::endl;

        table->clear(); // Wyczysc dla nastepnej tabeli
    }
}

// Glowne menu do interakcji z uzytkownikiem
void mainMenu() {
    int choice;
    bool exit_program = false;

    // Definiuj parametry testow wydajnosci
    const std::vector<int> test_sizes = { 10000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000 };
    const int num_data_sets = 10;
    const int repetitions_per_data_set = 100; // Zmieniono nazwe z 'rep' dla jasnosci

    while (!exit_program) {
        std::cout << "\n=== MAIN MENU ===" << std::endl;
        std::cout << "1. Run Performance Benchmarks (Insert and Remove)" << std::endl; // Zaktualizowany opis
        std::cout << "2. Show Demonstration of Hash Table Operations" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "Choose an option: ";
        std::cin >> choice;

        switch (choice) {
        case 1: {
            PerformanceTester tester; // Utworz instancje testera
            tester.run_tests(test_sizes, num_data_sets, repetitions_per_data_set, "performance_results.xlsx");
            break;
        }
        case 2:
            demonstration(); // Wywolaj demonstracje
            break;
        case 0:
            exit_program = true; // Ustaw flage wyjscia
            break;
        default:
            std::cout << "Invalid option. Please try again." << std::endl; // Nieprawidlowa opcja
            break;
        }
        // Wyczysc bufor wejsciowy w przypadku wejscia nienumerycznego
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

int main() {
    std::cout << "PROJECT: DICTIONARY IMPLEMENTATIONS BASED ON HASH TABLES" << std::endl;
    std::cout << "Implementations: Chaining, Open Addressing, Chaining with AVL Trees" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    mainMenu(); // Wywolaj glowne menu

    std::cout << "\nThank you for using the program. Exiting." << std::endl; // Komunikat pozegnalny
    return 0;
}
