#include "Player.h"
#include "Constants.h"

Player::Player() {
    mShape.setSize({PLAYER_WIDTH, PLAYER_HEIGHT});
    mShape.setOrigin({PLAYER_WIDTH / 2.f, PLAYER_HEIGHT / 2.f});
    mShape.setFillColor(sf::Color(60, 80, 180));
    mShape.setOutlineThickness(2.f);
    mShape.setOutlineColor(sf::Color::White);
    reset(CLASSIC_LIVES);
}

void Player::reset(int startingLives) {
    mState = State::OnPlatform;
    mLives = startingLives;
    mPos = {PLATFORM_X, PLATFORM_Y - PLAYER_HEIGHT / 2.f};
    mTarget = mPos;
    mJumpTimer = 0.f;
    mFallSpeed = 0.f;
    mShape.setPosition(mPos);
}

void Player::update(float dt) {
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
            if (mPos.y + PLAYER_HEIGHT / 2.f > FALL_LIMIT) {
                mLives--;
                if (mLives > 0) {
                    mState = State::Respawning;
                }
            }
            break;

        case State::Respawning:
            break;

        case State::OnBalloon:
        case State::OnPlatform:
            break;
    }
    mShape.setPosition(mPos);
}

void Player::draw(sf::RenderWindow& window) const {
    window.draw(mShape);
}

void Player::jumpTo(sf::Vector2f balloonPos) {
    if (mState == State::Jumping) return;
    mStart = mPos;
    mTarget = {balloonPos.x, balloonPos.y + BALLOON_RADIUS + PLAYER_HEIGHT / 2.f};
    mJumpTimer = 0.f;
    mState = State::Jumping;
}

void Player::respawnOn(sf::Vector2f balloonPos) {
    mPos = {balloonPos.x, balloonPos.y + BALLOON_RADIUS + PLAYER_HEIGHT / 2.f};
    mState = State::OnBalloon;
    mShape.setPosition(mPos);
}

void Player::setBalloonPosition(sf::Vector2f balloonPos) {
    if (mState == State::OnBalloon) {
        mPos = {balloonPos.x, balloonPos.y + BALLOON_RADIUS + PLAYER_HEIGHT / 2.f};
    }
}

void Player::startFalling() {
    if (mState == State::OnBalloon || mState == State::OnPlatform) {
        mState = State::Falling;
        mFallSpeed = 0.f;
    }
}