#include "Game.h"
#include "Constants.h"
#include "Balloon.h"
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

Game::Game()
    // Теперь игра по умолчанию запускается в Полный Экран (Fullscreen)
    : mWindow(sf::VideoMode::getDesktopMode(), "BalloonTyper", sf::Style::Default, sf::State::Fullscreen), mState(GameState::Menu), mCurrentMode(GameMode::Classic), mBalloons(mRM), mIsFullscreen(true)
{
    mWindow.setFramerateLimit(FPS_LIMIT);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // Первичная настройка видов под разрешение экрана
    sf::Vector2u size = mWindow.getSize();
    adjustViewports(size.x, size.y);

    mPlatform.setSize({100.f, 24.f});
    mPlatform.setOrigin({50.f, 12.f});
    mPlatform.setFillColor(sf::Color(80, 200, 120));

    try
    {
        // mRM.loadFont("main", Paths::FONT);
        mRM.loadFont("main", Paths::FONT_RM_400);
        mRM.loadFont("light", Paths::FONT_RM_300);
        mRM.loadFont("semibold", Paths::FONT_RM_600);
        mRM.loadFont("bold", Paths::FONT_RM_700);
        mRM.loadFont("CO-bold", Paths::FONT_CO_400);
    }
    catch (...)
    {
        mRM.loadFont("main", Paths::FONT_FALLBACK);
    }

    // ++
    try
    {
        mRM.loadTexture("bg", Paths::BG_TEXTURE);
        // Создаем объект бесконечного фона, передавая ему текстуру и высоту экрана (WINDOW_H)
        mBackground = std::make_unique<Background>(mRM.texture("bg"), static_cast<float>(WINDOW_H));
    }
    catch (const std::exception &e)
    {
        printf("Error loading background: %s\n", e.what());
    }

    try
    {
        mRM.loadTexture("cliff", Paths::CLIFF_TEXTURE);
        const sf::Texture &cliffTex = mRM.texture("cliff");

        // Создаем спрайт через make_unique, передавая текстуру
        mCliffSprite = std::make_unique<sf::Sprite>(cliffTex);

        // В SFML 3 ширина — это size.x, высота — это size.y
        sf::FloatRect bounds = mCliffSprite->getLocalBounds();
        mCliffSprite->setOrigin({0, 0});
        // mCliffSprite->setOrigin({bounds.size.x / 2.f, 0.f});
        // mCliffSprite->setScale({1.0f, 1.0f});

        float cliffScaleX = 0.35f;
        float cliffScaleY = 0.4f;
        mCliffSprite->setScale({cliffScaleX, cliffScaleY});
        printf("Cliff size: %f x %f\n", bounds.size.x, bounds.size.y);
        // mCliffSprite->setPosition({PLATFORM_X, PLATFORM_Y});
        mCliffSprite->setPosition({0, 330});
    }
    catch (const std::exception &e)
    {
        printf("Error loading cliff: %s\n", e.what());
    }
    // Пока мы подгоняем графику, сделай платформу чуть-чуть видимой (трафарет)
    mPlatform.setFillColor(sf::Color(0, 0, 0, 0));

    // +++
    for (int i = 0; i < Paths::BALLOON_COLORS_COUNT; ++i)
    {
        try
        {
            std::string texName = "balloon_" + std::to_string(i);
            std::string texPath = Paths::BALLOON_PREFIX + std::to_string(i) + ".png";
            mRM.loadTexture(texName, texPath);
        }
        catch (const std::exception &e)
        {
            printf("Error loading balloon %d: %s\n", i, e.what());
        }
    }
}

void Game::run()
{
    sf::Clock clock;
    while (mWindow.isOpen())
    {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f)
            dt = 0.1f;

        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents()
{
    while (const std::optional event = mWindow.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            mWindow.close();

        // Если пользователь тянет окно мышкой
        if (const auto *resized = event->getIf<sf::Event::Resized>())
        {
            adjustViewports(resized->size.x, resized->size.y);
        }

        if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
        {
            // Включаем/Выключаем полноэкранный режим на F11
            if (keyPressed->code == sf::Keyboard::Key::F11)
            {
                toggleFullscreen();
            }

            if (keyPressed->code == sf::Keyboard::Key::Escape)
            {
                if (mState == GameState::Playing || mState == GameState::GameOver || mState == GameState::Win)
                {
                    mState = GameState::Menu;
                }
                else if (mState == GameState::Menu)
                {
                    mWindow.close();
                }
            }
        }

        switch (mState)
        {
        case GameState::Menu:
            processEventsMenu(*event);
            break;
        case GameState::Playing:
            processEventsPlaying(*event);
            break;
        case GameState::GameOver:
        case GameState::Win:
            processEventsGameOver(*event);
            break;
        }
    }
}

void Game::processEventsMenu(const sf::Event &event)
{
    if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Num1)
        {
            startGame(GameMode::Classic);
        }
        else if (keyPressed->code == sf::Keyboard::Key::Num2)
        {
            startGame(GameMode::Endless);
        }
    }
}

