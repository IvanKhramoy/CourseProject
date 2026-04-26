#pragma once
#include <SFML/Graphics.hpp>
#include "ResourceManager.h"
#include "StatsTracker.h"
#include "BalloonManager.h"
#include "Constants.h"
#include "Player.h"
#include "Background.h"

enum class GameState { Menu, Playing, GameOver, Win };
enum class GameMode { Classic, Endless };

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();

    void processEventsMenu(const sf::Event& event);
    void renderMenu();

    void processEventsPlaying(const sf::Event& event);
    void updatePlaying(float dt);
    void renderPlaying();
    void startGame(GameMode mode);

    void processEventsGameOver(const sf::Event& event);
    void renderGameOver();

    sf::Text makeText(const std::string& str, unsigned int size, sf::Color color, float x, float y);
    void drawHUD();

    const sf::Texture* mLastSafeTexture = nullptr;

    bool mIsFinishing = false;
    bool mWaitingForFinalJump = false;

    // --- НОВЫЕ МЕТОДЫ ДЛЯ МАСШТАБИРОВАНИЯ ---
    void adjustViewports(unsigned int width, unsigned int height);
    void toggleFullscreen();

    float mCenterTextTimer = 0.f;

    float mLastSafeX = PLATFORM_X;
    
    
    sf::RenderWindow   mWindow;
    sf::View           mWorldView;
    sf::View           mHudView;
    GameState          mState;
    GameMode           mCurrentMode;
    
    ResourceManager    mRM;
    StatsTracker       mStats;
    BalloonManager     mBalloons;
    Player             mPlayer;
    
    sf::RectangleShape mPlatform;
    // sf::Sprite         mCliffSprite; // ++
    Balloon* mCurrentBalloon = nullptr;
    
    // sf::Sprite mBgSprite;
    std::unique_ptr<Background> mBackground;
    std::unique_ptr<sf::Sprite> mCliffSprite;
    
    sf::Clock          mGameClock;
    sf::Clock          mBlinkClock;
    bool               mShowCursor = true;
    bool               mIsWaitingForTyping = false; 
    
    // Флаг полноэкранного режима
    bool               mIsFullscreen = false;

    // struct Particle {
    //     sf::Vector2f pos;
    //     sf::Vector2f vel;
    //     float lifetime;
    //     float maxLifetime;
    // };
    // std::vector<Particle> mParticles; // Список всех искр на экране

    struct Explosion {
        sf::Sprite sprite;
        float timer = 0.f;
        int currentFrame = 0;
        bool finished = false;
        explicit Explosion(const sf::Texture& texture) : sprite(texture) {}
    };
    std::vector<Explosion> mActiveExplosions;

    // Константы для анимации (подстрой под свой спрайт)
    const int EXPL_FRAMES = 12;      // Сколько всего кадров в картинке
    const int EXPL_SIZE   = 96;     // Размер одного кадра (квадрат)
    const float EXPL_SPEED = 0.05f;  // Скорость смены кадра (в секундах)
    
    float mRespawnTimer = 0.f; // Таймер задержки перед появлением
    bool mIsDeadWaiting = false; 
};