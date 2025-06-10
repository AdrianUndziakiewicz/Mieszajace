#ifndef OPEN_ADDRESSING_HASH_TABLE_H
#define OPEN_ADDRESSING_HASH_TABLE_H

#include "hash_table_base.h" // Dolacza bazowa klase dla tabeli hashujacej


class OpenAddressingHashTable : public HashTableBase {
private:
    // Enumerator do oznaczania stanu miejsca w tabeli:
    // EMPTY: puste miejsce, nigdy nie bylo uzywane lub zostalo wyczyszczone.
    // OCCUPIED: miejsce zajete przez wazny element.
    // DELETED: miejsce zajete przez element, ktory zostal usuniety.
    //          Wazne dla probkowania liniowego, aby kontynuowac wyszukiwanie.
    enum class EntryState { EMPTY, OCCUPIED, DELETED };

    // Struktura reprezentujaca pojedynczy wpis w tabeli hashujacej.
    struct Entry {
        int key; // Klucz elementu
        int value; // Wartosc elementu
        EntryState state; // Stan tego wpisu

        Entry() : key(0), value(0), state(EntryState::EMPTY) {} // Konstruktor domyslny
        Entry(int k, int v) : key(k), value(v), state(EntryState::OCCUPIED) {} // Konstruktor z kluczem i wartoscia
    };

    std::vector<Entry> table; // Glowna tabela przechowujaca wpisy
    size_t table_size; // Aktualny rozmiar (pojemnosc) tabeli
    size_t current_size; // Liczba aktualnie przechowywanych elementow (nie wlaczajac DELETED)

    // Maksymalny wspolczynnik wypelnienia, po przekroczeniu ktorego tabela zostanie powiekszona.
    // Zazwyczaj niski dla adresowania otwartego, aby uniknac klastrowania.
    static constexpr double MAX_LOAD_FACTOR = 0.5;

    // Metoda do zmiany rozmiaru tabeli (podwajania jej pojemnosci).
    void resize() {
        size_t old_size = table_size; // Zapisz stary rozmiar
        auto old_table = std::move(table); // Przenies stara tabele (optymalizacja)

        table_size *= 2; // Podwoj rozmiar tabeli
        table.clear(); // Wyczysc biezaca (nowa) tabele
        table.resize(table_size); // Zmien rozmiar nowej tabeli
        current_size = 0; // Zresetuj licznik elementow

        // Przepisz wszystkie elementy ze starej tabeli do nowej.
        // Nalezy je ponownie wstawic, aby obliczyc nowe pozycje hash.
        for (const auto& entry : old_table) {
            if (entry.state == EntryState::OCCUPIED) {
                insert(entry.key, entry.value); // Uzyj metody insert do ponownego wstawienia
            }
        }
    }

    // Metoda probkujaca (probing) do znalezienia odpowiedniego indeksu dla klucza.
    // Uzywa probkowania liniowego.
    size_t probe(int key) const {
        size_t index = hash_function(key, table_size); // Oblicz poczatkowy indeks za pomoca funkcji hashujacej
        size_t original_index = index; // Zapisz poczatkowy indeks do wykrywania pelnej tabeli

        // Szukaj wolnego miejsca lub klucza:
        // Kontynuuj, dopoki nie znajdziesz pustego miejsca (EMPTY)
        // LUB (jesli miejsce nie jest puste):
        //    stan to DELETED (kontynuuj szukanie)
        //    LUB klucz w miejscu nie odpowiada szukanemu kluczowi
        while (table[index].state != EntryState::EMPTY &&
            (table[index].state == EntryState::DELETED || table[index].key != key)) {
            index = (index + 1) % table_size; // Przejdz do nastepnego miejsca (probkowanie liniowe)
            if (index == original_index) break; // Jesli wrocilismy do punktu poczatkowego, tabela jest pelna
        }

        return index; // Zwroc znaleziony indeks
    }

public:
    // Konstruktor, inicjalizuje tabele z podanym rozmiarem poczatkowym.
    explicit OpenAddressingHashTable(size_t initial_size = 16)
        : table_size(initial_size), current_size(0) {
        table.resize(table_size); // Zmien rozmiar wektora na poczatkowa pojemnosc
    }

