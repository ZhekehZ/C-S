#include "huffman.hpp"

#include <queue>
#include <vector>
#include <string>
#include <cassert>
#include <array>

namespace {

    // Структура для хранения битовой строки
    struct Bits {
        std::vector<uint8_t> data;
        uint8_t last_bit_pos; // позиция последнего бита в последнем байте

        Bits() : data{}, last_bit_pos(0) {}

        // Добавляет 1 бит в конец
        void push_back(bool bit) {
            if (last_bit_pos == 0) {
                data.push_back(0);
                last_bit_pos = 8;
            }
            --last_bit_pos;
            if (bit) {
                data.back() |= (1u << last_bit_pos);
            }
        }

        // Удаляет 1 бит из конца
        void pop_back() {
            assert(!data.empty() && "empty bit string");

            // Установка последнего бита равным 0
            uint8_t mask = 1u << last_bit_pos;
            data.back() = (data.back() | mask) ^ mask;

            ++last_bit_pos;
            if (last_bit_pos > 7) {
                data.pop_back();
                last_bit_pos = 0;
            }
        }

        // Устанавливает самый последний бит равным 1
        void set_back1() {
            assert(!data.empty() && "empty bit string");
            data.back() ^= (1u << last_bit_pos);
        }
    };


    // Дерево Хаффмана
    struct CodeTree {
        struct Node {
            uint64_t weight;
            Node *zero;
            Node *one;
            uint8_t symbol;

            // Конструктор для листа
            Node (uint64_t weight, uint8_t symbol) 
                : weight(weight)
                , zero(nullptr)
                , one(nullptr)
                , symbol(symbol) 
            {}

            // Конструктор для промежуточной вершины
            Node (Node *n1, Node *n2) 
                : weight(n1->weight + n2->weight)
                , zero(n1)
                , one(n2)
                , symbol(0)
            {}

            ~Node() {
                if (zero) {
                    delete zero;
                }
                if (one) {
                    delete one;
                }
            }
        };

        explicit CodeTree(Node* root = nullptr) : root(root) {}

        // Конструктор от массива частот
        explicit CodeTree(const std::array<uint64_t, 256> &freqs) {
            std::priority_queue<Node*, std::vector<Node*>, NodePtrComparator> nodes;
            for (size_t code = 0; code < 256; ++code) {
                if (freqs[code] > 0) {
                    Node *leaf = new Node{freqs[code], static_cast<uint8_t>(code)};
                    nodes.push(leaf);
                }
            }

            // Находим и объединяем два самых редких символа
            while (nodes.size() > 1) {
                auto node_zero = nodes.top();
                nodes.pop();
                auto node_one = nodes.top();
                nodes.pop();
                nodes.push(new Node{node_zero, node_one});
            }

            root = nodes.top();
        }

        void print() const {
            if (!root) {
                return;
            }
            // Если в дереве более одной вершины
            if (root->zero) {
                std::string prefix;
                recursive_print(root, &prefix);
                return;
            }
            // Если в дереве всего одна вершина
            std::cout << "0 " << static_cast<uint32_t>(root->symbol) << '\n';
        }

        // Создание таблицы кодирования
        std::array<Bits, 256> create_table() const {
            std::array<Bits, 256> result; // NRVO

            if (!root) {
                return result;
            }
            Bits code;
            if (!root->zero) {
                code.push_back(false);
                result[root->symbol] = code;
            } else {
                recursive_fill_table(&result, &code, root);
            }

            return result;
        }

        ~CodeTree() {
            if (root) {
                delete root;
            }
        }

        Node *root;

        static void recursive_print(const Node* node, std::string* prefix) {
            if (!node) {
                return;
            }
            prefix->push_back('0');
            recursive_print(node->zero, prefix);
            prefix->back() = '1';
            recursive_print(node->one, prefix);
            prefix->pop_back();
            if (!node->zero && !node->one) {
                std::cout << *prefix << ' ' 
                    << static_cast<uint32_t>(node->symbol) << '\n';
            }
        }

        static void recursive_fill_table(
            std::array<Bits, 256>* table, 
            Bits* curr_code,
            const Node* node
        ) {
            if (!node) {
                return;
            }
            // Если текущая вершина является листом
            if (!node->zero) {
                table->at(node->symbol) = *curr_code;
                return;
            }
            curr_code->push_back(false);
            recursive_fill_table(table, curr_code, node->zero);
            curr_code->set_back1();
            recursive_fill_table(table, curr_code, node->one);
            curr_code->pop_back();
        }

        struct NodePtrComparator {
            bool operator()(const Node* lhs, const Node* rhs) const {
                return lhs && rhs && lhs->weight > rhs->weight;
            }
        };
    };


