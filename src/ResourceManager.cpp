#include "ResourceManager.h"

void ResourceManager::loadFont(const std::string& id, const std::string& path) {
    sf::Font font;
    if (!font.openFromFile(path)) 
        throw std::runtime_error("Cannot load font: " + path);
    mFonts[id] = std::move(font);
}

const sf::Font& ResourceManager::font(const std::string& id) const {
    return mFonts.at(id);
}

void ResourceManager::loadTexture(const std::string& id, const std::string& path) {
    sf::Texture tex;
    if (!tex.loadFromFile(path))
        throw std::runtime_error("Cannot load texture: " + path);
    mTextures[id] = std::move(tex);
}

const sf::Texture& ResourceManager::texture(const std::string& id) const {
    return mTextures.at(id);
}

bool ResourceManager::hasTexture(const std::string& id) const {
    return mTextures.count(id) > 0;
}