void Game::processEventsPlaying(const sf::Event &event)
{
    if (const auto *textEntered = event.getIf<sf::Event::TextEntered>())
    {
        if (textEntered->unicode < 128)
        {
            char c = static_cast<char>(textEntered->unicode);
            if (std::isalpha(static_cast<unsigned char>(c)))
            {
                char typed = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

                Balloon *target = mBalloons.currentTarget(mPlayer.position().x);
                if (target)
                {
                    if (target->letter() == typed)
                    {
                        mStats.recordHit();

                        if (mIsWaitingForTyping)
                        {
                            mIsWaitingForTyping = false;
                        }
                        else
                        {
                            mBalloons.increaseSpeed();
                        }

                        if (mCurrentBalloon && mCurrentBalloon != target)
                        {
                            mCurrentBalloon->setState(Balloon::State::Done);
                        }

                        mCurrentBalloon = target;
                        mCurrentBalloon->startFalling();
                        mPlayer.jumpTo(target->position());
                    }
                    else
                    {
                        if (!mIsWaitingForTyping)
                        {
                            mStats.recordMiss();
                        }
                    }
                }
            }
        }
    }
}

void Game::processEventsGameOver(const sf::Event &event)
{
    if (const auto *keyPressed = event.getIf<sf::Event::KeyPressed>())
    {
        if (keyPressed->code == sf::Keyboard::Key::Enter)
        {
            startGame(mCurrentMode);
        }
    }
}

void Game::update(float dt)
{
    if (mBlinkClock.getElapsedTime().asSeconds() > 0.5f)
    {
        mShowCursor = !mShowCursor;
        mBlinkClock.restart();
    }

    if (mState == GameState::Playing && !mIsWaitingForTyping)
    {
        mStats.updateTime();
    }

    if (mState == GameState::Playing)
    {
        updatePlaying(dt);
    }

    //
    mCenterTextTimer += dt;
    if (mCenterTextTimer >= 3.0f)
    {
        mCenterTextTimer = 0.f; // Сбрасываем цикл каждые 3 секунды
    }

    if (mState == GameState::Playing)
    {
        mStats.updateTime();
        updatePlaying(dt);
    }
}

void Game::updatePlaying(float dt)
{
    // ДИНАМИЧЕСКАЯ ШИРИНА ЭКРАНА
    float currentViewWidth = mWorldView.getSize().x;

    float targetCamX = mPlayer.position().x + currentViewWidth / 6.f;
    targetCamX = std::max(targetCamX, currentViewWidth / 2.f);
    mWorldView.setCenter({targetCamX, WINDOW_H / 2.f});

    float cameraRightEdge = mWorldView.getCenter().x + currentViewWidth / 2.f;

    mBalloons.update(dt, cameraRightEdge, mPlayer.position().x);

    if (mCurrentBalloon && !mBalloons.isValid(mCurrentBalloon))
    {
        mCurrentBalloon = nullptr;
    }

    if (mPlayer.state() == Player::State::OnBalloon && mCurrentBalloon == nullptr)
    {
        mPlayer.startFalling();
    }

    mPlayer.update(dt);

    if (mCurrentBalloon && mPlayer.state() == Player::State::OnBalloon)
    {
        mPlayer.setBalloonPosition(mCurrentBalloon->position());
    }

    if (mPlayer.needsRespawn())
    {
        Balloon *nextB = mBalloons.nextAvailableBalloon(mPlayer.position().x);

        if (!nextB)
        {
            mBalloons.forceSpawnBalloonAt(mPlayer.position().x + 200.f);
            nextB = mBalloons.nextAvailableBalloon(mPlayer.position().x);
        }

        if (nextB)
        {
            mPlayer.respawnOn(nextB->position());

            if (mCurrentBalloon && mBalloons.isValid(mCurrentBalloon))
            {
                mCurrentBalloon->setState(Balloon::State::Done);
            }

            mCurrentBalloon = nextB;
            mIsWaitingForTyping = true;
        }
        else
        {
            mState = GameState::GameOver;
        }
    }

    if (!mPlayer.isAlive())
    {
        mState = GameState::GameOver;
        mStats.stopTimer();
    }

    if (mCurrentMode == GameMode::Classic && mStats.hits() >= CLASSIC_TARGET_BALLOONS)
    {
        mState = GameState::Win;
        mStats.stopTimer();
    }
}

