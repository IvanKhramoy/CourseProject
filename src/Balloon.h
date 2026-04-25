#pragma once
#include <SFML/Graphics.hpp>

class ResourceManager;

class Balloon {
public:
    enum class State { Idle, Falling, Done };

    // Теперь конструктор принимает готовую текстуру
    Balloon(char letter, float x, float y, float fallSpeed, const sf::Texture& texture, const ResourceManager& rm);

    void update(float dt);
    void draw(sf::RenderWindow& window) const;

    void startFalling();
    void setState(State s);
    void setAsTarget(bool t);

    char letter() const { return mLetter; }
    State state() const { return mState; }
    sf::Vector2f position() const { return mSprite.getPosition(); }

private:
    void centerText();

    char mLetter;
    State mState;
    float mFallSpeed;
    bool mIsTarget;
    
    sf::Sprite mSprite; // Спрайт картинки
    sf::Text mText;
};



// #pragma once
// #include <SFML/Graphics.hpp>

// class ResourceManager; // Предварительное объявление, чтобы не инклудить весь хедер

// class Balloon {
// public:
//     enum class State { Idle, Falling, Done };

//     Balloon(char letter, float x, float y, float fallSpeed, const ResourceManager& rm);

//     void update(float dt);
//     void draw(sf::RenderWindow& window) const;

//     void startFalling();
//     void setState(State s);
//     void setAsTarget(bool t);

//     char letter() const { return mLetter; }
//     State state() const { return mState; }
//     sf::Vector2f position() const { return mShape.getPosition(); }

// private:
//     sf::Color randomColor();
//     void centerText();

//     char mLetter;
//     State mState;
//     float mFallSpeed;
//     bool mIsTarget;
//     sf::CircleShape mShape;
//     sf::Text mText;
// };