#include <SDL.h>
#include <SDL_ttf.h> 
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <map>
#include <algorithm>
#include <chrono>
#include <random>
#include "json/json.hpp"
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <memory>
#include <SDL_image.h> // 添加缺少的頭文件

// Add new game states
enum GameState {
    MENU,
    PLAYING,
    GAME_OVER,
    TUTORIAL,
    PAUSED,
    LEADERBOARD,
    SETTINGS
};

// Add difficulty levels
enum class Difficulty {
    EASY,
    NORMAL,
    HARD,
    EXPERT
};

// Add new Menu state options
enum class MenuState {
    MAIN,
    MODE_SELECT,
    DIFFICULTY_SELECT,
    LEADERBOARD,
    TUTORIAL,
    SETTINGS,
    ACHIEVEMENTS
};

// Add new game modes
enum class GameMode {
    CLASSIC,
    TIME_ATTACK,
    ZEN_MODE,
    CHALLENGE
};

// Add game progression system
struct LevelData {
    int required_score;
    float speed_multiplier;
    int fruit_count;
    float special_fruit_chance;
    std::vector<std::string> available_powerups;
};

// Add player profile
struct PlayerProfile {
    std::string name;
    int total_score;
    std::map<std::string, int> achievements;
    std::map<GameMode, int> high_scores; // 確保 GameMode 已定義
    int games_played;
    float average_score;
    
    void serialize(nlohmann::json& j) {
        j = {
            {"name", name},
            {"total_score", total_score},
            {"achievements", achievements},
            {"high_scores", high_scores},
            {"games_played", games_played},
            {"average_score", average_score}
        };
    }

    void deserialize(const nlohmann::json& j) {
        name = j.at("name").get<std::string>();
        total_score = j.at("total_score").get<int>();
        achievements = j.at("achievements").get<std::map<std::string, int>>();
        high_scores = j.at("high_scores").get<std::map<GameMode, int>>();
        games_played = j.at("games_played").get<int>();
        average_score = j.at("average_score").get<float>();
    }
};

const int WIDTH = 800;
const int HEIGHT = 600;

// 調整 Color 結構體以包含 alpha
struct Color {
    int r, g, b, a; // 新增 alpha
};

// 更新所有 Color 初始化，包含 alpha 值
const Color WHITE = {255, 255, 255, 255};
const Color BLACK = {0, 0, 0, 255};
const Color RED = {255, 0, 0, 255};
const Color BLUE = {0, 0, 255, 255};
const Color GREEN = {0, 255, 0, 255};
const Color GRAY = {128, 128, 128, 255};
const Color YELLOW = {255, 255, 0, 255};

// Correct initialization of MODERN_COLORS
const Color DARK_PRIMARY = {18, 18, 18, 255};      // Dark background
const Color DARK_SECONDARY = {30, 30, 30, 255};    // Secondary dark
const Color ACCENT_BLUE = {66, 133, 244, 255};     // Google Blue
const Color ACCENT_GREEN = {52, 168, 83, 255};     // Success Green
const Color ACCENT_RED = {234, 67, 53, 255};       // Error Red
const Color TEXT_PRIMARY = {255, 255, 255, 255};   // White text
const Color TEXT_SECONDARY = {170, 170, 170, 255}; // Gray text
const Color BUTTON_HOVER = {45, 45, 45, 255};      // Button hover state
const Color BUTTON_ACTIVE = {60, 60, 60, 255};     // Button active state

const Color LIGHT_PRIMARY = {245, 245, 245, 255};
const Color LIGHT_SECONDARY = {230, 230, 230, 255};

struct Theme {
    Color background;
    Color primary;
    Color secondary;
    Color accent;
    Color error;
    Color text;
    Color shadow;
    Color button;
    Color track;
};

const Theme LIGHT_THEME = {
    {245, 245, 245, 255}, {66, 133, 244, 255}, {52, 168, 83, 255}, {251, 188, 4, 255}, {234, 67, 53, 255}, {32, 33, 36, 255}, {0, 0, 0, 50}, {255, 255, 255, 255}, {200, 200, 200, 255}
};

const Theme DARK_THEME = {
    {30, 30, 30, 255}, {138, 180, 248, 255}, {129, 201, 149, 255}, {253, 214, 99, 255}, {242, 139, 130, 255}, {232, 234, 237, 255}, {0, 0, 0, 80}, {70, 70, 70, 255}, {70, 70, 70, 255}
};

