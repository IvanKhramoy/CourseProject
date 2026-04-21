#pragma once
#include <SFML/Graphics.hpp>

class Background {
public:
    // Принимаем текстуру и целевую высоту (в нашем случае 600.f)
    Background(const sf::Texture& texture, float targetHeight);

    // Метод отрисовки, который сам посчитает, сколько раз отрисовать фон
    void draw(sf::RenderWindow& window, float cameraX, float viewWidth) const;

private:
    sf::Sprite mSprite;
    float mScaledWidth; // Ширина картинки после подгонки под высоту 600
};