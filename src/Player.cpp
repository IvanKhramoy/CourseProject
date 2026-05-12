#include "Player.h"

Player::Player(const ResourceManager &rm)
    : mRM(rm), mAnimTimer(0.f)
{
    // Текстуру зададим при вызове reset()
    reset(CLASSIC_LIVES);
}

void Player::reset(int startingLives)
{
    mState = State::OnPlatform;
    mLives = startingLives;
    mPos = {PLATFORM_X, PLATFORM_Y - PLAYER_HEIGHT / 2.f};
    mTarget = mPos;
    mJumpTimer = 0.f;
    mFallSpeed = 0.f;
    mAnimTimer = 0.f;
    mIsFinalJump = false;

}

void Player::update(float dt)
{
    switch (mState)
    {

    case State::Jumping:
        mJumpTimer += dt;
        {
            float t = mJumpTimer / JUMP_DURATION;
            if (t >= 1.f)
            {
                t = 1.f;
                if (mIsFinalJump)
                {
                    mState = State::OnPlatform;
                }
                else
                {
                    mState = State::OnBalloon;
                }
            }

            // 1. Линейная часть (движение из А в Б)
            mPos = mStart + (mTarget - mStart) * t;

            // 2. ПАРАБОЛИЧЕСКАЯ ЧАСТЬ (Арка)
            // Чем больше число (например, 100.f), тем выше прыгает ниндзя.
            float arcHeight = 120.f;

            // Математика: sin(0) = 0, sin(pi/2) = 1 (пик), sin(pi) = 0
            // Это создает идеальный горб прыжка
            float offset = std::sin(t * 3.14159f) * arcHeight;

            mPos.y -= offset; // Вычитаем, так как Y в SFML растет вниз
        }
        break;

    case State::Falling:
        mFallSpeed += 500.f * dt;
        mPos.y += mFallSpeed * dt;
        // if (mPos.y + PLAYER_HEIGHT / 2.f > FALL_LIMIT)
        // {
        //     mLives--;
        //     if (mLives > 0)
        //     {
        //         mState = State::Respawning;
        //     }
        // }
        break;

    case State::Respawning:
    case State::OnBalloon:
    case State::OnPlatform:
        break;
    }

    // Анимация и смена картинок происходит здесь
    updateAnimation(dt);
}

void Player::draw(sf::RenderWindow &window) const
{
    if (mSprite && mState != State::Respawning)
    {
        window.draw(*mSprite);
    }
}