const Theme FRUIT_THEME = {
    {230, 255, 230, 255}, // light green background
    {255, 102, 102, 255}, // soft red
    {255, 178, 102, 255}, // soft orange
    {178, 255, 102, 255}, // lime green
    {255, 51, 51, 255},   // bright red
    {51, 51, 51, 255},    // dark text
    {0, 0, 0, 50},        // shadow
    {255, 255, 255, 255}, // white
    {200, 200, 200, 255}  // light gray
};

// Correct initialization of MODERN_DARK_THEME
const Theme MODERN_DARK_THEME = {
    DARK_PRIMARY,    // background
    ACCENT_BLUE,     // primary
    DARK_SECONDARY,  // secondary
    ACCENT_GREEN,    // accent
    ACCENT_RED,      // error
    TEXT_PRIMARY,    // text
    {0, 0, 0, 40},   // shadow
    DARK_SECONDARY,  // button
    {40, 40, 40, 255}     // track
};

struct Fruit {
    std::string type;
    Color color;
    std::string sprite_path;
};

const std::vector<Fruit> FRUITS = {
    {"apple", {255, 0, 0, 255}, "sprites/apple.png"},
    {"banana", {255, 255, 0, 255}, "sprites/banana.png"},
    {"orange", {255, 165, 0, 255}, "sprites/orange.png"},
    {"grape", {128, 0, 128, 255}, "sprites/grape.png"}
};

// Add achievement system
struct Achievement {
    std::string name;
    std::string description;
    bool unlocked;
    int progress;
    int target;
};

// Add combo system
class ComboSystem {
public:
    ComboSystem() : current_combo(0), max_combo(0), combo_timer(0) {}
    
    void addCombo() {
        current_combo++;
        combo_timer = 2.0f; // 2 seconds to maintain combo
        max_combo = std::max(max_combo, current_combo);
    }
    
    void update(float dt) {
        if (combo_timer > 0) {
            combo_timer -= dt;
            if (combo_timer <= 0) {
                current_combo = 0;
            }
        }
    }
    
    int getComboMultiplier() const {
        return std::min(1 + current_combo / 3, 5); // Max 5x multiplier
    }

    void addBonus(int bonus) {
        // 實作 addBonus
    }

private:
    int current_combo;
    int max_combo;
    float combo_timer;
};

class Particle {
public:
    Particle(int x, int y, Color color) : x(x), y(y), color(color) {
        size = rand() % 5 + 2;
        lifetime = 1.0;
        velocity[0] = (rand() % 400 - 200) / 100.0;
        velocity[1] = (rand() % 400 - 200) / 100.0;
    }

    void update(float dt) {
        x += velocity[0];
        y += velocity[1];
        lifetime -= dt * 2;
        size = std::max(0.0f, size - dt * 2);
    }

    void draw(SDL_Renderer* renderer) {
        int alpha = static_cast<int>(255 * lifetime);
        if (alpha > 0) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // 設定混合模式
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha); // 加入 alpha
            SDL_Rect rect = {static_cast<int>(x - size), static_cast<int>(y - size), static_cast<int>(size * 2), static_cast<int>(size * 2)};
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    bool isDead() const { return lifetime <= 0; } // 加入 isDead 成員函數

private:
    float x, y;
    Color color;
    float size;
    float lifetime;
    float velocity[2];
};

// Add special effects manager
class EffectManager {
public:
    ~EffectManager() {
        for (auto particle : particles) {
            delete particle;
        }
        particles.clear();
    }

    void addEffect(const std::string& type, float x, float y) {
        if (type == "explosion") {
            for (int i = 0; i < 20; ++i) {
                particles.push_back(new Particle(x, y, randomColor()));
            }
        }
        // 添加更多效果類型
    }

    void update(float dt) {
        auto it = particles.begin();
        while (it != particles.end()) {
            (*it)->update(dt);
            if ((*it)->isDead()) {
                delete *it;
                it = particles.erase(it);
            } else {
                ++it;
            }
        }
    }

    void draw(SDL_Renderer* renderer) {
        for (auto& particle : particles) {
            particle->draw(renderer);
        }
    }

private:
    std::vector<Particle*> particles;

    Color randomColor() {
        return {
            rand() % 256,
            rand() % 256,
            rand() % 256,
            255
        };
    }
};

// Add high score system
class HighScoreManager {
public:
    HighScoreManager() {
        loadHighScores();
    }
    
