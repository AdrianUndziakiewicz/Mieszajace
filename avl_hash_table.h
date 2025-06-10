#ifndef AVL_HASH_TABLE_H
#define AVL_HASH_TABLE_H

#include "hash_table_base.h" // Dolacza bazowa klase dla tabeli hashujacej
#include <algorithm> // Wymagane dla std::max, uzywanego do obliczania wysokosci wezlow AVL

// Implementacja 3: Hash Table z kubelkami zawierajacymi drzewa AVL
// W tej implementacji, kazdy 'kubelek' (bucket) tabeli hashujacej
// zamiast listy do rozwiazywania kolizji, uzywa zbalansowanego drzewa binarnego (AVL tree).
class AVLHashTable : public HashTableBase {
private:
    // Struktura reprezentujaca pojedynczy wezel w drzewie AVL.
    struct AVLNode {
        int key;    // Klucz elementu
        int value;  // Wartosc elementu
        int height; // Wysokosc wezla (maksymalna dlugosc sciezki od tego wezla do liscia)
        AVLNode* left; // Wskaznik do lewego dziecka
        AVLNode* right; // Wskaznik do prawego dziecka

        // Konstruktor wezla AVL. Poczatkowo wysokosc to 1 (samotny wezel).
        AVLNode(int k, int v) : key(k), value(v), height(1), left(nullptr), right(nullptr) {}
    };

    std::vector<AVLNode*> table; // Glowna tabela - wektor wskaźników do korzeni drzew AVL
    size_t table_size;           // Aktualny rozmiar (pojemnosc) wektora tabeli
    size_t current_size;         // Liczba aktualnie przechowywanych elementow w calej tabeli (sumarycznie ze wszystkich drzew AVL)

    // Maksymalny wspolczynnik wypelnienia. W przypadku drzew AVL, moze byc wyzszy niz
    // w adresowaniu otwartym lub lancuchowaniu z listami, poniewaz operacje w drzewach
    // sa logarytmiczne, co zmniejsza wplyw dlugosci lancucha.
    static constexpr double MAX_LOAD_FACTOR = 1.0; // Czesto moze byc 1.0 lub wiecej

    // --- Funkcje pomocnicze dla drzewa AVL ---

    // Zwraca wysokosc wezla; 0 jesli wezel jest nullptr.
    int get_height(AVLNode* node) {
        return node ? node->height : 0;
    }

    // Oblicza wspolczynnik balansu wezla.
    // > 1 oznacza, ze lewe poddrzewo jest za wysokie.
    // < -1 oznacza, ze prawe poddrzewo jest za wysokie.
    int get_balance(AVLNode* node) {
        return node ? get_height(node->left) - get_height(node->right) : 0;
    }

    // Aktualizuje wysokosc wezla na podstawie wysokosci jego dzieci.
    void update_height(AVLNode* node) {
        if (node) {
            node->height = 1 + std::max(get_height(node->left), get_height(node->right));
        }
    }

    // Wykonuje rotacje w prawo na wezle 'y'. Zwraca nowy korzen poddrzewa.
    // Przed:      y           Po:       x
    //            / \                   / \
    //           x   T3                T1  y
    //          / \                       / \
    //         T1  T2                    T2  T3
    AVLNode* rotate_right(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;

        // Wykonaj rotacje
        x->right = y;
        y->left = T2;

        // Zaktualizuj wysokosci wezlow 'y' i 'x' (kolejnosc wazna!)
        update_height(y);
        update_height(x);

        return x; // Zwraca nowy korzen
    }

    // Wykonuje rotacje w lewo na wezle 'x'. Zwraca nowy korzen poddrzewa.
    // Przed:      x           Po:       y
    //            / \                   / \
    //           T1  y                 x   T3
    //              / \               / \
    //             T2  T3            T1  T2
    AVLNode* rotate_left(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;

        // Wykonaj rotacje
        y->left = x;
        x->right = T2;

        // Zaktualizuj wysokosci wezlow 'x' i 'y' (kolejnosc wazna!)
        update_height(x);
        update_height(y);

        return y; // Zwraca nowy korzen
    }

