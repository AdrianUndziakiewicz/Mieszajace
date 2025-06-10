#ifndef HASH_TABLE_BASE_H
#define HASH_TABLE_BASE_H

#include <iostream>   // Do operacji wejscia/wyjscia (np. std::cout)
#include <vector>     // Do uzycia dynamicznych tablic (std::vector) - ogolne zastosowanie
#include <functional> // Do uzycia std::hash

// Abstrakcyjna klasa bazowa dla wszystkich implementacji tabeli hashujacej
class HashTableBase {
public:
    // Wirtualny destruktor domyslny. Wymagany dla klas bazowych, aby zapewnic poprawne
    // zwolnienie pamieci dla obiektow klas pochodnych, gdy sa usuwane poprzez wskaznik
    // lub referencje do klasy bazowej.
    virtual ~HashTableBase() = default;

    // Czysto wirtualna metoda, ktora musi byc zaimplementowana przez klasy pochodne.
    // Zwraca nazwe konkretnej implementacji tabeli hashujacej (np. "Chaining Hash Table").
    virtual std::string get_name() const = 0;

    // Czysto wirtualna metoda do wstawiania pary klucz-wartosc do tabeli hashujacej.
    // Zwraca 'true', jesli wstawienie (lub aktualizacja) powiodlo sie, 'false' w przeciwnym razie.
    virtual bool insert(int key, int value) = 0;

    // Czysto wirtualna metoda do usuwania elementu o podanym kluczu z tabeli.
    // Zwraca 'true', jesli element zostal usuniety, 'false' jesli nie znaleziono klucza.
    virtual bool remove(int key) = 0;

    // Czysto wirtualna metoda do wyszukiwania wartosci skojarzonej z podanym kluczem.
    // Jesli klucz zostanie znaleziony, wartosc zostanie przypisana do referencji 'value'.
    // Zwraca 'true', jesli klucz zostal znaleziony, 'false' w przeciwnym razie.
    virtual bool find(int key, int& value) = 0;

    // Czysto wirtualna metoda do wyswietlania zawartosci tabeli hashujacej.
    virtual void display() = 0;

    // Czysto wirtualna metoda zwracajaca aktualna liczbe elementow w tabeli.
    virtual size_t size() const = 0;

    // Czysto wirtualna metoda do czyszczenia (usuwania wszystkich elementow) tabeli.
    virtual void clear() = 0;

protected:
    // Funkcja hashujaca, ktora przelicza klucz na indeks w tabeli.
    // Uzywa standardowej funkcji hash dla int i operatora modulo, aby
    // dostosowac wynik do rozmiaru tabeli.
    size_t hash_function(int key, size_t table_size) const {
        return std::hash<int>{}(key) % table_size; // std::hash generuje hash, a modulo ogranicza do rozmiaru tabeli
    }
};

#endif // HASH_TABLE_BASE_H