// --- ЛОГИКА АНИМАЦИИ ---
void Player::updateAnimation(float dt)
{
    mAnimTimer += dt;

    if (mErrorPulseTimer > 0.f)
        mErrorPulseTimer -= dt;

    float pulse = 1.0f;
    if (mErrorPulseTimer > 0.f)
    {
        // Синус от 0 до Пи дает идеальный "горб"
        pulse = 1.0f + 0.2f * std::sin((mErrorPulseTimer / 0.15f) * 3.14159f);
    }

    std::string currentTexture = "ninja-1"; // По умолчанию

    // 1. ВЫБИРАЕМ КАРТИНКУ В ЗАВИСИМОСТИ ОТ СОСТОЯНИЯ
    if (mState == State::OnPlatform)
    {
        currentTexture = "ninja-1";
    }
    else if (mState == State::OnBalloon || mState == State::Falling)
    {
        currentTexture = "ninja-4"; // Висим или падаем в сгруппированной позе
    }
    else if (mState == State::Jumping)
    {
        // РАЗБИВАЕМ ПРЫЖОК НА 5 ФАЗ (Твой сценарий 2-3-4-5-4)
        float t = mJumpTimer / JUMP_DURATION; // t от 0.0 до 1.0

        if (t < 0.2f)
            currentTexture = "ninja-2"; // 0-20% прыжка
        else if (t < 0.4f)
            currentTexture = "ninja-3"; // 20-40%
        else if (t < 0.6f)
            currentTexture = "ninja-4"; // 40-60%
        else if (t < 0.8f)
            currentTexture = "ninja-5"; // 60-80%
        else
            currentTexture = "ninja-4"; // 80-100%
    }

    // Применяем текстуру
    setNinjaTexture(currentTexture);

    // 2. ПРОЦЕДУРНАЯ АНИМАЦИЯ (Живое поведение)
    if (mSprite)
    {
        // Сброс трансформаций перед применением новых
        mSprite->setRotation(sf::degrees(0.f));

        float perspectiveScale = 1.0f;

        if (mIsFinalJump)
        {
            float t = mJumpTimer / JUMP_DURATION;
            // Уменьшаем на 25% (с 1.0 до 0.75). Можешь менять 0.25f для силы эффекта.
            float shrinkStrength = 0.25f;

            if (mState == State::Jumping)
            {
                perspectiveScale = 1.0f - (shrinkStrength * t);
            }
            else
            {
                // Если уже приземлился на финальную скалу, фиксируем уменьшенный размер
                perspectiveScale = 1.0f - shrinkStrength;
            }
        }

        mSprite->setScale({
            mSprite->getScale().x * perspectiveScale * pulse,  // <--- Добавили * pulse
            mSprite->getScale().y * perspectiveScale * pulse   // <--- Добавили * pulse
        });

        // mSprite->setScale({mSprite->getScale().x * perspectiveScale,
        //                    mSprite->getScale().y * perspectiveScale});

        if (mState == State::OnPlatform)
        {
            // Дыхание (сжатие/растяжение по вертикали на 2%)
            float breathe = 1.0f + 0.02f * std::sin(mAnimTimer * 4.0f);
            mSprite->setScale({mSprite->getScale().x, mSprite->getScale().y * breathe});
        }
        else if (mState == State::OnBalloon)
        {
            // Раскачивание на шаре (наклон влево-вправо на 3 градуса)
            float swing = 2.f * std::sin(mAnimTimer * 3.0f);
            mSprite->setRotation(sf::degrees(swing));
        }
    }
}

// Вспомогательная функция для установки картинки и правильного центрирования
void Player::setNinjaTexture(const std::string &textureId)
{
    const sf::Texture &tex = mRM.texture(textureId);

    if (!mSprite)
    {
        mSprite = std::make_unique<sf::Sprite>(tex);
    }
    else
    {
        mSprite->setTexture(tex, true); // true = сбросить размер под новую картинку
    }

    sf::FloatRect bounds = mSprite->getLocalBounds();

    // Масштабируем так, чтобы высота любой картинки равнялась PLAYER_HEIGHT из констант
    float visualMultiplier = 1.3f;
    float scale = PLAYER_HEIGHT / bounds.size.y * visualMultiplier;
    mSprite->setScale({scale, scale});

    // Центрируем Origin по центру "тела"
    mSprite->setOrigin({bounds.size.x / 2.f, bounds.size.y / 2.f});

    // Устанавливаем позицию
    mSprite->setPosition(mPos);
}

// --- ОСТАЛЬНЫЕ МЕТОДЫ (Без изменений) ---
void Player::jumpTo(sf::Vector2f balloonPos)
{
    if (mState == State::Jumping)
        return;
    mStart = mPos;
    mTarget = {balloonPos.x, balloonPos.y + BALLOON_RADIUS + PLAYER_HEIGHT / 2.f + 40.f};
    mJumpTimer = 0.f;
    mState = State::Jumping;
}

void Player::respawnOn(sf::Vector2f balloonPos)
{
    mPos = {balloonPos.x, balloonPos.y + BALLOON_RADIUS + PLAYER_HEIGHT / 2.f + 40.f};
    mState = State::OnBalloon;
    mSprite->setPosition(mPos);
}

void Player::setBalloonPosition(sf::Vector2f balloonPos)
{
    if (mState == State::OnBalloon)
    {
        mPos = {balloonPos.x, balloonPos.y + BALLOON_RADIUS + PLAYER_HEIGHT / 2.f + 40.f};
    }
}

void Player::startFalling()
{
    if (mState == State::OnBalloon || mState == State::OnPlatform)
    {
        mState = State::Falling;
        mFallSpeed = 0.f;
    }
}