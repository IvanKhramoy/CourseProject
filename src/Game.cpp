#include "Game.h"
#include "Constants.h"
#include "Balloon.h"
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>

Game::Game()
    // Теперь игра по умолчанию запускается в Полный Экран (Fullscreen)
    : mWindow(sf::VideoMode::getDesktopMode(), "BalloonTyper", sf::Style::Default, sf::State::Fullscreen), mState(GameState::Menu), mCurrentMode(GameMode::Classic), mBalloons(mRM), mPlayer(mRM), mIsFullscreen(true)
{
    mWindow.setFramerateLimit(FPS_LIMIT);
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    mRM.loadTexture("explosion", Paths::EXPLOSION_TEXTURE);

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
        mRM.loadFont("VR-regular", Paths::FONT_VR_400);
    }
    catch (...)
    {
        mRM.loadFont("main", Paths::FONT_FALLBACK);
    }

    try
    {
        mRM.loadTexture("ninja-1", Paths::NINJA_1);
        mRM.loadTexture("ninja-2", Paths::NINJA_2);
        mRM.loadTexture("ninja-3", Paths::NINJA_3);
        mRM.loadTexture("ninja-4", Paths::NINJA_4);
        mRM.loadTexture("ninja-5", Paths::NINJA_5);

        // Включаем сглаживание для всех текстур ниндзя
        for (int i = 1; i <= 5; ++i)
        {
            const_cast<sf::Texture &>(mRM.texture("ninja-" + std::to_string(i))).setSmooth(true);
        }
    }
    catch (const std::exception &e)
    {
        printf("Error loading ninja: %s\n", e.what());
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

    try
    {
        mRM.loadTexture("life", Paths::LIFE_TEXTURE);
        // Обязательно включаем сглаживание для такой большой картинки!
        const_cast<sf::Texture &>(mRM.texture("life")).setSmooth(true);
    }
    catch (const std::exception &e)
    {
        printf("Error loading life icon: %s\n", e.what());
    }

    try
    {
        mRM.loadTexture("counter", Paths::COUNTER_TEXTURE);
        const_cast<sf::Texture &>(mRM.texture("counter")).setSmooth(true);
    }
    catch (const std::exception &e)
    {
        printf("Error loading counter icon: %s\n", e.what());
    }

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

    for (int i = 1; i <= 5; ++i)
    {
        std::string id = "ninja-" + std::to_string(i);
        std::string path = "assets/textures/" + id + ".png";
        try
        {
            mRM.loadTexture(id, path);
            // Включаем сглаживание для четкости
            const_cast<sf::Texture &>(mRM.texture(id)).setSmooth(true);
        }
        catch (const std::exception &e)
        {
            printf("Error loading %s: %s\n", id.c_str(), e.what());
        }
    }
    // Инициализируем графику игрока после загрузки всех текстур
    // mPlayer.initGraphics(mRM);
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
                            mIsWaitingForTyping = false;
                        else
                            mBalloons.increaseSpeed();

                        mLastSafeX = target->position().x;
                        mLastSafeTexture = &target->getTexture();

                        // ПРОВЕРКА НА ФИНИШ
                        if (mCurrentMode == GameMode::Classic && mStats.hits() >= CLASSIC_TARGET_BALLOONS)
                        {
                            mIsFinishing = true;

                            // Отпускаем текущий шар, если он был
                            if (mCurrentBalloon && mCurrentBalloon != target)
                            {
                                mCurrentBalloon->setState(Balloon::State::Done);
                            }

                            // 1. Делаем текущим шаром тот, на который только что нажали
                            mCurrentBalloon = target;
                            mCurrentBalloon->startFalling();

                            mPlayer.jumpTo(target->position());

                            // Если это последний шар — взводим "режим ожидания финала"
                            if (mCurrentMode == GameMode::Classic && mStats.hits() >= CLASSIC_TARGET_BALLOONS)
                            {
                                mWaitingForFinalJump = true;
                            }
                        }
                        else
                        {
                            // Обычная логика прыжка (как была раньше)
                            if (mCurrentBalloon && mCurrentBalloon != target)
                            {
                                mCurrentBalloon->setState(Balloon::State::Done);
                            }
                            mCurrentBalloon = target;
                            mCurrentBalloon->startFalling();
                            mPlayer.jumpTo(target->position());
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
    // --- 1. ПЛАВНАЯ КАМЕРА ---
    float currentViewWidth = mWorldView.getSize().x;
    float targetCamX = mPlayer.position().x + currentViewWidth / 6.f;
    targetCamX = std::max(targetCamX, currentViewWidth / 2.f);

    float interpolationSpeed = 5.0f;
    sf::Vector2f currentCenter = mWorldView.getCenter();
    float newCamX = currentCenter.x + (targetCamX - currentCenter.x) * interpolationSpeed * dt;
    mWorldView.setCenter({newCamX, WINDOW_H / 2.f});

    // --- 2. ОБНОВЛЕНИЕ ЧАСТИЦ (ВЗРЫВА) ---
    // for (auto &p : mParticles)
    // {
    //     p.pos += p.vel * dt;
    //     p.lifetime -= dt;
    // }
    // mParticles.erase(std::remove_if(mParticles.begin(), mParticles.end(),
    //                                 [](const Particle &p)
    //                                 { return p.lifetime <= 0; }),
    //                  mParticles.end());

    for (auto &ex : mActiveExplosions)
    {
        ex.timer += dt;
        if (ex.timer >= EXPL_SPEED)
        {
            ex.timer = 0;
            ex.currentFrame++;

            if (ex.currentFrame < EXPL_FRAMES)
            {
                // Сдвигаем "окошко" выбора кадра вправо по картинке
                ex.sprite.setTextureRect(sf::IntRect({ex.currentFrame * EXPL_SIZE, 0}, {EXPL_SIZE, EXPL_SIZE}));
            }
            else
            {
                ex.finished = true; // Анимация закончилась
            }
        }
    }
    // Удаляем завершенные взрывы
    mActiveExplosions.erase(std::remove_if(mActiveExplosions.begin(), mActiveExplosions.end(),
                                           [](const Explosion &ex)
                                           { return ex.finished; }),
                            mActiveExplosions.end());

    // --- 3. ЛОГИКА СМЕРТИ И ЗАДЕРЖКИ ПЕРЕД РЕСПАВНОМ ---
    if (mIsDeadWaiting)
    {
        mRespawnTimer -= dt;
        if (mRespawnTimer <= 0.f)
        {
            mIsDeadWaiting = false; // Время вышло, пора воскрешать

            // Создаем платформу на месте последнего чекпоинта
            mCurrentBalloon = mBalloons.spawnRespawnPlatform(mLastSafeX, BALLOON_START_Y, *mLastSafeTexture);
            if (mCurrentBalloon)
            {
                mPlayer.respawnOn(mCurrentBalloon->position());
                mIsWaitingForTyping = true; // Показываем "Keep typing"
            }
        }
        return; // Пока ждем респавна, остальную логику (шары, прыжки) не крутим
    }

    // --- 4. ОБЫЧНОЕ ОБНОВЛЕНИЕ МИРА ---
    float cameraRightEdge = mWorldView.getCenter().x + currentViewWidth / 2.f;
    mBalloons.update(dt, cameraRightEdge, mPlayer.position().x);

    // Безопасность: если текущий шар удален менеджером
    if (mCurrentBalloon && !mBalloons.isValid(mCurrentBalloon))
    {
        mCurrentBalloon = nullptr;
    }

    mPlayer.update(dt);

    // Логика привязки к шару и ДЕТЕКЦИЯ ПАДЕНИЯ
    if (mPlayer.state() == Player::State::OnBalloon && mCurrentBalloon != nullptr)
    {
        mPlayer.setBalloonPosition(mCurrentBalloon->position());

        // Если ниндзя скрылся за нижним краем экрана
        if (mPlayer.position().y - (PLAYER_HEIGHT / 2.f) > WINDOW_H)
        {
            mPlayer.loseLife();
            // Создаем взрыв в месте падения
            // for (int i = 0; i < 20; ++i)
            // {
            //     float angle = static_cast<float>(std::rand() % 360) * 3.14159f / 180.f;
            //     float speed = static_cast<float>(std::rand() % 200 + 50);
            //     mParticles.push_back({
            //         {mPlayer.position().x, (float)WINDOW_H},             // Позиция взрыва (низ экрана)
            //         {std::cos(angle) * speed, -std::sin(angle) * speed}, // Скорость вверх и в бока
            //         0.8f                                                 // Время жизни частицы
            //     });
            // }

            // if (mCurrentBalloon && mBalloons.isValid(mCurrentBalloon))
            // {
            //     // Переводим шар в состояние "Готово", чтобы менеджер его удалил в этом же кадре
            //     mCurrentBalloon->setState(Balloon::State::Done);
            // }

            // // mPlayer.startFalling(); // Отнимаем жизнь внутри Player
            // mCurrentBalloon = nullptr;
            // mIsDeadWaiting = true; // Включаем ожидание
            // mRespawnTimer = 1.2f;  // Пауза 1.2 секунды на созерцание взрыва

            // Explosion ex;
            // ex.sprite.setTexture(mRM.texture("explosion"));
            sf::Vector2f explosionPos = mPlayer.position();
            if (mCurrentBalloon && mBalloons.isValid(mCurrentBalloon))
            {
                explosionPos = mCurrentBalloon->position();

                // --- 2. УДАЛЯЕМ ШАР МГНОВЕННО ---
                // Он исчезнет из логики, но его "заменит" собой вспышка взрыва
                mCurrentBalloon->setState(Balloon::State::Done);
            }
            mCurrentBalloon = nullptr;
            Explosion ex(mRM.texture("explosion"));

            // Вырезаем первый кадр (самый левый верхний квадрат)
            ex.sprite.setTextureRect(sf::IntRect({0, 0}, {EXPL_SIZE, EXPL_SIZE}));

            // Центрируем и ставим в место падения
            ex.sprite.setOrigin({EXPL_SIZE / 2.f, EXPL_SIZE / 2.f});
            // ex.sprite.setPosition({mPlayer.position().x, (float)WINDOW_H - 50.f});
            ex.sprite.setPosition(explosionPos);

            // Можно немного увеличить взрыв, если он мелкий
            ex.sprite.setScale({2.5f, 2.5f});

            mActiveExplosions.push_back(ex);

            // Логика смерти (как была)
            if (mCurrentBalloon && mBalloons.isValid(mCurrentBalloon))
            {
                mCurrentBalloon->setState(Balloon::State::Done);
            }
            mCurrentBalloon = nullptr;
            mIsDeadWaiting = true;
            mRespawnTimer = 1.2f;
        }
    }

    // --- 5. ФИНАЛЬНЫЙ ПРЫЖОК (ПОБЕДА) ---
    if (mWaitingForFinalJump && mPlayer.state() == Player::State::OnBalloon)
    {
        sf::Vector2f finishPos = {mPlayer.position().x + 300.f, PLATFORM_Y};
        mPlayer.jumpTo(finishPos);
        mWaitingForFinalJump = false;
        mIsFinishing = true;
    }

    // --- 6. ПРОВЕРКИ СОСТОЯНИЙ ---
    if (!mPlayer.isAlive() && !mIsDeadWaiting)
    {
        mState = GameState::GameOver;
        mStats.stopTimer();
    }

    if (mIsFinishing && mPlayer.state() != Player::State::Jumping)
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

    mLastSafeTexture = &mRM.texture("balloon_0");

    mPlatform.setPosition({PLATFORM_X, PLATFORM_Y});
    mWorldView.setCenter({mWorldView.getSize().x / 2.f, WINDOW_H / 2.f}); // Учитываем новую ширину
    mIsFinishing = false;
    mWaitingForFinalJump = false;
    mState = GameState::Playing;
}

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

    // for (const auto &p : mParticles)
    // {
    //     sf::RectangleShape dot({4.f, 4.f});
    //     dot.setPosition(p.pos);
    //     dot.setFillColor(sf::Color(255, 150, 50, static_cast<int>(255 * (p.lifetime / 0.8f))));
    //     mWindow.draw(dot);
    // }
    for (const auto &ex : mActiveExplosions)
    {
        mWindow.draw(ex.sprite);
    }
}

