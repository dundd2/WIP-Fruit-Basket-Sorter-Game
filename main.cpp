#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL2_gfxPrimitives.h>
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
    std::map<GameMode, int> high_scores; // Ensure GameMode is defined
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

// Adjust Color struct to include alpha
struct Color {
    int r, g, b, a; // Add alpha
};

// Update all Color initializations to include alpha values
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
        // Implement addBonus
    }

    void resetCombo() {
        current_combo = 0;
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
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Set blend mode
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, alpha); // Add alpha
            SDL_Rect rect = {static_cast<int>(x - size), static_cast<int>(y - size), static_cast<int>(size * 2), static_cast<int>(size * 2)};
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    bool isDead() const { return lifetime <= 0; } // Add isDead member function

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
        // Add more effect types
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
        nlohmann::json j = high_scores;
        std::ofstream file("highscores.json");
        if (!file) {
            std::cerr << "Unable to open file to save high scores" << std::endl;
            return;
        }
        file << j.dump(4);
    }

private:
    std::vector<int> high_scores;
    
    void loadHighScores() {
        std::ifstream file("highscores.json");
        if (!file) {
            std::cerr << "Unable to open file to load high scores" << std::endl;
            return;
        }
        nlohmann::json j;
        file >> j;
        high_scores = j.get<std::vector<int>>();
    }
};

// Add power-up system
class PowerUpManager {
public:
    void addPowerUp(const std::string& type) {
        active_powerups[type] = 10.0f; // 10 seconds duration
    }
    
