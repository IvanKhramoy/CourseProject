#include "Balloon.h"
#include "Constants.h"
#include "ResourceManager.h"

Balloon::Balloon(char letter, float x, float fallSpeed, const ResourceManager& rm)
    : mLetter(letter), mState(State::Idle), mFallSpeed(fallSpeed), mIsTarget(false), mText(rm.font("main")) 
{
    mShape.setRadius(BALLOON_RADIUS);
    mShape.setOrigin({BALLOON_RADIUS, BALLOON_RADIUS});
    mShape.setPosition({x, BALLOON_START_Y});
    mShape.setFillColor(randomColor());
    mShape.setOutlineThickness(3.f);
    mShape.setOutlineColor(sf::Color(255, 255, 255, 80));

    mText.setString(std::string(1, letter));
    mText.setCharacterSize(32);
    mText.setFillColor(sf::Color::White);
    mText.setStyle(sf::Text::Style::Bold);
    centerText();
}

void Balloon::update(float dt) {
    if (mState == State::Falling) {
        mShape.move({0.f, mFallSpeed * dt});
        mText.setPosition(mShape.getPosition());
        if (mShape.getPosition().y > WINDOW_H + BALLOON_RADIUS)
            mState = State::Done;
    }
}

void Balloon::draw(sf::RenderWindow& window) const {
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

void Balloon::startFalling() { mState = State::Falling; }
void Balloon::setState(State s) { mState = s; }
void Balloon::setAsTarget(bool t) { mIsTarget = t; }

sf::Color Balloon::randomColor() {
    static const sf::Color palette[] = {
        sf::Color(220, 80, 60),  sf::Color(230, 150, 40),
        sf::Color(200, 80, 160), sf::Color(60, 160, 220)
    };
    static int idx = 0;
    return palette[(idx++) % 4];
}

void Balloon::centerText() {
    sf::FloatRect b = mText.getLocalBounds();
    mText.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
    mText.setPosition(mShape.getPosition());
}