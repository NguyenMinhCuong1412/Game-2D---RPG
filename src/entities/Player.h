#pragma once
#include "entities/Entity.h"
#include "character/CharacterType.h"
#include "entities/Projectile.h"
class Enemy;
class GameData;
#include "entities/AnimatedSprite.h"
#include "common/lib.h"

// Lớp con kế thừa từ Entity
class Player : public Entity {
public:
    struct FrameInfo { // struct phụ trợ - đại diện cho 1 khung hình (Frame) hoạt ảnh
        const sf::Texture* texture = nullptr; // Con trỏ trỏ đến ảnh chứa Frame
        sf::IntRect rect; // Khung chữ nhật xác định vị trí và kích thước của Frame đó trên bức ảnh
    };
private:
    const GameData& mGameData; // Tham chiếu đến dữ liệu gốc của game
    CharacterType mType;

    // Chỉ số RPG
    int mHP = 100;              // Máu hiện tại
    int mMaxHP = 100;           // Máu tối đa
    int mLevel = 1;             // Cấp độ
    int mEXP = 0;               // Điểm kinh nghiệm hiện tại
    int mGold = 0;              // Số vàng sở hữu
    int mPotions = 3;           // Số lượng bình máu đang có
    int mHPScrollCount = 0;       // Số lượng cuộn HP
    int mDamageScrollCount = 0;   // Số lượng cuộn Damage
    int mDefenseScrollCount = 0;  // Số lượng cuộn Defense
    float mHitFlashTimer = 0.f;   // Đếm ngược thời gian nhấp nháy đỏ khi bị trúng đòn
    float mInvulnerableTimer = 0.f; // Thời gian bất tử tạm thời (sau khi trúng đòn/lướt)

    // Di chuyển, hướng quay mặt
    float mSpeed = 200.f;                    // Tốc độ di chuyển cơ bản (pixel/giây)
    sf::Vector2f mVelocity{ 0.f, 0.f };      // Vận tốc di chuyển 2D (vx, vy)
    sf::Vector2f mFacingDirection{ 1.f, 0.f }; // Hướng nhân vật đang nhìn (Mặc định: Phải)

    // Kỹ năng lướt (Dash)
    float mDashTimer = 0.f;                  // Đếm ngược thời gian đang trong trạng thái lướt
    float mDashCooldown = 0.f;               // Thời gian hồi chiêu Lướt còn lại
    sf::Vector2f mDashDirection{ 0.f, 0.f };  // Hướng lướt

    // Hồi chiêu Đánh thường và Bộ Kỹ năng (Q, E, R)
    float mAttackTimer = 0.f;       // Thời gian đang vung đòn đánh thường
    float mAttackCooldown = 0.f;    // Thời gian hồi đòn đánh thường
    float mQCooldown = 0.f;         // Hồi chiêu Q còn lại
    float mECooldown = 0.f;         // Hồi chiêu E còn lại
    float mRCooldown = 0.f;         // Hồi chiêu R còn lại
    float mQCooldownMax = 8.f;      // Thời gian hồi chiêu Q tối đa (8s)
    float mECooldownMax = 12.f;      // Thời gian hồi chiêu E tối đa (12s)
    float mRCooldownMax = 20.f;     // Thời gian hồi chiêu R tối đa (20s)

    // Trạng thái và Buff riêng của từng Lớp nhân vật
    float mShieldActiveTimer = 0.f;          // Thời gian bật Khiên (Knight)
    float mWhirlwindTimer = 0.f;             // Thời gian múa kiếm xoay tròn (Swordsman/Knight)
    float mArcherAtkSpeedBuffTimer = 0.f;    // Tăng tốc đánh cho Cung thủ

    // Shape đồ họa phụ trợ (SFML Visual / Debug Shapes)
    sf::CircleShape mShape;              // Hitbox hình tròn của Player
    sf::RectangleShape mMeleeSlashShape; // Hình chữ nhật mô phỏng vùng chém cận chiến
    sf::CircleShape mShieldShape;        // Vòng tròn hiệu ứng Khiên
    sf::CircleShape mWhirlwindShape;     // Vòng tròn hiệu ứng Xoay kiếm