    // Rekurencyjna funkcja wstawiajaca element do drzewa AVL.
    // Zwraca korzen (potencjalnie nowy) poddrzewa.
    // 'inserted' to flaga przekazywana przez referencje, informujaca czy wstawiono nowy element,
    // czy tylko zaktualizowano istniejacy.
    AVLNode* insert_avl(AVLNode* node, int key, int value, bool& inserted) {
        // Standardowe wstawianie BST: jesli dotarlismy do nullptr, tworzymy nowy wezel.
        if (!node) {
            inserted = true; // Oznacz jako wstawiony nowy element
            return new AVLNode(key, value);
        }

        // Przejdz do lewego lub prawego poddrzewa
        if (key < node->key) {
            node->left = insert_avl(node->left, key, value, inserted);
        }
        else if (key > node->key) {
            node->right = insert_avl(node->right, key, value, inserted);
        }
        else {
            // Klucz juz istnieje - aktualizuj wartosc i oznacz jako nie wstawiony nowy element.
            node->value = value;
            inserted = false;
            return node; // Zwracamy niezmieniony wezel
        }

        // Po rekurencyjnym wywolaniu, aktualizuj wysokosc bieżącego wezla.
        update_height(node);

        // Sprawdz wspolczynnik balansu i wykonaj odpowiednie rotacje, jesli drzewo jest niezbalansowane.
        int balance = get_balance(node);

        // Cztery przypadki niezbalansowania AVL:

        // 1. Lewa-lewa (Left-Left Case)
        // Drzewo jest "przechylone" w lewo, a nowy element jest w lewym poddrzewie lewego dziecka.
        if (balance > 1 && key < node->left->key) {
            return rotate_right(node);
        }

        // 2. Prawa-prawa (Right-Right Case)
        // Drzewo jest "przechylone" w prawo, a nowy element jest w prawym poddrzewie prawego dziecka.
        if (balance < -1 && key > node->right->key) {
            return rotate_left(node);
        }

        // 3. Lewa-prawa (Left-Right Case)
        // Drzewo jest "przechylone" w lewo, ale nowy element jest w prawym poddrzewie lewego dziecka.
        // Wymaga dwoch rotacji: lewo na dziecku, potem prawo na wezle.
        if (balance > 1 && key > node->left->key) {
            node->left = rotate_left(node->left); // Rotacja w lewo na lewym dziecku
            return rotate_right(node);             // Rotacja w prawo na bieżącym wezle
        }

        // 4. Prawa-lewa (Right-Left Case)
        // Drzewo jest "przechylone" w prawo, ale nowy element jest w lewym poddrzewie prawego dziecka.
        // Wymaga dwoch rotacji: prawo na dziecku, potem lewo na wezle.
        if (balance < -1 && key < node->right->key) {
            node->right = rotate_right(node->right); // Rotacja w prawo na prawym dziecku
            return rotate_left(node);                  // Rotacja w lewo na bieżącym wezle
        }

        return node; // Zwroc niezmieniony wezel, jesli jest zbalansowany
    }

    // Znajduje wezel z najmniejszym kluczem w danym poddrzewie (najbardziej na lewo).
    AVLNode* find_min(AVLNode* node) {
        while (node->left) {
            node = node->left;
        }
        return node;
    }

