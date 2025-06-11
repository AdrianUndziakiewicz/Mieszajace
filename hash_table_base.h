#ifndef HASH_TABLE_BASE_H
#define HASH_TABLE_BASE_H

#include <iostream>   // Do operacji wejscia/wyjscia (np. std::cout)
#include <vector>     // Do uzycia dynamicznych tablic (std::vector)


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
    virtual void void clear() = 0; // Poprawka: powinno byc void, a nie void void


protected:
    
    // Algorytm:
    // 1. Bierze bezwzgledna wartosc klucza (aby obsluzyc klucze ujemne).
    // 2. Mnozy klucz przez duza liczbe pierwsza (np. 2654435761ULL - popularna wartosc).
    // 3. Wykonuje operacje XOR z przesunieciem bitowym (pomaga rozproszyc bity).
    // 4. Wynik rzutuje na size_t, a nastepnie wykonuje operacje modulo przez rozmiar tabeli.
    size_t hash_function(int key, size_t table_size) const {
        // Upewnij sie, ze klucz jest dodatni, aby uniknac problemow z modulo dla ujemnych liczb
        unsigned int ukey = static_cast<unsigned int>(key); // Uzyj unsigned int dla operacji bitowych

        // Popularna heurystyka haszujaca (np. z algorytmu Boba Jenkinsa, FNV, itp.)
        // Ta konkretna jest prosta, ale skuteczniejsza niz samo modulo.
        ukey = ((ukey >> 16) ^ ukey) * 0x45d9f3b; // Mnozenie i XOR z przesunieciem
        ukey = ((ukey >> 16) ^ ukey) * 0x45d9f3b; // Powtorzenie dla lepszego rozproszenia
        ukey = (ukey >> 16) ^ ukey;             // Koncowy XOR

        // Zawsze zwroc wynik modulo table_size, aby dopasowac do zakresu tablicy
        return static_cast<size_t>(ukey) % table_size;
    }
};

#endif // HASH_TABLE_BASE_H