void Game::drawHUD()
{
    float hudW = mHudView.getSize().x;

    // --- 1. ГЛОБАЛЬНЫЕ НАСТРОЙКИ ПОЛОЖЕНИЯ (МЕНЬШЕ И ПРАВЕЕ) ---
    // startX — единая левая граница для иконки шара и первого ниндзя
    float startX = hudW - 130.f; 
    float currentY = WINDOW_H - 40.f; // Нижняя точка для ряда жизней

    // --- 2. РИСУЕМ ИКОНКИ ЖИЗНЕЙ (НИЖНИЙ РЯД, УМЕНЬШЕННЫЙ) ---
    int maxLives = (mCurrentMode == GameMode::Classic) ? CLASSIC_LIVES : ENDLESS_LIVES;
    for (int i = 0; i < maxLives; ++i) {
        if (mRM.hasTexture("life")) {
            sf::Sprite s(mRM.texture("life"));
            
            // РАЗМЕР: Высота 24 пикселя (было 32)
            float iconScale = 24.f / s.getLocalBounds().size.y;
            s.setScale({iconScale, iconScale});
            
            // Выравнивание: левый край спрайта привязан к позиции
            s.setOrigin({0.f, s.getLocalBounds().size.y / 2.f});
            
            // Позиция: начинаем ровно с startX, шаг между иконками 32 пикселя
            s.setPosition({startX + (i * 32.f), currentY});

            if (i < mPlayer.lives()) s.setColor(sf::Color::White);
            else s.setColor(sf::Color(50, 50, 50, 120)); 

            mWindow.draw(s);
        }
    }

    // Расстояние между строками стало меньше (45 пикселей)
    currentY -= 45.f;

    // --- 3. РИСУЕМ ПРОГРЕСС (ВЕРХНИЙ РЯД, УМЕНЬШЕННЫЙ) ---
    if (mRM.hasTexture("counter")) {
        // Иконка шара
        sf::Sprite counterIcon(mRM.texture("counter"));
        
        // РАЗМЕР: Высота 34 пикселя (было 45)
        float counterScale = 34.f / counterIcon.getLocalBounds().size.y;
        counterIcon.setScale({counterScale, counterScale});
        
        // Выравнивание: левый край иконки шара тоже ровно в startX
        counterIcon.setOrigin({0.f, counterIcon.getLocalBounds().size.y / 2.f});
        counterIcon.setPosition({startX, currentY});
        mWindow.draw(counterIcon);

        // Число прогресса
        std::string progressStr = std::to_string(mStats.hits());
        sf::Text txtProgress(mRM.font("light"), progressStr, 64);
        txtProgress.setFillColor(sf::Color::White);
        txtProgress.setStyle(sf::Text::Style::Bold);
        
        // ТЕКСТ ТОЖЕ МЕНЬШЕ: масштаб 0.45 (было 0.6)
        txtProgress.setScale({0.45f, 0.45f}); 

        // Ставим число справа от иконки
        sf::FloatRect valB = txtProgress.getLocalBounds();
        txtProgress.setOrigin({0.f, valB.position.y + valB.size.y / 2.f});
        
        // Отступаем от иконки шара всего 8 пикселей
        float iconWidth = counterIcon.getGlobalBounds().size.x;
        txtProgress.setPosition({startX + iconWidth + 8.f, currentY});
        
        mWindow.draw(txtProgress);
    }

    // --- 4. ЦЕНТРАЛЬНЫЙ ТЕКСТ (БЕЗ ИЗМЕНЕНИЙ ПО ЛОГИКЕ) ---
    auto drawCenterMessage = [&](const std::string &textStr) {
        int alpha = 255;
        if (mCenterTextTimer > 2.5f) {
            if (mCenterTextTimer <= 2.75f) alpha = static_cast<int>(255.f * (1.f - (mCenterTextTimer - 2.5f) / 0.25f));
            else alpha = static_cast<int>(255.f * ((mCenterTextTimer - 2.75f) / 0.25f));
        }

        sf::Color mainColor(255, 248, 237, alpha);
        sf::Text msg = makeText(textStr, 80, mainColor, hudW / 2.f, WINDOW_H / 2.f + 20.f);

        msg.setFont(mRM.font("VR-regular"));
        msg.setStyle(sf::Text::Style::Bold);
        msg.setOutlineColor(sf::Color(95, 180, 205, alpha));
        msg.setOutlineThickness(10.f);
        msg.setScale({0.35f, 0.35f});
        msg.setLetterSpacing(0.5);

        sf::FloatRect bounds = msg.getLocalBounds();
        msg.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});

        sf::Text shadow = msg;
        sf::Color shadowColor(0, 0, 0, static_cast<int>(alpha * 0.3f));
        shadow.setFillColor(shadowColor);
        shadow.setOutlineColor(shadowColor);
        shadow.setOutlineThickness(7.f);
        shadow.move({3.f, 3.f});

        mWindow.draw(shadow);
        mWindow.draw(msg);
    };

    if (mPlayer.state() == Player::State::OnPlatform) drawCenterMessage("Type letter to begin");
    else if (mIsWaitingForTyping) drawCenterMessage("Keep typing...");
}

// --- ВОЗВРАЩАЕМ МЕТОД, КОТОРЫЙ ПОТЕРЯЛСЯ ---
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