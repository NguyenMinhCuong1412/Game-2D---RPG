#include "core/Game.h"
#include "states/MainMenuState.h"

// Danh sách khởi tạo
// Default: phóng to, thu nhỏ, tắt
Game::Game() : mWindow(sf::VideoMode({800, 600}), "Legend of the Realm", sf::Style::Default) { 
    mWindow.setFramerateLimit(60); // Giới hạn số khung hình tối đa - giúp tránh quá tải GPU/CPU
    try { // Nạp font chữ - file mất, catch bắt ngoại lệ và in ra lỗi tránh crash game
        mFonts.load(Fonts::Main, "assets/fonts/arial.ttf");
        mFonts.load(Fonts::Medieval, "assets/fonts/medieval.ttf");
        mFonts.load(Fonts::Pixel, "assets/fonts/pixel.ttf");
    } catch (const std::exception& e) { std::cerr << "Error loading resources: " << e.what() << std::endl; }
    // Đưa trạng thái Menu chính (MainMenuState) vào danh sách quản lý - Menu xuất hiện dầu tiên
    mStateManager.pushState(std::make_unique<MainMenuState>(mStateManager, mWindow, mFonts.get(Fonts::Pixel), mGameData), StateID::MainMenu);
}

void Game::run() {
    sf::Clock clock; // Tạo đồng hồ bấm giờ của SFML
    while (mWindow.isOpen()) { // Cửa số mở
        processEvents(); 
        float dt = clock.restart().asSeconds(); // dt - Delta Time: thời gian(giây) trôi qua giữa 2 frame liên tiếp -> game chạy đồng tốc trên mọi máy
        update(dt); 
        render();
    }
}

void Game::processEvents() {
    while (const std::optional<sf::Event> event = mWindow.pollEvent()) { // Trả về sự kiện tiếp theo trong hàng đợi sự kiện (event queue)
        if (event->is<sf::Event::Closed>()) mWindow.close(); // Đóng cửa sổ -> dừng vòng lặp .run() -> thoát game
        else if (const auto* resized = event->getIf<sf::Event::Resized>()) { // Thay dổi kích thước cửa sổ 
            (void)resized; // Thủ thuật cố tình bỏ qua biến
            sf::View defaultView({400.f, 300.f}, {800.f, 600.f}); // Đặt lại tâm cửa sổ tránh méo mó
            mWindow.setView(defaultView); // Buộc màn hình game đúng với tâm cửa sổ
        }
        mStateManager.handleEvent(*event); // Chuyển giao sự kiện khác cho State quản lý
    }
}

void Game::update(float dt) { mStateManager.update(dt); } // Chuyển giao toàn bộ việc liên quan đến xử lý dt cho State

void Game::render() { 
    mWindow.clear(); // Xóa sạch màn hình ở frame cũ
    mStateManager.draw(mWindow); // Yêu cầu State hiện tại vẽ tất cả các đối tượng của state đó
    mWindow.display(); // Đưa kết quả lên màn hình
}
