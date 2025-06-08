#ifndef OPEN_ADDRESSING_HASH_TABLE_H
#define OPEN_ADDRESSING_HASH_TABLE_H

#include "hash_table_base.h"


class OpenAddressingHashTable : public HashTableBase {
private:
    enum class EntryState { EMPTY, OCCUPIED, DELETED };

    struct Entry {
        int key;
        int value;
        EntryState state;

        Entry() : key(0), value(0), state(EntryState::EMPTY) {}
        Entry(int k, int v) : key(k), value(v), state(EntryState::OCCUPIED) {}
    };

    std::vector<Entry> table;
    size_t table_size;
    size_t current_size;

    static constexpr double MAX_LOAD_FACTOR = 0.5;

    void resize() {
        size_t old_size = table_size;
        auto old_table = std::move(table);

        table_size *= 2;
        table.clear();
        table.resize(table_size);
        current_size = 0;

        for (const auto& entry : old_table) {
            if (entry.state == EntryState::OCCUPIED) {
                insert(entry.key, entry.value);
            }
        }
    }

    size_t probe(int key) const {
        size_t index = hash_function(key, table_size);
        size_t original_index = index;

        while (table[index].state != EntryState::EMPTY &&
            (table[index].state == EntryState::DELETED || table[index].key != key)) {
            index = (index + 1) % table_size;
            if (index == original_index) break;
        }

        return index;
    }

public:
    explicit OpenAddressingHashTable(size_t initial_size = 16)
        : table_size(initial_size), current_size(0) {
        table.resize(table_size);
    }

    bool insert(int key, int value) override {
        if (static_cast<double>(current_size) / table_size > MAX_LOAD_FACTOR) {
            resize();
        }

        size_t index = probe(key);

        if (table[index].state == EntryState::OCCUPIED && table[index].key == key) {
            table[index].value = value;
            return true;
        }

        if (table[index].state != EntryState::OCCUPIED) {
            table[index] = Entry(key, value);
            current_size++;
            return true;
        }

        return false;
    }

    bool remove(int key) override {
        size_t index = probe(key);

        if (table[index].state == EntryState::OCCUPIED && table[index].key == key) {
            table[index].state = EntryState::DELETED;
            current_size--;
            return true;
        }

        return false;
    }

    bool find(int key, int& value) override {
        size_t index = probe(key);

        if (table[index].state == EntryState::OCCUPIED && table[index].key == key) {
            value = table[index].value;
            return true;
        }

        return false;
    }

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

    size_t size() const override { return current_size; }

    void clear() override {
        for (auto& entry : table) {
            entry.state = EntryState::EMPTY;
        }
        current_size = 0;
    }
};

#endif