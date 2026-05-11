#pragma once
#include <SFML/Graphics.hpp>
#include "ResourceManager.h"
#include "StatsTracker.h"
#include "BalloonManager.h"
#include "HighscoreManager.h"
#include "Constants.h"
#include "Player.h"
#include "Background.h"
#include <SFML/Audio.hpp>

enum class GameState
{
    Menu,
    Playing,
    GameOver,
    Win
};
enum class GameMode
{
    Classic,
    Endless
};

class Game
{
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();

    void processEventsMenu(const sf::Event &event);
    void renderMenu();

    void processEventsPlaying(const sf::Event &event);
    void updatePlaying(float dt);
    void renderPlaying();
    void startGame(GameMode mode);

    void processEventsGameOver(const sf::Event &event);
    void renderGameOver();

    sf::Text makeText(const std::string &str, unsigned int size, sf::Color color, float x, float y);
    void drawHUD();

    const sf::Texture *mLastSafeTexture = nullptr;

    bool mIsFinishing = false;
    bool mWaitingForFinalJump = false;

    // --- НОВЫЕ МЕТОДЫ ДЛЯ МАСШТАБИРОВАНИЯ ---
    void adjustViewports(unsigned int width, unsigned int height);
    void toggleFullscreen();

    sf::Vector2f mBgOffset = {0.f, 0.f};

    float mMenuAnimTimer = 0.f;

    float mCenterTextTimer = 0.f;

    float mWinDelayTimer = 0.f;

    float mLastSafeX = PLATFORM_X;

    sf::Vector2f mMousePos;
    bool mIsMouseClicked = false;

    // Вспомогательная функция для создания прямоугольника со скругленными углами
    sf::ConvexShape createRoundedRect(float width, float height, float radius);

    // Функция, которая рисует кнопку и возвращает true, если по ней кликнули
    bool drawButton(const std::string &text, float x, float y, float width, float height, sf::Color bgColor, sf::Color textColor, sf::Color outlineColor);

    sf::RenderWindow mWindow;
    sf::View mWorldView;
    sf::View mHudView;
    GameState mState;
    GameMode mCurrentMode;

    ResourceManager mRM;
    StatsTracker mStats;
    BalloonManager mBalloons;
    Player mPlayer;

    sf::Music mMusicMenu;
    sf::Music mMusicGame;

    std::unique_ptr<sf::Sound> mExplosionSound;

    std::unique_ptr<sf::Sound> mCorrectSound;
    std::unique_ptr<sf::Sound> mErrorSound;

    // Метод для плавного обновления громкости
    void updateMusic(float dt);

    // Вспомогательная функция для переключения
    void playStateMusic(GameState state);

    sf::RectangleShape mPlatform;
    // sf::Sprite         mCliffSprite; // ++
    Balloon *mCurrentBalloon = nullptr;

    // sf::Sprite mBgSprite;
    std::unique_ptr<Background> mBackground;
    std::unique_ptr<sf::Sprite> mCliffSprite;

    std::unique_ptr<sf::Sprite> mMenuBgSprite;

    std::unique_ptr<sf::Sprite> mEndCliffSprite;
    bool mShowEndCliff = false;

    sf::Clock mGameClock;
    sf::Clock mBlinkClock;
    bool mShowCursor = true;
    bool mIsWaitingForTyping = false;

    float mFinalWaitTimer = 0.f;

    // Флаг полноэкранного режима
    bool mIsFullscreen = false;

    struct Particle
    {
        sf::Vector2f pos;
        sf::Vector2f vel;
        float lifetime;
    };
    std::vector<Particle> mParticles; // <--- Эта строка должна быть ТУТ!

    struct Explosion
    {
        sf::Sprite sprite;
        float timer = 0.f;
        int currentFrame = 0;
        bool finished = false;
        explicit Explosion(const sf::Texture &texture) : sprite(texture) {}
    };
    std::vector<Explosion> mActiveExplosions;

    // Константы для анимации (подстрой под свой спрайт)
    const int EXPL_FRAMES = 12;     // Сколько всего кадров в картинке
    const int EXPL_SIZE = 96;       // Размер одного кадра (квадрат)
    const float EXPL_SPEED = 0.05f; // Скорость смены кадра (в секундах)

    float mRespawnTimer = 0.f; // Таймер задержки перед появлением
    bool mIsDeadWaiting = false;

    float mLastDt = 0.f;
    std::map<std::string, float> mButtonHoverProgress;

    bool mIsNewRecord = false;
    bool mShowLeaderboard = false;
    std::vector<int> mTopScores;
};