#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <stdexcept>

class ResourceManager {
public:
    void loadFont(const std::string& id, const std::string& path);
    const sf::Font& font(const std::string& id) const;

    void loadTexture(const std::string& id, const std::string& path);
    const sf::Texture& texture(const std::string& id) const;
    bool hasTexture(const std::string& id) const;

private:
    std::map<std::string, sf::Font> mFonts;
    std::map<std::string, sf::Texture> mTextures;
};