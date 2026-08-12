#pragma once
#include "entities/Entity.h"
#include "world/Map.h"
#include "entities/AnimatedSprite.h"
#include "database/GameData.h"
#include "common/lib.h"
#include "entities/Projectile.h"

// Máy trạng thái trí tuệ nhân tạo
enum class AIState { Idle, Chase, Attack, Skill1, Skill2, Return };

class Enemy : public Entity {
public:
    struct FrameInfo {
        const sf::Texture* texture = nullptr;
        sf::IntRect rect;
    };
private:
    const GameData& mGameData; // Tham chiếu đến cơ sở dữ liệu trò chơi

    std::string mName;        // Tên quái (vd: "Goblin", "Dragon_Boss")
    Region mRegion;           // Khu vực phân bố
    int mLevel;               // Cấp độ của quái
    sf::Vector2f mSpawnPoint; // Tọa độ xuất hiện ban đầu (dùng khi Return)
    bool mIsBoss;             // Có phải là Boss hay không?

    AIState mAIState = AIState::Idle; // Trạng thái AI mặc định

    // Chỉ số sinh tồn và Tương tác
    int mHP;                  // Máu hiện tại
    int mMaxHP;               // Máu tối đa
    float mDamage;            // Sát thương gây ra cho Player
    float mDefense;           // Giáp
    float mSpeed;             // Tốc độ di chuyển
    int mEXPValue;            // Điểm kinh nghiệm thưởng cho Player khi hạ gục
    int mGoldValue;           // Số vàng thưởng cho Player khi hạ gục

    // Phạm vi hoạt động và Bộ đếm thời gian
    float mDetectionRange = 250.f; // Tầm phát hiện Player (Pixel)
    float mAttackRange = 35.f;     // Tầm đánh
    float mLeashRange = 600.f;     // Tầm dây xích (Đuổi quá 600px so với mSpawnPoint sẽ quay về)
    float mAttackCooldown = 0.f;  // Thời gian hồi đòn đánh
    float mHitFlashTimer = 0.f;   // Đếm ngược hiệu ứng nhấp nháy đỏ khi bị trúng đòn
    bool mFlipX = false;          // Lật hình ảnh (true = Quay sang trái)
    bool mIsHovered = false;      // Con trỏ chuột có đang di chuyển qua quái không (để hiện viền/UI)

    // Shape Đồ họa và Thanh Máu (HP Bar)
    float mRadius;                 // Bán kính Hitbox
    sf::CircleShape mShape;        // Hitbox hình tròn mặc định (fallback)
    sf::RectangleShape mHPBarBg;   // Khung nền đen của thanh máu trên đầu
    sf::RectangleShape mHPBarFg;   // Thanh máu màu đỏ/xanh phía trên

    // Quản lý Hoạt ảnh và Đạn/Đòn đánh
    std::unique_ptr<AnimatedSprite> mAnimSprite; // Sprite hoạt ảnh nâng cao
    bool mHasSprite = false;                     // Đã nạp Sprite hay chưa

    // Danh sách lưu trữ các Frame hoạt ảnh theo trạng thái:
    std::vector<FrameInfo> mIdleFrames;
    std::vector<FrameInfo> mWalkFrames;
    std::vector<FrameInfo> mAttackFrames;
    std::vector<FrameInfo> mHurtFrames;
    std::vector<FrameInfo> mDeathFrames;
    std::vector<FrameInfo> mSkill1Frames;
    std::vector<FrameInfo> mSkill2Frames;

    std::optional<sf::Sprite> mRenderSprite; // Sprite thực tế xuất ra màn hình
    float mAnimTimer = 0.f;                  // Đếm thời gian đổi Frame
    std::size_t mAnimFrameIdx = 0;           // Frame hiện tại
    std::size_t mPrevAnimFrameIdx = 0;       // Frame trước đó
    bool mAttackHasHit = false;              // Đánh dấu đòn đánh hiện tại đã gây sát thương chưa (tránh trúng đòn nhiều lần)
    bool mAttackAnimFinished = false;        // Animation đánh đã kết thúc chưa
    int mAttackPhase = 0;                    // Phân đoạn đòn đánh (combo/skill)
public:
    Enemy(std::string name, Region region, int level, sf::Vector2f spawnPoint, bool isBoss, const GameData& gameData);

    void update(float dt) override; // Cập nhật chung (Ghi đè từ Entity)
    // Cập nhật logic AI chính (di chuyển, chuyển trạng thái, đánh Player, bắn Projectile):
    void updateAI(float dt, sf::Vector2f playerPos, int& playerHP, std::vector<Projectile>& projectiles);
    void draw(sf::RenderWindow& window) override; // Vẽ quái và thanh máu lên màn hình

    void takeDamage(float damage);
    bool isDead() const { return mHP <= 0; }

    void setHovered(bool value) { mIsHovered = value; }

    std::string getName() const { return mName; }
    Region getRegion() const { return mRegion; }
    int getHP() const { return mHP; }
    int getMaxHP() const { return mMaxHP; }
    int getEXPValue() const { return mEXPValue; }
    int getGoldValue() const { return mGoldValue; }
    float getDefense() const { return mDefense; }
    int getLevel() const { return mLevel; }
    bool getIsBoss() const { return mIsBoss; }

    float getRadius() const { return mRadius; }
    float getAttackRange() const { return mAttackRange; }

    void resetBossState() {
        mHP = mMaxHP;
        mAIState = AIState::Idle;
        mHitFlashTimer = 0.f;
        mAttackCooldown = 0.f;
        mAttackHasHit = false;
        mPosition = mSpawnPoint;
    }

    void addIdleFrame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mIdleFrames.push_back({tex, rect});
    }

    void addWalkFrame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mWalkFrames.push_back({tex, rect});
    }

    void addAttackFrame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mAttackFrames.push_back({tex, rect});
    }

    void addHurtFrame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mHurtFrames.push_back({tex, rect});
    }

    void addDeathFrame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mDeathFrames.push_back({tex, rect});
    }

    void addSkill1Frame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mSkill1Frames.push_back({tex, rect});
    }

    void addSkill2Frame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mSkill2Frames.push_back({tex, rect});
    }

    void addSpriteSheetFrames(const sf::Texture* tex, const std::string& animType) {
        if (!tex) return;
        auto sz = tex->getSize();
        if (sz.y > 0 && sz.x >= sz.y) {
            int frameW = static_cast<int>(sz.y);
            int frameCount = static_cast<int>(sz.x) / frameW;
            for (int i = 0; i < frameCount; ++i) {
                sf::IntRect rect(sf::Vector2i(i * frameW, 0), sf::Vector2i(frameW, frameW));
                if (animType == "idle") addIdleFrame(tex, rect);
                else if (animType == "walk") addWalkFrame(tex, rect);
                else if (animType == "attack") addAttackFrame(tex, rect);
                else if (animType == "hurt") addHurtFrame(tex, rect);
                else if (animType == "death") addDeathFrame(tex, rect);
                else if (animType == "skill_1") addSkill1Frame(tex, rect);
                else if (animType == "skill_2") addSkill2Frame(tex, rect);
                else if (animType == "skill") addSkill1Frame(tex, rect);
            }
        }
    }

    void loadAnimations(const sf::Texture* idleTex, const sf::Texture* walkTex, const sf::Texture* attackTex);
};
