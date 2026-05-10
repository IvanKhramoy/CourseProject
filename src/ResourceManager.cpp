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

// const sf::Texture& ResourceManager::texture(const std::string& id) const {
//     return mTextures.at(id);
// }

const sf::Texture& ResourceManager::texture(const std::string& id) const {
    auto it = mTextures.find(id);
    if (it == mTextures.end()) {
        // ВМЕСТО ВЫЛЕТА МЫ ПИШЕМ ЭТО:
        printf("\n[!!!] CRITICAL: Texture '%s' is missing!\n", id.c_str());
        printf("[!!!] Check your 'assets/textures' folder and naming.\n");

        // Пытаемся вернуть хоть что-то, чтобы игра не упала
        if (!mTextures.empty()) {
            return mTextures.begin()->second; 
        }
        
        // Если вообще ничего нет, тогда только вылет
        throw std::runtime_error("No textures loaded at all. Missing: " + id);
    }
    return it->second;
}

bool ResourceManager::hasTexture(const std::string& id) const {
    return mTextures.count(id) > 0;
}

void ResourceManager::loadSound(const std::string& id, const std::string& path) {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(path))
        throw std::runtime_error("Cannot load sound: " + path);
    mSounds[id] = std::move(buffer);
}

const sf::SoundBuffer& ResourceManager::sound(const std::string& id) const {
    return mSounds.at(id);
}