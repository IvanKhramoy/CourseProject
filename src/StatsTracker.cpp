#include "StatsTracker.h"
#include "Constants.h"
#include <SFML/System/Time.hpp>

void StatsTracker::reset() {
    mScore = 0;
    mHits = 0;
    mMisses = 0;
    mElapsedTime = 0.f;
    mClock.restart();
}

void StatsTracker::recordHit() {
    mHits++;
    mScore += SCORE_PER_HIT;
}   

void StatsTracker::recordMiss() {
    mMisses++;
}

void StatsTracker::updateTime() {
    mElapsedTime = mClock.getElapsedTime().asSeconds();
}

void StatsTracker::stopTimer() {
    // Просто не обновляем mElapsedTime
}

float StatsTracker::accuracy() const {
    int total = mHits + mMisses;
    if (total == 0) return 100.f;
    return (static_cast<float>(mHits) / static_cast<float>(total)) * 100.f;
}

float StatsTracker::cpm() const {
    // Переводим секунды в минуты
    float minutes = mElapsedTime / 60.f;
    
    // Защита от деления на ноль (в самом начале игры)
    if (minutes < 0.0001f) return 0.f;
    
    // Делим количество правильных нажатий на время в минутах
    return static_cast<float>(mHits) / minutes;
}