#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <cmath>
#include <string.h>
#include <sstream>
#include <wchar.h>
#include <omp.h>
#include <iostream>
#include <set>
#include <algorithm>
#include <functional>

using namespace std;

//количество испытаний
const int N = 100;

//количество потоков(блоков)
const int threadNum=4;

const char* readFromFile(const char* path);
map<string, int> countWordsByBlocks(const char* text, int number);
map<string, int> countWordsByBlocksOMP(const char* text, int number);
double runOneOMP(const char* text, int threadNumber);
double runOneSimple(const char* text, int threadNumber);
map<string, int> countTextWords(const char* text, size_t len);
double countWithKoefOMP(const char* text, int threadNumber);
double countWithKoefSimple(const char* text, int threadNumber);
void printSortingMap(map<string, int> map);

int main(int argc, char** argv) {

    cout << "Analyzing is starting!" << "\n";
    
    //загружаем книгу из текстового файла
    //const char* testText = readFromFile("book_x1.txt");
    const char* testText = readFromFile("book1.txt");
    //const char* testText = readFromFile("book_x4.txt");
    //const char* testText = readFromFile("book_x8.txt");
    printSortingMap(countWordsByBlocks(testText, threadNum));
    
    cout<<"Thread= "<<threadNum<<"  OpenMP Mean: "<<countWithKoefOMP(testText, threadNum)<<"\n";
    cout<<"Thread= "<<threadNum<<"  Simple Mean: "<<countWithKoefSimple(testText, threadNum)<<"\n";
         
     return 0;
}

//запуск одного раза OpenMP расчета
double runOneOMP(const char* text, int threadNumber){
     double startTime = omp_get_wtime();
     countWordsByBlocksOMP(text, threadNumber);
     double endTime = omp_get_wtime();
     double result = endTime - startTime;
     return result;
}

//запуск одного раза бычного расчета
double runOneSimple(const char* text, int threadNumber){
     double startTime = omp_get_wtime();
     map<string, int> counted = countWordsByBlocks(text, threadNumber);
     double endTime = omp_get_wtime();
     double result = endTime - startTime;
     return result;
}

//тестирование программы 100 раз и расчтеты с использованием коэф. Стьюдента
double countWithKoefOMP(const char* text, int threadNumber){
     //коэфициент стьюдента для большого количества испытаний на 95%
    const double stKoef = 1.96;
    
    //результаты испытаний
    double durations[N];
    double sum = 0;
    
    for (int i = 0; i < N; i++) {
       durations[i] = runOneOMP(text, threadNumber);
    }

    //суммируем все результаты
    for (int i = 0; i < N; i++) {
        sum += durations[i];
    }

    //среднее арифметическое значение
    double mean = sum / N;
    
    //считаем дисперсию (стандартное отклонение)
    double variance = 0;
    for (int i = 0; i < N; i++) {
        variance += pow(durations[i] - mean, 2);
    }
    variance = variance / (N - 1);
    
    //Стандартное отклонение
    double standardDeviation = sqrt(variance);
    double stError = standardDeviation / sqrt(N);
    
    //определяем уровень значимости
    double marginError = stKoef*stError;
    
    //cout << "\nСреднее значение: " << mean << endl;
    //cout << "Стандартное отклонение: " << standardDeviation << endl;
    //cout << "погрешность: " << marginError << endl;
    
    return mean;
}

//печать кол-ва слов в тексте по возростанию их встречи
void printSortingMap(map<string, int> map){
     set<pair<int, string>> s;  

     for (auto const &kv : map)
         s.emplace(kv.second, kv.first);  

     for (auto const &vk : s)
         std::cout  << vk.first  <<" "<< vk.second << std::endl;
}

//тестирование программы 100 раз и расчтеты с использованием коэф. Стьюдента
double countWithKoefSimple(const char* text, int threadNumber){
     //коэфициент стьюдента для большого количества испытаний при 95% уверенности
    const double stKoef = 1.96;
  
    
    //результаты испытаний
    double durations[N];
    double sum = 0;
    
    for (int i = 0; i < N; i++) {
       durations[i] = runOneSimple(text, threadNumber);
    }

    //суммируем все результаты
    for (int i = 0; i < N; i++) {
        sum += durations[i];
    }

    //среднее значение
    double mean = sum / N;
    
    //
    double variance = 0;
    for (int i = 0; i < N; i++) {
        variance += pow(durations[i] - mean, 2);
    }
    variance = variance / (N - 1);
    
    double standardDeviation = sqrt(variance);
    double stError = standardDeviation / sqrt(N);
    double margin = stKoef*stError;
    
   //cout << "\nСреднее значение: " << mean << endl;
   //cout << "Стандартное отклонение: " << standardDeviation << endl;
   //cout << "Погрешность: " << margin << endl;
    
    return mean;
}


