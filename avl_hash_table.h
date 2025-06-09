#ifndef AVL_HASH_TABLE_H
#define AVL_HASH_TABLE_H

#include "hash_table_base.h"

// Hash Table z kube³kami zawieraj¹cymi drzewa AVL
class AVLHashTable : public HashTableBase {
private:
    struct AVLNode {
        int key;
        int value;
        int height;
        AVLNode* left;
        AVLNode* right;

        AVLNode(int k, int v) : key(k), value(v), height(1), left(nullptr), right(nullptr) {}
    };

    std::vector<AVLNode*> table;
    size_t table_size;
    size_t current_size;

    static constexpr double MAX_LOAD_FACTOR = 1.0;

    // funkcje pomocnicze dla drzewa AVL
    int get_height(AVLNode* node) {
        return node ? node->height : 0;
    }

    int get_balance(AVLNode* node) {
        return node ? get_height(node->left) - get_height(node->right) : 0;
    }

    void update_height(AVLNode* node) {
        if (node) {
            node->height = 1 + std::max(get_height(node->left), get_height(node->right));
        }
    }

    AVLNode* rotate_right(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;

        x->right = y;
        y->left = T2;

        update_height(y);
        update_height(x);

        return x;
    }

    AVLNode* rotate_left(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;

        y->left = x;
        x->right = T2;

        update_height(x);
        update_height(y);

        return y;
    }

    AVLNode* insert_avl(AVLNode* node, int key, int value, bool& inserted) {
        if (!node) {
            inserted = true;
            return new AVLNode(key, value);
        }

        if (key < node->key) {
            node->left = insert_avl(node->left, key, value, inserted);
        }
        else if (key > node->key) {
            node->right = insert_avl(node->right, key, value, inserted);
        }
        else {
            node->value = value;
            inserted = false;
            return node;
        }

        update_height(node);

        // SprawdŸ balans i wykonaj rotacje
        int balance = get_balance(node);

        // lewa-lewa
        if (balance > 1 && key < node->left->key) {
            return rotate_right(node);
        }

        //prawa-prawa
        if (balance < -1 && key > node->right->key) {
            return rotate_left(node);
        }

        // lewa-prawa
        if (balance > 1 && key > node->left->key) {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }

        // prawa-lewa
        if (balance < -1 && key < node->right->key) {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }

        return node;
    }

    AVLNode* find_min(AVLNode* node) {
        while (node->left) {
            node = node->left;
        }
        return node;
    }

    AVLNode* remove_avl(AVLNode* node, int key, bool& removed) {
        if (!node) {
            removed = false;
            return node;
        }

        if (key < node->key) {
            node->left = remove_avl(node->left, key, removed);
        }
        else if (key > node->key) {
            node->right = remove_avl(node->right, key, removed);
        }
        else {
            removed = true;

            if (!node->left || !node->right) {
                AVLNode* temp = node->left ? node->left : node->right;

                if (!temp) {
                    temp = node;
                    node = nullptr;
                }
                else {
                    *node = *temp;
                }
                delete temp;
            }
            else {
                AVLNode* temp = find_min(node->right);
                node->key = temp->key;
                node->value = temp->value;

                bool dummy;
                node->right = remove_avl(node->right, temp->key, dummy);
            }
        }

        if (!node) return node;

        update_height(node);

        int balance = get_balance(node);

        // rotacje po usuniêciu
        if (balance > 1 && get_balance(node->left) >= 0) {
            return rotate_right(node);
        }

        if (balance > 1 && get_balance(node->left) < 0) {
            node->left = rotate_left(node->left);
            return rotate_right(node);
        }

        if (balance < -1 && get_balance(node->right) <= 0) {
            return rotate_left(node);
        }

        if (balance < -1 && get_balance(node->right) > 0) {
            node->right = rotate_right(node->right);
            return rotate_left(node);
        }

        return node;
    }

    bool find_avl(AVLNode* node, int key, int& value) {
        if (!node) return false;

        if (key == node->key) {
            value = node->value;
            return true;
        }
        else if (key < node->key) {
            return find_avl(node->left, key, value);
        }
        else {
            return find_avl(node->right, key, value);
        }
    }

    void clear_avl(AVLNode* node) {
        if (node) {
            clear_avl(node->left);
            clear_avl(node->right);
            delete node;
        }
    }

    void display_avl(AVLNode* node, int depth = 0) {
        if (node) {
            display_avl(node->right, depth + 1);
            for (int i = 0; i < depth; ++i) std::cout << "  ";
            std::cout << "(" << node->key << "," << node->value << ")" << std::endl;
            display_avl(node->left, depth + 1);
        }
    }

    void resize() {
        size_t old_size = table_size;
        auto old_table = std::move(table);

        table_size *= 2;
        table.clear();
        table.resize(table_size, nullptr);
        current_size = 0;

        for (AVLNode* root : old_table) {
            if (root) {
                collect_and_reinsert(root);
                clear_avl(root);
            }
        }
    }

    void collect_and_reinsert(AVLNode* node) {
        if (node) {
            insert(node->key, node->value);
            collect_and_reinsert(node->left);
            collect_and_reinsert(node->right);
        }
    }

public:
    explicit AVLHashTable(size_t initial_size = 16)
        : table_size(initial_size), current_size(0) {
        table.resize(table_size, nullptr);
    }

    ~AVLHashTable() {
        clear();
    }

    bool insert(int key, int value) override {
        if (static_cast<double>(current_size) / table_size > MAX_LOAD_FACTOR) {
            resize();
        }

        size_t index = hash_function(key, table_size);
        bool inserted;
        table[index] = insert_avl(table[index], key, value, inserted);

        if (inserted) {
            current_size++;
        }

        return true;
    }

    bool remove(int key) override {
        size_t index = hash_function(key, table_size);
        bool removed;
        table[index] = remove_avl(table[index], key, removed);

        if (removed) {
            current_size--;
        }

        return removed;
    }

    bool find(int key, int& value) override {
        size_t index = hash_function(key, table_size);
        return find_avl(table[index], key, value);
    }

    void display() override {
        std::cout << "=== AVL Hash Table ===" << std::endl;
        for (size_t i = 0; i < table_size; ++i) {
            std::cout << "Bucket " << i << ":" << std::endl;
            if (table[i]) {
                display_avl(table[i], 1);
            }
            else {
                std::cout << "  [EMPTY]" << std::endl;
            }
        }
        std::cout << "Size: " << current_size << std::endl;
    }

    size_t size() const override { return current_size; }

    void clear() override {
        for (AVLNode*& root : table) {
            clear_avl(root);
            root = nullptr;
        }
        current_size = 0;
    }
 std::string get_name() const override {
     return "AVL Hash Table";
 }
};

#endif
