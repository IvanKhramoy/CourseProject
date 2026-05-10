#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <cmath>
#include "Constants.h"
#include "ResourceManager.h"

class Player
{
public:
    enum class State
    {
        OnPlatform,
        Jumping,
        OnBalloon,
        Falling,
        Respawning
    };

    // Теперь игрок принимает менеджер ресурсов
    explicit Player(const ResourceManager &rm);

    void reset(int startingLives);
    void update(float dt);
    void draw(sf::RenderWindow &window) const;

    void jumpTo(sf::Vector2f balloonPos);
    void respawnOn(sf::Vector2f balloonPos);
    void setBalloonPosition(sf::Vector2f balloonPos);
    void startFalling();

    void setFinalJump(bool final) { mIsFinalJump = final; }

    int lives() const { return mLives; }
    State state() const { return mState; }
    sf::Vector2f position() const { return mPos; }
    bool isAlive() const { return mLives > 0; }
    bool needsRespawn() const { return mState == State::Respawning; }

    void loseLife()
    {
        if (mLives > 0)
            mLives--;
    }

private:
    void updateAnimation(float dt);
    void setNinjaTexture(const std::string &textureId);

    bool mIsFinalJump = false;

    const ResourceManager &mRM;
    std::unique_ptr<sf::Sprite> mSprite; // Спрайт ниндзя

    sf::Vector2f mPos;
    sf::Vector2f mStart;
    sf::Vector2f mTarget;
    State mState;
    int mLives;
    float mJumpTimer;
    float mFallSpeed;
    float mAnimTimer; // Таймер для дыхания и раскачивания
};