    std::unique_ptr<AnimatedSprite> mAnimSprite; // Con trỏ quản lý Sprite hoạt ảnh
    bool mHasSprite = false;                     // Kiểm tra đã nạp Sprite hay chưa

    // Mảng chứa các Frame cho từng trạng thái:
    std::vector<FrameInfo> mIdleFrames;   // Đứng yên
    std::vector<FrameInfo> mWalkFrames;   // Di chuyển
    std::vector<FrameInfo> mAttackFrames; // Đánh thường
    std::vector<FrameInfo> mSkillQFrames; // Chiêu Q
    std::vector<FrameInfo> mSkillEFrames; // Chiêu E
    std::vector<FrameInfo> mSkillRFrames; // Chiêu R

    std::optional<sf::Sprite> mRenderSprite; // Sprite thực tế dùng để render lên màn hình
    float mAnimTimer = 0.f;                  // Đếm thời gian để chuyển Frame
    std::size_t mAnimFrameIdx = 0;           // Chỉ số Frame hiện tại đang hiển thị
    float mSkillAnimTimer = 0.f;             // Đếm thời gian chạy animation kỹ năng
    std::string mCurrentSkillAnim = "";      // Tên animation kỹ năng đang phát

    // Logic Đạn và Phân đoạn Sát thương (Damage Phases)
    bool mPendingBasicProjectile = false; // Đánh thường có tạo đạn không (chờ đúng frame mới bắn)
    bool mPendingSkillEProjectile = false;// Chiêu E có chờ bắn đạn không
    bool mPendingSkillRProjectile = false;// Chiêu R có chờ bắn đạn không
    float mLancerSpeedBuffTimer = 0.f;        // Buff tốc chạy Thương thủ
    float mSwordsmanAtkSpeedBuffTimer = 0.f;  // Buff tốc đánh Kiếm sĩ
    float mAttackRangePreviewTimer = 0.f;     // Thời gian hiển thị vòng tròn tầm đánh

    // Quản lý nhịp gây sát thương (tránh 1 skill gây sát thương 60 lần/giây):
    int mWhirlwindDamagePhase = 0;
    int mKnightRDamagePhase = 0;
    int mLancerEDamagePhase = 0;
    int mLancerRDamagePhase = 0;
    int mSwordsmanEDamagePhase = 0;
    std::size_t mPrevAnimFrameIdx = 0;   // Frame trước đó (dùng để kiểm tra khi nào vừa đổi sang frame mới)

public:
    Player(CharacterType type, const GameData& gameData);

    void update(float dt) override; // Cập nhật logic cơ bản (Ghi đè từ Entity)
    // Cập nhật logic nâng cao (có tương tác với Đạn, Cửa sổ game, Kẻ địch):
    void update(float dt, std::vector<Projectile>& projectiles, sf::RenderWindow& window, const std::vector<std::unique_ptr<Enemy>>& enemies);
    void draw(sf::RenderWindow& window) override; // Vẽ Player lên cửa sổ
    // Xử lý phím bấm & chuột từ người dùng:
    void handleInput(sf::RenderWindow& window, std::vector<Projectile>& projectiles, const std::vector<std::unique_ptr<Enemy>>& enemies);

    // Getter chỉ số cơ bản
    CharacterType getType() const { return mType; }
    float getRadius() const { return 20.f; }
    float getAttackRange() const;
    std::size_t getAnimFrameIndex() const { return mAnimFrameIdx; }
    int getHP() const { return mHP; }
    int getMaxHP() const { return mMaxHP; }
    float getDefense() const;
    int getLevel() const { return mLevel; }
    int getEXP() const { return mEXP; }
    int getGold() const { return mGold; }
    int getPotions() const { return mPotions; }
    float getDamage() const;
    bool isDead() const { return mHP <= 0; }

