#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class Balloon; // Предварительное объявление
class ResourceManager;

class BalloonManager
{
public:
    explicit BalloonManager(const ResourceManager &rm);
    ~BalloonManager();
    void reset(bool isClassicMode);
    void update(float dt, float cameraRightEdge, float playerX);
    void draw(sf::RenderWindow &window) const;
    void setAllowedChars(const std::vector<std::uint32_t>& chars) { mPool = chars; }

    Balloon *currentTarget(float playerX);
    Balloon *nextAvailableBalloon(float playerX);

    void forceSpawnBalloonAt(float x); // <-- Исправление: экстренный спавн
    void increaseSpeed();

    Balloon* spawnRespawnPlatform(float x, float y,  const sf::Texture& tex);    

    int balloonsSpawned() const { return mBalloonsSpawned; }
    bool hasTarget() const;
    // +
    bool isValid(const Balloon *b) const;

private:
    void spawnBalloon();
    void markTarget(float playerX);

    const ResourceManager &mRM;
    std::vector<std::unique_ptr<Balloon>> mBalloons;
    float mFallSpeed;
    float mNextX;
    int mBalloonsSpawned;
    bool mIsClassicMode;

    std::vector<std::uint32_t> mPool = {'a'};
};