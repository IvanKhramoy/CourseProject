#include "Background.h"
#include <cmath>

// Передаем текстуру в mSprite через список инициализации (двоеточие)
Background::Background(const sf::Texture& texture, float targetHeight) 
    : mSprite(texture) 
{
    // Теперь mSprite.setTexture(texture) писать не нужно, она уже установлена!
    
    // Подгоняем высоту картинки под высоту окна (600)
    // Добавил static_cast<float>, чтобы компилятор не ругался на деление
    float scaleY = targetHeight / static_cast<float>(texture.getSize().y);
    mSprite.setScale({scaleY, scaleY});
    
    // Запоминаем новую физическую ширину картинки после масштабирования
    mScaledWidth = static_cast<float>(texture.getSize().x) * scaleY;
}

void Background::draw(sf::RenderWindow& window, float cameraX, float viewWidth) const {
    float leftEdge = cameraX - (viewWidth / 2.f);
    int startTileIndex = static_cast<int>(std::floor(leftEdge / mScaledWidth));
    int numTilesToDraw = static_cast<int>(std::ceil(viewWidth / mScaledWidth)) + 2;

    for (int i = 0; i < numTilesToDraw; ++i) {
        sf::Sprite tempSprite = mSprite; 
        tempSprite.setPosition({(startTileIndex + i) * mScaledWidth, 0.f});
        window.draw(tempSprite);
    }
}