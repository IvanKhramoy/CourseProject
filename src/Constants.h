#pragma once
#include <string>

constexpr unsigned int WINDOW_W   = 800;
constexpr unsigned int WINDOW_H   = 600;
constexpr unsigned int FPS_LIMIT  = 60;

// ─── Игрок и Окружение ───────────────────────────────────
constexpr int   CLASSIC_LIVES      = 2;
constexpr int   ENDLESS_LIVES      = 1;
constexpr float PLAYER_WIDTH       = 32.f;  // Уменьшили (было 48)
constexpr float PLAYER_HEIGHT      = 48.f;  // Уменьшили (было 72)
constexpr float JUMP_DURATION      = 0.2f;
constexpr float PLATFORM_X         = 40.f;  // Сдвинули к самому левому краю (было 100)
constexpr float PLATFORM_Y         = 450.f;
constexpr float FALL_LIMIT         = WINDOW_H - 20.f;

// ─── Режимы Игры ─────────────────────────────────────────
constexpr int   CLASSIC_TARGET_BALLOONS = 50;

// ─── Шары ────────────────────────────────────────────────
constexpr float BALLOON_RADIUS     = 32.f;  // Уменьшили (было 48)
constexpr float BALLOON_SPACING    = 130.f; // Сильно сблизили шары (было 240)
constexpr float BALLOON_START_Y    = 120.f; // Чуть приподняли их (было 150)
constexpr int   BALLOON_Y_VARIATION = 40;
constexpr float INITIAL_FALL_SPEED = 100.f;
constexpr float SPEED_INCREMENT    = 5.f;
constexpr float MAX_FALL_SPEED     = 400.f;

constexpr int   SCORE_PER_HIT      = 10;

namespace Paths {
    inline const std::string FONT          = "assets/fonts/arial.ttf";
    inline const std::string FONT_RM_300   = "assets/fonts/RobotoMono-300.ttf";
    inline const std::string FONT_RM_400   = "assets/fonts/RobotoMono-400.ttf";
    inline const std::string FONT_RM_600   = "assets/fonts/RobotoMono-600.ttf";
    inline const std::string FONT_RM_700   = "assets/fonts/RobotoMono-700.ttf";
    inline const std::string FONT_CO_400   = "assets/fonts/ConcertOne-Regular.ttf";
    inline const std::string FONT_FALLBACK = "/System/Library/Fonts/Supplemental/Arial.ttf";
    // inline const std::string FONT_FALLBACK = "/System/Library/Fonts/Supplemental/Arial.ttf";
    // inline const std::string FONT_RM_300 = "/System/Library/Fonts/Supplemental/Arial.ttf";
    // inline const std::string FONT_RM_400 = "/System/Library/Fonts/Supplemental/Arial.ttf";
    // inline const std::string FONT_RM_600 = "/System/Library/Fonts/Supplemental/Arial.ttf";
    // inline const std::string FONT_RM_700 = "/System/Library/Fonts";
    inline const std::string BG_TEXTURE    = "assets/textures/Back.png";

    inline const std::string CLIFF_TEXTURE = "assets/textures/Cliff.png";

    inline const std::string BALLOON_PREFIX = "assets/textures/balloon_";
    constexpr int BALLOON_COLORS_COUNT = 4;
    // inline const std::string FONT       = "../assets/fonts/arial.ttf";
    // inline const std::string FONT_FALLBACK = "/System/Library/Fonts/Supplemental/Arial.ttf";
    // inline const std::string BG_TEXTURE    = "../assets/textures/Back.jpg";
}