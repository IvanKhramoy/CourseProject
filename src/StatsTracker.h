#pragma once
#include <SFML/System/Clock.hpp>

class StatsTracker {
public:
    void reset();
    void recordHit();
    void recordMiss();
    void updateTime();
    void stopTimer();

    int score() const { return mScore; }
    int hits() const { return mHits; }
    int misses() const { return mMisses; }
    float time() const { return mElapsedTime; }

    float accuracy() const;
    float cpm() const;

private:
    int mScore = 0;
    int mHits = 0;
    int mMisses = 0;
    float mElapsedTime = 0.f;
    sf::Clock mClock;
};