#include "BalloonManager.h"
#include "Balloon.h"
#include "Constants.h"
#include <algorithm>
#include <cstdlib>

BalloonManager::~BalloonManager() = default;

BalloonManager::BalloonManager(const ResourceManager& rm)
    : mRM(rm), mFallSpeed(INITIAL_FALL_SPEED), mNextX(PLATFORM_X + 200.f), mBalloonsSpawned(0), mIsClassicMode(true) {}

void BalloonManager::reset(bool isClassicMode) {
    mBalloons.clear();
    mFallSpeed = INITIAL_FALL_SPEED;
    mNextX = PLATFORM_X + 200.f;
    mBalloonsSpawned = 0;
    mIsClassicMode = isClassicMode;
}

void BalloonManager::update(float dt, float cameraRightEdge, float playerX) {
    for (auto& b : mBalloons) b->update(dt);

    mBalloons.erase(std::remove_if(mBalloons.begin(), mBalloons.end(),
        [&](const auto& b){ 
            return b->state() == Balloon::State::Done || b->position().x < playerX - 300.f; 
        }), mBalloons.end());

    while (mNextX < cameraRightEdge + BALLOON_SPACING) {
        if (!mIsClassicMode || mBalloonsSpawned < CLASSIC_TARGET_BALLOONS) {
            spawnBalloon();
        } else {
            break; 
        }
    }

    markTarget(playerX);
}

void BalloonManager::draw(sf::RenderWindow& window) const {
    for (const auto& b : mBalloons) b->draw(window);
}

Balloon* BalloonManager::currentTarget(float playerX) {
    for (auto& b : mBalloons) {
        if (b->state() == Balloon::State::Idle && b->position().x > playerX + 10.f) {
            return b.get();
        }
    }
    return nullptr;
}

Balloon* BalloonManager::nextAvailableBalloon(float playerX) {
    for (auto& b : mBalloons) {
        if (b->position().x > playerX) {
            return b.get();
        }
    }
    return nullptr;
}

// Экстренный спавн шарика на случай, если игрок умер, а впереди пусто
void BalloonManager::forceSpawnBalloonAt(float x) {
    char letter = 'a' + (std::rand() % 26);
    mBalloons.push_back(std::make_unique<Balloon>(letter, x, mFallSpeed, mRM));
    mNextX = x + BALLOON_SPACING;
}

void BalloonManager::increaseSpeed() { 
    mFallSpeed = std::min(mFallSpeed + SPEED_INCREMENT, MAX_FALL_SPEED); 
}

bool BalloonManager::hasTarget() const {
    for(const auto& b: mBalloons) {
        if (b->state() == Balloon::State::Idle) return true;
    }
    return false;
}

void BalloonManager::spawnBalloon() {
    char letter = 'a' + (std::rand() % 26);
    mBalloons.push_back(std::make_unique<Balloon>(letter, mNextX, mFallSpeed, mRM));
    mNextX += BALLOON_SPACING;
    mBalloonsSpawned++;
}

void BalloonManager::markTarget(float playerX) {
    Balloon* t = currentTarget(playerX);
    for (auto& b : mBalloons) b->setAsTarget(b.get() == t);
}
// +
bool BalloonManager::isValid(const Balloon* b) const {
    if (!b) return false;
    for (const auto& balloon : mBalloons) {
        if (balloon.get() == b) return true;
    }
    return false;
}