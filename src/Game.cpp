#include "Game.h"
#include "Constants.h"
#include "Balloon.h"
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <SFML/System/Time.hpp>

Game::Game()
    : mWindow(sf::VideoMode({WINDOW_W, WINDOW_H}), "BalloonTyper", sf::Style::Close)
    , mState(GameState::Menu)
    , mCurrentMode(GameMode::Classic)
    , mBalloons(mRM)
{
    mWindow.setFramerateLimit(FPS_LIMIT);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    mWorldView.setSize({(float)WINDOW_W, (float)WINDOW_H});
    mHudView.setSize({(float)WINDOW_W, (float)WINDOW_H});
    mHudView.setCenter({WINDOW_W / 2.f, WINDOW_H / 2.f});

    mPlatform.setSize({150.f, 20.f});
    mPlatform.setOrigin({75.f, 10.f});
    mPlatform.setFillColor(sf::Color(80, 200, 120));

    try { mRM.loadFont("main", Paths::FONT); } 
    catch (...) { mRM.loadFont("main", Paths::FONT_FALLBACK); }
}

void Game::run() {
    sf::Clock clock;
    while (mWindow.isOpen()) {
        float dt = clock.restart().asSeconds();
        
        // <-- ИСПРАВЛЕНИЕ 1: Ограничиваем dt, чтобы игрок не провалился сквозь пол при лаге
        if (dt > 0.1f) dt = 0.1f; 

        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents() {
    while (const std::optional event = mWindow.pollEvent()) {
        if (event->is<sf::Event::Closed>()) mWindow.close();

        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                if (mState == GameState::Playing || mState == GameState::GameOver || mState == GameState::Win) {
                     mState = GameState::Menu;
                } else if (mState == GameState::Menu) {
                     mWindow.close();
                }
            }
        }

        switch (mState) {
            case GameState::Menu:     processEventsMenu(*event);     break;
            case GameState::Playing:  processEventsPlaying(*event);  break;
            case GameState::GameOver: 
            case GameState::Win:      processEventsGameOver(*event); break;
        }
    }
}

void Game::processEventsMenu(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Num1) {
            startGame(GameMode::Classic);
        } else if (keyPressed->code == sf::Keyboard::Key::Num2) {
             startGame(GameMode::Endless);
        }
    }
}

void Game::processEventsPlaying(const sf::Event& event) {
    if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
        if (textEntered->unicode < 128) {
            char c = static_cast<char>(textEntered->unicode);
            if (std::isalpha(static_cast<unsigned char>(c))) {
                char typed = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                
                Balloon* target = mBalloons.currentTarget(mPlayer.position().x);
                if (target) {
                    if (target->letter() == typed) {
                        mStats.recordHit();
                        mBalloons.increaseSpeed();
                        
                        if (mCurrentBalloon) mCurrentBalloon->setState(Balloon::State::Done);
                        
                        mCurrentBalloon = target;
                        mCurrentBalloon->startFalling();
                        
                        if(mPlayer.state() == Player::State::OnBalloon && !mCurrentBalloon) {
                             mPlayer.setBalloonPosition(target->position());
                        } else {
                            mPlayer.jumpTo(target->position());
                        }

                    } else {
                        mStats.recordMiss();
                        if (mPlayer.state() == Player::State::OnBalloon) mPlayer.startFalling();
                    }
                }
            }
        }
    }
}

void Game::processEventsGameOver(const sf::Event& event) {
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->code == sf::Keyboard::Key::Enter) {
            startGame(mCurrentMode);
        }
    }
}

void Game::update(float dt) {
    if (mBlinkClock.getElapsedTime().asSeconds() > 0.5f) {
        mShowCursor = !mShowCursor;
        mBlinkClock.restart();
    }
    if (mState == GameState::Playing) {
        mStats.updateTime();
        updatePlaying(dt);
    }
}

void Game::updatePlaying(float dt) {
    float targetCamX = mPlayer.position().x + WINDOW_W / 6.f;
    targetCamX = std::max(targetCamX, WINDOW_W / 2.f);
    mWorldView.setCenter({targetCamX, WINDOW_H / 2.f});

    float cameraRightEdge = mWorldView.getCenter().x + WINDOW_W / 2.f;

    mBalloons.update(dt, cameraRightEdge, mPlayer.position().x);
    mPlayer.update(dt);

    if (mCurrentBalloon && mPlayer.state() == Player::State::OnBalloon) {
        mPlayer.setBalloonPosition(mCurrentBalloon->position());
        if (mCurrentBalloon->state() == Balloon::State::Done) {
            mPlayer.startFalling();
            mCurrentBalloon = nullptr;
        }
    }

    // <-- ИСПРАВЛЕНИЕ 2: Безопасное воскрешение
    if (mPlayer.needsRespawn()) {
        Balloon* nextB = mBalloons.nextAvailableBalloon(mPlayer.position().x);
        
        if (!nextB) {
            // Если мы умерли, а шаров впереди нет, создаем "спасательный" шар
            mBalloons.forceSpawnBalloonAt(mPlayer.position().x + 200.f);
            nextB = mBalloons.nextAvailableBalloon(mPlayer.position().x);
        }

        if (nextB) {
            mPlayer.respawnOn(nextB->position());
            if (mCurrentBalloon) mCurrentBalloon->setState(Balloon::State::Done);
            mCurrentBalloon = nextB; 
        } else {
            // Резервный выход, если что-то пошло не так
            mState = GameState::GameOver;
        }
    }

    if (!mPlayer.isAlive()) {
        mState = GameState::GameOver;
        mStats.stopTimer();
    }

    if (mCurrentMode == GameMode::Classic && mStats.hits() >= CLASSIC_TARGET_BALLOONS) {
        mState = GameState::Win;
        mStats.stopTimer();
    }
}