    // Tương tác Chỉ số và Vật phẩm
    void takeDamage(int damage);           // Bị trúng đòn (trừ HP, tính giáp, bật bất tử tạm thời)
    void gainEXP(int amount);              // Nhận điểm kinh nghiệm (tự lên cấp nếu đủ EXP)
    void gainGold(int amount);             // Nhận vàng
    void setInvulnerable(float duration) { mInvulnerableTimer = duration; } // Đặt thời gian bất tử
    void addPotion() { mPotions++; }       // Thêm 1 bình máu
    void buyPotion();                      // Mua bình máu (trừ vàng)
    void usePotion();                      // Dùng bình máu (hồi HP)

    // Kiểm tra Trạng thái (State Queries)
    bool isDashing() const { return mDashTimer > 0.f; }        // Có đang lướt không?
    bool isAttacking() const { return mAttackTimer > 0.f; }    // Có đang đánh thường không?
    bool isShieldActive() const { return mShieldActiveTimer > 0.f; } // Có đang bật khiên không?
    bool isWhirlwindActive() const { return mWhirlwindTimer > 0.f; } // Có đang múa kiếm không?
    bool isMeleeClass() const { return mType == CharacterType::Knight || mType == CharacterType::Lancer || mType == CharacterType::Swordsman; } // Có phải tướng cận chiến không?

    // Quản lý Hồi chiêu (Cooldown Getters & Setters)
    float getDashCooldown() const { return mDashCooldown; }
    float getDashCooldownMax() const { return 0.8f; }
    float getQCooldown() const { return mQCooldown; }
    float getECooldown() const { return mECooldown; }
    float getRCooldown() const { return mRCooldown; }
    float getQCooldownMax() const { return mQCooldownMax; }
    float getECooldownMax() const { return mECooldownMax; }
    float getRCooldownMax() const { return mRCooldownMax; }
    void setQCooldown(float val) { mQCooldown = val; }
    void setECooldown(float val) { mECooldown = val; }
    void setRCooldown(float val) { mRCooldown = val; }

    // Kiểm tra Phạm vi và Quản lý Phân đoạn Sát thương
    
    // Kiểm tra kẻ địch có nằm trong tầm chém cận chiến không
    bool isEnemyInMeleeRange(const Enemy& enemy, float reach = 65.f, float width = 60.f) const; 

    // Getters/Setters cho nhịp gây sát thương của từng chiêu thức:
    int getWhirlwindDamagePhase() const { return mWhirlwindDamagePhase; }
    void setWhirlwindDamagePhase(int phase) { mWhirlwindDamagePhase = phase; }
    int getKnightRDamagePhase() const { return mKnightRDamagePhase; }
    void setKnightRDamagePhase(int phase) { mKnightRDamagePhase = phase; }
    int getLancerEDamagePhase() const { return mLancerEDamagePhase; }
    void setLancerEDamagePhase(int phase) { mLancerEDamagePhase = phase; }
    int getLancerRDamagePhase() const { return mLancerRDamagePhase; }
    void setLancerRDamagePhase(int phase) { mLancerRDamagePhase = phase; }
    int getSwordsmanEDamagePhase() const { return mSwordsmanEDamagePhase; }
    void setSwordsmanEDamagePhase(int phase) { mSwordsmanEDamagePhase = phase; }

    // Setters phục vụ Save/Load Game và Buffs
    void addHPScrollCount(int val) { mHPScrollCount += val; recalculateStats(); }
    void addDamageScrollCount(int val) { mDamageScrollCount += val; recalculateStats(); }
    void addDefenseScrollCount(int val) { mDefenseScrollCount += val; recalculateStats(); }
    int getHPScrollCount() const { return mHPScrollCount; }
    int getDamageScrollCount() const { return mDamageScrollCount; }
    int getDefenseScrollCount() const { return mDefenseScrollCount; }

