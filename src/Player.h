#pragma once
#include <SFML/Graphics.hpp>

class Player {
public:
    enum class State { OnPlatform, Jumping, OnBalloon, Falling, Respawning };

    Player();

    void reset(int startingLives);
    void update(float dt);
    void draw(sf::RenderWindow& window) const;

    void jumpTo(sf::Vector2f balloonPos);
    void respawnOn(sf::Vector2f balloonPos);
    void setBalloonPosition(sf::Vector2f balloonPos);
    void startFalling();

    int lives() const { return mLives; }
    State state() const { return mState; }
    sf::Vector2f position() const { return mPos; }
    bool isAlive() const { return mLives > 0; }
    bool needsRespawn() const { return mState == State::Respawning; }

private:
    sf::RectangleShape mShape;
    sf::Vector2f mPos;
    sf::Vector2f mStart;
    sf::Vector2f mTarget;
    State mState;
    int mLives;
    float mJumpTimer;
    float mFallSpeed;
};