    void addScore(int score) {
        high_scores.push_back(score);
        std::sort(high_scores.begin(), high_scores.end(), std::greater<int>());
        if (high_scores.size() > 10) {
            high_scores.resize(10);
        }
        saveHighScores();
    }
    
    const std::vector<int>& getHighScores() const {
        return high_scores;
    }

    void saveHighScores() {
        // 實作 saveHighScores
        std::ofstream file("highscores.dat");
        for (const auto& score : high_scores) {
            file << score << std::endl;
        }
    }

private:
    std::vector<int> high_scores;
    
    void loadHighScores() {
        std::ifstream file("highscores.dat");
        int score;
        while (file >> score) {
            high_scores.push_back(score);
        }
    }
};

// Add power-up system
class PowerUpManager {
public:
    void addPowerUp(const std::string& type) {
        active_powerups[type] = 10.0f; // 10 seconds duration
    }
    
    void update(float dt, Game& game) {
        for (auto it = active_powerups.begin(); it != active_powerups.end();) {
            it->second -= dt;
            if (it->second <= 0) {
                deactivatePowerUp(it->first, game);
                it = active_powerups.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    bool isPowerUpActive(const std::string& type) const {
        return active_powerups.find(type) != active_powerups.end();
    }

    void activatePowerUp(const std::string& type, Game& game) {
        active_powerups[type] = 10.0f; // 有效時間 10 秒
        if (type == "double_points") {
            game.score_multiplier *= 2;
        }
        // 實作其他 power-up 效果
    }

    void deactivatePowerUp(const std::string& type, Game& game) {
        if (type == "double_points") {
            game.score_multiplier /= 2;
        }
        // 撤銷其他 power-up 效果
    }

private:
    std::map<std::string, float> active_powerups;
};

class Button {
protected:
    SDL_Rect rect;
    std::string text;
    Color color;

public:
    Button(int x, int y, int width, int height, const std::string& text, Color color)
        : rect{x, y, width, height}, text(text), color(color) {}

    virtual void draw(SDL_Renderer* renderer, TTF_Font* font) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, 255);
        SDL_RenderDrawRect(renderer, &rect);

        SDL_Surface* text_surface = TTF_RenderText_Solid(font, text.c_str(), {BLACK.r, BLACK.g, BLACK.b, 255}); // 加入 alpha
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        int text_width, text_height;
        SDL_QueryTexture(text_texture, NULL, NULL, &text_width, &text_height);
        SDL_Rect text_rect = {rect.x + (rect.w - text_width) / 2, rect.y + (rect.h - text_height) / 2, text_width, text_height};
        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
        SDL_FreeSurface(text_surface);
        SDL_DestroyTexture(text_texture);
    }

    bool isClicked(int x, int y) {
        return x > rect.x && x < rect.x + rect.w && y > rect.y && y < rect.y + rect.h;
    }
};

// Add modern UI components
class ModernButton : public Button {
public:
    ModernButton(int x, int y, int width, int height, const std::string& text, Color color)
        : Button(x, y, width, height, text, color) {
        corner_radius = 8;  // Rounded corners
        has_shadow = true;
    }

    void draw(SDL_Renderer* renderer, TTF_Font* font) override { // 確認覆寫正確
        // Draw shadow if enabled
        if (has_shadow) {
            drawShadow(renderer);
        }

        // Draw rounded rectangle background
        drawRoundedRect(renderer, rect, corner_radius, color);

        // Draw hover effect
        if (is_hovered) {
            drawRoundedRect(renderer, rect, corner_radius, BUTTON_HOVER);
        }

        // Draw text with modern font
        SDL_Color text_color = toSDLColor(is_hovered ? TEXT_PRIMARY : TEXT_SECONDARY);
        drawText(renderer, font, text, text_color);
    }

private:
    int corner_radius;
    bool has_shadow;
    bool is_hovered = false;

    void drawShadow(SDL_Renderer* renderer) {
        // 實作 drawShadow
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 50);
        SDL_Rect shadow_rect = {rect.x + 5, rect.y + 5, rect.w, rect.h};
        drawRoundedRect(renderer, shadow_rect, corner_radius, {0, 0, 0, 50});
    }

    void drawRoundedRect(SDL_Renderer* renderer, SDL_Rect rect, int radius, Color color) {
        // 實作 drawRoundedRect
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        // Draw rounded rectangle (simplified)
        SDL_RenderFillRect(renderer, &rect);
    }

