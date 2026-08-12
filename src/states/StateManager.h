#pragma once
#include "common/lib.h"

class State;

enum class StateID {
    None,
    MainMenu,
    CharacterSelection,
    Playing,
    Pause,
    GameOver,
    Victory,
    LoadGame,
    HowToPlay,
    Settings
};

// Bộ quản lý chứa các State - xử lý logic và vẽ State nằm trên cùng Stack
class StateManager {
private: 
    struct StateInfo { // Thông tin của 1 State
        std::unique_ptr<State> state; // Con trỏ trỏ tới State đó
        StateID id; // ID định danh của State đó
    };
    std::vector<StateInfo> mStates; // Danh sách các State đang hoạt động
    std::vector<std::pair<std::unique_ptr<State>, StateID>> mPendingPush; // Danh sách các State đang chờ đẩy vào Stack
    int mPendingPopCount = 0; // Số lượng State đang trong Stack
    bool mClearStates = false; // Cờ báo hiệu cần xóa sạch State đang có

    void applyPendingChanges(); // Hàm cốt lõi: Áp dụng các thay đổi về State một cách an toàn cuối mỗi Frame
public:
    StateManager();
    ~StateManager();

    void pushState(std::unique_ptr<State> state, StateID id); // Yêu cầu thêm State vào Stack
    void popState(); // Yêu cầu xóa State khỏi Stack
    void changeState(std::unique_ptr<State> state, StateID id); // Đăng ký chuyển đổi trạng thái (xóa sạch toàn bộ State cũ và nạp State mới)

    void handleEvent(const sf::Event& event); // Phân phối sự kiện hệ thống xuống State hiện tại
    void update(float dt); // Cập nhật logic toán học
    void draw(sf::RenderWindow& window);
};