    void update(float dt, std::function<void(const std::string&)> activateEffect, std::function<void(const std::string&)> deactivateEffect) {
        for (auto it = active_powerups.begin(); it != active_powerups.end();) {
            it->second -= dt;
            if (it->second <= 0) {
                deactivateEffect(it->first);
                it = active_powerups.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    bool isPowerUpActive(const std::string& type) const {
        return active_powerups.find(type) != active_powerups.end();
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

        SDL_Surface* text_surface = TTF_RenderText_Solid(font, text.c_str(), {BLACK.r, BLACK.g, BLACK.b, 255}); // Add alpha
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
        : Button(x, y, width, height, text, color), is_hovered(false) {
        corner_radius = 8;  // Rounded corners
        has_shadow = true;
    }

    void draw(SDL_Renderer* renderer, TTF_Font* font) override { // Ensure correct override
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

    void handleEvent(SDL_Event& event) {
        if (event.type == SDL_MOUSEMOTION) {
            int mx = event.motion.x;
            int my = event.motion.y;
            is_hovered = isClicked(mx, my);
        }
    }

private:
    int corner_radius;
    bool has_shadow;
    bool is_hovered;

    void drawShadow(SDL_Renderer* renderer) {
        // Implement drawShadow
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 50);
        SDL_Rect shadow_rect = {rect.x + 5, rect.y + 5, rect.w, rect.h};
        drawRoundedRect(renderer, shadow_rect, corner_radius, {0, 0, 0, 50});
    }

    void drawRoundedRect(SDL_Renderer* renderer, SDL_Rect rect, int radius, Color color) {
        // Implement drawRoundedRect
        roundedRectangleRGBA(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h,
                             radius, color.r, color.g, color.b, color.a);
    }

    void drawText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color) {
        // Implement drawText
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
    bool isActive() const { return !completed; } // Add isActive member function
    
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
        horizontal_speed = (rand() % 200 - 100) / 100.0f; // Speed between -1.0 and 1.0
        rotation_speed = (rand() % 100 - 50); // Random rotation speed
    }

    void draw(SDL_Renderer* renderer) {
        if (!texture) {
            texture.reset(loadTexture(renderer, fruit.sprite_path));
            if (!texture) return;
        }
        SDL_Rect dest_rect = { static_cast<int>(x), static_cast<int>(y), width, height };
        SDL_RenderCopyEx(renderer, texture.get(), nullptr, &dest_rect, rotation, nullptr, SDL_FLIP_NONE);
    }

    void update(float dt) {
        if (falling) {
            x += horizontal_speed * dt;
            y += fall_speed * dt;
            rotation += rotation_speed * dt;
        }
    }

    float getX() const { return x; } // Add getX member function
    float getY() const { return y; } // Add getY member function
    const Fruit& getFruit() const { return fruit; }
    bool isFalling() const { return falling; }      // Add isFalling function
    void setFalling(bool value) { falling = value; } // Add setFalling function

    // ... keep other necessary methods ...

    ~FallingFruit() {
        if (texture) {
            SDL_DestroyTexture(texture.get());
            texture = nullptr;
        }
    }

private:
    float x, y;
    Fruit fruit;
    bool falling = false; // Define falling variable
    float rotation;
    float fall_speed;
    bool special;
    std::unique_ptr<SDL_Texture, SDL_Deleter> texture; // Add texture member
    int width = 40; // Add width member
    int height = 40; // Add height member
    float horizontal_speed; // Define horizontal_speed
    float rotation_speed; // Define rotation_speed
};

// 1. Modify Basket class to add renderer parameter and member variable

class Basket {
public:
    Basket(int x, int y, const Fruit& fruit_type, SDL_Renderer* renderer) 
        : x(x), y(y), fruit(fruit_type), catch_count(0), renderer(renderer) {
        texture.reset(loadTexture(renderer, fruit.sprite_path));
        if (!texture) {
            use_color = true;
        }
    }

    void draw(SDL_Renderer* renderer) {
        if (texture && !use_color) {
            SDL_Rect dest_rect = { x, y, width, height };
            SDL_RenderCopy(renderer, texture.get(), nullptr, &dest_rect);
        } else {
            // Use color to draw basket
            SDL_SetRenderDrawColor(renderer, fruit.color.r, fruit.color.g, fruit.color.b, 255);
            SDL_Rect rect = { x, y, width, height };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    bool catchFruit(FallingFruit* fruit) {
        // Check if fruit is in basket range
        return (fruit->getX() >= x && fruit->getX() <= x + 80);
    }

    bool isClicked(int x, int y) const {
        return x > this->x && x < this->x + 80 && y > this->y && y < this->y + 40;
    }

    const Fruit& getFruit() const { return fruit; } // Add getFruit member function

private:
    int x, y;
    int width = 80;
    int height = 40;
    Fruit fruit;
    int catch_count;
    std::unique_ptr<SDL_Texture, SDL_Deleter> texture;
    bool use_color = false;
    SDL_Renderer* renderer;  // Add renderer member variable
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
            button->draw(renderer, fonts["regular"].get());
        }
        
        // Draw bottom bar with version and credits
        drawBottomBar();
    }

    void createUI() {
        // Implement createUI
        // Initialize UI elements
    }

    void drawBackground() {
        // Implement drawBackground
        SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
        SDL_RenderClear(renderer);
    }

    void drawTopBar() {
        // Implement drawTopBar
        // Example: Draw a simple top bar
        SDL_Rect top_bar = {0, 0, WIDTH, 50};
        SDL_SetRenderDrawColor(renderer, DARK_PRIMARY.r, DARK_PRIMARY.g, DARK_PRIMARY.b, DARK_PRIMARY.a);
        SDL_RenderFillRect(renderer, &top_bar);
    }

    void drawCurrentView() {
        // Implement drawCurrentView
        // Example: Depending on current view, draw relevant UI
    }

    void drawLogo() {
        // Implement drawLogo
        // Example: Load and render a logo texture
    }

    void drawBottomBar() {
        // Implement drawBottomBar
        SDL_Rect bottom_bar = { 0, HEIGHT - 30, WIDTH, 30 };
        drawRoundedRect(renderer, bottom_bar, 0, DARK_PRIMARY);
        drawText(fonts["regular"].get(), "Version 1.0 | Credits", 10, HEIGHT - 25, TEXT_SECONDARY);
    }

    void drawComboMeter() {
        // Implement drawComboMeter
        SDL_Rect meter_bg = { WIDTH - 220, 10, 200, 30 };
        drawRoundedRect(renderer, meter_bg, 5, DARK_SECONDARY);
        // Assume a combo value
        int combo = 5; // Example value
        SDL_Rect meter_fg = { WIDTH - 220, 10, static_cast<int>(200 * (combo / 10.0f)), 30 };
        drawRoundedRect(renderer, meter_fg, 5, ACCENT_GREEN);
        drawText(fonts["regular"].get(), "Combo: " + std::to_string(combo), WIDTH - 210, 15, TEXT_PRIMARY);
    }

    void drawPowerUpIndicators() {
        // Implement drawPowerUpIndicators
        // Draw currently active power-up indicators
        int x = 10;
        int y = HEIGHT - 50;
        for (const auto& powerup : {"double_points", "slow_motion"}) {
            if (hud_manager->powerUpManager.isPowerUpActive(powerup)) {
                // ...implement drawing...
            }
        }
    }

    void drawProgressBar() {
        // Implement drawProgressBar
        // Draw progress bar, e.g., game progress
        SDL_Rect progress_bg = { 10, HEIGHT - 90, WIDTH - 20, 20 };
        drawRoundedRect(renderer, progress_bg, 5, DARK_SECONDARY);
        // Assume progress is 50%
        float progress = 0.5f; // Example value
        SDL_Rect progress_fg = { 10, HEIGHT - 90, static_cast<int>((WIDTH - 20) * progress), 20 };
        drawRoundedRect(renderer, progress_fg, 5, ACCENT_GREEN);
    }

    void drawRoundedRect(SDL_Renderer* renderer, SDL_Rect rect, int radius, Color color) {
        // Implement drawRoundedRect
        roundedRectangleRGBA(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h,
                             radius, color.r, color.g, color.b, color.a);
    }

private:
    void loadFonts() {
        fonts["regular"].reset(TTF_OpenFont("assets/fonts/Roboto-Regular.ttf", 24));
        fonts["bold"].reset(TTF_OpenFont("assets/fonts/Roboto-Bold.ttf", 24));
        fonts["light"].reset(TTF_OpenFont("assets/fonts/Roboto-Light.ttf", 24));
        if (!fonts["regular"] || !fonts["bold"] || !fonts["light"]) {
            std::cerr << "Failed to load fonts: " << TTF_GetError() << std::endl;
        }
    }

    void drawBlurredBackground() {
        // Implement gaussian blur effect
        // A simplified example, for actual implementation consider using a more complex blur algorithm or library
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 100); // Semi-transparent black
        SDL_RenderFillRect(renderer, nullptr); // Fill entire background
    }

    void drawText(TTF_Font* font, const std::string& text, int x, int y, Color color) {
        // Implement text drawing
        SDL_Color sdl_color = toSDLColor(color);
        SDL_Surface* text_surface = TTF_RenderText_Solid(font, text.c_str(), sdl_color);
        SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        int text_width, text_height;
        SDL_QueryTexture(text_texture, NULL, NULL, &text_width, &text_height);
        SDL_Rect text_rect = { x, y, text_width, text_height };
        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
        SDL_FreeSurface(text_surface);
        SDL_DestroyTexture(text_texture);
    }

    SDL_Renderer* renderer;
    std::map<std::string, std::unique_ptr<TTF_Font, SDL_Deleter>> fonts;
    std::vector<ModernButton*> menu_buttons;
};

class MenuManager {
public:
    MenuManager(SDL_Renderer* renderer) : renderer(renderer) {
        // ...initialize menu...
    }
    void draw() {
        // ...draw menu...
    }
    void handleEvent(SDL_Event& event) {
        // ...handle menu events...
    }
private:
    SDL_Renderer* renderer;
    
};

// 4. Implement HUDManager::draw() method

class HUDManager {
public:
    HUDManager(SDL_Renderer* renderer) : renderer(renderer) {
        font.reset(TTF_OpenFont("assets/fonts/Roboto-Regular.ttf", 24));
        if (!font) {
            std::cerr << "Font loading failed: " << TTF_GetError() << std::endl;
        }
    }

    void draw(int score, int combo) {
        // Draw score
        std::string score_text = "Score: " + std::to_string(score);
        drawText(score_text, 10, 10, {255, 255, 255, 255});

        // Draw combo
        std::string combo_text = "Combo: " + std::to_string(combo);
        drawText(combo_text, 10, 40, {255, 255, 0, 255});
    }

private:
    SDL_Renderer* renderer;
    std::unique_ptr<TTF_Font, SDL_Deleter> font;

    void drawText(const std::string& text, int x, int y, SDL_Color color) {
        SDL_Surface* surface = TTF_RenderText_Solid(font.get(), text.c_str(), color);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        int text_width, text_height;
        SDL_QueryTexture(texture, NULL, NULL, &text_width, &text_height);
        SDL_Rect text_rect = { x, y, text_width, text_height };
        SDL_RenderCopy(renderer, texture, NULL, &text_rect);
        SDL_DestroyTexture(texture);
    }
};

// 5. Modify drawRoundedRect function to use SDL2_gfx library to actually draw rounded rectangles

void ModernButton::drawRoundedRect(SDL_Renderer* renderer, SDL_Rect rect, int radius, Color color) {
    roundedRectangleRGBA(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h,
                         radius, color.r, color.g, color.b, color.a);
}

// Similarly, modify drawRoundedRect function in UIManager:

void UIManager::drawRoundedRect(SDL_Renderer* renderer, SDL_Rect rect, int radius, Color color) {
    roundedRectangleRGBA(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.h,
                         radius, color.r, color.g, color.b, color.a);
}

// Ensure SDL2_gfx header is included at the beginning of the file

#include <SDL2/SDL2_gfxPrimitives.h>

// 6. Optimize frame rate management in the game loop

void run() {
    // ...existing code...
    const int FPS = 60;
    const int FRAME_DELAY = 1000 / FPS;  // Milliseconds per frame

    while (running) {
        Uint32 frame_start = SDL_GetTicks();

        // ...existing code (event handling, update, draw)...

        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }
}

// Additionally, draw HUD in PlayingState::draw()

void draw(Game& game) override {
    // ...existing code...

    // Draw HUD, pass current score and combo count
    game.hud_manager->draw(score, combo_system.getComboMultiplier());

    SDL_RenderPresent(game.getRenderer());
}

// 1. Use smart pointers to manage SDL resources, define deleter for SDL resources

struct SDL_Deleter {
    void operator()(SDL_Texture* texture) const {
        if (texture) SDL_DestroyTexture(texture);
    }
    void operator()(TTF_Font* font) const {
        if (font) TTF_CloseFont(font);
    }
    void operator()(SDL_Surface* surface) const {
        if (surface) SDL_FreeSurface(surface);
    }
    void operator()(SDL_Renderer* renderer) const {
        if (renderer) SDL_DestroyRenderer(renderer);
    }
    void operator()(SDL_Window* window) const {
        if (window) SDL_DestroyWindow(window);
    }
};

class Game {
public:
    Game() : running(true) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
            logSDLError("SDL Initialization Failed");
            exit(1);
        }
        if (TTF_Init() == -1) {
            logSDLError("TTF Initialization Failed");
            exit(1);
        }
        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags)) {
            logSDLError("SDL_image Initialization Failed");
            exit(1);
        }
        // Create window
        window.reset(SDL_CreateWindow("Fruit Basket Sorter Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN));
        if (!window) {
            logSDLError("Window Creation Failed");
            exit(1);
        }
        // Create renderer
        renderer.reset(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));
        if (!renderer) {
            logSDLError("Renderer Creation Failed");
            exit(1);
        }
        // Load font
        font.reset(TTF_OpenFont("assets/fonts/Roboto-Regular.ttf", 24));
        if (!font) {
            logSDLError("Font Loading Failed");
            exit(1);
        }
        // Initialize other members
        ui_manager = std::make_unique<UIManager>(renderer.get());
        current_state = std::make_unique<MenuState>(*ui_manager);
        hud_manager = std::make_unique<HUDManager>(renderer.get());
        // ...existing code...
    }

    ~Game() {
        // Resources will be automatically released, no need to manually delete
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

            // Add frame rate limit, delay at least 16 milliseconds per loop (about 60 FPS)
            Uint32 frame_time = SDL_GetTicks() - current_time;
            if (frame_time < 16) {
                SDL_Delay(16 - frame_time);
            }
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

    SDL_Renderer* getRenderer() const { return renderer.get(); }

    // ...existing code...

    std::unique_ptr<HUDManager> hud_manager;

private:
    std::unique_ptr<SDL_Window, SDL_Deleter> window;
    std::unique_ptr<SDL_Renderer, SDL_Deleter> renderer;
    std::unique_ptr<TTF_Font, SDL_Deleter> font;
    bool running;
    std::unique_ptr<State> current_state;
    std::unique_ptr<UIManager> ui_manager;
};