    void drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color) {
        // 實作 drawText
        SDL_Surface* text_surface = TTF_RenderText_Solid(font, text.c_str(), color);
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        int text_width, text_height;
        SDL_QueryTexture(text_texture, NULL, NULL, &text_width, &text_height);
        SDL_Rect text_rect = {rect.x + (rect.w - text_width) / 2, rect.y + (rect.h - text_height) / 2, text_width, text_height};
        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
        SDL_FreeSurface(text_surface);
        SDL_DestroyTexture(text_texture);
    }
};

// Add power-up system
class PowerUp {
public:
    PowerUp(const std::string& type) : type(type), active(false), duration(5.0f) {}
    
    void activate() {
        active = true;
        timer = duration;
    }
    
    void update(float dt) {
        if (active) {
            timer -= dt;
            if (timer <= 0) {
                active = false;
            }
        }
    }
    
    bool isActive() const { return active; }
    std::string getType() const { return type; }

private:
    std::string type;
    bool active;
    float duration;
    float timer;
};

// Add animation system
class Animation {
public:
    Animation(const std::string& name, int frames, float duration)
        : name(name), current_frame(0), total_frames(frames), 
          frame_time(duration / frames), timer(0) {}
    
    void update(float dt) {
        timer += dt;
        if (timer >= frame_time) {
            current_frame = (current_frame + 1) % total_frames;
            timer = 0;
        }
    }
    
    int getCurrentFrame() const { return current_frame; }

private:
    std::string name;
    int current_frame;
    int total_frames;
    float frame_time;
    float timer;
};

// Add statistics tracking
struct PlayerStats {
    int total_games;
    int fruits_matched;
    int highest_combo;
    int total_score;
    std::map<std::string, int> fruit_type_matches;
    
    void serialize(nlohmann::json& j) {
        j = nlohmann::json{
            {"total_games", total_games},
            {"fruits_matched", fruits_matched},
            {"highest_combo", highest_combo},
            {"total_score", total_score},
            {"fruit_type_matches", fruit_type_matches}
        };
    }
    
    void deserialize(const nlohmann::json& j) {
        total_games = j["total_games"];
        fruits_matched = j["fruits_matched"];
        highest_combo = j["highest_combo"];
        total_score = j["total_score"];
        fruit_type_matches = j["fruit_type_matches"].get<std::map<std::string, int>>();
    }
};

// Add daily challenge system
class DailyChallenge {
public:
    DailyChallenge() : completed(false), current_challenge(""), last_update("") {
        loadChallenges();
        checkAndUpdateDaily();
    }
    
    bool isCompleted() const { return completed; }
    const std::string& getDescription() const { return current_challenge; }
    bool isActive() const { return !completed; } // 加入 isActive 成員函數
    
private:
    void loadChallenges() {
        challenges = {
            "Match 50 fruits without mistakes",
            "Reach 1000 points in Time Attack mode",
            "Get a 20x combo",
            "Complete level with only special fruits"
        };
    }
    
    void checkAndUpdateDaily() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d");
        std::string today = ss.str();
        
        if (today != last_update) {
            current_challenge = challenges[rand() % challenges.size()];
            completed = false;
            last_update = today;
        }
    }
    
    std::vector<std::string> challenges;
    std::string current_challenge;
    std::string last_update;
    bool completed;
};

// Enhance power-up system
class EnhancedPowerUp : public PowerUp {
public:
    EnhancedPowerUp(const std::string& type, float duration, float strength)
        : PowerUp(type), strength(strength) {
        effects["double_points"] = [](Game& game) { game.multiplyScore(2.0f); };
        effects["slow_motion"] = [](Game& game) { game.setTimeScale(0.5f); };
        effects["fruit_rain"] = [](Game& game) { game.spawnBonusFruits(); };
        effects["combo_booster"] = [](Game& game) { game.boostCombo(); };
    }
    
    void applyEffect(Game& game) {
        if (effects.find(getType()) != effects.end()) {
            effects[getType()](game);
        }
    }

private:
    float strength;
    std::map<std::string, std::function<void(Game&)>> effects;
};

class FallingFruit {
public:
    FallingFruit(float x, float y, const Fruit& fruit_type) 
        : x(x), y(y), fruit(fruit_type), falling(false), 
          rotation(0), fall_speed(200), special(rand() % 10 == 0) {
        horizontal_speed = (rand() % 200 - 100) / 100.0f; // -1.0 到 1.0 之間的速度
        rotation_speed = (rand() % 100 - 50); // 隨機旋轉速度
    }

