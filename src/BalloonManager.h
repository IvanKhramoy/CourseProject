#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include "Balloon.h"
#include "Constants.h"
#include "ResourceManager.h"

class BalloonManager {
public:
    explicit BalloonManager(const ResourceManager& rm)
        : mRM(rm), mFallSpeed(INITIAL_FALL_SPEED), mNextX(PLATFORM_X + 200.f), mBalloonsSpawned(0), mIsClassicMode(true) {}

    void reset(bool isClassicMode) {
        mBalloons.clear();
        mFallSpeed = INITIAL_FALL_SPEED;
        mNextX     = PLATFORM_X + 200.f;
        mBalloonsSpawned = 0;
        mIsClassicMode = isClassicMode;
    }

    void update(float dt, float cameraRightEdge, float playerX) {
        for (auto& b : mBalloons) b->update(dt);

        mBalloons.erase(std::remove_if(mBalloons.begin(), mBalloons.end(),
            [&](const auto& b){ 
                return b->state() == Balloon::State::Done || b->position().x < playerX - 300.f; 
            }), mBalloons.end());

        // Генерируем шары, пока камера движется, НО если это классика - ограничиваем количество
        while (mNextX < cameraRightEdge + BALLOON_SPACING) {
            if (!mIsClassicMode || mBalloonsSpawned < CLASSIC_TARGET_BALLOONS) {
                spawnBalloon();
            } else {
                break; // Достигли лимита в классическом режиме
            }
        }

        markTarget(playerX);
    }

    void draw(sf::RenderWindow& window) const {
        for (const auto& b : mBalloons) b->draw(window);
    }

    Balloon* currentTarget(float playerX) {
        for (auto& b : mBalloons) {
            if (b->state() == Balloon::State::Idle && b->position().x > playerX + 10.f) {
                return b.get();
            }
        }
        return nullptr;
    }

    // Найти ближайший шар впереди для воскрешения
    Balloon* nextAvailableBalloon(float playerX) {
         for (auto& b : mBalloons) {
            if (b->position().x > playerX) {
                return b.get();
            }
        }
        return nullptr;
    }

    void increaseSpeed() { mFallSpeed = std::min(mFallSpeed + SPEED_INCREMENT, MAX_FALL_SPEED); }

    int balloonsSpawned() const { return mBalloonsSpawned; }
    bool hasTarget() const {
        for(auto& b: mBalloons) if (b->state() == Balloon::State::Idle) return true;
        return false;
    }

private:
    void spawnBalloon() {
        char letter = 'a' + (std::rand() % 26);
        mBalloons.push_back(std::make_unique<Balloon>(letter, mNextX, mFallSpeed, mRM));
        mNextX += BALLOON_SPACING;
        mBalloonsSpawned++;
    }

    void markTarget(float playerX) {
        Balloon* t = currentTarget(playerX);
        for (auto& b : mBalloons) b->setAsTarget(b.get() == t);
    }

    const ResourceManager& mRM;
    std::vector<std::unique_ptr<Balloon>> mBalloons;
    float mFallSpeed;
    float mNextX;
    int mBalloonsSpawned;
    bool mIsClassicMode;
};