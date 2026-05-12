#include "Game.h"
#include "Constants.h"
#include "Balloon.h"
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <cstdint>

Game::Game()
    : mWindow(sf::VideoMode::getDesktopMode(), "BalloonTyper", sf::Style::Default, sf::State::Fullscreen, sf::ContextSettings{0, 0, 16}), mState(GameState::Menu), mCurrentMode(GameMode::Classic), mBalloons(mRM), mPlayer(mRM), mIsFullscreen(true)
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
        mRM.loadTexture("menu_bg", Paths::MENU_BG_TEXTURE);
        const sf::Texture &bgTex = mRM.texture("menu_bg");

        // Включаем сглаживание, чтобы картинка не была пиксельной
        const_cast<sf::Texture &>(bgTex).setSmooth(true);

        mMenuBgSprite = std::make_unique<sf::Sprite>(bgTex);

        // Масштабируем так, чтобы фон был чуть больше экрана (на 5%),
        // чтобы при движении не было видно краев
        float scale = (static_cast<float>(WINDOW_H) / bgTex.getSize().y) * 1.05f;
        mMenuBgSprite->setScale({scale, scale});

        // Ставим точку привязки в центр
        mMenuBgSprite->setOrigin({bgTex.getSize().x / 2.f, 0.f});
        mMenuBgSprite->setColor(sf::Color(200, 200, 230));
    }
    catch (const std::exception &e)
    {
        printf("Error loading menu background: %s\n", e.what());
    }

    try
    {
        mRM.loadTexture("cliff", Paths::CLIFF_TEXTURE);
        const sf::Texture &cliffTex = mRM.texture("cliff");

        // Создаем спрайт через make_unique, передавая текстуру
        mCliffSprite = std::make_unique<sf::Sprite>(cliffTex);

        sf::FloatRect bounds = mCliffSprite->getLocalBounds();
        mCliffSprite->setOrigin({0, 0});

        float cliffScaleX = 0.35f;
        float cliffScaleY = 0.4f;
        mCliffSprite->setScale({cliffScaleX, cliffScaleY});
        mCliffSprite->setPosition({0, 330});
    }
    catch (const std::exception &e)
    {
        printf("Error loading cliff: %s\n", e.what());
    }
    mPlatform.setFillColor(sf::Color(0, 0, 0, 0));

    try
    {
        mRM.loadTexture("end_cliff", Paths::END_CLIFF_TEXTURE);
        mEndCliffSprite = std::make_unique<sf::Sprite>(mRM.texture("end_cliff"));

        float endScaleX = 0.45f;
        float endScaleY = 0.5f;
        mEndCliffSprite->setScale({endScaleX, endScaleY});
        mEndCliffSprite->setOrigin({0.f, 0.f});
    }
    catch (const std::exception &e)
    {
        printf("Error loading end cliff: %s\n", e.what());
    }

    try
    {
        mRM.loadTexture("life", Paths::LIFE_TEXTURE);
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

    auto loadMusic = [](sf::Music &music, const std::string &path)
    {
        if (!music.openFromFile(path))
        {
            printf("Error loading music: %s\n", path.c_str());
        }
        music.setLooping(true); 
        music.setVolume(0.f);   
    };

    loadMusic(mMusicMenu, Paths::MUSIC_MENU);
    loadMusic(mMusicGame, Paths::MUSIC_GAME);

    mMusicMenu.play();

    try
    {
        mRM.loadSound("explosion_sfx", Paths::SOUND_EXPLOSION);

        // Создаем проигрыватель через make_unique и сразу передаем ему буфер из менеджера
        mExplosionSound = std::make_unique<sf::Sound>(mRM.sound("explosion_sfx"));
        mExplosionSound->setVolume(80.f);
    }
    catch (const std::exception &e)
    {
        printf("Error loading sound: %s\n", e.what());
    }

    try
    {
        // Загружаем буферы в ResourceManager
        mRM.loadSound("correct_sfx", Paths::SOUND_CORRECT);
        mRM.loadSound("error_sfx", Paths::SOUND_ERROR);

        // Создаем проигрыватели
        mCorrectSound = std::make_unique<sf::Sound>(mRM.sound("correct_sfx"));
        mErrorSound = std::make_unique<sf::Sound>(mRM.sound("error_sfx"));

        // Настраиваем громкость
        mCorrectSound->setVolume(500.f);
        mErrorSound->setVolume(1200.f); 
    }
    catch (const std::exception &e)
    {
        printf("Error loading feedback sounds: %s\n", e.what());
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
}

void Game::run()
{
    sf::Clock clock;
    while (mWindow.isOpen())
    {
        float dt = clock.restart().asSeconds();
        if (dt > 0.1f)
            dt = 0.1f;

        mLastDt = dt;

        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents()
{
    mIsMouseClicked = false;
    while (const std::optional event = mWindow.pollEvent())
    {
        if (event->is<sf::Event::Closed>())
            mWindow.close();

        if (const auto *mouseBtn = event->getIf<sf::Event::MouseButtonReleased>())
        {
            if (mouseBtn->button == sf::Mouse::Button::Left)
            {
                mIsMouseClicked = true;
            }
        }

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
}

void Game::processEventsPlaying(const sf::Event &event)
{
    if (mIsDeadWaiting)
        return;
    if (mPlayer.state() == Player::State::Jumping)
        return;

    if (const auto *textEntered = event.getIf<sf::Event::TextEntered>())
    {
        // Используем 32-битный Юникод для поддержки кириллицы и спецсимволов
        std::uint32_t typed = textEntered->unicode;

        // Игнорируем управляющие клавиши (Enter, Backspace, Esc и т.д. — это коды < 32)
        if (typed < 32)
            return;

        Balloon *target = mBalloons.currentTarget(mPlayer.position().x);

        if (target)
        {
            if (target->letter() == typed)
            {

                if (mCorrectSound)
                    mCorrectSound->play();
                mStats.recordHit();
                mStrikeCount = 0; 

                // Выход из режима "Keep typing"
                if (mIsWaitingForTyping)
                    mIsWaitingForTyping = false;
                else
                    mBalloons.increaseSpeed();

                // Обновляем чекпоинт
                mLastSafeX = target->position().x;
                mLastSafeTexture = &target->getTexture();

                // Обработка шаров
                if (mCurrentBalloon && mCurrentBalloon != target)
                {
                    mCurrentBalloon->setState(Balloon::State::Done);
                }
                mCurrentBalloon = target;
                mCurrentBalloon->startFalling();

                // Прыгаем на шар
                mPlayer.jumpTo(target->position());

                // Проверка на финиш уровня
                if (mCurrentMode == GameMode::Classic && mStats.hits() >= CLASSIC_TARGET_BALLOONS)
                {
                    mWaitingForFinalJump = true;
                }
            }
            else
            {
    
                if (!mIsWaitingForTyping && mPlayer.state() != Player::State::OnPlatform)
                {
                    mStats.recordMiss();
                    mStrikeCount++;

                    if (mStrikeCount >= 3)
                    {
                        // === 3 ОШИБКИ: ВЗРЫВ ТЕКУЩЕГО ШАРА ===
                        mPlayer.loseLife();
                        if (mExplosionSound)
                            mExplosionSound->play();

                        mPlayer.hide(); // Скрываем ниндзя

                        sf::Vector2f explosionPos = mPlayer.position();
                        if (mCurrentBalloon)
                        {
                            explosionPos = mCurrentBalloon->position();
                            mCurrentBalloon->setState(Balloon::State::Done);
                        }
                        mCurrentBalloon = nullptr;

                        // Визуальный взрыв
                        Explosion ex(mRM.texture("explosion"));
                        ex.sprite.setTextureRect(sf::IntRect({0, 0}, {EXPL_SIZE, EXPL_SIZE}));
                        ex.sprite.setOrigin({EXPL_SIZE / 2.f, EXPL_SIZE / 2.f});
                        ex.sprite.setPosition(explosionPos);
                        ex.sprite.setScale({3.0f, 3.0f});
                        mActiveExplosions.push_back(ex);

                        // Запускаем задержку перед респавном
                        mIsDeadWaiting = true;
                        mRespawnTimer = 1.2f;
                        mStrikeCount = 0;
                    }
                    else
                    {
                        // Меньше 3 ошибок - звук ошибки и вздрагивание ниндзя
                        if (mErrorSound)
                            mErrorSound->play();
                        mPlayer.triggerErrorPulse();
                    }
                }
            }
        }
    }
}

void Game::processEventsGameOver(const sf::Event &event)
{

}

void Game::update(float dt)
{
    // --- ПОЛУЧАЕМ КООРДИНАТЫ МЫШКИ ---
    // mapPixelToCoords переводит пиксели экрана в координаты нашего интерфейса (hudView)
    sf::Vector2i pixelPos = sf::Mouse::getPosition(mWindow);
    mMousePos = mWindow.mapPixelToCoords(pixelPos, mHudView);

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
    mMenuAnimTimer += dt;
    //
    mCenterTextTimer += dt;
    if (mCenterTextTimer >= 3.0f)
    {
        mCenterTextTimer = 0.f; // Сбрасываем цикл каждые 3 секунды
    }

    updateMusic(dt);

    if (mState == GameState::Playing)
    {
        mStats.updateTime();
        updatePlaying(dt);
    }

    if (mState == GameState::Menu)
    {
        if (std::rand() % 30 == 0)
        {
            mParticles.push_back({{static_cast<float>(std::rand() % WINDOW_W), -20.f},
                                  {static_cast<float>(std::rand() % 20 - 10), 60.f + (std::rand() % 20)},
                                  8.0f});
        }
        for (auto &p : mParticles)
        {
            // Легкое покачивание влево-вправо (эффект ветра)
            p.pos.x += std::sin(mMenuAnimTimer + p.pos.y * 0.01f) * 0.5f;
            p.pos += p.vel * dt; // Падение вниз
            p.lifetime -= dt;    // Уменьшение времени жизни
        }

        // 3. УДАЛЕНИЕ "мертвых" частиц
        mParticles.erase(std::remove_if(mParticles.begin(), mParticles.end(),
                                        [](const Particle &p)
                                        { return p.lifetime <= 0; }),
                         mParticles.end());
    }
    else
    {
        // Очистка списка при выходе из меню в игру
        if (!mParticles.empty())
            mParticles.clear();
    }
}

void Game::updateAllowedChars()
{
    mAllowedChars.clear();

    // Защита: если ничего не выбрано, используем строчные буквы по умолчанию
    if (!mUseLowerCase && !mUseUpperCase && !mUseDigits && !mUseSpecial)
    {
        mUseLowerCase = true;
    }

    if (mCurrentLanguage == Language::English)
    {
        if (mUseLowerCase)
        {
            for (std::uint32_t c = 'a'; c <= 'z'; ++c)
                mAllowedChars.push_back(c);
        }
        if (mUseUpperCase)
        {
            for (std::uint32_t c = 'A'; c <= 'Z'; ++c)
                mAllowedChars.push_back(c);
        }
    }
    else
    {
        // Русские буквы в Юникоде (Кириллица)
        if (mUseLowerCase)
        {
            for (std::uint32_t c = 0x0430; c <= 0x044F; ++c)
                mAllowedChars.push_back(c);
            mAllowedChars.push_back(0x0451); // буква 'ё'
        }
        if (mUseUpperCase)
        {
            for (std::uint32_t c = 0x0410; c <= 0x042F; ++c)
                mAllowedChars.push_back(c);
            mAllowedChars.push_back(0x0401); // буква 'Ё'
        }
    }

    if (mUseDigits)
    {
        for (std::uint32_t c = '0'; c <= '9'; ++c)
            mAllowedChars.push_back(c);
    }

    if (mUseSpecial)
    {
        // Вот та самая полная строка спецсимволов
        std::string spec = "!@#$%^&*()_+-=[]{}|;:',.<>/?";
        for (char c : spec)
        {
            mAllowedChars.push_back(static_cast<std::uint32_t>(static_cast<unsigned char>(c)));
        }
    }
}

void Game::updatePlaying(float dt)
{
    // --- 1. ПЛАВНАЯ КАМЕРА С ОГРАНИЧЕНИЕМ ГРАНИЦ ---
    float currentViewWidth = mWorldView.getSize().x;

    // Обычная цель: игрок чуть левее центра
    float targetCamX = mPlayer.position().x + currentViewWidth / 4.f;

    // ОГРАНИЧЕНИЕ СЛЕВА (Старт)
    targetCamX = std::max(targetCamX, currentViewWidth / 2.f);

    // ОГРАНИЧЕНИЕ СПРАВА (Финиш)
    // Если мы в классическом режиме и скала уже "поставлена" в startGame
    if (mCurrentMode == GameMode::Classic && mShowEndCliff)
    {
        // Рассчитываем предел: камера должна остановиться так, чтобы
        // правый край экрана совпадал с правым краем скалы (или чуть дальше)
        float cliffX = mEndCliffSprite->getPosition().x;

        // maxCamX — это точка, дальше которой центр камеры не пойдет.
        // Мы хотим, чтобы скала была видна у правого края, поэтому вычитаем половину ширины экрана.
        // Добавим +150.f, чтобы скала не была "прилеплена" к самому краю, а была видна целиком.
        float maxCamX = cliffX + 180.f - (currentViewWidth / 2.f);

        targetCamX = std::min(targetCamX, maxCamX);
    }

    float lerpFactor = 1.0f - std::exp(-8.0f * dt);

    sf::Vector2f currentCenter = mWorldView.getCenter();
    float newCamX = currentCenter.x + (targetCamX - currentCenter.x) * lerpFactor;

    mWorldView.setCenter({newCamX, WINDOW_H / 2.f});

    // --- 2. ОБНОВЛЕНИЕ ЧАСТИЦ/ВЗРЫВОВ (Без изменений) ---
    for (auto &ex : mActiveExplosions)
    {
        ex.timer += dt;
        if (ex.timer >= EXPL_SPEED)
        {
            ex.timer = 0;
            ex.currentFrame++;
            if (ex.currentFrame < EXPL_FRAMES)
                ex.sprite.setTextureRect(sf::IntRect({ex.currentFrame * EXPL_SIZE, 0}, {EXPL_SIZE, EXPL_SIZE}));
            else
                ex.finished = true;
        }
    }
    mActiveExplosions.erase(std::remove_if(mActiveExplosions.begin(), mActiveExplosions.end(), [](const Explosion &ex)
                                           { return ex.finished; }),
                            mActiveExplosions.end());

    // --- 3. ЛОГИКА СМЕРТИ (Без изменений) ---
    if (mIsDeadWaiting)
    {
        mRespawnTimer -= dt;
        if (mRespawnTimer <= 0.f)
        {
            mIsDeadWaiting = false;
            mCurrentBalloon = mBalloons.spawnRespawnPlatform(mLastSafeX, BALLOON_START_Y, *mLastSafeTexture);
            if (mCurrentBalloon)
            {
                mPlayer.respawnOn(mCurrentBalloon->position());
                mIsWaitingForTyping = true;
            }
        }
        return;
    }

    // --- 4. ОБНОВЛЕНИЕ МИРА ---
    float cameraRightEdge = mWorldView.getCenter().x + currentViewWidth / 2.f;
    mBalloons.update(dt, cameraRightEdge, mPlayer.position().x);

    if (mCurrentBalloon && !mBalloons.isValid(mCurrentBalloon))
        mCurrentBalloon = nullptr;

    mPlayer.update(dt);

    if (mPlayer.state() == Player::State::OnBalloon && mCurrentBalloon != nullptr)
    {
        mPlayer.setBalloonPosition(mCurrentBalloon->position());

        if (mPlayer.position().y - (PLAYER_HEIGHT / 2.f) > WINDOW_H)
        {
            mPlayer.loseLife();
            if (mExplosionSound)
                mExplosionSound->play();
            mStats.recordMiss();

            sf::Vector2f explosionPos = mPlayer.position();
            if (mCurrentBalloon)
            {
                explosionPos = mCurrentBalloon->position();
                mCurrentBalloon->setState(Balloon::State::Done);
            }
            mCurrentBalloon = nullptr;

            Explosion ex(mRM.texture("explosion"));
            ex.sprite.setTextureRect(sf::IntRect({0, 0}, {EXPL_SIZE, EXPL_SIZE}));
            ex.sprite.setOrigin({EXPL_SIZE / 2.f, EXPL_SIZE / 2.f});
            ex.sprite.setPosition(explosionPos);
            ex.sprite.setScale({2.5f, 2.5f});
            mActiveExplosions.push_back(ex);

            mIsDeadWaiting = true;
            mRespawnTimer = 1.2f;
        }
    }

    if (mWaitingForFinalJump && mPlayer.state() == Player::State::OnBalloon)
    {
        // Увеличиваем таймер ожидания
        mFinalWaitTimer += dt;

        // Ждем, например, 0.8 секунды (можно поменять на 1.0f для большей паузы)
        if (mFinalWaitTimer >= 0.8f)
        {
            if (mEndCliffSprite)
            {
                // Прыгаем на скалу (теперь этот прыжок тоже будет по дуге!)
                sf::Vector2f finishPos = {mEndCliffSprite->getPosition().x + 115.f, PLATFORM_Y - 145.f};
                mPlayer.setFinalJump(true);
                mPlayer.jumpTo(finishPos);
            }

            mWaitingForFinalJump = false;
            mIsFinishing = true;
            mFinalWaitTimer = 0.f; // Сбрасываем для следующего раза
        }
    }

    // --- 6. ПРОВЕРКИ СОСТОЯНИЙ ---
    if (!mPlayer.isAlive() && !mIsDeadWaiting)
    {
        mState = GameState::GameOver;
        mStats.stopTimer();

        if (mCurrentMode == GameMode::Endless)
        {
            mIsNewRecord = HighscoreManager::submitScore(mStats.score());
            mTopScores = HighscoreManager::load();
        }
    }

    if (mIsFinishing)
    {
        // Проверяем: если ниндзя закончил прыжок и уже стоит на платформе
        if (mPlayer.state() == Player::State::OnPlatform)
        {
            // Начинаем отсчет 1.5 секунд
            mWinDelayTimer += dt;

            if (mWinDelayTimer >= 1.f)
            {
                mState = GameState::Win;
                mStats.stopTimer();
                mWinDelayTimer = 0.f; // Сброс
            }
        }
    }
}

void Game::render()
{
    mWindow.clear(sf::Color(61, 94, 135));

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

void Game::updateMusic(float dt)
{
    const float fadeSpeed = 100.f / 0.7f;

    float targetUI = (mState == GameState::Menu ||
                      mState == GameState::GameOver ||
                      mState == GameState::Win)
                         ? 50.f
                         : 0.f;

    // Целевая громкость игры = 100 только во время игры
    float targetGame = (mState == GameState::Playing) ? 100.f : 0.f;

    auto fade = [&](sf::Music &music, float target)
    {
        float current = music.getVolume();
        if (current < target)
        {
            music.setVolume(std::min(target, current + fadeSpeed * dt));
            if (music.getStatus() != sf::Music::Status::Playing)
                music.play();
        }
        else if (current > target)
        {
            music.setVolume(std::max(target, current - fadeSpeed * dt));
            if (music.getVolume() == 0.f)
                music.pause();
        }
    };

    fade(mMusicMenu, targetUI);
    fade(mMusicGame, targetGame);
}

void Game::renderMenu()
{
    mWindow.clear(sf::Color(20, 20, 30));

    // --- 1. ФОН И ПАРАЛЛАКС (Оставляем как есть) ---
    if (mMenuBgSprite)
    {
        sf::Vector2f mouseNormalized = {
            (mMousePos.x - mHudView.getSize().x / 2.f) / (mHudView.getSize().x / 2.f),
            (mMousePos.y - mHudView.getSize().y / 2.f) / (mHudView.getSize().y / 2.f)};

        sf::Vector2f targetOffset = {-mouseNormalized.x * 15.f, -mouseNormalized.y * 10.f};
        mBgOffset += (targetOffset - mBgOffset) * 2.0f * mLastDt;

        float panX = (mHudView.getSize().x / 2.f) + std::sin(mMenuAnimTimer * 0.5f) * 10.f;
        mMenuBgSprite->setPosition({panX + mBgOffset.x, mBgOffset.y});
        mWindow.draw(*mMenuBgSprite);
    }

    // --- 2. ЛЕПЕСТКИ (Оставляем как есть) ---
    for (const auto &p : mParticles)
    {
        sf::CircleShape petal(3.f);
        petal.setScale({1.5f, 1.0f});
        petal.setPosition(p.pos);
        petal.setRotation(sf::degrees(p.pos.y * 0.4f));
        sf::Color sakura(255, 190, 210, static_cast<int>(180 * (p.lifetime / 8.0f)));
        petal.setFillColor(sakura);
        mWindow.draw(petal);
    }

    // --- 3. ЗАГОЛОВОК ---
    float cx = mHudView.getSize().x / 2.f;
    float titleY = 200.f + std::sin(mCenterTextTimer * 1.0f) * 4.f;

    sf::Text title = makeText("Ninja Typing", 160, sf::Color::White, cx, titleY);
    title.setFont(mRM.font("bold"));
    title.setStyle(sf::Text::Style::Bold);
    title.setLetterSpacing(0.7f);
    title.setScale({0.5f, 0.5f});
    sf::FloatRect b = title.getLocalBounds();
    title.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});

    sf::Text shadow = title;
    shadow.setFillColor(sf::Color(0, 0, 0, 150));
    shadow.move({4.f, 4.f});
    mWindow.draw(shadow);
    mWindow.draw(title);

    // --- 4. ПОДГОТОВКА КНОПОК ---
    // Сохраняем позицию мыши и блокируем её, если открыты настройки
    sf::Vector2f realMousePos = mMousePos;
    if (mShowSettings)
        mMousePos = {-1000.f, -1000.f};

    // Объявляем все цвета ОДИН раз здесь
    sf::Color greenBtn(90, 215, 151, 70);
    sf::Color blueBtn(70, 150, 220, 100);
    sf::Color goldenOutline(255, 195, 43, 200);
    sf::Color whiteBtn(255, 255, 255, 70);
    sf::Color exitOutline(180, 180, 180);

    if (drawButton("Classic Mode", cx, 340.f, 260.f, 50.f, greenBtn, sf::Color::White, goldenOutline))
    {
        startGame(GameMode::Classic);
    }

    if (drawButton("Endless Mode", cx, 400.f, 260.f, 50.f, greenBtn, sf::Color::White, goldenOutline))
    {
        startGame(GameMode::Endless);
    }

    if (drawButton("Settings", cx, 460.f, 260.f, 50.f, blueBtn, sf::Color::White, goldenOutline))
    {
        mShowSettings = true;
        updateAllowedChars();
        mIsMouseClicked = false;
    }

    if (drawButton("Exit", cx, 530.f, 180.f, 50.f, whiteBtn, sf::Color(50, 50, 50), exitOutline))
    {
        mWindow.close();
    }

    mMousePos = realMousePos;

    if (mShowSettings)
    {
        renderSettingsModal();
    }
}

bool Game::drawCheckbox(const std::string &textStr, float x, float y, bool &state)
{
    float size = 25.f;
    sf::RectangleShape box({size, size});
    box.setPosition({x, y});
    box.setOrigin({0.f, size / 2.f});
    box.setFillColor(sf::Color(255, 255, 255, 50));
    box.setOutlineThickness(2.f);
    box.setOutlineColor(sf::Color::White);

    bool isHovered = box.getGlobalBounds().contains(mMousePos);
    if (isHovered)
        box.setOutlineColor(sf::Color::Yellow);

    if (state)
    {
        sf::RectangleShape check({size - 10.f, size - 10.f});
        check.setOrigin({(size - 10.f) / 2.f, (size - 10.f) / 2.f});
        check.setPosition({x + size / 2.f, y});
        check.setFillColor(sf::Color(180, 255, 100));
        mWindow.draw(check);
    }

    sf::Text txt = makeText(textStr, 42, sf::Color::White, x + size + 15.f, y);
    txt.setFont(mRM.font("main"));
    txt.setScale({0.32f, 0.32f});
    sf::FloatRect b = txt.getLocalBounds();
    txt.setOrigin({0.f, b.position.y + b.size.y / 2.f});

    mWindow.draw(box);
    mWindow.draw(txt);

    if (isHovered && mIsMouseClicked)
    {
        state = !state;       
        updateAllowedChars(); // Сразу обновляем пул букв
        return true;
    }
    return false;
}

bool Game::drawRadioButton(const std::string &textStr, float x, float y, bool isActive)
{
    float radius = 12.f;
    sf::CircleShape circle(radius);
    circle.setPosition({x, y});
    circle.setOrigin({radius, radius});
    circle.setFillColor(sf::Color(255, 255, 255, 50));
    circle.setOutlineThickness(2.f);
    circle.setOutlineColor(sf::Color::White);

    bool isHovered = circle.getGlobalBounds().contains(mMousePos);
    if (isHovered)
        circle.setOutlineColor(sf::Color::Yellow);

    if (isActive)
    {
        sf::CircleShape dot(6.f);
        dot.setOrigin({6.f, 6.f});
        dot.setPosition({x, y});
        dot.setFillColor(sf::Color(180, 255, 100));
        mWindow.draw(dot);
    }

    sf::Text txt = makeText(textStr, 42, sf::Color::White, x + 25.f, y);
    txt.setFont(mRM.font("main"));
    txt.setScale({0.32f, 0.32f});
    sf::FloatRect b = txt.getLocalBounds();
    txt.setOrigin({0.f, b.position.y + b.size.y / 2.f});

    mWindow.draw(circle);
    mWindow.draw(txt);

    return (isHovered && mIsMouseClicked);
}

void Game::renderSettingsModal()
{
    float cx = mHudView.getSize().x / 2.f;

    sf::RectangleShape overlay({mHudView.getSize().x, (float)WINDOW_H});
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    mWindow.draw(overlay);

    sf::ConvexShape box = createRoundedRect(500.f, 400.f, 20.f);
    box.setPosition({cx, WINDOW_H / 2.f});
    box.setFillColor(sf::Color(40, 75, 115));
    box.setOutlineThickness(4.f);
    box.setOutlineColor(sf::Color(255, 195, 43, 200));
    mWindow.draw(box);

    sf::Text title = makeText("SETTINGS", 70, sf::Color::Yellow, cx, 130.f);
    title.setFont(mRM.font("bold"));
    title.setScale({0.35f, 0.35f});
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
    mWindow.draw(title);

    // Блокировка мыши под окном
    sf::Vector2f realMousePos = mMousePos;
    // Оставляем mMousePos для взаимодействия с элементами окна
    float langY = 190.f;
sf::Text langLabel = makeText("Language:", 60, sf::Color::White, cx - 75.f, langY);
    langLabel.setFont(mRM.font("main"));
    langLabel.setScale({0.35f, 0.35f}); 
    
    sf::FloatRect lb = langLabel.getLocalBounds();
    langLabel.setOrigin({lb.position.x + lb.size.x, lb.position.y + lb.size.y / 2.f});
    mWindow.draw(langLabel);
    if (drawRadioButton("English", cx - 40.f, langY, mCurrentLanguage == Language::English)) {
        mCurrentLanguage = Language::English;
        updateAllowedChars();
    }
    if (drawRadioButton("Russian", cx + 90.f, langY, mCurrentLanguage == Language::Russian)) {
        mCurrentLanguage = Language::Russian;
        updateAllowedChars();
    }

    float startY = 245.f;
    float step = 45.f;
    float startX = cx - 180.f; 

    drawCheckbox("Lower Case Letters", startX, startY, mUseLowerCase);
    drawCheckbox("Upper Case Letters", startX, startY + step, mUseUpperCase);
    drawCheckbox("Digits", startX, startY + step * 2, mUseDigits);
    drawCheckbox("Special symbols", startX, startY + step * 3, mUseSpecial);

    if (drawButton("Save & Close", cx, 460.f, 220.f, 45.f, sf::Color(0, 174, 84, 50), sf::Color::White, sf::Color(255, 195, 43, 200)))
    {
        mShowSettings = false;
        mIsMouseClicked = false; // Также поглощаем клик при выходе
    }
}

void Game::startGame(GameMode mode)
{
    mCurrentMode = mode;
    mStats.reset();
    mFinalWaitTimer = 0.f;

    updateAllowedChars(); // Генерируем вектор mAllowedChars на основе галочек
    mBalloons.setAllowedChars(mAllowedChars);

    // 1. Сброс игрока и чекпоинтов
    int startLives = (mode == GameMode::Classic) ? CLASSIC_LIVES : ENDLESS_LIVES;
    mPlayer.reset(startLives);

    mLastSafeX = PLATFORM_X; // Сбрасываем чекпоинт на начальную платформу
    mLastSafeTexture = &mRM.texture("balloon_0");

    // 2. Сброс менеджера шаров
    mBalloons.reset(mode == GameMode::Classic);
    mCurrentBalloon = nullptr;
    mIsWaitingForTyping = false;

    mIsNewRecord = false;
    mShowLeaderboard = false;

    mStrikeCount = 0;

    // 3. Настройка камеры и платформы
    mPlatform.setPosition({PLATFORM_X, PLATFORM_Y});
    // Мгновенно ставим камеру в начало, чтобы не было рывка
    mWorldView.setCenter({mWorldView.getSize().x / 2.f, WINDOW_H / 2.f});

    // 4. ЛОГИКА ФИНАЛЬНОЙ СКАЛЫ (Только для Classic Mode)
    if (mCurrentMode == GameMode::Classic)
    {
        // Рассчитываем, где физически будет находиться последний шар
        // Логика спавна: PLATFORM_X + SPACING * 1.5 (первый) + (N-1) * SPACING
        float firstBalloonX = PLATFORM_X + BALLOON_SPACING * 1.5f;
        float lastBalloonX = firstBalloonX + (CLASSIC_TARGET_BALLOONS - 1) * BALLOON_SPACING;

        // Ставим скалу через 250 пикселей ПОСЛЕ последнего шара
        float endCliffX = lastBalloonX + 200.f;

        if (mEndCliffSprite)
        {
            mEndCliffSprite->setPosition({endCliffX, 330.f});
            mShowEndCliff = true; // Показываем скалу сразу, она "ждет" в конце уровня
        }
    }
    else
    {
        // В бесконечном режиме скалы не существует
        mShowEndCliff = false;
    }

    // 5. Сброс флагов финала
    mWinDelayTimer = 0.f;
    mIsFinishing = false;
    mWaitingForFinalJump = false;

    mState = GameState::Playing;
}

void Game::renderPlaying()
{
    mWindow.clear(sf::Color(80, 160, 220));

    // Рисуем бесконечный фон, передавая позицию камеры и ширину экрана
    mWindow.setView(mWorldView);
    if (mBackground)
    {
        mBackground->draw(mWindow, mWorldView.getCenter().x, mWorldView.getSize().x);
    }
    if (mCliffSprite)
    {
        mWindow.draw(*mCliffSprite);
    }
    if (mShowEndCliff && mEndCliffSprite)
    {
        mWindow.draw(*mEndCliffSprite);
    }
    mWindow.draw(mPlatform);
    mBalloons.draw(mWindow);
    mPlayer.draw(mWindow);

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
    for (int i = 0; i < maxLives; ++i)
    {
        if (mRM.hasTexture("life"))
        {
            sf::Sprite s(mRM.texture("life"));

            // РАЗМЕР: Высота 24 пикселя (было 32)
            float iconScale = 24.f / s.getLocalBounds().size.y;
            s.setScale({iconScale, iconScale});

            // Выравнивание: левый край спрайта привязан к позиции
            s.setOrigin({0.f, s.getLocalBounds().size.y / 2.f});

            // Позиция: начинаем ровно с startX, шаг между иконками 32 пикселя
            s.setPosition({startX + (i * 32.f), currentY});

            if (i < mPlayer.lives())
                s.setColor(sf::Color::White);
            else
                s.setColor(sf::Color(50, 50, 50, 120));

            mWindow.draw(s);
        }
    }

    // Расстояние между строками стало меньше (45 пикселей)
    currentY -= 45.f;

    // --- 3. РИСУЕМ ПРОГРЕСС (ВЕРХНИЙ РЯД, УМЕНЬШЕННЫЙ) ---
    if (mRM.hasTexture("counter"))
    {
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
    auto drawCenterMessage = [&](const std::string &textStr)
    {
        int alpha = 255;
        if (mCenterTextTimer > 2.5f)
        {
            if (mCenterTextTimer <= 2.75f)
                alpha = static_cast<int>(255.f * (1.f - (mCenterTextTimer - 2.5f) / 0.25f));
            else
                alpha = static_cast<int>(255.f * ((mCenterTextTimer - 2.75f) / 0.25f));
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

    if (mPlayer.state() == Player::State::OnPlatform)
        drawCenterMessage("Type letter to begin");
    else if (mIsWaitingForTyping)
        drawCenterMessage("Keep typing...");
}

void Game::renderGameOver()
{
    float cx = mHudView.getSize().x / 2.f;

    std::string titleText = "";
    sf::Color titleColor = sf::Color::White;
    float pulseScale = 1.0f;

    if (mCurrentMode == GameMode::Endless)
    {
        if (mIsNewRecord)
        {
            titleText = "NEW RECORD!";
            titleColor = sf::Color(255, 215, 0); // Золотой
            pulseScale = 1.0f + 0.05f * std::sin(mMenuAnimTimer * 3.0f);
        }
    }
    else
    {
        if (mState == GameState::Win)
        {
            titleText = "YOU WIN!";
            titleColor = sf::Color(141, 255, 52); 
        }
        else
        {
            titleText = "GAME OVER";
            titleColor = sf::Color(255, 52, 86); 
        }
    }

    if (!titleText.empty())
    {
        sf::Text over = makeText(titleText, 140, titleColor, cx, 80.f);
        over.setFont(mRM.font("bold"));
        over.setStyle(sf::Text::Style::Bold);
        over.setScale({0.4f * pulseScale, 0.4f * pulseScale});

        sf::FloatRect b = over.getLocalBounds();
        over.setOrigin({b.position.x + b.size.x / 2.f, b.position.y + b.size.y / 2.f});

        sf::Text shadow = over;
        shadow.setFillColor(sf::Color(0, 0, 0, 150));
        shadow.move({2.f, 2.f});

        mWindow.draw(shadow);
        mWindow.draw(over);
    }

    auto drawStatWidget = [&](float x, float y, float radius, float percent, sf::Color ringColor, const std::string &mainVal, const std::string &subVal, const std::string &labelName)
    {
        // А. Рисуем темный базовый круг (подложка)
        sf::CircleShape base(radius);
        base.setOrigin({radius, radius});
        base.setPosition({x, y});
        base.setFillColor(sf::Color(40, 60, 90, 200)); 
        base.setOutlineThickness(7.f);
        base.setOutlineColor(sf::Color(66, 102, 150)); 
        mWindow.draw(base);

        int segments = 200; // Качество круга (чем больше, тем круглее)
        int activeSegments = static_cast<int>(segments * percent);

        if (activeSegments > 0)
        {
            float thickness = 5.f; // Толщина цветного кольца
            // TriangleStrip строит кольцо, соединяя точки зигзагом
            sf::VertexArray ring(sf::PrimitiveType::TriangleStrip, (activeSegments + 1) * 2);

            float angleStep = (2.f * 3.141592654f) / segments;
            float startAngle = -3.141592654f / 2.f; // Начинаем с 12 часов (верх)

            for (int i = 0; i <= activeSegments; ++i)
            {
                float angle = startAngle + i * angleStep;
                float cosA = std::cos(angle);
                float sinA = std::sin(angle);

                // Внутренняя точка
                ring[i * 2].position = {x + (radius - thickness) * cosA, y + (radius - thickness) * sinA};
                ring[i * 2].color = ringColor;

                // Внешняя точка
                ring[i * 2 + 1].position = {x + radius * cosA, y + radius * sinA};
                ring[i * 2 + 1].color = ringColor;
            }
            mWindow.draw(ring);
        }

        // В. Тексты внутри и под кругом
        auto drawCenteredText = [&](const std::string &txt, float ty, int size, sf::Color col, bool isBold)
        {
            sf::Text t(mRM.font("light"), txt, size);
            t.setFillColor(col);
            if (isBold)
                t.setStyle(sf::Text::Style::Bold);
            t.setScale({0.35f, 0.35f});
            sf::FloatRect bounds = t.getLocalBounds();
            t.setOrigin({bounds.position.x + bounds.size.x / 2.f, bounds.position.y + bounds.size.y / 2.f});
            t.setPosition({x, ty});
            t.setLetterSpacing(.85f);
            mWindow.draw(t);
        };

        drawCenteredText(mainVal, y - 10.f, 100, sf::Color::White, true);           
        drawCenteredText(subVal, y + 25.f, 34, sf::Color(200, 200, 200), false);   
        drawCenteredText(labelName, y + radius + 30.f, 60, sf::Color::White, true); 
    };

    float widgetY = 260.f; // Высота центров кругов
    float rad = 80.f;      // Радиус кругов

    // Форматирование чисел до 1 знака после запятой
    std::ostringstream accStr, timeStr, cpmStr;
    accStr << std::fixed << std::setprecision(1) << mStats.accuracy() << "%";
    timeStr << std::fixed << std::setprecision(1) << mStats.time() << "s";
    cpmStr << std::fixed << std::setprecision(0) << mStats.cpm();

    float accPercent = mStats.accuracy() / 100.f;
    sf::Color accColor = (accPercent > 0.9f) ? sf::Color(255, 200, 50) : sf::Color(220, 80, 80); // Желтый если > 90%, иначе красный
    drawStatWidget(cx - 220.f, widgetY, rad, accPercent, accColor, accStr.str(), "real accuracy", "accuracy");

    drawStatWidget(cx, widgetY - 20.f, rad * 0.85f, 1.0f, sf::Color(100, 150, 200), timeStr.str(), "duration", "time");

    float cpmPercent = std::min(1.0f, mStats.cpm() / 250.f);
    drawStatWidget(cx + 220.f, widgetY, rad, cpmPercent, sf::Color(255, 200, 50), cpmStr.str(), "cpm", "speed");

    // --- 4. ИТОГОВЫЙ СЧЕТ (ПОД КРУГАМИ) ---
    sf::Text scoreTxt = makeText(std::to_string(mStats.score()), 120, sf::Color::White, cx, 400.f);
    scoreTxt.setFont(mRM.font("VR-regular"));
    scoreTxt.setScale({0.5f, 0.5f});
    sf::FloatRect scB = scoreTxt.getLocalBounds();
    scoreTxt.setOrigin({scB.position.x + scB.size.x / 2.f, scB.position.y + scB.size.y / 2.f});
    mWindow.draw(scoreTxt);

    sf::Text scoreLabel = makeText("TOTAL SCORE", 50, sf::Color(200, 200, 200), cx, 440.f);
    scoreLabel.setFont(mRM.font("light"));
    scoreLabel.setScale({0.35f, 0.35f});
    sf::FloatRect slB = scoreLabel.getLocalBounds();
    scoreLabel.setOrigin({slB.position.x + slB.size.x / 2.f, slB.position.y + slB.size.y / 2.f});
    mWindow.draw(scoreLabel);

    sf::RectangleShape line({200.f, 1.7f});
    line.setOrigin({100.f, 1.f});
    line.setPosition({cx, 425.f});
    line.setFillColor(sf::Color(255, 255, 255, 100));
    mWindow.draw(line);

    // Защита от кликов сквозь модальное окно
    sf::Vector2f realMousePos = mMousePos;
    if (mShowLeaderboard)
        mMousePos = {-1000.f, -1000.f};

    // --- 4. КНОПКИ (РАЗДЕЛЕННАЯ ЛОГИКА) ---
    sf::Color whiteBtn(255, 255, 255, 70);
    sf::Color greenBtn(90, 215, 151, 70);
    sf::Color blueBtn(70, 150, 220, 100);
    sf::Color goldenOutline(255, 195, 43, 200);
    sf::Color exitOutline(180, 180, 180);

    if (mCurrentMode == GameMode::Endless)
    {
        // ТРИ КНОПКИ В РЯД (С таблицей рекордов)
        if (drawButton("Play Again", cx - 240.f, 520.f, 200.f, 50.f, greenBtn, sf::Color::White, goldenOutline))
        {
            startGame(mCurrentMode);
        }
        if (drawButton("Best Results", cx, 520.f, 220.f, 50.f, blueBtn, sf::Color::White, goldenOutline))
        {
            mShowLeaderboard = true;
        }
        if (drawButton("Back to Menu", cx + 240.f, 520.f, 200.f, 50.f, whiteBtn, sf::Color::White, exitOutline))
        {
            mState = GameState::Menu;
        }
    }
    else
    {
        if (drawButton("Play Again", cx - 130.f, 520.f, 200.f, 50.f, greenBtn, sf::Color::White, goldenOutline))
        {
            startGame(mCurrentMode);
        }
        if (drawButton("Back to Menu", cx + 130.f, 520.f, 200.f, 50.f, whiteBtn, sf::Color::White, exitOutline))
        {
            mState = GameState::Menu;
        }
    }

    mMousePos = realMousePos; // Возвращаем мышь

    if (mShowLeaderboard)
    {
        // Темный полупрозрачный фон на весь экран
        sf::RectangleShape overlay({mHudView.getSize().x, (float)WINDOW_H});
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        mWindow.draw(overlay);

        sf::ConvexShape modalBox = createRoundedRect(400.f, 380.f, 20.f);
        modalBox.setPosition({cx, WINDOW_H / 2.f});
        modalBox.setFillColor(sf::Color(40, 75, 115)); 
        modalBox.setOutlineThickness(4.f);
        modalBox.setOutlineColor(sf::Color(255, 195, 43, 200)); 
        mWindow.draw(modalBox);

        sf::Text title = makeText("TOP 5 RESULTS", 70, sf::Color::White, cx, 160.f);
        title.setFont(mRM.font("bold"));
        title.setScale({0.4f, 0.4f});
        sf::FloatRect tb = title.getLocalBounds();
        title.setOrigin({tb.position.x + tb.size.x / 2.f, tb.position.y + tb.size.y / 2.f});
        mWindow.draw(title);

        sf::RectangleShape line({300.f, 2.f});
        line.setOrigin({150.f, 1.f});
        line.setPosition({cx, 190.f});
        line.setFillColor(sf::Color(255, 255, 255, 100));
        mWindow.draw(line);

        float anchorX = cx - 25.f;

        float horizontalGap = 80.f;

        for (size_t i = 0; i < mTopScores.size(); ++i)
        {
            float rowY = 230.f + i * 42.f; 

            sf::Color rowColor = (mTopScores[i] == mStats.score() && mStats.score() > 0) ? sf::Color(255, 215, 0) : sf::Color::White;

            sf::Text numTxt(mRM.font("main"), std::to_string(i + 1) + ".", 64);
            numTxt.setFillColor(rowColor);
            numTxt.setScale({0.35f, 0.35f});

            sf::FloatRect nB = numTxt.getLocalBounds();
            // Origin в правый край (position.x + size.x)
            numTxt.setOrigin({nB.position.x + nB.size.x, nB.position.y + nB.size.y / 2.f});
            numTxt.setPosition({cx - (horizontalGap), rowY});
            mWindow.draw(numTxt);

            sf::Text ptsTxt(mRM.font("main"), "  " + std::to_string(mTopScores[i]) + " points", 64);
            ptsTxt.setFillColor(rowColor);
            ptsTxt.setScale({0.35f, 0.35f});

            sf::FloatRect pB = ptsTxt.getLocalBounds();
            // Origin в левый край (position.x)
            ptsTxt.setOrigin({pB.position.x, pB.position.y + pB.size.y / 2.f});
            ptsTxt.setPosition({cx - (horizontalGap / 2.f), rowY});
            // ptsTxt.setPosition({anchorX, rowY});
            mWindow.draw(ptsTxt);
        }

        // Кнопка "Х" для закрытия (в правом верхнем углу плашки)
        if (drawButton("X", cx + 160.f, 150.f, 40.f, 40.f, sf::Color(220, 80, 80, 80), sf::Color::White, sf::Color::White))
        {
            mShowLeaderboard = false;
        }
    }
}

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

// --- ГЕНЕРАТОР ЗАКРУГЛЕННЫХ ПРЯМОУГОЛЬНИКОВ ---
sf::ConvexShape Game::createRoundedRect(float width, float height, float radius)
{
    sf::ConvexShape shape;
    int pointsPerCorner = 10;
    shape.setPointCount(pointsPerCorner * 4);

    float pi = 3.141592654f;
    int pointIdx = 0;

    // Функция для генерации дуги
    auto addCorner = [&](float cx, float cy, float startAngle)
    {
        for (int i = 0; i < pointsPerCorner; ++i)
        {
            float angle = startAngle + (i * (pi / 2.f) / (pointsPerCorner - 1));
            shape.setPoint(pointIdx++, sf::Vector2f(cx + radius * std::cos(angle), cy + radius * std::sin(angle)));
        }
    };

    // 4 угла
    addCorner(width - radius, height - radius, 0.f);   // Правый нижний
    addCorner(radius, height - radius, pi / 2.f);      // Левый нижний
    addCorner(radius, radius, pi);                     // Левый верхний
    addCorner(width - radius, radius, pi * 3.f / 2.f); // Правый верхний

    // Центрируем Origin, чтобы кнопка масштабировалась из центра
    shape.setOrigin({width / 2.f, height / 2.f});
    return shape;
}

// --- УМНАЯ КНОПКА (IMGUI) ---
bool Game::drawButton(const std::string &textStr, float x, float y, float width, float height, sf::Color bgColor, sf::Color textColor, sf::Color outlineColor)
{
    // 1. АНИМАЦИЯ НАВЕДЕНИЯ
    float &progress = mButtonHoverProgress[textStr];

    sf::FloatRect hitbox({x - width / 2.f, y - height / 2.f}, {width, height});
    bool isHovered = hitbox.contains(mMousePos);

    float animSpeed = 1.0f / 0.2f;
    if (isHovered)
    {
        progress += mLastDt * animSpeed;
        if (progress > 1.0f)
            progress = 1.0f;
    }
    else
    {
        progress -= mLastDt * animSpeed;
        if (progress < 0.0f)
            progress = 0.0f;
    }

    float currentScale = 1.0f + (0.05f * progress);

    sf::Color currentColor = bgColor;
    currentColor.r = std::min(255, currentColor.r + static_cast<int>(30 * progress));
    currentColor.g = std::min(255, currentColor.g + static_cast<int>(30 * progress));
    currentColor.b = std::min(255, currentColor.b + static_cast<int>(30 * progress));

    // 2. ОТРИСОВКА КНОПКИ
    sf::ConvexShape btnShape = createRoundedRect(width, height, 20.f);
    btnShape.setPosition({x, y});
    btnShape.setScale({currentScale, currentScale});
    btnShape.setFillColor(currentColor);
    btnShape.setOutlineThickness(1.3f);
    btnShape.setOutlineColor(outlineColor);

    mWindow.draw(btnShape);

    // 3. ОТРИСОВКА ТЕКСТА КНОПКИ (С ОПТИЧЕСКИМ ВЫРАВНИВАНИЕМ)
    // textOffsetY можно оставить -2.f или поставить 0.f, так как мы меняем Origin
    float textOffsetY = 0.f;
    sf::Text txt = makeText(textStr, 70, textColor, x, y + textOffsetY);
    txt.setFont(mRM.font("VR-regular"));
    txt.setScale({0.4f * currentScale, 0.4f * currentScale});

    sf::FloatRect b = txt.getLocalBounds();

    // --- МАГИЯ ОПТИЧЕСКОГО ЦЕНТРИРОВАНИЯ ---
    // Вместо b.position.y + b.size.y / 2.f (который включает хвосты букв y, g, p)
    // мы используем фиксированную высоту "тела" букв.
    // Для размера 70 число 28.f — это примерно середина заглавных букв.
    float opticalCenterY = 47.f;

    txt.setOrigin({
        b.position.x + b.size.x / 2.f, // По горизонтали оставляем точный центр
        opticalCenterY                 // По вертикали фиксируем "глазную" середину
    });

    mWindow.draw(txt);

    return (isHovered && mIsMouseClicked);
}