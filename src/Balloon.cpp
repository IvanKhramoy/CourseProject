#include "Balloon.h"
#include "Constants.h"
#include "ResourceManager.h"

Balloon::Balloon(char letter, float x, float y, float fallSpeed, const sf::Texture& texture, const ResourceManager& rm)
    : mLetter(letter), mState(State::Idle), mFallSpeed(fallSpeed), mIsTarget(false)
    , mSprite(texture) // Инициализируем спрайт текстурой (SFML 3)
    , mText(rm.font("main")) 
{
    // 1. Масштабируем картинку шара под константу BALLOON_RADIUS
    // Мы хотим, чтобы ширина картинки стала равна диаметру (радиус * 2)
    float scale = (BALLOON_RADIUS * 2.f) / mSprite.getLocalBounds().size.x;
    mSprite.setScale({scale, scale});

    // 2. Устанавливаем Origin
    // X - по центру ширины. 
    // Y - по центру САМОГО ШАРА (без корзины). Если шар занимает верхнюю половину картинки, 
    // то центр шара будет примерно на 1/3 или 1/4 высоты всей картинки.
    // Если текст или ниндзя съезжают, поменяй второе значение (например, вместо ...size.x / 2.f напиши просто 40.f)
    mSprite.setOrigin({mSprite.getLocalBounds().size.x / 2.f, mSprite.getLocalBounds().size.x / 2.f});
    
    mSprite.setPosition({x, y});

    // ++++
    mText.setFont(rm.font("main")); 

    // 2. Делаем шрифт очень крупным (чтобы был четким)'
    mText.setCharacterSize(80); 

    // 3. Сжимаем его визуально в 3 раза (метод сверхвысокой четкости)
    mText.setScale({0.38f, 0.38f}); 

    mText.setFillColor(sf::Color::White);
    mText.setStyle(sf::Text::Style::Bold);

    // 3. Настраиваем текст
    mText.setString(std::string(1, letter));
    // mText.setCharacterSize(80);
    // mText.setCharacterSize(26);
    // mText.setScale({0.35f, 0.35f});
    // mText.setFillColor(sf::Color::White);
    // mText.setStyle(sf::Text::Style::Bold);
    centerText();
}

void Balloon::update(float dt) {
    if (mState == State::Falling) {
        mSprite.move({0.f, mFallSpeed * dt});
        mText.setPosition(mSprite.getPosition());
        if (mSprite.getPosition().y > WINDOW_H + (BALLOON_RADIUS * 3.f)) // Даем шару с корзиной полностью улететь
            mState = State::Done;
    }
}

void Balloon::draw(sf::RenderWindow& window) const {
    if (mState == State::Done) return;
    if (mIsTarget) {
        // Оставляем небольшое круглое свечение позади шара-цели
        sf::CircleShape glow(BALLOON_RADIUS + 6.f);
        glow.setOrigin({BALLOON_RADIUS + 6.f, BALLOON_RADIUS + 6.f});
        glow.setPosition(mSprite.getPosition());
        glow.setFillColor(sf::Color(255, 255, 255, 70));
        window.draw(glow);
    }
    window.draw(mSprite);
    window.draw(mText);
}

void Balloon::startFalling() { mState = State::Falling; }
void Balloon::setState(State s) { mState = s; }
void Balloon::setAsTarget(bool t) { mIsTarget = t; }

void Balloon::centerText() {
    sf::FloatRect b = mText.getLocalBounds();
    // Чуть-чуть приподнимаем текст (-5.f), чтобы он был ровно по центру надувной части
    mText.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f + 5.f});
    // mSprite.getPosition() + sf::Vector2f(0.f, -10.f);
    mText.setPosition(mSprite.getPosition());
}
