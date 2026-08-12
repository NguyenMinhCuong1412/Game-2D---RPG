#pragma once
#include "common/lib.h"

// class Template - nạp các tài nguyên nặng và lưu trữ vào RAM để tái sử dụng 
// Hàm khởi tạo 
template <typename Resource, typename Identifier>

class ResourceHolder {
private:
    // Ánh xạ ID sang 1 con trỏ thông minh trỏ tới Resource(tải nguyên)
    // unique_ptr: quản lý bộ nhớ tự động cho tài nguyên trên RAM tránh rò rỉ bộ nhớ
    std::map<Identifier, std::unique_ptr<Resource>> mResourceMap;
public:
    // Nhận 1 ID định danh và đường dẫn file -> tải lên RAM
    void load(Identifier id, const std::string& filename) {
        auto resource = std::make_unique<Resource>(); // Tạo con trỏ thông minh
        bool success = false; // Đánh dấu việc tải tài nguyên
        // File tài nguyên lỗi -> không crash -> tạo tài nguyên tạm thời thay thế (C++ 17)
        if constexpr (std::is_same_v<Resource, sf::Font>) { // Xử lý font
            if (fs::exists(filename)) success = resource->openFromFile(filename); // Mở file nếu tồn tại
            if (!success) { // Thiếu file in cảnh cáo
                std::cout << "[ResourceHolder] Warning: Font file '" << filename << "' missing. Operating with default system font." << std::endl;
                if (fs::exists("assets/fonts/arial.ttf")) success = resource->openFromFile("assets/fonts/arial.ttf");
                else success = true; // Nếu font mặc định không có -> chấp nhận để SFML 3 sử dụng font hệ thống
            }
        } else if constexpr (std::is_same_v<Resource, sf::Texture>) { // Xử lý hình ảnh
            if (fs::exists(filename)) success = resource->loadFromFile(filename); // Mở file nếu tồn tại
            if (!success) { // Thiếu ảnh in cảnh báo -> vẽ ô vuông đại diện cho vật thể thiếu ảnh
                std::cout << "[ResourceHolder] Warning: Texture file '" << filename << "' missing. Creating procedural fallback." << std::endl;
                sf::Image fallbackImg; // Tạo 1 ảnh tạm thời
                fallbackImg.resize({32, 32}, sf::Color(180, 180, 200, 220)); // Tạo ô hình vuông
                for (unsigned int x = 0; x < 32; ++x) {
                    fallbackImg.setPixel({x, 0}, sf::Color::Yellow);
                    fallbackImg.setPixel({x, 31}, sf::Color::Yellow);
                    fallbackImg.setPixel({0, x}, sf::Color::Yellow);
                    fallbackImg.setPixel({31, x}, sf::Color::Yellow);
                } // Vẽ viền màu vàng xung quanh 4 cạnh của hình vuông
                (void)resource->loadFromImage(fallbackImg); // Đưa ô vuông cho tài nguyên sử dụng
                success = true; // Giúp không văng game vì thiếu tài nguyên
            }
        } else if constexpr (std::is_same_v<Resource, sf::SoundBuffer>) { // Xử lý âm thanh
            if (fs::exists(filename)) success = resource->loadFromFile(filename); // Mở file nếu tồn tại
            if (!success) { // Thiếu âm thanh in cảnh báo 
                std::cout << "[ResourceHolder] Warning: SoundBuffer '" << filename << "' missing. Audio playback skipped." << std::endl;
                success = true; // Giúp không văng game, chỉ hiệu ứng âm thanh im lặng
            }
        } else if (fs::exists(filename)) success = resource->loadFromFile(filename); // Xử lý tài nguyên chung

        if (!success) std::cerr << "[ResourceHolder] Error: Critical failure loading " << filename << std::endl;
        mResourceMap[id] = std::move(resource); // Chuyển quyền sở hữu con trỏ tài nguyên vàng bảng ánh xạ theo ID 
    }

    // Lấy tài nguyên tương ứng với ID - phiên bản thay đổi được bảng
    Resource& get(Identifier id) {
        auto found = mResourceMap.find(id); // Tìm tài nguyên theo ID trong bảng
        if (found == mResourceMap.end()) { // Nếu chưa có -> in cảnh báo
            std::cout << "[ResourceHolder] Auto-creating missing resource slot for identifier." << std::endl;
            auto defaultRes = std::make_unique<Resource>(); // Tạo đối tượng rỗng
            if constexpr (std::is_same_v<Resource, sf::Texture>) { // Xử lý riêng cho hình ảnh
                sf::Image fallbackImg; // Tạo bức ảnh tạm
                fallbackImg.resize({32, 32}, sf::Color(100, 100, 100, 180)); // Tạo kích thước, màu
                (void)defaultRes->loadFromImage(fallbackImg); // Tải tạm vào tài nguyên thiếu
            }
            auto inserted = mResourceMap.insert(std::make_pair(id, std::move(defaultRes))); // Chèn đối tượng mặc định vào bảng
            return *inserted.first->second; // Trả về tài nguyên được con trỏ trỏ vào
        }
        return *found->second; // Nếu đã có
    }

    // Hàm kiểm tra sự tồn tại
    bool has(Identifier id) const { return mResourceMap.find(id) != mResourceMap.end(); }

    // Lấy tài nguyên tương ứng với ID - phiên bản chỉ đọc bảng
    const Resource& get(Identifier id) const {
        auto found = mResourceMap.find(id);
        if (found == mResourceMap.end()) return const_cast<ResourceHolder*>(this)->get(id); // Nếu không tìm thấy ép kiểu gỡ bỏ thuộc tính const -> gọi hàm get() ở trên
        return *found->second;
    }
};