void Game::render()
{
    mWindow.clear(sf::Color(30, 30, 40));

    if (mState == GameState::Playing)
    {
        mWindow.setView(mWorldView);
        renderPlaying();

        mWindow.setView(mHudView);
        drawHUD();
    }
    else
    {
        mWindow.setView(mHudView);
        if (mState == GameState::Menu)
            renderMenu();
        else
            renderGameOver();
    }

    mWindow.display();
}

sf::Text Game::makeText(const std::string &str, unsigned int size, sf::Color color, float x, float y)
{
    sf::Text text(mRM.font("main"), str, size);
    text.setFillColor(color);
    sf::FloatRect b = text.getLocalBounds();
    text.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});
    text.setPosition({x, y});
    return text;
}

void Game::renderMenu()
{
    float cx = mHudView.getSize().x / 2.f; // Центруем по динамической ширине

    auto title = makeText("BalloonTyper", 52, sf::Color(255, 210, 80), cx, 160);
    title.setStyle(sf::Text::Style::Bold);
    mWindow.draw(title);

    mWindow.draw(makeText("Press 1: Classic Mode (50 Balloons, 2 Lives)", 24, sf::Color(100, 220, 160), cx, 280));
    mWindow.draw(makeText("Press 2: Endless Mode (Infinite, 1 Life)", 24, sf::Color(220, 100, 160), cx, 340));
    mWindow.draw(makeText("Press ESC to exit", 18, sf::Color(160, 160, 180), cx, 420));
}

void Game::startGame(GameMode mode)
{
    mCurrentMode = mode;
    mStats.reset();

    int startLives = (mode == GameMode::Classic) ? CLASSIC_LIVES : ENDLESS_LIVES;
    mPlayer.reset(startLives);

    mBalloons.reset(mode == GameMode::Classic);
    mCurrentBalloon = nullptr;
    mIsWaitingForTyping = false;

    mPlatform.setPosition({PLATFORM_X, PLATFORM_Y});
    mWorldView.setCenter({mWorldView.getSize().x / 2.f, WINDOW_H / 2.f}); // Учитываем новую ширину
    mState = GameState::Playing;
}

// void Game::renderPlaying() {
//     mWindow.clear(sf::Color(80, 160, 220));
//     mWindow.draw(mPlatform);
//     mBalloons.draw(mWindow);
//     mPlayer.draw(mWindow);
// }

void Game::renderPlaying()
{
    mWindow.clear(sf::Color(80, 160, 220));

    // Рисуем бесконечный фон, передавая позицию камеры и ширину экрана
    if (mBackground)
    {
        mBackground->draw(mWindow, mWorldView.getCenter().x, mWorldView.getSize().x);
    }
    if (mCliffSprite)
    {
        mWindow.draw(*mCliffSprite);
    }
    mWindow.draw(mPlatform);
    mBalloons.draw(mWindow);
    mPlayer.draw(mWindow);
}

void Game::drawHUD()
{
    float hudW = mHudView.getSize().x; // Динамическая ширина экрана

    // 1. Лямбда для красивой отрисовки текста с выравниванием по ПРАВОМУ краю
    auto drawRightAligned = [&](const std::string &str, float y)
    {
        sf::Text txt(mRM.font("light"), str, 64);
        txt.setFillColor(sf::Color::White);
        txt.setStyle(sf::Text::Style::Bold);
        txt.setScale({0.35f, 0.35f});

        sf::FloatRect b = txt.getLocalBounds();
        // Смещаем центр (origin) в правую часть текста
        txt.setOrigin({b.position.x + b.size.x, b.position.y + b.size.y / 2.f});
        // Устанавливаем отступ 40 пикселей от правого края монитора
        txt.setPosition({hudW - 40.f, y});

        mWindow.draw(txt);
    };

    // 2. Рисуем статистику в столбик (снизу вверх)
    float currentY = WINDOW_H - 40.f; // Самая нижняя строчка

    drawRightAligned("Score: " + std::to_string(mStats.score()), currentY);
    currentY -= 35.f; // Поднимаемся чуть выше

    if (mCurrentMode == GameMode::Classic)
    {
        drawRightAligned("Progress: " + std::to_string(mStats.hits()) + "/" + std::to_string(CLASSIC_TARGET_BALLOONS), currentY);
        currentY -= 35.f;
    }

    std::string hearts = "Lives: ";
    for (int i = 0; i < mPlayer.lives(); i++)
        hearts += "* ";
    drawRightAligned(hearts, currentY);

    // Функция для стилизации центральных сообщений (белые с голубой обводкой)
    auto drawCenterMessage = [&](const std::string &textStr)
    {
        int alpha = 220; // Базовое значение

        if (mCenterTextTimer > 2.5f)
        {
            if (mCenterTextTimer <= 2.75f)
            {
                // Фаза затухания (2.5 -> 2.75)
                float ratio = (mCenterTextTimer - 2.5f) / 0.25f; // От 0.0 до 1.0
                alpha = static_cast<int>(180.f * (1.f - ratio));
            }
            else
            {
                // Фаза появления (2.75 -> 3.0)
                float ratio = (mCenterTextTimer - 2.75f) / 0.25f; // От 0.0 до 1.0
                alpha = static_cast<int>(180.f * ratio);
            }
        }

        sf::Text msg = makeText(textStr, 120, sf::Color(255, 255, 255, alpha), hudW / 2.f, WINDOW_H / 2.f + 100.f);

        msg.setFont(mRM.font("CO-bold"));
        msg.setStyle(sf::Text::Style::Bold);
        msg.setOutlineColor(sf::Color(111, 157, 255, alpha * 0.7));
        msg.setOutlineThickness(8.f);
        msg.setScale({0.35f, 0.35f});
        msg.setLetterSpacing(0.5);

        sf::FloatRect bounds = msg.getLocalBounds();

        msg.setOrigin({bounds.position.x + bounds.size.x / 2.f,
                       bounds.position.y + bounds.size.y / 2.f});

        mWindow.draw(msg);
    };

    // Проверяем, стоит ли игрок на начальной платформе
    if (mPlayer.state() == Player::State::OnPlatform)
    {
        drawCenterMessage("Type letter to begin");
    }
    // Если мы потеряли жизнь и ждем ввода
    else if (mIsWaitingForTyping)
    {
        drawCenterMessage("Keep typing...");
    }
}