// Add Color to SDL_Color conversion function
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
    MenuState(UIManager& ui_manager) : ui_manager(ui_manager) { // Pass UIManager reference
        // Initialize buttons using ui_manager.getFont()
        start_button = std::make_unique<ModernButton>(WIDTH / 2 - 100, HEIGHT / 2 - 50, 200, 50, "Start Game", FRUIT_THEME.primary);
        quit_button = std::make_unique<ModernButton>(WIDTH / 2 - 100, HEIGHT / 2 + 50, 200, 50, "Quit", FRUIT_THEME.error);
    }

    void handleEvent(Game& game, SDL_Event& event) override {
        start_button->handleEvent(event);
        quit_button->handleEvent(event);

        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            int x = event.button.x;
            int y = event.button.y;
            if (start_button->isClicked(x, y)) {
                game.changeState(std::make_unique<PlayingState>(game));
            } else if (quit_button->isClicked(x, y)) {
                game.quit();
            }
        }
        
    }

    void update(Game& game, float dt) override {
        // ...possible animation updates...
    }

    void draw(Game& game) override {
        // Use ui_manager.getFont()
        ui_manager.draw();
        // ...existing code...
        SDL_SetRenderDrawColor(game.getRenderer(), FRUIT_THEME.background.r, FRUIT_THEME.background.g, FRUIT_THEME.background.b, 255);
        SDL_RenderClear(game.getRenderer());

        // Draw title
        SDL_Surface* title_surface = TTF_RenderText_Solid(game.font.get(), "Fruit Basket Sorter", {FRUIT_THEME.text.r, FRUIT_THEME.text.g, FRUIT_THEME.text.b});
        SDL_Texture* title_texture = SDL_CreateTextureFromSurface(game.getRenderer(), title_surface);
        int title_width, title_height;
        SDL_QueryTexture(title_texture, NULL, NULL, &title_width, &title_height);
        SDL_Rect title_rect = {WIDTH / 2 - title_width / 2, HEIGHT / 4, title_width, title_height};
        SDL_RenderCopy(game.getRenderer(), title_texture, NULL, &title_rect);
        SDL_FreeSurface(title_surface);
        SDL_DestroyTexture(title_texture);

        // Draw buttons
        start_button->draw(game.getRenderer(), game.font.get());
        quit_button->draw(game.getRenderer(), game.font.get());
    }

