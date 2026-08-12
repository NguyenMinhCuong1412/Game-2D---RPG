#include "states/StateManager.h"
#include "states/State.h"

StateManager::StateManager() = default;
StateManager::~StateManager() = default;

void StateManager::applyPendingChanges() {
    // Xóa toàn bộ State cũ nếu có cờ Clear
    if (mClearStates) {
        mStates.clear();
        mClearStates = false;
    }

    // Pop các State ra theo số lượng yêu cầu
    while (mPendingPopCount > 0 && !mStates.empty()) {
        mStates.pop_back(); // Giải phóng State trên cùng (unique_ptr tự giải phóng RAM)
        mPendingPopCount--;
    }
    mPendingPopCount = 0;

    // Push các State mới từ danh sách chờ vào danh sách hoạt động chính
    for (auto& pushInfo : mPendingPush) mStates.push_back({ std::move(pushInfo.first), pushInfo.second });
    mPendingPush.clear();
}

void StateManager::pushState(std::unique_ptr<State> state, StateID id) {
    if (!mClearStates) // Không có lệnh xóa toàn bộ State đang chạy
        for (const auto& info : mStates) if (info.id == id) return; // Kiểm tra xem State này đã nằm trong danh sách đang hoạt động
    for (const auto& pair : mPendingPush) if (pair.second == id) return;  // Kiểm tra xem State này đã nằm trong hàng đợi chờ nạp chưa
    mPendingPush.push_back({std::move(state), id}); // Đưa State mới vào danh sách chờ xử lý cuối frame
}

void StateManager::popState() { mPendingPopCount++; } // Chỉ tăng số lượng chờ xóa, tránh xóa trực tiếp gây crash game

void StateManager::changeState(std::unique_ptr<State> state, StateID id) {
    for (const auto& pair : mPendingPush) if (pair.second == id) return; // Kiểm tra xem State này đã có sẵn trong danh sách chờ nạp
    mClearStates = true; // Kích hoạt cờ yêu cầu dọn dẹp sạch sẽ toàn bộ State cũ
    mPendingPush.push_back({std::move(state), id}); // Đẩy State mới vào hàng chờ nạp
}

void StateManager::handleEvent(const sf::Event& event) {
    if (!mStates.empty()) mStates.back().state->handleEvent(event);  // Chỉ gửi sự kiện cho duy nhất State đang hiển thị trên cùng (back của vector)
    applyPendingChanges(); // Cập nhật ngay các thay đổi về State nếu có phát sinh yêu cầu chuyển đổi
}

void StateManager::update(float dt) {
    if (!mStates.empty()) mStates.back().state->update(dt); // Chỉ cập nhật logic cho duy nhất State trên cùng
    applyPendingChanges(); // Thực thi các yêu cầu chuyển đổi State an toàn
}

void StateManager::draw(sf::RenderWindow& window) { for (auto& info : mStates) info.state->draw(window); }