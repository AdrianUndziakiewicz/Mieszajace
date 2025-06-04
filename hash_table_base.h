#ifndef HASH_TABLE_BASE_H
#define HASH_TABLE_BASE_H

#include <iostream>
#include <vector>
#include <list>
#include <functional>

// Abstrakcyjna klasa bazowa dla wszystkich implementacji hash table
class HashTableBase {
public:
    virtual ~HashTableBase() = default;
    virtual bool insert(int key, int value) = 0;
    virtual bool remove(int key) = 0;
    virtual bool find(int key, int& value) = 0;
    virtual void display() = 0;
    virtual size_t size() const = 0;
    virtual void clear() = 0;
    
protected:
    // Funkcja hash - dzielenie modulo
    size_t hash_function(int key, size_t table_size) const {
        return std::hash<int>{}(key) % table_size;
    }
};

#endif // HASH_TABLE_BASE_H