    void draw(SDL_Renderer* renderer) {
        if (!texture) {
            texture = loadTexture(renderer, fruit.sprite_path);
            if (!texture) return;
        }
        SDL_Rect dest_rect = { static_cast<int>(x), static_cast<int>(y), width, height };
        SDL_RenderCopyEx(renderer, texture, nullptr, &dest_rect, rotation, nullptr, SDL_FLIP_NONE);
    }

    void update(float dt) {
        if (falling) {
            x += horizontal_speed * dt;
            y += fall_speed * dt;
            rotation += rotation_speed * dt;
        }
    }

    float getX() const { return x; } // 加入 getX 成員函數
    float getY() const { return y; } // 加入 getY 成員函數
    const Fruit& getFruit() const { return fruit; }
    bool isFalling() const { return falling; }      // 加入 isFalling 函式
    void setFalling(bool value) { falling = value; } // 加入 setFalling 函式

    // ... keep other necessary methods ...

    ~FallingFruit() {
        if (texture) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
    }

private:
    float x, y;
    Fruit fruit;
    bool falling = false; // 定義 falling 變數
    float rotation;
    float fall_speed;
    bool special;
    SDL_Texture* texture = nullptr; // Add texture member
    int width = 40; // Add width member
    int height = 40; // Add height member
    float horizontal_speed; // 定義 horizontal_speed
    float rotation_speed; // 定義 rotation_speed
};

class Basket {
public:
    Basket(int x, int y, const Fruit& fruit_type) 
        : x(x), y(y), fruit(fruit_type), catch_count(0) {}

    void draw(SDL_Renderer* renderer) {
        // Draw basket
        SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown color
        SDL_Rect base = {x, y, 80, 40};
        SDL_RenderFillRect(renderer, &base);
        
        // Draw basket label
        SDL_SetRenderDrawColor(renderer, fruit.color.r, fruit.color.g, fruit.color.b, 255);
        SDL_Rect label = {x + 30, y - 10, 20, 20};
        SDL_RenderFillRect(renderer, &label);
    }

    bool catchFruit(FallingFruit* fruit) {
        // Check if fruit is in basket range
        return (fruit->getX() >= x && fruit->getX() <= x + 80);
    }

    bool isClicked(int x, int y) const {
        return x > this->x && x < this->x + 80 && y > this->y && y < this->y + 40;
    }

    const Fruit& getFruit() const { return fruit; } // 加入 getFruit 成員函數

private:
    int x, y;
    Fruit fruit;
    int catch_count;
};

// Add new Tutorial system
class TutorialManager {
public:
    TutorialManager() : current_step(0) {
        loadSteps();
    }

    void startTutorial() {
        current_step = 0;
        steps = {
            {"Welcome", "Welcome to Fruit Sorter! Let's learn how to play."},
            {"Basic Matching", "Click the basket that matches the fruit color."},
            {"Combos", "Match fruits quickly to build combo multipliers!"},
            {"Power-ups", "Collect special fruits for powerful bonuses!"}
        };
    }
    
    bool isComplete() const { return current_step >= steps.size(); }
    void nextStep() { if (!isComplete()) current_step++; }
    
private:
    struct TutorialStep {
        std::string title;
        std::string description;
    };
    std::vector<TutorialStep> steps;
    size_t current_step;

    void loadSteps() {
        steps = {
            {"Welcome", "Welcome to Fruit Sorter! Let's learn how to play."},
            {"Basic Matching", "Click the basket that matches the fruit color."},
            {"Combos", "Match fruits quickly to build combo multipliers!"},
            {"Power-ups", "Collect special fruits for powerful bonuses!"}
        };
    }
};

// Add new UI manager
class UIManager {
public:
    UIManager(SDL_Renderer* renderer) : renderer(renderer) {
        loadFonts();
        createUI();
    }

    void draw() {
        drawBackground();
        drawTopBar();
        drawCurrentView();
    }

    void drawModernMenu() {
        // Draw blurred background
        drawBlurredBackground();
        
        // Draw centered logo
        drawLogo();
        
        // Draw modern buttons
        for (auto& button : menu_buttons) {
            button->draw(renderer, fonts["regular"]);
        }
        
        // Draw bottom bar with version and credits
        drawBottomBar();
    }

    void createUI() {
        // 實作 createUI
        // Initialize UI elements
    }

    void drawBackground() {
        // 實作 drawBackground
        SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
        SDL_RenderClear(renderer);
    }