void Game::renderGameOver()
{
    float cx = mHudView.getSize().x / 2.f;

    std::string titleText = (mState == GameState::Win) ? "YOU WIN!" : "GAME OVER";
    sf::Color titleColor = (mState == GameState::Win) ? sf::Color(80, 220, 80) : sf::Color(220, 80, 80);

    auto over = makeText(titleText, 56, titleColor, cx, 120);
    over.setStyle(sf::Text::Style::Bold);
    mWindow.draw(over);

    std::ostringstream statsStr;
    statsStr << std::fixed << std::setprecision(1);

    if (mCurrentMode == GameMode::Classic)
    {
        statsStr << "Time: " << mStats.time() << "s\n";
    }
    else
    {
        statsStr << "Balloons cleared: " << mStats.hits() << "\n";
    }

    statsStr << "WPM: " << std::setprecision(0) << mStats.wpm() << "\n"
             << "Accuracy: " << std::setprecision(1) << mStats.accuracy() << "%";

    mWindow.draw(makeText(statsStr.str(), 24, sf::Color::White, cx, 240));

    mWindow.draw(makeText("ENTER to play again", 20, sf::Color(100, 220, 160), cx, 400));
    mWindow.draw(makeText("ESC for Menu", 20, sf::Color(160, 160, 180), cx, 450));
}

// -----------------------------------------------------
// МАСШТАБИРОВАНИЕ И ЭКРАН
// -----------------------------------------------------

void Game::adjustViewports(unsigned int width, unsigned int height)
{
    if (height == 0)
        return; // Защита от сворачивания окна

    // Вычисляем фактическое соотношение сторон монитора (например 16:9)
    float aspect = static_cast<float>(width) / static_cast<float>(height);

    // Высота всегда фиксирована (600), а ширина "раздвигается" под монитор
    float newWidth = WINDOW_H * aspect;

    // Обновляем размер камер (расширяем обзор, но не сплющиваем графику)
    mWorldView.setSize({newWidth, static_cast<float>(WINDOW_H)});
    mHudView.setSize({newWidth, static_cast<float>(WINDOW_H)});

    // Центруем HUD по новой ширине
    mHudView.setCenter({newWidth / 2.f, WINDOW_H / 2.f});

    // Камера занимает весь экран, никаких черных полос (0,0 до 1,1)
    mWorldView.setViewport(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));
    mHudView.setViewport(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));
}

void Game::toggleFullscreen()
{
    mIsFullscreen = !mIsFullscreen;
    if (mIsFullscreen)
    {
        mWindow.create(sf::VideoMode::getDesktopMode(), "BalloonTyper", sf::Style::Default, sf::State::Fullscreen);
    }
    else
    {
        mWindow.create(sf::VideoMode({WINDOW_W, WINDOW_H}), "BalloonTyper", sf::Style::Default, sf::State::Windowed);
    }
    mWindow.setFramerateLimit(FPS_LIMIT);

    // После изменения окна пересчитываем расширение камеры
    sf::Vector2u size = mWindow.getSize();
    adjustViewports(size.x, size.y);
}