private:
    std::unique_ptr<ModernButton> start_button;
    std::unique_ptr<ModernButton> quit_button;
    UIManager& ui_manager;
};

class PlayingState : public State {
public:
    PlayingState(Game& game) : game(game), score(0) { // Initialize game here
        initializeFruits();
        initializeBaskets();
    }

    void handleEvent(Game& game, SDL_Event& event) override {
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            handleClick(game, x, y);
        }
        // Handle other events
        
    }

    void update(Game& game, float dt) override {
        // Update game logic
        updateFruits(dt);
        updatePowerUps(dt);
        combo_system.update(dt);
        // Update timers, etc.
        
    }

    void draw(Game& game) override {
        // Clear screen
        SDL_SetRenderDrawColor(game.getRenderer(), FRUIT_THEME.background.r, FRUIT_THEME.background.g, FRUIT_THEME.background.b, 255);
        SDL_RenderClear(game.getRenderer());

        // Draw game elements
        drawFruits(game.getRenderer());
        drawBaskets(game.getRenderer());
        game.hud_manager->draw(score, combo_system.getComboMultiplier());

        SDL_RenderPresent(game.getRenderer());
        
    }

private:
    void handleClick(Game& game, int x, int y) {
        // Handle click events, e.g., check if basket is clicked
        for (const auto& basket : baskets) {
            if (basket->isClicked(x, y)) {
                if (current_fruit && basket->getFruit().type == current_fruit->getFruit().type) {
                    // Match successful, handle it
                    score += 10 * combo_system.getComboMultiplier();
                    combo_system.addCombo();
                    effects.addEffect("explosion", current_fruit->getX(), current_fruit->getY());
                    current_fruit.reset();
                } else {
                    // Match failed, reset combo
                    combo_system.resetCombo();
                }
                break;
            }
        }
    }

    void updateFruits(float dt) {
        // Update fruit positions and states
        if (current_fruit) {
            current_fruit->update(dt);
            if (current_fruit->getY() > HEIGHT) {
                // Fruit hit the ground, reset combo
                combo_system.resetCombo();
                current_fruit.reset();
            }
        } else {
            spawnFruit();
        }
    }

    void updatePowerUps(float dt) {
        power_ups.update(dt, game);
    }

    void drawFruits(SDL_Renderer* renderer) {
        if (current_fruit) {
            current_fruit->draw(renderer);
        }
        effects.draw(renderer);
    }

    void drawBaskets(SDL_Renderer* renderer) {
        // Draw baskets
        for (const auto& basket : baskets) {
            basket->draw(renderer);
        }
    }

    void initializeFruits() {
        // Initialize fruits
        for (int i = 0; i < 5; ++i) { // Start with 5 fruits
            spawnFruit();
        }
    }

    // 2. Modify PlayingState::initializeBaskets(), pass renderer when creating Basket

    void initializeBaskets() {
        // Initialize baskets
        int spacing = WIDTH / FRUITS.size();
        for (size_t i = 0; i < FRUITS.size(); ++i) {
            baskets.push_back(std::make_unique<Basket>(
                i * spacing + spacing / 2 - 40, HEIGHT - 60, FRUITS[i], game.getRenderer()));
        }
    }

    // Use container of smart pointers
    std::vector<std::unique_ptr<FallingFruit>> falling_fruits;
    std::vector<std::unique_ptr<Basket>> baskets;
    FallingFruit* current_fruit = nullptr;
    // ...existing code...

    Game& game;
    int score;
    ComboSystem combo_system;
    std::unique_ptr<FallingFruit> current_fruit;
    PowerUpManager power_ups;
    EffectManager effects;

    // Add missing methods
    void multiplyScore(float factor) {
        score = static_cast<int>(score * factor);
    }

    void setTimeScale(float scale) {
        // Adjust game time scale
    }

    void spawnBonusFruits() {
        // Spawn extra fruits
    }

    void boostCombo() {
        combo_system.addCombo();
    }

    // 3. In PlayingState::spawnFruit(), set fruit to falling state

    void spawnFruit() {
        int index = rand() % FRUITS.size();
        float x = rand() % (WIDTH - 40);
        current_fruit = std::make_unique<FallingFruit>(x, -40, FRUITS[index]);
        current_fruit->setFalling(true);  // Add this line
    }
};

