// Toolbox.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <string_view>
#include <any>
#include <typeinfo>
#include <map>
#include <functional>
#include <utility>
#include <vector>
#include <numeric>
#include <mutex>
#include <stdexcept>
#include <memory>
#include <initializer_list>
#include <atomic>
#include <algorithm>

// __declspec(novtable) ensures that the class
// will throw an error if its member functions and variables are accessed.
#define interface class __declspec(novtable)

class test {
private:
    unsigned long long a;
    unsigned long b;
    unsigned int c;
    unsigned short d;
    unsigned char e;
public:
    void foo() {
    
    }

    int bar() {
    
    }

    /// <summary>
    /// __dict__
    /// </summary>
    operator std::map<std::string, std::any>() const {
        return {
            std::make_pair<std::string, std::any>("a", this->a),
            std::make_pair<std::string, std::any>("b", this->b),
            std::make_pair<std::string, std::any>("c", this->c),
            std::make_pair<std::string, std::any>("d", this->d),
            std::make_pair<std::string, std::any>("e", this->e),
            std::make_pair<std::string, std::any>("foo()", &test::foo),
            std::make_pair<std::string, std::any>("bar()", &test::bar)
        };
    }
};

/// <summary>
/// A toolbox entry.
/// </summary>
class toolbox_entry {
private:
    std::string m_name;
    std::any m_value;
public:
    toolbox_entry(std::string_view p_sv_name, std::any p_a_value) :
        m_name(p_sv_name), m_value(p_a_value) {}

    std::string_view get_name() const {
        return this->m_name;
    }

    template<class T>
    T &get_value() {
        T *ptr_value = std::any_cast<T>(&this->m_value);

        if (!ptr_value) {
            throw std::bad_any_cast();
        }

        return *ptr_value;
    }

    const std::type_info &get_type() const noexcept {
        return this->m_value.type();
    }
};

class toolbox_class {
private:
    std::vector<std::shared_ptr<toolbox_entry>> members;
public:
    std::shared_ptr<toolbox_entry> get(std::string_view p_sv_name) {
        std::vector<std::shared_ptr<toolbox_entry>>::iterator it =
            std::find_if(this->members.begin(), this->members.end(),
            [&](const std::shared_ptr<toolbox_entry> &member) { 
                return member->get_name() == p_sv_name;
            }
        );

        return (it == std::end(this->members) ? nullptr : *it);
    }

    void push_back(const std::shared_ptr<toolbox_entry> &member) {
        this->members.push_back(member);
    }

    void push_back(std::shared_ptr<toolbox_entry> &&member) {
        this->members.push_back(std::move(member));
    }

    std::vector<std::shared_ptr<toolbox_entry>>::iterator begin() { return this->members.begin(); }
    std::vector<std::shared_ptr<toolbox_entry>>::const_iterator begin() const { return this->members.begin(); }
    std::vector<std::shared_ptr<toolbox_entry>>::const_iterator cbegin() { return this->members.cbegin(); }
    std::vector<std::shared_ptr<toolbox_entry>>::iterator end() { return this->members.end(); }
    std::vector<std::shared_ptr<toolbox_entry>>::const_iterator end() const { return this->members.end(); }
    std::vector<std::shared_ptr<toolbox_entry>>::const_iterator cend() { return this->members.cend(); }
};

class toolbox {
private:
    std::map<size_t, std::shared_ptr<toolbox_entry>> toolbox_entries;

    class toolbox_entries_key_generator {
    private:
        std::vector<size_t> keys;
        std::mutex keys_mutex;
        const size_t block_allocation_size;
        size_t current_key_index;
    public:
        toolbox_entries_key_generator(size_t size) : 
            keys(size), block_allocation_size(size), current_key_index(0U) {
            std::iota(this->keys.begin(), this->keys.end(), 0U);
        }

        size_t generate_key() {
            std::lock_guard<std::mutex> keys_lock(this->keys_mutex);

            if (this->current_key_index == this->keys.size()) {
                this->keys.resize(this->keys.size() + this->block_allocation_size);

                std::iota(
                    this->keys.begin() + this->current_key_index, 
                    this->keys.end(), 
                    this->current_key_index
                );
            }

            current_key_index++;

            return this->keys[this->current_key_index];
        }
   } toolbox_entry_key_generator;
public:
    class entry_not_found : public std::runtime_error {
    public:
        entry_not_found(std::string_view entry_name) : 
            std::runtime_error(
                "Entry \"" + 
                std::string(entry_name) + 
                "\" not found."
            ) 
        {}
    };

    toolbox() : toolbox_entry_key_generator(100U) {}

    void create(std::string_view p_sv_name, std::any p_a_value) {
        this->toolbox_entries.insert(
            std::make_pair<size_t, std::shared_ptr<toolbox_entry>>(
                toolbox_entry_key_generator.generate_key(),
                std::make_shared<toolbox_entry>(
                    p_sv_name,
                    p_a_value
                )
            )
        );
    }

    void create(
        std::string_view p_sv_class_name,
        std::initializer_list<
            std::pair<
                std::string_view,                                        // Name
                std::any                                                 // Value
            >
        > member_fields,
        std::initializer_list<
            std::pair<
                std::string_view,                                        // Name
                std::function<std::any(std::any * /* this pointer*/ )>   // Value (Function)
            >
        > member_functions
    ) {
        this->toolbox_entries.insert(
            std::make_pair<size_t, std::shared_ptr<toolbox_entry>>(
                toolbox_entry_key_generator.generate_key(),
                std::make_shared<toolbox_entry>(

                )
            )
        );
    }

    template<class T>
    T &get(std::string_view p_sv_name) {
        std::map<size_t, std::shared_ptr<toolbox_entry>>::iterator m_s_te =
            std::find_if(
                this->toolbox_entries.begin(),
                this->toolbox_entries.end(),
                [&](const std::pair<size_t, std::shared_ptr<toolbox_entry>> &p_p_s_te) {
                    return p_p_s_te.second->get_type() == typeid(T) &&
                        p_p_s_te.second->get_name() == p_sv_name;
                }
            );

        if (m_s_te == this->toolbox_entries.end()) {
            throw entry_not_found(p_sv_name);
        }

        return m_s_te->second->get_value<T>();
    }
};

int main() {
    toolbox t;

    t.create("oingo boingo", 27);

    std::cout << t.get<int>("oingo boingo") << std::endl;

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
