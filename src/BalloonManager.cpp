#include "BalloonManager.h"
#include "Balloon.h"
#include "Constants.h"
#include "ResourceManager.h"
#include <algorithm>
#include <cstdlib>

BalloonManager::~BalloonManager() = default;

BalloonManager::BalloonManager(const ResourceManager& rm)
    : mRM(rm), mFallSpeed(INITIAL_FALL_SPEED), mNextX(PLATFORM_X + BALLOON_SPACING * 1.5f), mBalloonsSpawned(0), mIsClassicMode(true) {}

void BalloonManager::reset(bool isClassicMode) {
    mBalloons.clear();
    mFallSpeed = INITIAL_FALL_SPEED;
    // И здесь тоже заменяем:
    mNextX = PLATFORM_X + BALLOON_SPACING * 1.5f;
    mBalloonsSpawned = 0;
    mIsClassicMode = isClassicMode;
}

void BalloonManager::update(float dt, float cameraRightEdge, float playerX) {
    for (auto& b : mBalloons) b->update(dt);

    mBalloons.erase(std::remove_if(mBalloons.begin(), mBalloons.end(),
        [&](const auto& b){ 
            return b->state() == Balloon::State::Done || b->position().x < playerX - 800.f; 
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

void BalloonManager::forceSpawnBalloonAt(float x) {
    char letter = 'a' + (std::rand() % 26);
    
    int colorIndex = std::rand() % Paths::BALLOON_COLORS_COUNT;
    const sf::Texture& tex = mRM.texture("balloon_" + std::to_string(colorIndex));

    float randomOffset = static_cast<float>((std::rand() % (BALLOON_Y_VARIATION * 2)) - BALLOON_Y_VARIATION);
    float spawnY = BALLOON_START_Y + randomOffset;
    
    mBalloons.push_back(std::make_unique<Balloon>(letter, x, spawnY, mFallSpeed, tex, mRM));
    
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
    
    // Выбираем случайный цвет от 0 до BALLOON_COLORS_COUNT-1
    int colorIndex = std::rand() % Paths::BALLOON_COLORS_COUNT;
    const sf::Texture& tex = mRM.texture("balloon_" + std::to_string(colorIndex));

    float randomOffset = static_cast<float>((std::rand() % (BALLOON_Y_VARIATION * 2)) - BALLOON_Y_VARIATION);
    float spawnY = BALLOON_START_Y + randomOffset;
    
    // Передаем текстуру
    mBalloons.push_back(std::make_unique<Balloon>(letter, mNextX, spawnY, mFallSpeed, tex, mRM));
    
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

Balloon* BalloonManager::spawnRespawnPlatform(float x, float y, const sf::Texture& tex) {
    // Используем переданную текстуру tex вместо стандартной balloon_0
    auto platform = std::make_unique<Balloon>('\0', x, y, 0.f, tex, mRM);
    platform->clearLetter();
    
    Balloon* ptr = platform.get();
    mBalloons.push_back(std::move(platform));
    return ptr;
}