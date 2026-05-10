#pragma once
#include <SFML/Graphics.hpp>

class ResourceManager;

class Balloon
{
public:
    enum class State
    {
        Idle,
        Falling,
        Done
    };

    // Теперь конструктор принимает готовую текстуру
    Balloon(char letter, float x, float y, float fallSpeed, const sf::Texture &texture, const ResourceManager &rm);

    void update(float dt);
    void draw(sf::RenderWindow &window);

    void startFalling();
    void setState(State s);
    void setAsTarget(bool t);

    void clearLetter()
    {
        mLetter = '\0';
        mText.setString("");
    }

    char letter() const { return mLetter; }
    State state() const { return mState; }
    sf::Vector2f position() const { return mSprite.getPosition(); }
    // Возвращает ссылку на текущую текстуру спрайта
    const sf::Texture &getTexture() const { return mSprite.getTexture(); }

private:
    void centerText();

    char mLetter;
    State mState;
    float mFallSpeed;
    bool mIsTarget;

    sf::Sprite mSprite; // Спрайт картинки
    sf::Text mText;

    // sf::CircleShape mGlow;     // Вместо спрайта свечения
    sf::VertexArray mGlow;
    sf::ConvexShape mArrowL;   // Левая стрелка
    sf::ConvexShape mArrowR;   // Правая стрелка
    float mTargetAnimTimer = 0.f; // Таймер для пульсации
};