    void drawTopBar() {
        // 實作 drawTopBar
    }

    void drawCurrentView() {
        // 實作 drawCurrentView
    }

    void drawLogo() {
        // 實作 drawLogo
    }

    void drawBottomBar() {
        // 實作 drawBottomBar
    }

    void drawComboMeter() {
        // 實作 drawComboMeter
    }

    void drawPowerUpIndicators() {
        // 實作 drawPowerUpIndicators
    }

    void drawProgressBar() {
        // 實作 drawProgressBar
    }

    void drawRoundedRect(SDL_Renderer* renderer, SDL_Rect rect, int radius, Color color) {
        // 實作 drawRoundedRect
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        // Draw rounded rectangle (simplified)
        SDL_RenderFillRect(renderer, &rect);
    }

private:
    void loadFonts() {
        fonts["regular"] = TTF_OpenFont("assets/fonts/Roboto-Regular.ttf", 24);
        fonts["bold"] = TTF_OpenFont("assets/fonts/Roboto-Bold.ttf", 24);
        fonts["light"] = TTF_OpenFont("assets/fonts/Roboto-Light.ttf", 24);
        if (!fonts["regular"] || !fonts["bold"] || !fonts["light"]) {
            std::cerr << "Failed to load fonts: " << TTF_GetError() << std::endl;
        }
    }

    void drawBlurredBackground() {
        // Implement gaussian blur effect
    }

    void drawText(TTF_Font* font, const std::string& text, int x, int y, Color color) {
        // Implement text drawing
    }

    SDL_Renderer* renderer;
    std::map<std::string, TTF_Font*> fonts;
    std::vector<ModernButton*> menu_buttons;
};

class MenuManager {
public:
    MenuManager(SDL_Renderer* renderer) : renderer(renderer) {
        // ...初始化菜單...
    }
    void draw() {
        // ...繪製菜單...
    }
    void handleEvent(SDL_Event& event) {
        // ...處理菜單事件...
    }
private:
    SDL_Renderer* renderer;
    
};

class HUDManager {
public:
    HUDManager(SDL_Renderer* renderer) : renderer(renderer) {
        // ...初始化 HUD...
    }
    void draw() {
        // ...繪製 HUD...
    }
private:
    SDL_Renderer* renderer;
    
};

class Game {
public:
    Game() : window(nullptr), renderer(nullptr), font(nullptr), running(true) {
        // 初始化 SDL
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
            std::cerr << "SDL 初始化失敗！錯誤：" << SDL_GetError() << std::endl;
            exit(1);
        }
        // 初始化 TTF
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init: " << TTF_GetError() << std::endl;
            exit(1);
        }
        // 初始化 SDL_image
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            std::cerr << "SDL_image 初始化失敗！錯誤：" << IMG_GetError() << std::endl;
            exit(1);
        }
        // 創建窗口
        window = SDL_CreateWindow("Fruit Basket Sorter Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "窗口創建失敗！錯誤：" << SDL_GetError() << std::endl;
            exit(1);
        }
        // 創建渲染器
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "渲染器創建失敗！錯誤：" << SDL_GetError() << std::endl;
            exit(1);
        }
        // 加載字體
        font = TTF_OpenFont("assets/fonts/Roboto-Regular.ttf", 24);
        if (!font) {
            std::cerr << "字體加載失敗！錯誤：" << TTF_GetError() << std::endl;
            exit(1);
        }
        // 初始化其他成員變量
        current_state = std::make_unique<MenuState>();
        // ...existing code...
    }

    ~Game() {
        // 資源清理
        if (font) {
            TTF_CloseFont(font);
            font = nullptr;
        }
        if (renderer) {
            SDL_DestroyRenderer(renderer);
            renderer = nullptr;
        }
        if (window) {
            SDL_DestroyWindow(window);
            window = nullptr;
        }
        TTF_Quit();
        IMG_Quit();
        SDL_Quit();
    }

    void run() {
        SDL_Event e;
        float dt = 0.0f;
        Uint32 last_time = SDL_GetTicks();
        while (running) {
            Uint32 current_time = SDL_GetTicks();
            dt = (current_time - last_time) / 1000.0f;
            last_time = current_time;

            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    running = false;
                } else {
                    handleEvent(e);
                }
            }
            update(dt);
            draw();
        }
    }

    void handleEvent(SDL_Event& event) {
        if (current_state) {
            current_state->handleEvent(*this, event);
        }
    }

    void update(float dt) {
        if (current_state) {
            current_state->update(*this, dt);
        }
    }

    void draw() {
        if (current_state) {
            current_state->draw(*this);
        }
    }

    void changeState(std::unique_ptr<State> new_state) {
        current_state = std::move(new_state);
    }

    void quit() {
        running = false;
    }

    SDL_Renderer* getRenderer() const { return renderer; }

    // ...existing code...

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    bool running;
    std::unique_ptr<State> current_state;
    // ...existing code...
};

