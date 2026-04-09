#pragma once
#include <SFML/Graphics.hpp>
#include "Constants.h"

class Player {
public:
    enum class State { OnPlatform, Jumping, OnBalloon, Falling, Respawning };

    Player() {
        mShape.setSize({PLAYER_WIDTH, PLAYER_HEIGHT});
        mShape.setOrigin({PLAYER_WIDTH / 2.f, PLAYER_HEIGHT / 2.f});
        mShape.setFillColor(sf::Color(60, 80, 180));
        mShape.setOutlineThickness(2.f);
        mShape.setOutlineColor(sf::Color::White);
        reset(CLASSIC_LIVES); // По умолчанию
    }

    void reset(int startingLives) {
        mState      = State::OnPlatform;
        mLives      = startingLives;
        mPos        = {PLATFORM_X, PLATFORM_Y - PLAYER_HEIGHT / 2.f};
        mTarget     = mPos;
        mJumpTimer  = 0.f;
        mFallSpeed  = 0.f;
        mShape.setPosition(mPos);
    }

    void update(float dt) {
        switch (mState) {
            case State::Jumping:
                mJumpTimer += dt;
                {
                    float t = mJumpTimer / JUMP_DURATION;
                    if (t >= 1.f) {
                        t = 1.f;
                        mState = State::OnBalloon;
                    }
                    mPos = mStart + (mTarget - mStart) * t;
                }
                break;

            case State::Falling:
                mFallSpeed += 500.f * dt;
                mPos.y += mFallSpeed * dt;
                // Проверяем границу падения
                if (mPos.y + PLAYER_HEIGHT / 2.f > FALL_LIMIT) {
                    mLives--;
                    if (mLives > 0) {
                        mState = State::Respawning; // Ждем воскрешения
                    }
                }
                break;
            
            case State::Respawning:
                // Здесь можно добавить анимацию взрыва в будущем
                break;

            case State::OnBalloon:
            case State::OnPlatform:
                break;
        }
        mShape.setPosition(mPos);
    }

    void draw(sf::RenderWindow& window) const { window.draw(mShape); }

    void jumpTo(sf::Vector2f balloonPos) {
        if (mState == State::Jumping) return;
        mStart     = mPos;
        mTarget    = {balloonPos.x, balloonPos.y + BALLOON_RADIUS + PLAYER_HEIGHT / 2.f};
        mJumpTimer = 0.f;
        mState     = State::Jumping;
    }

    void respawnOn(sf::Vector2f balloonPos) {
        mPos = {balloonPos.x, balloonPos.y + BALLOON_RADIUS + PLAYER_HEIGHT / 2.f};
        mState = State::OnBalloon;
        mShape.setPosition(mPos);
    }

    void setBalloonPosition(sf::Vector2f balloonPos) {
        if (mState == State::OnBalloon) {
            mPos = {balloonPos.x, balloonPos.y + BALLOON_RADIUS + PLAYER_HEIGHT / 2.f};
        }
    }

    void startFalling() {
        if (mState == State::OnBalloon || mState == State::OnPlatform) {
            mState     = State::Falling;
            mFallSpeed = 0.f;
        }
    }

    int         lives()    const { return mLives;  }
    State       state()    const { return mState;  }
    sf::Vector2f position() const { return mPos;   }
    bool        isAlive()  const { return mLives > 0; }
    bool        needsRespawn() const { return mState == State::Respawning; }

private:
    sf::RectangleShape mShape;
    sf::Vector2f       mPos;
    sf::Vector2f       mStart;
    sf::Vector2f       mTarget;
    State              mState;
    int                mLives;
    float              mJumpTimer;
    float              mFallSpeed;
};