    std::vector<uint8_t> encode_buffer(
        const uint8_t* raw_data,
        const uint64_t size, 
        const CodeTree& tree
    ) {
        std::vector<uint8_t> encoded; // NRVO

        std::array<Bits, 256> table = tree.create_table();

        uint8_t current_offset = 0; // текущий отступ от начала байта
        encoded.push_back(0);
        for (uint64_t i = 0; i < size; ++i) {
            Bits& symbol = table[raw_data[i]];

            // Если код длинный, записываем сначала его целые байты
            for (size_t j = 0; j < symbol.data.size() - 1; ++j) {
                encoded.back() |= symbol.data[j] >> current_offset;
                encoded.push_back(symbol.data[j] << (8u - current_offset));
            }

            // Дописываем остаток
            encoded.back() |= symbol.data.back() >> current_offset;
            uint8_t new_offset = current_offset + (8 - symbol.last_bit_pos);
            if (new_offset > 7) {
                new_offset -= 8;
                encoded.push_back(symbol.data.back() << (8u - current_offset));
            }

            current_offset = new_offset;
        }

        // Сохранение количества значимых битов в последнем байте.
        // Если в последнем байте нет ни одного значимого бита, то     (****)
        //  последний байт уже содержит 0 и не нужно ничего сохранять.
        if (current_offset > 0) {
            encoded.push_back(current_offset);
        }
        return encoded;
    }


    // Вспомогательная функция для кодирования
    void recursive_encode_tree(
        const CodeTree::Node* node,
        std::vector<uint8_t>* symbols,
        Bits* encoded_tree
    ) {
        if (!node) {
            return;
        }

        // Если находимся в листе
        if (!node->zero) {
            symbols->push_back(node->symbol);
            encoded_tree->push_back(false);
            return;
        }

        encoded_tree->push_back(true);
        recursive_encode_tree(node->zero, symbols, encoded_tree);
        recursive_encode_tree(node->one, symbols, encoded_tree);
    }


    // Кодирование дерева оптимальным способом:
    //   https://neerc.ifmo.ru/wiki/index.php?title=Оптимальное_хранение_словаря_в_алгоритме_Хаффмана
    //   (раздел Эффективное решение)
    std::vector<uint8_t> encode_tree(const CodeTree& tree) {
        std::vector<uint8_t> encoded; // NRVO
        encoded.push_back(0); // Резервируем байт для размера словаря

        Bits encoded_tree;
        recursive_encode_tree(tree.root, &encoded, &encoded_tree);

        // Хранится размер алфавита -1 (чтобы размер поместился в 1 байт).
        //  Так как 1 байт зарезервирован, нужно вычесть 2.
        size_t alphabet_size = encoded.size() - 2;
        encoded[0] = static_cast<uint8_t>(alphabet_size);

        assert(!encoded_tree.data.empty());
        encoded.insert(
            encoded.end(),
            encoded_tree.data.begin(),
            encoded_tree.data.end()
        );

        return encoded;
    }


    struct DecoderStruct {
        const uint8_t* next_symbol{};  // Следующий символ алфавита
        const uint8_t* next_byte{};    // Текущий байт буфера
        uint8_t current_bit_pos = 7; // Номер рассматриваемого бита

        bool get_next_bit() {
            bool bit =  (*next_byte & (1u << current_bit_pos)) != 0;
            if (current_bit_pos == 0) {
                ++next_byte;
                current_bit_pos = 7;
            } else {
                --current_bit_pos;
            }
            return bit;
        }

        const uint8_t* get_next_byte() {
            if (current_bit_pos < 7) {
                ++next_byte;
            }
            current_bit_pos = 7;
            return next_byte;
        }
    };
    

    // Вспомогательная функция для декодирования дерева.
    CodeTree::Node* recursive_decode_tree(DecoderStruct* ds) {
        // Если bit равен true, то сначала идем в левого сына, потом
        //   в правого. Иначе создаем лист.
        if (ds->get_next_bit()) {
            CodeTree::Node* zero = recursive_decode_tree(ds);
            CodeTree::Node* one = recursive_decode_tree(ds);
            return new CodeTree::Node{zero, one};
        } else {
            uint8_t symbol = *(ds->next_symbol++);
            return new CodeTree::Node{0, symbol};
        }
    }

    // Декодирование дерева.
    //   Обновляет data: после завершения работы указывает на следующий
    //   байт после таблицы.
    CodeTree::Node* decode_tree(const uint8_t** data) {
        // В первом байте хранится размер алфавита - 1
        const uint8_t* buffer = *data;
        uint16_t alphabet_size = *(buffer++) + 1;
        DecoderStruct ds{buffer, buffer + alphabet_size};
        auto tree = recursive_decode_tree(&ds);
        *data = ds.get_next_byte();
        return tree;
    }