    void setHP(int hp) { mHP = hp; }
    void setMaxHP(int maxHP) { mMaxHP = maxHP; }
    void setLevel(int level) { mLevel = level; }
    void setEXP(int exp) { mEXP = exp; }
    void setGold(int gold) { mGold = gold; }
    void setPotions(int potions) { mPotions = potions; }
    void setHPScrollCount(int sc) { mHPScrollCount = sc; }
    void setDamageScrollCount(int sc) { mDamageScrollCount = sc; }
    void setDefenseScrollCount(int sc) { mDefenseScrollCount = sc; }
    void recalculateStats();

    void setShieldActiveTimer(float val) { mShieldActiveTimer = val; }
    void setWhirlwindTimer(float val) { mWhirlwindTimer = val; }
    void setArcherAtkSpeedBuffTimer(float val) { mArcherAtkSpeedBuffTimer = val; }
    void setLancerSpeedBuffTimer(float val) { mLancerSpeedBuffTimer = val; }
    void setSwordsmanAtkSpeedBuffTimer(float timer) { mSwordsmanAtkSpeedBuffTimer = timer; }

    // Kích hoạt lướt (Dash):
    void startDash(float timerVal, float cooldownVal, sf::Vector2f dir) {
        mDashTimer = timerVal;
        mDashCooldown = cooldownVal;
        mDashDirection = dir;
    }

    void setAttackTimer(float val) { mAttackTimer = val; }
    sf::Vector2f getFacingDirection() const { return mFacingDirection; }
    void setFacingDirection(sf::Vector2f dir) { mFacingDirection = dir; }
    void setPendingSkillEProjectile(bool val) { mPendingSkillEProjectile = val; }
    void setPendingSkillRProjectile(bool val) { mPendingSkillRProjectile = val; }

    void playSkillAnimation(const std::string& animName, float duration) {
        mSkillAnimTimer = duration;
        mCurrentSkillAnim = animName;
    }
    bool isPlayingSkillAnimation() const { return mSkillAnimTimer > 0.f; }
    std::string getCurrentSkillAnim() const { return mCurrentSkillAnim; }
    float getAttackRangePreviewTimer() const { return mAttackRangePreviewTimer; }

    // Hệ thống Cắt và Nạp Sprite Sheet (Animation Loader Utilities)

    // Thêm 1 Frame vào danh sách tương ứng (Idle, Walk, Attack, Q, E, R)
    void addIdleFrame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) 
            rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mIdleFrames.push_back({tex, rect});
    }

    void addWalkFrame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) 
            rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mWalkFrames.push_back({tex, rect});
    }

    void addSkillQFrame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) 
            rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mSkillQFrames.push_back({tex, rect});
    }

    void addSkillEFrame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) 
            rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mSkillEFrames.push_back({tex, rect});
    }

    void addSkillRFrame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) 
            rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mSkillRFrames.push_back({tex, rect});
    }

    void addAttackFrame(const sf::Texture* tex, sf::IntRect rect = sf::IntRect()) {
        if (!tex) return;
        if (rect.size.x == 0 || rect.size.y == 0) 
            rect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(static_cast<int>(tex->getSize().x), static_cast<int>(tex->getSize().y)));
        mAttackFrames.push_back({tex, rect});
    }

    // Tự động cắt một dải ảnh Sprite Sheet thành các Frame nhỏ dựa trên tỉ lệ chiều rộng/chiều cao
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
                else if (animType == "skill_q") addSkillQFrame(tex, rect);
                else if (animType == "skill_e") addSkillEFrame(tex, rect);
                else if (animType == "skill_r") addSkillRFrame(tex, rect);
            }
        }
    }

    // Tải toàn bộ danh sách Texture hoạt ảnh cho Player
    void loadAnimations(const sf::Texture* idleTex, const sf::Texture* walkTex, const sf::Texture* attackTex, const sf::Texture* qTex = nullptr, const sf::Texture* eTex = nullptr, const sf::Texture* rTex = nullptr);
};