// 添加 Color 到 SDL_Color 的轉換函式
SDL_Color toSDLColor(const Color& color) {
    return SDL_Color{ static_cast<Uint8>(color.r), static_cast<Uint8>(color.g), static_cast<Uint8>(color.b), static_cast<Uint8>(color.a) };
}

class State {
public:
    virtual ~State() = default;
    virtual void handleEvent(Game& game, SDL_Event& event) = 0;
    virtual void update(Game& game, float dt) = 0;
    virtual void draw(Game& game) = 0;
protected:
    State() = default;
};

class MenuState : public State {
public:
    MenuState(Game& game) {
        // 初始化按鈕
        start_button = std::make_unique<ModernButton>(WIDTH / 2 - 100, HEIGHT / 2 - 50, 200, 50, "Start Game", FRUIT_THEME.primary);
        quit_button = std::make_unique<ModernButton>(WIDTH / 2 - 100, HEIGHT / 2 + 50, 200, 50, "Quit", FRUIT_THEME.error);
    }

    void handleEvent(Game& game, SDL_Event& event) override {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            if (start_button->isClicked(x, y)) {
                game.changeState(std::make_unique<PlayingState>(game));
            } else if (quit_button->isClicked(x, y)) {
                game.quit();
            }
        }
        
    }

    void update(Game& game, float dt) override {
        // ...可能的動畫更新...
    }

    void draw(Game& game) override {
        SDL_SetRenderDrawColor(game.getRenderer(), FRUIT_THEME.background.r, FRUIT_THEME.background.g, FRUIT_THEME.background.b, 255);
        SDL_RenderClear(game.getRenderer());

        // 繪製標題
        SDL_Surface* title_surface = TTF_RenderText_Solid(game.font, "Fruit Basket Sorter", {FRUIT_THEME.text.r, FRUIT_THEME.text.g, FRUIT_THEME.text.b});
        SDL_Texture* title_texture = SDL_CreateTextureFromSurface(game.getRenderer(), title_surface);
        int title_width, title_height;
        SDL_QueryTexture(title_texture, NULL, NULL, &title_width, &title_height);
        SDL_Rect title_rect = {WIDTH / 2 - title_width / 2, HEIGHT / 4, title_width, title_height};
        SDL_RenderCopy(game.getRenderer(), title_texture, NULL, &title_rect);
        SDL_FreeSurface(title_surface);
        SDL_DestroyTexture(title_texture);

        // 繪製按鈕
        start_button->draw(game.getRenderer(), game.font);
        quit_button->draw(game.getRenderer(), game.font);
    }

private:
    std::unique_ptr<ModernButton> start_button;
    std::unique_ptr<ModernButton> quit_button;
};

// 定義新的 PlayingState，將遊戲邏輯移入
class PlayingState : public State {
public:
    PlayingState(Game& game) {
        initializeFruits();
        initializeBaskets();
    }

    void handleEvent(Game& game, SDL_Event& event) override {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            handleClick(game, x, y);
        }
        // 處理其他事件
        
    }

    void update(Game& game, float dt) override {
        // 更新遊戲邏輯
        updateFruits(dt);
        updatePowerUps(dt);
        // 更新計時器等
        
    }

    void draw(Game& game) override {
        // 清屏
        SDL_SetRenderDrawColor(game.getRenderer(), FRUIT_THEME.background.r, FRUIT_THEME.background.g, FRUIT_THEME.background.b, 255);
        SDL_RenderClear(game.getRenderer());

        // 繪製遊戲元素
        drawFruits(game.getRenderer());
        drawBaskets(game.getRenderer());
        game.hud_manager->draw();

        
    }

