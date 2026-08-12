#pragma once
#include "common/lib.h"

// Abstract Base Class (Lớp cơ sở trừu tượng) - toàn bộ các đối tượng chuyển động trong game
class Entity {
protected:
    sf::Vector2f mPosition; // Lưu trữ tọa độ 2 chiều (x, y) kiểu số thực trung tâm của đối tượng
public:
    Entity() : mPosition(0.f, 0.f) {}
    virtual ~Entity() = default;

    // Định nghĩa giao diện cập nhật trạng thái (update) và vẽ hình (draw) bắt buộc mọi lớp con phải có
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    // Đọc và thay đổi vị trí đối tượng
    sf::Vector2f getPosition() const { return mPosition; }
    void setPosition(sf::Vector2f position) { mPosition = position; }
};