// Define GameOverState
class GameOverState : public State {
public:
    GameOverState(Game& game, int final_score, UIManager& ui_manager) : score(final_score), ui_manager(ui_manager) {
        // Initialize button
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
        // ...possible animation updates...
    }

    void draw(Game& game) override {
        // Use ui_manager.getFont()
        ui_manager.draw();
        // ...existing code...
        SDL_SetRenderDrawColor(game.getRenderer(), FRUIT_THEME.background.r, FRUIT_THEME.background.g, FRUIT_THEME.background.b, 255);
        SDL_RenderClear(game.getRenderer());

        // Draw "Game Over" text
        SDL_Surface* game_over_surface = TTF_RenderText_Solid(game.font.get(), "Game Over!", {FRUIT_THEME.text.r, FRUIT_THEME.text.g, FRUIT_THEME.text.b});
        SDL_Texture* game_over_texture = SDL_CreateTextureFromSurface(game.getRenderer(), game_over_surface);
        int game_over_width, game_over_height;
        SDL_QueryTexture(game_over_texture, NULL, NULL, &game_over_width, &game_over_height);
        SDL_Rect game_over_rect = {WIDTH / 2 - game_over_width / 2, HEIGHT / 4, game_over_width, game_over_height};
        SDL_RenderCopy(game.getRenderer(), game_over_texture, NULL, &game_over_rect);

        SDL_FreeSurface(game_over_surface);
        SDL_DestroyTexture(game_over_texture);

        // Draw play again button
        play_again_button->draw(game.getRenderer(), game.font.get());
    }

private:
    int score;
    std::unique_ptr<ModernButton> play_again_button;
    UIManager& ui_manager;
};