    // Rekurencyjna funkcja usuwajaca element z drzewa AVL.
    // Zwraca korzen (potencjalnie nowy) poddrzewa.
    // 'removed' to flaga przekazywana przez referencje, informujaca czy element zostal usuniety.
    AVLNode* remove_avl(AVLNode* node, int key, bool& removed) {
        if (!node) {
            removed = false; // Element nie znaleziony
            return node;
        }

        // Standardowe usuwanie BST:
        if (key < node->key) {
            node->left = remove_avl(node->left, key, removed);
        }
        else if (key > node->key) {
            node->right = remove_avl(node->right, key, removed);
        }
        else { // Znaleziono wezel do usuniecia (key == node->key)
            removed = true; // Oznacz, ze element zostal znaleziony i bedzie usuniety

            // Przypadek 1: Wezel z jednym dzieckiem lub bez dzieci
            if (!node->left || !node->right) {
                AVLNode* temp = node->left ? node->left : node->right;

                if (!temp) { // Brak dzieci (wezel jest lisciem)
                    temp = node; // temp wskazuje na wezel do usuniecia
                    node = nullptr; // Ustaw wezel na nullptr
                }
                else { // Ma jedno dziecko
                    *node = *temp; // Skopiuj dane dziecka do bieżącego wezla
                }
                delete temp; // Zwolnij pamiec starego wezla lub wezla-dziecka
            }
            else { // Przypadek 2: Wezel z dwoma dziecmi
                // Znajdz nastepnika (najmniejszy element w prawym poddrzewie)
                AVLNode* temp = find_min(node->right);
                // Skopiuj dane nastepnika do bieżącego wezla
                node->key = temp->key;
                node->value = temp->value;

                // Rekurencyjnie usun nastepnika z prawego poddrzewa
                bool dummy_removed_flag; // Flaga tymczasowa, bo wiemy, ze element zostanie usuniety
                node->right = remove_avl(node->right, temp->key, dummy_removed_flag);
            }
        }

        // Jesli wezel stal sie nullptr (np. usunieto lisc), nie ma co balansowac.
        if (!node) return node;

        // Po rekurencyjnym wywolaniu, aktualizuj wysokosc bieżącego wezla.
        update_height(node);

        // Sprawdz wspolczynnik balansu i wykonaj rotacje w celu zbalansowania.
        int balance = get_balance(node);

        // Rotacje po usunieciu (podobne do wstawiania, ale warunki balansu sa inne):

        // Lewa-lewa (Left-Left Case)
        // Drzewo jest niezbalansowane w lewo, a lewe dziecko jest zbalansowane lub przechylone w lewo.
        if (balance > 1 && get_balance(node->left) >= 0) {
            return rotate_right(node);
        }

        // Lewa-prawa (Left-Right Case)
        // Drzewo jest niezbalansowane w lewo, a lewe dziecko jest przechylone w prawo.
        if (balance > 1 && get_balance(node->left) < 0) {
            node->left = rotate_left(node->left); // Rotacja w lewo na lewym dziecku
            return rotate_right(node);             // Rotacja w prawo na bieżącym wezle
        }

        // Prawa-prawa (Right-Right Case)
        // Drzewo jest niezbalansowane w prawo, a prawe dziecko jest zbalansowane lub przechylone w prawo.
        if (balance < -1 && get_balance(node->right) <= 0) {
            return rotate_left(node);
        }

        // Prawa-lewa (Right-Left Case)
        // Drzewo jest niezbalansowane w prawo, a prawe dziecko jest przechylone w lewo.
        if (balance < -1 && get_balance(node->right) > 0) {
            node->right = rotate_right(node->right); // Rotacja w prawo na prawym dziecku
            return rotate_left(node);                  // Rotacja w lewo na bieżącym wezle
        }

        return node; // Zwroc zbalansowany wezel
    }

    // Rekurencyjna funkcja wyszukujaca element w drzewie AVL.
    bool find_avl(AVLNode* node, int key, int& value) {
        if (!node) return false; // Nie znaleziono elementu

        if (key == node->key) { // Znaleziono element
            value = node->value;
            return true;
        }
        else if (key < node->key) { // Szukaj w lewym poddrzewie
            return find_avl(node->left, key, value);
        }
        else { // Szukaj w prawym poddrzewie
            return find_avl(node->right, key, value);
        }
    }

    // Rekurencyjnie usuwa wszystkie wezly w drzewie (zwolnienie pamieci).
    void clear_avl(AVLNode* node) {
        if (node) {
            clear_avl(node->left);  // Najpierw lewe poddrzewo
            clear_avl(node->right); // Potem prawe poddrzewo
            delete node;            // Na koncu bieżący wezel
        }
    }

    // Rekurencyjna funkcja do wyswietlania drzewa AVL (inorder traversal, z wcieciami).
    // Uzywane glownie do debugowania.
    void display_avl(AVLNode* node, int depth = 0) {
        if (node) {
            display_avl(node->right, depth + 1); // Najpierw prawe dziecko (dla czytelniejszego widoku "drzewa")
            for (int i = 0; i < depth; ++i) std::cout << "  "; // Wciecia dla poziomu zagniezdzenia
            std::cout << "(" << node->key << "," << node->value << ")" << std::endl;
            display_avl(node->left, depth + 1); // Potem lewe dziecko
        }
    }

    // Zmienia rozmiar tabeli hashujacej, podwajajac jej pojemnosc.
    // Wymaga ponownego wstawienia wszystkich elementow, poniewaz ich indeksy hash moga sie zmienic.
    void resize() {
        size_t old_size = table_size; // Zapisz stary rozmiar
        auto old_table = std::move(table); // Przenies stara tabele (wektor korzeni AVL)

        table_size *= 2; // Podwoj rozmiar tabeli
        table.clear();   // Wyczysc nowa tabele
        table.resize(table_size, nullptr); // Zmien rozmiar wektora, inicjujac wskaźniki na nullptr
        current_size = 0; // Zresetuj licznik elementow

        // Przepisz wszystkie elementy ze starej tabeli do nowej.
        // Nalezy przejsc przez kazde drzewo AVL w starej tabeli, zebrac jego elementy,
        // a nastepnie wstawic je ponownie do nowej tabeli.
        for (AVLNode* root : old_table) {
            if (root) {
                collect_and_reinsert(root); // Zbierz elementy z drzewa i wstaw je
                clear_avl(root); // Zwolnij pamiec starego drzewa po przepisaniu elementow
            }
        }
    }

