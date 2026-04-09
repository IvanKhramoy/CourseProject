#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <string>
#include <stdexcept>

// ─────────────────────────────────────────────────────────
//  ResourceManager
//  Загружает ресурсы один раз и раздаёт const-ссылки.
//  Использование:
//    ResourceManager rm;
//    rm.loadFont("main", "../assets/fonts/arial.ttf");
//    const sf::Font& f = rm.font("main");
// ─────────────────────────────────────────────────────────
class ResourceManager
{
public:
    // ── Шрифты ───────────────────────────────────────────
    void loadFont(const std::string& id, const std::string& path) {
    sf::Font font;
    if (!font.openFromFile(path)) // В SFML 3 используем openFromFile
        throw std::runtime_error("Cannot load font: " + path);
    mFonts[id] = std::move(font);
}

    const sf::Font &font(const std::string &id) const
    {
        return mFonts.at(id);
    }

    // ── Текстуры ─────────────────────────────────────────
    void loadTexture(const std::string &id, const std::string &path)
    {
        sf::Texture tex;
        if (!tex.loadFromFile(path))
            throw std::runtime_error("Cannot load texture: " + path);
        mTextures[id] = std::move(tex);
    }

    const sf::Texture &texture(const std::string &id) const
    {
        return mTextures.at(id);
    }

    bool hasTexture(const std::string &id) const
    {
        return mTextures.count(id) > 0;
    }

private:
    std::map<std::string, sf::Font> mFonts;
    std::map<std::string, sf::Texture> mTextures;
};