    std::vector<uint8_t> decode_buffer(
        const uint8_t* buffer,
        uint64_t size,
        const CodeTree& tree
    ) {
        std::vector<uint8_t> decoded; // NRVO

        uint8_t last_byte_data = *(buffer + size - 1);
        // Если последний байт равен 0, то считаем, что это часть 
        //     буфера (случай (****)). Иначе он содержит число 
        //     значимых битов в последнем байте.
        if (last_byte_data == 0) {
            ++size;
        }

        uint8_t last_bit_pos = 8 - last_byte_data; // Позиция последнего 
                                                //   бита в последнем байте
        const uint8_t* from = buffer;
        const uint8_t* to = buffer + size - 2;

        uint8_t current_bit = 7;
        CodeTree::Node* current_node = tree.root;

        // Если в дереве ввсего одна вершина
        if (current_node && (!current_node->zero)) {
            // Декодируем целые байты
            for (uint64_t i = 0; i < size - 2; ++i){
                for (int j = 0; j < 8; ++j) {
                    decoded.push_back(current_node->symbol);
                }
            }

            // Декодируем последний байт
            while (last_bit_pos++ < 8) {
                decoded.push_back(current_node->symbol);
            }
        } else {
            // Если дерево из более, чем 1 вершины
            while ((from < to) || ((from == to) && (current_bit >= last_bit_pos))) {
                bool bit = (*from & (1u << current_bit)) != 0; // текущий бит
                if (current_bit == 0) {
                    current_bit = 7;
                    ++from;
                } else {
                    --current_bit;
                }

                assert(current_node);
                current_node = bit ? current_node->one : current_node->zero;
                // Если дошли до листа, выписываем символ и возвращаемся в корень
                if (!current_node->zero) {
                    decoded.push_back(current_node->symbol);
                    current_node = tree.root;
                }
            }
        }

        return decoded;
    }

    void print_summary(uint64_t from, uint64_t to, uint64_t table) {
        std::cout << from << '\n' << to << '\n' << table << '\n';
    }

    uint64_t get_file_size(std::istream& istr) {
        if (!istr) {
            return 0;
        }
        istr.seekg(0, std::istream::end);
        uint64_t size = istr.tellg();
        istr.seekg(0, std::istream::beg);
        return size;
    }

} // \HUFFMAN

void encode(std::istream& istr, std::ostream& ostr, bool verbose) {
    uint64_t size = get_file_size(istr);

    if (!ostr || size == 0) {
        print_summary(0, 0, 0);
        return;
    }

    char* buffer = new char[size];
    istr.read(buffer, size);

    std::array<uint64_t, 256> freqs {};
    for (uint64_t i = 0; i < size; ++i) {
        auto index = static_cast<const uint8_t>(buffer[i]);
        ++freqs[index];
    }

    auto tree = CodeTree(freqs);

    auto encoded_buffer = encode_buffer(
        reinterpret_cast<uint8_t*>(buffer), size, tree
    );
    auto encoded_table = encode_tree(tree);

    delete[] buffer;

    // Последний байт буфера хранит информацию о количестве значимых
    //    битов в предпоследнем байте.
    print_summary(size, encoded_buffer.size() - 1, encoded_table.size() + 1);
    if (verbose) {
        tree.print();
    }

    ostr.write(
        reinterpret_cast<char*>(encoded_table.data()), 
        encoded_table.size()
    );
    ostr.write(
        reinterpret_cast<char*>(encoded_buffer.data()), 
        encoded_buffer.size()
    );
}


void decode(std::istream& istr, std::ostream& ostr, bool verbose) {
    uint64_t size = get_file_size(istr);

    if (!ostr || size == 0) {
        print_summary(0, 0, 0);
        return;
    }

    char* buffer = new char[size];
    istr.read(buffer, size);

    const auto *current_buffer = reinterpret_cast<const uint8_t*>(buffer);
    const uint8_t *origin = current_buffer;

    // decode_tree сдвигает current_buffer на начало буфера с данными
    CodeTree tree{decode_tree(&current_buffer)};
    auto table_size = static_cast<uint64_t>(current_buffer - origin);
    uint64_t data_size = size - table_size;
    
    auto decoded = decode_buffer(current_buffer, data_size, tree);

    delete[] buffer;

    // Последний байт буфера хранит информацию о количестве значимых
    //    битов в предпоследнем байте.
    print_summary(data_size - 1, decoded.size(), table_size + 1);
    if (verbose) {
        tree.print();
    }

    ostr.write(
        reinterpret_cast<char*>(decoded.data()),
        decoded.size()
    );
}