    // Pomocnicza funkcja rekurencyjna do zbierania elementow z drzewa AVL
    // i wstawiania ich do nowej (lub bieżącej) tabeli hashujacej podczas resize'u.
    void collect_and_reinsert(AVLNode* node) {
        if (node) {
            insert(node->key, node->value); // Wstaw element do nowej tabeli
            collect_and_reinsert(node->left);  // Rekurencyjnie dla lewego dziecka
            collect_and_reinsert(node->right); // Rekurencyjnie dla prawego dziecka
        }
    }

public:
    // Konstruktor, inicjalizuje tabele z podanym rozmiarem poczatkowym.
    // Kazdy element wektora jest inicjalizowany na nullptr (pusty kubel).
    explicit AVLHashTable(size_t initial_size = 16)
        : table_size(initial_size), current_size(0) {
        table.resize(table_size, nullptr); // Ustaw poczatkowy rozmiar wektora wskaźników
    }

    // Destruktor. Zapewnia zwolnienie calej zaalokowanej pamieci dynamicznej
    // dla wezlow AVL, wywolujac metode clear().
    ~AVLHashTable() {
        clear();
    }

    // Wstawia pare klucz-wartosc do tabeli.
    // Zwraca true, jesli wstawienie/aktualizacja sie powiodla.
    bool insert(int key, int value) override {
        // Sprawdz wspolczynnik wypelnienia. Jesli przekroczony, zmien rozmiar tabeli.
        if (static_cast<double>(current_size) / table_size > MAX_LOAD_FACTOR) {
            resize();
        }

        size_t index = hash_function(key, table_size); // Oblicz indeks koszyka
        bool inserted_new_node; // Flaga do sledzenia, czy nowy wezel zostal faktycznie wstawiony
        table[index] = insert_avl(table[index], key, value, inserted_new_node); // Wstaw do drzewa AVL

        if (inserted_new_node) {
            current_size++; // Zwieksz licznik elementow tylko jesli dodano nowy wezel
        }

        return true; // Zawsze true, jesli operacja insert_avl sie powiodla
    }

    // Usuwa element z podanym kluczem z tabeli.
    // Zwraca true, jesli element zostal usuniety, false w przeciwnym razie.
    bool remove(int key) override {
        size_t index = hash_function(key, table_size); // Oblicz indeks koszyka
        bool removed_node; // Flaga do sledzenia, czy wezel zostal faktycznie usuniety
        table[index] = remove_avl(table[index], key, removed_node); // Usun z drzewa AVL

        if (removed_node) {
            current_size--; // Zmniejsz licznik elementow tylko jesli usunieto wezel
        }

        return removed_node; // Zwroc true/false z funkcji remove_avl
    }

    // Znajduje wartosc skojarzona z podanym kluczem.
    // Zwraca true, jesli klucz zostal znaleziony, a wartosc jest przypisana do 'value',
    // false w przeciwnym razie.
    bool find(int key, int& value) override {
        size_t index = hash_function(key, table_size); // Oblicz indeks koszyka
        return find_avl(table[index], key, value); // Szukaj w drzewie AVL
    }

    // Wyswietla zawartosc tabeli hashujacej.
    void display() override {
        std::cout << "=== AVL Hash Table ===" << std::endl;
        for (size_t i = 0; i < table_size; ++i) {
            std::cout << "Bucket " << i << ":" << std::endl;
            if (table[i]) {
                display_avl(table[i], 1); // Wyswietl drzewo AVL w danym kubku, z wcieciem 1
            }
            else {
                std::cout << "  [EMPTY]" << std::endl; // Kubel jest pusty
            }
        }
        std::cout << "Total Size: " << current_size << " / Table Capacity: " << table_size << std::endl; // Poprawiony opis rozmiaru
    }

    // Zwraca aktualna liczbe elementow w tabeli.
    size_t size() const override { return current_size; }

    // Czyści tabele, zwalniajac pamiec wszystkich drzew AVL i resetujac licznik.
    void clear() override {
        for (AVLNode*& root : table) { // Iteruj przez wszystkie korzenie drzew w tabeli
            clear_avl(root); // Wyczysc kazde drzewo AVL (zwolnij pamiec wezlow)
            root = nullptr; // Ustaw korzen na nullptr po usunieciu wezlow
        }
        current_size = 0; // Zresetuj licznik elementow
    }

    // Zwraca nazwe implementacji tabeli hashujacej.
    std::string get_name() const override {
        return "AVL Hash Table";
    }
};

#endif // AVL_HASH_TABLE_H
