#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

class HighscoreManager {
public:
    // Загружает топ-5 из файла
    static std::vector<int> load() {
        std::vector<int> scores;
        std::ifstream file("highscores.txt");
        int s;
        while (file >> s) scores.push_back(s);
        file.close();
        
        // Дозаполняем нулями, если в файле меньше 5 записей
        while (scores.size() < 5) scores.push_back(0);
        std::sort(scores.rbegin(), scores.rend());
        return scores;
    }

    // Проверяет, является ли результат рекордом, и сохраняет
    static bool submitScore(int newScore) {
        auto scores = load();
        bool isRecord = (!scores.empty() && newScore > scores[0]);
        
        scores.push_back(newScore);
        std::sort(scores.rbegin(), scores.rend());
        if (scores.size() > 5) scores.resize(5); // Оставляем только топ-5

        std::ofstream file("highscores.txt");
        for (int s : scores) file << s << "\n";
        file.close();

        return isRecord;
    }
};