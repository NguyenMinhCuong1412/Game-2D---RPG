#pragma once
#include "common/lib.h"
#include "states/StateManager.h"
#include "core/ResourceHolder.h"
#include "database/GameData.h"

// Font chữ
namespace Fonts {
    enum ID { Main, Medieval, Pixel };
}

// Đối tượng kiểm soát toàn bộ vòng đời của game
class Game {
private:
    void processEvents();  // Lắng nghe, xử lý các thao tác (sự kiện) của người dùng
    void update(float dt); // Cập nhật trạng thái logic của game dựa trên dt 
    void render();         // Hiển thị tất cả các đối tượng hình ảnh lên màn hình

    sf::RenderWindow mWindow;                   // Cửa sổ dựng hình của SFML, chứa mọi thứ game vẽ lên
    GameData mGameData;                         // Chứa dữ liệu của game
    StateManager mStateManager;                 // Bộ quản lý State dùng điều phối logic chuyển cảnh
    ResourceHolder<sf::Font, Fonts::ID> mFonts; // Bộ lưu trữ font tập trung, nạp sẵn các font chữ để toàn bộ game sử dụng thông qua ID
public:
    Game();     // Khởi tạo cửa sổ game, nạp font chữ và nạp trạng thái menu chính
    void run(); // Vòng lặp game cập nhật và vẽ game liên tục tới khi chương trình dừng
};
