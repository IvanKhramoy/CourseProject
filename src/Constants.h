#pragma once
#include <string>

constexpr unsigned int WINDOW_W   = 800;
constexpr unsigned int WINDOW_H   = 600;
constexpr unsigned int FPS_LIMIT  = 60;

// ─── Игрок и Окружение ───────────────────────────────────
constexpr int   CLASSIC_LIVES      = 3;
constexpr int   ENDLESS_LIVES      = 1;
constexpr float PLAYER_WIDTH       = 32.f;  // Уменьшили (было 48)
constexpr float PLAYER_HEIGHT      = 48.f;  // Уменьшили (было 72)
constexpr float JUMP_DURATION      = 0.4f;
constexpr float PLATFORM_X         = 40.f;  // Сдвинули к самому левому краю (было 100)
constexpr float PLATFORM_Y         = 450.f;
constexpr float FALL_LIMIT         = WINDOW_H + 50.f;

// ─── Режимы Игры ─────────────────────────────────────────
constexpr int   CLASSIC_TARGET_BALLOONS = 50;

// ─── Шары ────────────────────────────────────────────────
constexpr float BALLOON_RADIUS     = 37.f;  // Уменьшили (было 48)
constexpr float BALLOON_SPACING    = 140.f; // Сильно сблизили шары (было 240)
constexpr float BALLOON_START_Y    = 120.f; // Чуть приподняли их (было 150)
constexpr int   BALLOON_Y_VARIATION = 40;
constexpr float INITIAL_FALL_SPEED = 65.f;
constexpr float SPEED_INCREMENT    = 1.f;
constexpr float MAX_FALL_SPEED     = 250.f;

constexpr int   SCORE_PER_HIT      = 10;

namespace Paths {
    inline const std::string FONT          = "assets/fonts/arial.ttf";
    inline const std::string FONT_RM_300   = "assets/fonts/RobotoMono-300.ttf";
    inline const std::string FONT_RM_400   = "assets/fonts/RobotoMono-400.ttf";
    inline const std::string FONT_RM_600   = "assets/fonts/RobotoMono-600.ttf";
    inline const std::string FONT_RM_700   = "assets/fonts/RobotoMono-700.ttf";
    inline const std::string FONT_CO_400   = "assets/fonts/ConcertOne-Regular.ttf";
    inline const std::string FONT_VR_400   = "assets/fonts/VarelaRound-Regular.ttf";
    inline const std::string FONT_FALLBACK = "/System/Library/Fonts/Supplemental/Arial.ttf";
    inline const std::string EXPLOSION_TEXTURE = "assets/textures/explosion.png";
    inline const std::string LIFE_TEXTURE = "assets/textures/avatar.png";
    inline const std::string COUNTER_TEXTURE = "assets/textures/counter.png";

    inline const std::string BG_TEXTURE    = "assets/textures/Back.png";

    inline const std::string CLIFF_TEXTURE = "assets/textures/Cliff.png";

    inline const std::string BALLOON_PREFIX = "assets/textures/balloon_";

    inline const std::string NINJA_1 = "assets/textures/ninja-1.png";
    inline const std::string NINJA_2 = "assets/textures/ninja-2.png";
    inline const std::string NINJA_3 = "assets/textures/ninja-3.png";
    inline const std::string NINJA_4 = "assets/textures/ninja-4.png";
    inline const std::string NINJA_5 = "assets/textures/ninja-5.png";
    constexpr int BALLOON_COLORS_COUNT = 4;
    // inline const std::string FONT       = "../assets/fonts/arial.ttf";
    // inline const std::string FONT_FALLBACK = "/System/Library/Fonts/Supplemental/Arial.ttf";
    // inline const std::string BG_TEXTURE    = "../assets/textures/Back.jpg";
}