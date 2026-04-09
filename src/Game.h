#pragma once
#include <SFML/Graphics.hpp>
#include "Constants.h"
#include "ResourceManager.h"
#include "StatsTracker.h"
#include "BalloonManager.h"
#include "Player.h"

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

    sf::RenderWindow   mWindow;
    sf::View           mWorldView;
    sf::View           mHudView;
    GameState          mState;
    GameMode           mCurrentMode; // Текущий режим игры

    ResourceManager    mRM;
    StatsTracker       mStats;
    BalloonManager     mBalloons;
    Player             mPlayer;

    sf::RectangleShape mPlatform;
    Balloon* mCurrentBalloon = nullptr;

    sf::Clock          mGameClock;
    sf::Clock          mBlinkClock;
    bool               mShowCursor = true;
};