//подсчет слов путем разбияения текста на блоки
map<string, int> countWordsByBlocksOMP(const char* text, int number) {
    size_t len = strlen(text);
    map<string, int> tmpMap;
    vector<size_t> indexes;

    //расчитываем размер блока (размер текста на кол-во потоков)
    size_t blockSize = len/number+1;
    size_t blockStart = 0;
    size_t blockEnd;

    //для каждого блока сохраняем позиции начала и конца
    for (int i = 0; i < number; i++) {
        indexes.push_back(blockStart);
        blockEnd = blockStart + blockSize;
        
        //проверяем, чтобы блоки не разрезали слова на части, поэтому
        //смещаем позцию конца блока до первого разделителя
        while (true) {
            if ((blockEnd > len) || (text[blockEnd] == ' ')) {
                break;
            }
            blockEnd++;
        }
        
        blockStart = blockEnd + 1;
        if (blockStart > len) {
            break;
        }
    }

    //для каждого блока производим подсчет результатов
    //после подсчета результаты объединяем в одну переменную
    #pragma omp parallel for
    for (int i = 0; i < number; i++) {
        blockStart = indexes[i];
        blockEnd = i == (number - 1) ? len : indexes[i + 1];
        
        //считаем слова в блоке текста
        auto localMap = countTextWords(text + blockStart, blockEnd - blockStart);
        
        //объединяем результаты
        #pragma omp critical(critsec)
        for (auto& el : localMap) {
            int prevCount = tmpMap.count(el.first) ? tmpMap[el.first] : 0;
            tmpMap[el.first] = prevCount + el.second;
        }
    }
    
    return tmpMap;
}

//подсчет слов путем разбияения текста на блоки
map<string, int> countWordsByBlocks(const char* text, int number) {
    size_t len = strlen(text);
    map<string, int> tmpMap;
    vector<size_t> indexes;

    //расчитываем размер блока (размер текста на кол-во потоков)
    size_t blockSize = len/number+1;
    size_t blockStart = 0;
    size_t blockEnd;

    //для каждого блока сохраняем позиции начала и конца
    for (int i = 0; i < number; i++) {
        indexes.push_back(blockStart);
        blockEnd = blockStart + blockSize;
        
        //проверяем, чтобы блоки не разрезали слова на части, поэтому
        //смещаем позцию конца блока до первого разделителя
        while (true) {
            if ((blockEnd > len) || (text[blockEnd] == ' ')) {
                break;
            }
            blockEnd++;
        }
        
        blockStart = blockEnd + 1;
        if (blockStart > len) {
            break;
        }
    }

    //для каждого блока производим подсчет результатов
    //после подсчета результаты объединяем в одну переменную
    for (int i = 0; i < number; i++) {
        blockStart = indexes[i];
        blockEnd = i == (number - 1) ? len : indexes[i + 1];
        
        //считаем слова в блоке текста
        auto localMap = countTextWords(text + blockStart, blockEnd - blockStart);
        
        //объединяем результаты
        for (auto& el : localMap) {
            int prevCount = tmpMap.count(el.first) ? tmpMap[el.first] : 0;
            tmpMap[el.first] = prevCount + el.second;
        }
    }
    
    return tmpMap;
}

//подсчет слов в тексте
map<string, int> countTextWords(const char* text, size_t len) {

    //сохраняем входной текст 
    char* localText = new char[len + 1];
    strncpy(localText, text, len);
    localText[len] = '\0';

    //указываем список знаков пунктуации
    const char* delimiter = " .,\"?-!()";
    char *saveptr;

    map<string, int> tmpMap;

    //реентерабельное извлечение токенов из строки
    //saveptr используется для учёта контекста между последующими 
    //вызовами при анализе одной и той же строки.
    char* token = strtok_r(localText, delimiter, &saveptr);

    while (token != NULL) {
        int savedToken = 0;
        
        //ищем сохраненное значение для найденного токена
        if (tmpMap.find(token) != tmpMap.end())
            savedToken = tmpMap[token];
        tmpMap[token] = savedToken + 1;
        
        //продолжаем поиск токенов
        token = strtok_r(NULL, delimiter, &saveptr);
    }
    delete[] localText;
    return tmpMap;
}

const char* readFromFile(const char* path) {
    
    //открываем файл на чтение
    FILE *file = fopen(path, "r");
    
    //проверяем, открылся ли файл
    if (file == NULL) {
        perror("Файл не удалось открыть!\n");
        exit(1);
    }

    //устанавливаем позиционирование на конец файла
    fseek(file, 0, SEEK_END);
    
    //получаем размер текста для инициализации буфера
    long textSize = (size_t) ftell(file);
    
    //устанавливаем позиционирование обратно на начало
    fseek (file, 0, SEEK_SET);

    //выделяем память для буфера
    char* buffer = (char*) malloc((size_t) textSize);

    //копируем текст из файла в созданный буфер readedObjects==textSize
    size_t readedObjects = fread(buffer, 1, textSize, file);
    
    return buffer;
}
