#pragma once
#include <SFML/System/Clock.hpp>
#include "Constants.h"

class StatsTracker {
public:
    void reset() {
        mScore   = 0;
        mHits    = 0;
        mMisses  = 0;
        mElapsedTime = 0.f;
        mClock.restart();
    }

    void recordHit() {
        mHits++;
        mScore += SCORE_PER_HIT;
    }

    void recordMiss() {
        mMisses++;
    }

    void updateTime() {
        mElapsedTime = mClock.getElapsedTime().asSeconds();
    }

    void stopTimer() {
        // Мы просто перестаем вызывать updateTime(), 
        // поэтому mElapsedTime зафиксирует последнее значение
    }

    int score()    const { return mScore;   }
    int hits()     const { return mHits;    }
    int misses()   const { return mMisses;  }
    float time()   const { return mElapsedTime; }

    float accuracy() const {
        int total = mHits + mMisses;
        if (total == 0) return 100.f;
        return 100.f * mHits / total;
    }

    float wpm() const {
        float minutes = mElapsedTime / 60.f;
        if (minutes < 0.0001f) return 0.f;
        return mHits / minutes;
    }

private:
    int        mScore   = 0;
    int        mHits    = 0;
    int        mMisses  = 0;
    float      mElapsedTime = 0.f;
    sf::Clock  mClock;
};