// Add logging functionality
void logSDLError(const std::string& msg) {
    std::cerr << msg << " Error: " << SDL_GetError() << std::endl;
}

// Add logging where needed, e.g.:
SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        logSDLError("Failed to load image: " + path);
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) {
        logSDLError("Failed to create texture: " + path);
    }
    return texture;
}

// Update draw function of FallingFruit, use error handling
void FallingFruit::draw(SDL_Renderer* renderer) {
    if (!texture) {
        texture.reset(loadTexture(renderer, fruit.sprite_path));
        if (!texture) return;
    }
    SDL_Rect dest_rect = { static_cast<int>(x), static_cast<int>(y), width, height };
    SDL_RenderCopyEx(renderer, texture.get(), nullptr, &dest_rect, rotation, nullptr, SDL_FLIP_NONE);
}

// Initialize initial state in Game constructor
Game::Game() : running(true) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        logSDLError("SDL Initialization Failed");
        exit(1);
    }
    if (TTF_Init() == -1) {
        logSDLError("TTF Initialization Failed");
        exit(1);
    }
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        logSDLError("SDL_image Initialization Failed");
        exit(1);
    }
    // Create window
    window.reset(SDL_CreateWindow("Fruit Basket Sorter Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN));
    if (!window) {
        logSDLError("Window Creation Failed");
        exit(1);
    }
    // Create renderer
    renderer.reset(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));
    if (!renderer) {
        logSDLError("Renderer Creation Failed");
        exit(1);
    }
    // Load font
    font.reset(TTF_OpenFont("assets/fonts/Roboto-Regular.ttf", 24));
    if (!font) {
        logSDLError("Font Loading Failed");
        exit(1);
    }
    // Initialize other members
    current_state = std::make_unique<MenuState>();
    hud_manager = std::make_unique<HUDManager>(renderer.get());
    // ...existing code...
}

// Implement Game::changeState() method
void Game::changeState(std::unique_ptr<State> new_state) {
    current_state = std::move(new_state);
}

// Modify game loop in main function
int main(int argc, char* args[]) {
    srand(static_cast<unsigned int>(time(0)));
    Game game;
    game.run();
    return 0;
}
