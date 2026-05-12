#include "Balloon.h"
#include "Constants.h"
#include "ResourceManager.h"

Balloon::Balloon(std::uint32_t letter, float x, float y, float fallSpeed, const sf::Texture &texture, const ResourceManager &rm)
    : mLetter(letter), mState(State::Idle), mFallSpeed(fallSpeed), mIsTarget(false), mSprite(texture) // Инициализируем спрайт текстурой (SFML 3)
      ,
      mText(rm.font("main"))
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
    // mText.setString(std::string(1, letter));
   mText.setString(sf::String(static_cast<char32_t>(letter)));

    centerText();

    mGlow.setPrimitiveType(sf::PrimitiveType::TriangleFan);
    // 33 вершины: 1 центр + 32 луча вокруг
    mGlow.resize(34);

    // Стрелочки (рисуем треугольники)
    auto initArrow = [&](sf::ConvexShape &s)
    {
        s.setPointCount(3);
        s.setPoint(0, {0.f, 0.f});
        s.setPoint(1, {15.f, 7.5f});
        s.setPoint(2, {0.f, 15.f});
        s.setFillColor(sf::Color(180, 255, 100)); // Салатовый цвет
        s.setOrigin({7.5f, 7.5f});
    };
    initArrow(mArrowL);
    initArrow(mArrowR);
}

void Balloon::update(float dt)
{
    if (mIsTarget)
        mTargetAnimTimer += dt;
    if (mState == State::Falling)
    {
        mSprite.move({0.f, mFallSpeed * dt});
        mText.setPosition(mSprite.getPosition());
        if (mSprite.getPosition().y > WINDOW_H + (BALLOON_RADIUS * 3.f)) // Даем шару с корзиной полностью улететь
            mState = State::Done;
    }
}

void Balloon::draw(sf::RenderWindow &window) 
{
    if (mState == State::Done)
        return;

    if (mIsTarget) {
        float radius = BALLOON_RADIUS * 1.4f;
        
        mGlow[0].position = mSprite.getPosition();
        mGlow[0].color = sf::Color(255, 215, 0, 150);

        for (int i = 0; i <= 33; ++i) {
            float angle = i * (6.2831853f / 32.f) + (mTargetAnimTimer * 0.3f); 
            float r = (i % 2 == 0) ? radius : radius * 0.6f; 
            
            sf::Vector2f pos = {
                mSprite.getPosition().x + r * std::cos(angle),
                mSprite.getPosition().y + r * std::sin(angle)
            };
            
            mGlow[i + 1].position = pos;
            mGlow[i + 1].color = sf::Color(100, 200, 255, 0); 
        }
        window.draw(mGlow);

        float wave = std::sin(mTargetAnimTimer * 3.0f) * 1.5f; 
        int alpha = static_cast<int>(150 + 105 * std::sin(mTargetAnimTimer * 2.0f)); 

        mArrowL.setFillColor(sf::Color(180, 255, 100, alpha));
        mArrowL.setPosition(mSprite.getPosition() + sf::Vector2f(-BALLOON_RADIUS - 12.f - wave, 0.f));
        
        mArrowR.setScale({-1.f, 1.f});
        mArrowR.setFillColor(sf::Color(180, 255, 100, alpha));
        mArrowR.setPosition(mSprite.getPosition() + sf::Vector2f(BALLOON_RADIUS + 12.f + wave, 0.f));

        window.draw(mArrowL);
        window.draw(mArrowR);
    }

    window.draw(mSprite);
    window.draw(mText);
}

void Balloon::startFalling() { mState = State::Falling; }
void Balloon::setState(State s) { mState = s; }
void Balloon::setAsTarget(bool t) { mIsTarget = t; }

void Balloon::centerText()
{
    sf::FloatRect b = mText.getLocalBounds();
    // Чуть-чуть приподнимаем текст (-5.f), чтобы он был ровно по центру надувной части
    mText.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f + 5.f});
    // mSprite.getPosition() + sf::Vector2f(0.f, -10.f);
    mText.setPosition(mSprite.getPosition());
}
