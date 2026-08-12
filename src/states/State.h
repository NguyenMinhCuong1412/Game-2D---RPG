#pragma once
#include "common/lib.h"
#include "states/StateManager.h"

// Abstract Base Class (Lớp cơ sở trừu tượng) -> định nghĩa một Interface (giao diện chung) cho tất cả các trạng thái trong game
class State {
protected:
    StateManager& mStateManager; // Lưu tham chiếu tới bộ quản lý State
public:
    State(StateManager& stateManager) : mStateManager(stateManager) {}
    virtual ~State() = default; // Destructor ảo mặc định -> tránh gây rò rỉ bộ nhớ nghiêm trọng khi hủy lớp cha
    
    // Hàm thuần ảo (pure virtual function) - ép các lớp con buộc phải cài đặt cụ thể hàm
    virtual void handleEvent(const sf::Event& event) = 0; // Lắng nghe phím/chuột đặc thù của màn hình đó
    virtual void update(float dt) = 0; // Cập nhật logic toán học của màn hình đó
    virtual void draw(sf::RenderWindow& window) = 0; // Vẽ giao diện đặc thù của màn hình đó
};
