#pragma once
#include <SFML/Graphics.hpp>
#include "Constants.h"
#include "ResourceManager.h"

class Balloon {
public:
    enum class State { Idle, Falling, Done };

    Balloon(char letter, float x, float fallSpeed, const ResourceManager& rm)
        : mLetter(letter), mState(State::Idle), mFallSpeed(fallSpeed), mIsTarget(false), mText(rm.font("main")) 
    {
        mShape.setRadius(BALLOON_RADIUS);
        mShape.setOrigin({BALLOON_RADIUS, BALLOON_RADIUS});
        mShape.setPosition({x, BALLOON_START_Y}); // Все шары появляются на одной высоте
        mShape.setFillColor(randomColor());
        mShape.setOutlineThickness(3.f);
        mShape.setOutlineColor(sf::Color(255, 255, 255, 80));

        mText.setString(std::string(1, letter));
        mText.setCharacterSize(32);
        mText.setFillColor(sf::Color::White);
        mText.setStyle(sf::Text::Style::Bold);
        centerText();
    }

    void update(float dt) {
        if (mState == State::Falling) {
            mShape.move({0.f, mFallSpeed * dt});
            mText.setPosition(mShape.getPosition());
            if (mShape.getPosition().y > WINDOW_H + BALLOON_RADIUS)
                mState = State::Done;
        }
    }

    void draw(sf::RenderWindow& window) const {
        if (mIsTarget) {
            sf::CircleShape glow(BALLOON_RADIUS + 8.f);
            glow.setOrigin({BALLOON_RADIUS + 8.f, BALLOON_RADIUS + 8.f});
            glow.setPosition(mShape.getPosition());
            glow.setFillColor(sf::Color(255, 255, 255, 50));
            window.draw(glow);
        }
        window.draw(mShape);
        window.draw(mText);
    }

    void startFalling() { mState = State::Falling; }
    void setState(State s) { mState = s; }
    void setAsTarget(bool t) { mIsTarget = t; }

    char letter() const { return mLetter; }
    State state() const { return mState; }
    sf::Vector2f position() const { return mShape.getPosition(); }

private:
    sf::Color randomColor() {
        static const sf::Color palette[] = {
            sf::Color(220, 80, 60),  sf::Color(230, 150, 40),
            sf::Color(200, 80, 160), sf::Color(60, 160, 220)
        };
        static int idx = 0;
        return palette[(idx++) % 4];
    }

    void centerText() {
        sf::FloatRect b = mText.getLocalBounds();
        mText.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
        mText.setPosition(mShape.getPosition());
    }

    char mLetter;
    State mState;
    float mFallSpeed;
    bool mIsTarget;
    sf::CircleShape mShape;
    sf::Text mText;
};