    // Wstawia pare klucz-wartosc do tabeli.
    // Zwraca true, jesli wstawienie/aktualizacja sie powiodla, false w przeciwnym razie.
    bool insert(int key, int value) override {
        // Sprawdz wspolczynnik wypelnienia, jesli przekroczony, zmien rozmiar tabeli.
        if (static_cast<double>(current_size) / table_size > MAX_LOAD_FACTOR) {
            resize();
        }

        size_t index = probe(key); // Znajdz odpowiedni indeks dla klucza

        // Jesli znaleziono zajete miejsce z tym samym kluczem, zaktualizuj wartosc.
        if (table[index].state == EntryState::OCCUPIED && table[index].key == key) {
            table[index].value = value; // Aktualizuj wartosc
            return true;
        }

        // Jesli miejsce jest puste lub oznaczone jako usuniete, wstaw nowy element.
        if (table[index].state != EntryState::OCCUPIED) {
            table[index] = Entry(key, value); // Utworz nowy wpis
            current_size++; // Zwieksz licznik elementow
            return true;
        }

        return false; // Tabela jest pelna (nie mozna wstawic, mimo probkowania)
    }

    // Usuwa element z podanym kluczem z tabeli.
    // Zwraca true, jesli element zostal usuniety, false w przeciwnym razie.
    bool remove(int key) override {
        size_t index = probe(key); // Znajdz indeks klucza

        // Jesli znaleziono zajete miejsce z tym samym kluczem, oznacz jako usuniety.
        if (table[index].state == EntryState::OCCUPIED && table[index].key == key) {
            table[index].state = EntryState::DELETED; // Oznacz jako usuniety (tzw. lazy deletion)
            current_size--; // Zmniejsz licznik elementow
            return true;
        }

        return false; // Element nie znaleziony
    }

    // Znajduje wartosc skojarzona z podanym kluczem.
    // Zwraca true, jesli klucz zostal znaleziony, a wartosc jest przypisana do 'value', false w przeciwnym razie.
    bool find(int key, int& value) override {
        size_t index = probe(key); // Znajdz indeks klucza

        // Jesli znaleziono zajete miejsce z tym samym kluczem, przypisz wartosc.
        if (table[index].state == EntryState::OCCUPIED && table[index].key == key) {
            value = table[index].value; // Przypisz znaleziona wartosc
            return true;
        }

        return false; // Klucz nie znaleziony
    }

    // Wyswietla zawartosc tabeli hashujacej.
    void display() override {
        std::cout << "=== Open Addressing Hash Table ===" << std::endl;
        for (size_t i = 0; i < table_size; ++i) {
            std::cout << "Index " << i << ": ";
            if (table[i].state == EntryState::OCCUPIED) {
                std::cout << "(" << table[i].key << "," << table[i].value << ")";
            }
            else if (table[i].state == EntryState::DELETED) {
                std::cout << "[DELETED]";
            }
            else {
                std::cout << "[EMPTY]";
            }
            std::cout << std::endl;
        }
        std::cout << "Size: " << current_size << "/" << table_size << std::endl;
    }

    // Zwraca aktualna liczbe elementow w tabeli.
    size_t size() const override { return current_size; }

    // Czyści tabele, ustawiajac wszystkie wpisy na EMPTY.
    void clear() override {
        for (auto& entry : table) {
            entry.state = EntryState::EMPTY; // Ustaw stan na pusty
        }
        current_size = 0; // Zresetuj licznik elementow
    }

    // Zwraca nazwe implementacji tabeli hashujacej.
    // Uwaga: Tutaj powinno byc "Open Addressing Hash Table", a nie "AVL Hash Table".
    std::string get_name() const override {
        return "Open Addressing Hash Table";
    }
};

#endif // OPEN_ADDRESSING_HASH_TABLE_H