void Game::render() {
    mWindow.clear(sf::Color(30, 30, 40));

    if (mState == GameState::Playing) {
        mWindow.setView(mWorldView);
        renderPlaying();
        
        mWindow.setView(mHudView);
        drawHUD();
    } else {
        mWindow.setView(mHudView);
        if (mState == GameState::Menu) renderMenu();
        else renderGameOver();
    }

    mWindow.display();
}

sf::Text Game::makeText(const std::string& str, unsigned int size, sf::Color color, float x, float y) {
    sf::Text text(mRM.font("main"), str, size);
    text.setFillColor(color);
    sf::FloatRect b = text.getLocalBounds();
    text.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
    text.setPosition({x, y});
    return text;
}

void Game::renderMenu() {
    float cx = WINDOW_W / 2.f;
    auto title = makeText("BalloonTyper", 52, sf::Color(255, 210, 80), cx, 160);
    title.setStyle(sf::Text::Style::Bold);
    mWindow.draw(title);

    mWindow.draw(makeText("Press 1: Classic Mode (50 Balloons, 2 Lives)", 24, sf::Color(100, 220, 160), cx, 280));
    mWindow.draw(makeText("Press 2: Endless Mode (Infinite, 1 Life)", 24, sf::Color(220, 100, 160), cx, 340));
    mWindow.draw(makeText("Press ESC to exit", 18, sf::Color(160, 160, 180), cx, 420));
}

void Game::startGame(GameMode mode) {
    mCurrentMode = mode;
    mStats.reset();
    
    int startLives = (mode == GameMode::Classic) ? CLASSIC_LIVES : ENDLESS_LIVES;
    mPlayer.reset(startLives);
    
    mBalloons.reset(mode == GameMode::Classic);
    mCurrentBalloon = nullptr;
    mPlatform.setPosition({PLATFORM_X, PLATFORM_Y});
    mWorldView.setCenter({WINDOW_W / 2.f, WINDOW_H / 2.f});
    mState = GameState::Playing;
}

void Game::renderPlaying() {
    mWindow.clear(sf::Color(80, 160, 220));
    mWindow.draw(mPlatform);
    mBalloons.draw(mWindow);
    mPlayer.draw(mWindow);
}

void Game::drawHUD() {
    std::string hearts = "Lives: ";
    for (int i = 0; i < mPlayer.lives(); i++) hearts += "* ";
    mWindow.draw(makeText(hearts, 22, sf::Color::White, 110, 25));
    
    if (mCurrentMode == GameMode::Classic) {
         mWindow.draw(makeText("Progress: " + std::to_string(mStats.hits()) + "/" + std::to_string(CLASSIC_TARGET_BALLOONS), 22, sf::Color::White, WINDOW_W / 2.f, 25));
    }
    
    mWindow.draw(makeText("Score: " + std::to_string(mStats.score()), 22, sf::Color::White, WINDOW_W - 110, 25));
}

void Game::renderGameOver() {
    float cx = WINDOW_W / 2.f;
    
    std::string titleText = (mState == GameState::Win) ? "YOU WIN!" : "GAME OVER";
    sf::Color titleColor = (mState == GameState::Win) ? sf::Color(80, 220, 80) : sf::Color(220, 80, 80);

    auto over = makeText(titleText, 56, titleColor, cx, 120);
    over.setStyle(sf::Text::Style::Bold);
    mWindow.draw(over);

    std::ostringstream statsStr;
    statsStr << std::fixed << std::setprecision(1);

    if (mCurrentMode == GameMode::Classic) {
        statsStr << "Time: " << mStats.time() << "s\n";
    } else {
        statsStr << "Balloons cleared: " << mStats.hits() << "\n";
    }
    
    statsStr << "WPM: " << std::setprecision(0) << mStats.wpm() << "\n"
             << "Accuracy: " << std::setprecision(1) << mStats.accuracy() << "%";

    mWindow.draw(makeText(statsStr.str(), 24, sf::Color::White, cx, 240));

    mWindow.draw(makeText("ENTER to play again", 20, sf::Color(100, 220, 160), cx, 400));
    mWindow.draw(makeText("ESC for Menu", 20, sf::Color(160, 160, 180), cx, 450));
}