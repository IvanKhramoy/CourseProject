#pragma once
#include <string>

constexpr unsigned int WINDOW_W   = 800;
constexpr unsigned int WINDOW_H   = 600;
constexpr unsigned int FPS_LIMIT  = 60;

// ─── Игрок и Окружение ───────────────────────────────────
constexpr int   CLASSIC_LIVES      = 2;
constexpr int   ENDLESS_LIVES      = 1;
constexpr float PLAYER_WIDTH       = 40.f;
constexpr float PLAYER_HEIGHT      = 60.f;
constexpr float JUMP_DURATION      = 0.2f;
constexpr float PLATFORM_X         = 100.f;
constexpr float PLATFORM_Y         = 450.f;
constexpr float FALL_LIMIT         = WINDOW_H - 20.f; // Граница падения

// ─── Режимы Игры ─────────────────────────────────────────
constexpr int   CLASSIC_TARGET_BALLOONS = 50;

// ─── Шары ────────────────────────────────────────────────
constexpr float BALLOON_RADIUS     = 40.f;
constexpr float BALLOON_SPACING    = 200.f;
constexpr float BALLOON_START_Y    = 150.f;
constexpr float INITIAL_FALL_SPEED = 100.f;
constexpr float SPEED_INCREMENT    = 5.f;
constexpr float MAX_FALL_SPEED     = 400.f;

constexpr int   SCORE_PER_HIT      = 10;

namespace Paths {
    inline const std::string FONT       = "../assets/fonts/arial.ttf";
    inline const std::string FONT_FALLBACK = "/System/Library/Fonts/Supplemental/Arial.ttf";
}