private:
    void handleClick(Game& game, int x, int y) {
        // 處理點擊事件，例如檢查籃子是否被點擊
        for (const auto& basket : baskets) {
            if (basket->isClicked(x, y)) {
                if (current_fruit && basket->getFruit().type == current_fruit->getFruit().type) {
                    // 匹配成功，進行處理
                    // ...existing code...
                }
                break;
            }
        }
    }

    void updateFruits(float dt) {
        // 更新水果的位置和狀態
        for (auto& fruit : falling_fruits) {
            fruit->update(dt);
            if (fruit->getY() > HEIGHT) {
                // 處理水果落地
                // ...existing code...
            }
        }
    }

    void drawFruits(SDL_Renderer* renderer) {
        // 繪製掉落的水果
        for (const auto& fruit : falling_fruits) {
            fruit->draw(renderer);
        }
    }

    void drawBaskets(SDL_Renderer* renderer) {
        // 繪製籃子
        for (const auto& basket : baskets) {
            basket->draw(renderer);
        }
    }

    void initializeFruits() {
        // 初始化水果
        // ...existing code...
    }

    void initializeBaskets() {
        // 初始化籃子
        // ...existing code...
    }

    // 使用智能指標的容器
    std::vector<std::unique_ptr<FallingFruit>> falling_fruits;
    std::vector<std::unique_ptr<Basket>> baskets;
    FallingFruit* current_fruit = nullptr;
    // ...existing code...
};

// 定義 GameOverState
class GameOverState : public State {
public:
    GameOverState(Game& game, int final_score) : score(final_score) {
        // 初始化按鈕
        play_again_button = std::make_unique<ModernButton>(WIDTH / 2 - 100, HEIGHT / 2 + 50, 200, 50, "Play Again", FRUIT_THEME.primary);
    }

    void handleEvent(Game& game, SDL_Event& event) override {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            if (play_again_button->isClicked(x, y)) {
                game.changeState(std::make_unique<PlayingState>(game));
            }
        }
        
    }

    void update(Game& game, float dt) override {
        // ...可能的動畫更新...
    }

    void draw(Game& game) override {
        SDL_SetRenderDrawColor(game.getRenderer(), FRUIT_THEME.background.r, FRUIT_THEME.background.g, FRUIT_THEME.background.b, 255);
        SDL_RenderClear(game.getRenderer());

        // 繪製 "Game Over" 文字
        SDL_Surface* game_over_surface = TTF_RenderText_Solid(game.font, "Game Over!", {FRUIT_THEME.text.r, FRUIT_THEME.text.g, FRUIT_THEME.text.b});
        SDL_Texture* game_over_texture = SDL_CreateTextureFromSurface(game.getRenderer(), game_over_surface);
        int game_over_width, game_over_height;
        SDL_QueryTexture(game_over_texture, NULL, NULL, &game_over_width, &game_over_height);
        SDL_Rect game_over_rect = {WIDTH / 2 - game_over_width / 2, HEIGHT / 4, game_over_width, game_over_height};
        SDL_RenderCopy(game.getRenderer(), game_over_texture, NULL, &game_over_rect);
        SDL_FreeSurface(game_over_surface);
        SDL_DestroyTexture(game_over_texture);

        // 繪製重新開始按鈕
        play_again_button->draw(game.getRenderer(), game.font);
    }

private:
    int score;
    std::unique_ptr<ModernButton> play_again_button;
};

// 添加日誌記錄功能
void logSDLError(const std::string& msg) {
    std::cerr << msg << " 錯誤： " << SDL_GetError() << std::endl;
}

// 在需要的地方添加日誌，例如：
SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        logSDLError("加載圖片失敗：" + path);
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        logSDLError("創建紋理失敗：" + path);
    }
    return texture;
}

// 更新 FallingFruit 的 draw 函數，使用錯誤處理
void FallingFruit::draw(SDL_Renderer* renderer) {
    if (!texture) {
        texture = loadTexture(renderer, fruit.sprite_path);
        if (!texture) return;
    }
    SDL_Rect dest_rect = { static_cast<int>(x), static_cast<int>(y), width, height };
    SDL_RenderCopyEx(renderer, texture, nullptr, &dest_rect, rotation, nullptr, SDL_FLIP_NONE);
}

// 在 Game 構造函數中，初始化初始狀態
Game::Game() : running(true) {
    // ...existing initialization code...

    current_state = std::make_unique<MenuState>();
}

// 實現 Game::changeState() 方法
void Game::changeState(std::unique_ptr<State> new_state) {
    current_state = std::move(new_state);
}

// 修改 main 函數中的遊戲循環
int main(int argc, char* args[]) {
    srand(static_cast<unsigned int>(time(0)));
    Game game;
    game.run();
    return 0;
}
