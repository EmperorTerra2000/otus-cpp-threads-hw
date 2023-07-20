// Read files and prints top k word by frequency

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <vector>
#include <chrono>
#include <memory>

#include <thread>
#include <mutex>

const size_t TOPK = 10;

using Counter = std::map<std::string, std::size_t>;

std::string tolower(const std::string &str);

void count_words(std::string, Counter&);

void print_topk(std::ostream& stream, const Counter&, const size_t k);

void merge_dictionaries(Counter& base, std::vector<Counter>& els);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: topk_words [FILES...]\n";
        return EXIT_FAILURE;
    }

    auto start = std::chrono::high_resolution_clock::now();

    // общий словарь
    Counter freq_dict;

    // массив словарей под каждый поток
    std::vector<Counter> dictionaries{};
    // зарезервируем размер массива равный кол-ву файлов
    dictionaries.resize(argc - 1);

    // вектор потоков
    std::vector<std::thread> t_threads{};

    for (int i = 1; i < argc; ++i) {
        /*
            1 арг - путь к файлу
            2 арг - элемент массива-словаря
        */
        t_threads.push_back(std::move(
            std::thread{count_words, 
                        std::string(argv[i]), 
                        std::ref(dictionaries[i - 1])}
        ));
    }

    for(int i = 0; i < argc - 1; i++){
        t_threads.at(i).join(); 
    }

    merge_dictionaries(freq_dict, dictionaries);

    print_topk(std::cout, freq_dict, TOPK);

    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Elapsed time is " << elapsed_ms.count() << " us\n";
}

/*
    1 арг - базовый элемент для слияния элементов массива
    2 арг - массив словарей
*/
void merge_dictionaries(Counter& base, std::vector<Counter>& els){
    // присваиваем первый элемент массива-словаря
    // он будет базой для дальнейшего слияния всех словарей
    base = els[0];

    // проходим по массиву словарей
    for(size_t i = 1; i < els.size(); i++){
        // проходим по множеству
        for(auto it = els[i].cbegin(); it != els[i].cend(); it++){
            // проверяем есть ли такое слово в базовом словаре
            // если нет, то создаем новую запись, инициал-ем нулем,
            // дальше увеличим значение частоты
            if(base.find(it->first) == base.end()){
                base[it->first] = 0;
            }

            base[it->first] += it->second;
        }
    }
}

std::string tolower(const std::string &str) {
    std::string lower_str;
    std::transform(std::cbegin(str), std::cend(str),
                   std::back_inserter(lower_str),
                   [](unsigned char ch) { return std::tolower(ch); });
    return lower_str;
};

void count_words(std::string fileName, Counter& counter) {
    std::ifstream input{fileName};

    if (!input.is_open()) {
        std::cerr << "Failed to open file " << fileName << '\n';
        exit(-1);
    }
    
    std::for_each(std::istream_iterator<std::string>(input),
            std::istream_iterator<std::string>(),
            [&counter](const std::string &s) { ++counter[tolower(s)]; });     
}

void print_topk(std::ostream& stream, const Counter& counter, const size_t k) {
    std::vector<Counter::const_iterator> words;
    words.reserve(counter.size());
    for (auto it = std::cbegin(counter); it != std::cend(counter); ++it) {
        words.push_back(it);
    }

    std::partial_sort(
        std::begin(words), std::begin(words) + k, std::end(words),
        [](auto lhs, auto &rhs) { return lhs->second > rhs->second; });

    std::for_each(
        std::begin(words), std::begin(words) + k,
        [&stream](const Counter::const_iterator &pair) {
            stream << std::setw(4) << pair->second << " " << pair->first
                      << '\n';
        });
}