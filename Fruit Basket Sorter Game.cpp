#include <SDL2/SDL.h>
#include <SDL_mixer.h>
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
#include <json/json.hpp> // Add JSON support for save data
#include <iomanip>
#include <sstream>
#include <unordered_map>

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
    std::map<GameMode, int> high_scores;
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
};

const int WIDTH = 800;
const int HEIGHT = 600;

struct Color {
    int r, g, b;
};

const Color WHITE = {255, 255, 255};
const Color BLACK = {0, 0, 0};
const Color RED = {255, 0, 0};
const Color BLUE = {0, 0, 255};
const Color GREEN = {0, 255, 0};
const Color GRAY = {128, 128, 128};
const Color YELLOW = {255, 255, 0};

// Correct initialization of MODERN_COLORS
const Color DARK_PRIMARY = {18, 18, 18};      // Dark background
const Color DARK_SECONDARY = {30, 30, 30};    // Secondary dark
const Color ACCENT_BLUE = {66, 133, 244};     // Google Blue
const Color ACCENT_GREEN = {52, 168, 83};     // Success Green
const Color ACCENT_RED = {234, 67, 53};       // Error Red
const Color TEXT_PRIMARY = {255, 255, 255};   // White text
const Color TEXT_SECONDARY = {170, 170, 170}; // Gray text
const Color BUTTON_HOVER = {45, 45, 45};      // Button hover state
const Color BUTTON_ACTIVE = {60, 60, 60};     // Button active state

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
    {245, 245, 245}, {66, 133, 244}, {52, 168, 83}, {251, 188, 4}, {234, 67, 53}, {32, 33, 36}, {0, 0, 0, 50}, {255, 255, 255}, {200, 200, 200}
};

const Theme DARK_THEME = {
    {30, 30, 30}, {138, 180, 248}, {129, 201, 149}, {253, 214, 99}, {242, 139, 130}, {232, 234, 237}, {0, 0, 0, 80}, {70, 70, 70}, {70, 70, 70}
};

const Theme FRUIT_THEME = {
    {230, 255, 230}, // light green background
    {255, 102, 102}, // soft red
    {255, 178, 102}, // soft orange
    {178, 255, 102}, // lime green
    {255, 51, 51},   // bright red
    {51, 51, 51},    // dark text
    {0, 0, 0, 50},   // shadow
    {255, 255, 255}, // white
    {200, 200, 200}  // light gray
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
    {40, 40, 40}     // track
};

struct Fruit {
    std::string type;
    Color color;
    std::string sprite_path;
};

const std::vector<Fruit> FRUITS = {
    {"apple", {255, 0, 0}, "sprites/apple.png"},
    {"banana", {255, 255, 0}, "sprites/banana.png"},
    {"orange", {255, 165, 0}, "sprites/orange.png"},
    {"grape", {128, 0, 128}, "sprites/grape.png"}
};

// Add new game modes
enum class GameMode {
    CLASSIC,
    TIME_ATTACK,
    ZEN_MODE,
    CHALLENGE
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

private:
    int current_combo;
    int max_combo;
    float combo_timer;
};

// Add special effects manager
class EffectManager {
public:
    void addEffect(const std::string& type, float x, float y) {
        if (type == "explosion") {
            for (int i = 0; i < 20; i++) {
                // 初始化粒子
                particles.push_back(new Particle(x, y, randomColor()));
            }
        }
        // 添加更多效果類型
    }
    
    void update(float dt) {
        particles.erase(
            std::remove_if(particles.begin(), particles.end(),
                [dt](Particle* p) {
                    p->update(dt);
                    return p->isDead();
                }),
            particles.end()
        );
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
            static_cast<int>(rand() % 256),
            static_cast<int>(rand() % 256),
            static_cast<int>(rand() % 256)
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

private:
    std::vector<int> high_scores;
    
    void loadHighScores() {
        std::ifstream file("highscores.dat");
        int score;
        while (file >> score) {
            high_scores.push_back(score);
        }
    }
    
    void saveHighScores() {
        std::ofstream file("highscores.dat");
        for (int score : high_scores) {
            file << score << std::endl;
        }
    }
};

// Add power-up system
class PowerUpManager {
public:
    void addPowerUp(const std::string& type) {
        active_powerups[type] = 10.0f; // 10 seconds duration
    }
    
    void update(float dt) {
        for (auto it = active_powerups.begin(); it != active_powerups.end();) {
            it->second -= dt;
            if (it->second <= 0) {
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

class SoundManager {
public:
    SoundManager() {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        }
        loadSounds();
    }

    ~SoundManager() {
        for (auto& sound : sounds) {
            Mix_FreeChunk(sound.second);
        }
        Mix_CloseAudio();
    }

    void play(const std::string& sound_name) {
        if (!muted && sounds.find(sound_name) != sounds.end()) {
            Mix_PlayChannel(-1, sounds[sound_name], 0);
        }
    }

    void toggleMute() {
        muted = !muted;
        if (muted) {
            Mix_Pause(-1);
        } else {
            Mix_Resume(-1);
        }
    }

private:
    void loadSounds() {
        sounds["correct"] = Mix_LoadWAV("assets/sounds/correct.wav");
        sounds["wrong"] = Mix_LoadWAV("assets/sounds/wrong.wav");
        sounds["click"] = Mix_LoadWAV("assets/sounds/click.wav");
        sounds["game_over"] = Mix_LoadWAV("assets/sounds/game_over.wav");
        sounds["background"] = Mix_LoadWAV("assets/sounds/background.wav");
        if (!sounds["background"]) {
            std::cerr << "Background sound failed to load: " << Mix_GetError() << std::endl;
        } else {
            Mix_VolumeChunk(sounds["background"], MIX_MAX_VOLUME / 2);
            Mix_PlayChannel(-1, sounds["background"], -1);
        }
    }

    std::unordered_map<std::string, Mix_Chunk*> sounds;
    bool muted = false;
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
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b); // 移除 alpha
            SDL_Rect rect = {static_cast<int>(x - size), static_cast<int>(y - size), static_cast<int>(size * 2), static_cast<int>(size * 2)};
            SDL_RenderFillRect(renderer, &rect);
        }
    }

private:
    float x, y;
    Color color;
    float size;
    float lifetime;
    float velocity[2];
};

class Button {
public:
    Button(int x, int y, int width, int height, const std::string& text, Color color)
        : rect{x, y, width, height}, text(text), color(color) {}

    void draw(SDL_Renderer* renderer, TTF_Font* font) {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, BLACK.r, BLACK.g, BLACK.b, 255);
        SDL_RenderDrawRect(renderer, &rect);

        SDL_Surface* text_surface = TTF_RenderText_Solid(font, text.c_str(), {BLACK.r, BLACK.g, BLACK.b});
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

private:
    SDL_Rect rect;
    std::string text;
    Color color;
};

// Add modern UI components
class ModernButton : public Button {
public:
    ModernButton(int x, int y, int width, int height, const std::string& text, Color color)
        : Button(x, y, width, height, text, color) {
        corner_radius = 8;  // Rounded corners
        has_shadow = true;
    }

    void draw(SDL_Renderer* renderer, TTF_Font* font) override {
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
        SDL_Color text_color = is_hovered ? TEXT_PRIMARY : TEXT_SECONDARY;
        drawText(renderer, font, text, text_color);
    }

private:
    int corner_radius;
    bool has_shadow;
    bool is_hovered = false;
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
    DailyChallenge() {
        loadChallenges();
        checkAndUpdateDaily();
    }
    
    bool isCompleted() const { return completed; }
    const std::string& getDescription() const { return current_challenge; }
    
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
        std::string today = std::put_time(std::localtime(&time), "%Y-%m-%d");
        
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
        effects["double_points"] = [](Game* game) { game->multiplyScore(2.0f); };
        effects["slow_motion"] = [](Game* game) { game->setTimeScale(0.5f); };
        effects["fruit_rain"] = [](Game* game) { game->spawnBonusFruits(); };
        effects["combo_booster"] = [](Game* game) { game->boostCombo(); };
    }
    
    void applyEffect(Game* game) {
        if (effects.find(getType()) != effects.end()) {
            effects[getType()](game);
        }
    }

private:
    float strength;
    std::map<std::string, std::function<void(Game*)>> effects;
};

class FallingFruit {
public:
    FallingFruit(float x, float y, const Fruit& fruit_type) 
        : x(x), y(y), fruit(fruit_type), falling(false), 
          rotation(0), fall_speed(200), special(rand() % 10 == 0) {}

    void draw(SDL_Renderer* renderer) {
        // Replace train drawing with fruit drawing
        SDL_SetRenderDrawColor(renderer, fruit.color.r, fruit.color.g, fruit.color.b, 255);
        
        // Draw fruit as a circle
        const int radius = 20;
        for(int w = 0; w < radius * 2; w++) {
            for(int h = 0; h < radius * 2; h++) {
                int dx = radius - w;
                int dy = radius - h;
                if((dx*dx + dy*dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer, x + w, y + h);
                }
            }
        }

        // Draw shine effect for special fruits
        if (special) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 128);
            SDL_Rect shine = {(int)x + 5, (int)y + 5, 10, 10};
            SDL_RenderFillRect(renderer, &shine);
        }
    }

    void update(float dt) {
        if (falling) {
            y += fall_speed * dt;
            rotation += 90 * dt; // Rotate while falling
        }
    }

    // ... keep other necessary methods ...

private:
    float x, y;
    Fruit fruit;
    bool falling;
    float rotation;
    float fall_speed;
    bool special;
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

private:
    int x, y;
    Fruit fruit;
    int catch_count;
};

// Add new Tutorial system
class TutorialManager {
public:
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

    void drawRoundedRect(SDL_Renderer* renderer, SDL_Rect rect, int radius, Color color) {
        // Implement rounded rectangle drawing
    }

    void drawText(TTF_Font* font, const std::string& text, int x, int y, Color color) {
        // Implement text drawing
    }

    void drawComboMeter() {
        // Implement combo meter drawing
    }

    void drawPowerUpIndicators() {
        // Implement power-up indicators drawing
    }

    void drawProgressBar() {
        // Implement progress bar drawing
    }

    SDL_Renderer* renderer;
    std::map<std::string, TTF_Font*> fonts;
    std::vector<ModernButton*> menu_buttons;
};

class Game {
public:
    Game() : state(MENU), score(0), current_fruit_index(0), all_fruits_falling(false) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
            std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        }
        if (TTF_Init() == -1) {
            std::cerr << "TTF_Init: " << TTF_GetError() << std::endl;
        }
        window = SDL_CreateWindow("Fruit Basket Sorter Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        font = TTF_OpenFont("assets/fonts/arial.ttf", 36);
        sound_manager = new SoundManager();
        resetGame();
        createButtons();
        current_mode = GameMode::CLASSIC;
        time_remaining = 60.0f;
        level = 1;
        difficulty_multiplier = 1.0f;
        initializeAchievements();
        ui_manager = new UIManager(renderer);
        ui_manager->createUI();
    }

    ~Game() {
        TTF_CloseFont(font);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        delete sound_manager;
        delete ui_manager;
    }

    void run() {
        bool running = true;
        while (running) {
            float dt = 1.0f / 60.0f;
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    running = false;
                } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    if (!handleClick(x, y)) {
                        running = false;
                    }
                }
            }
            update(dt);
            draw();
            SDL_Delay(1000 / 60);
        }
    }

private:
    void resetGame() {
        falling_fruits.clear();
        baskets.clear();
        score = 0;
        current_fruit_index = 0;
        all_fruits_falling = false;
        initializeFruits();
    }

    void createButtons() {
        start_button = new ModernButton(WIDTH / 2 - 100, HEIGHT / 2 - 50, 200, 50, "Start Game", FRUIT_THEME.primary);
        quit_button = new ModernButton(WIDTH / 2 - 100, HEIGHT / 2 + 50, 200, 50, "Quit", FRUIT_THEME.error);
        play_again_button = new ModernButton(WIDTH / 2 - 100, HEIGHT / 2 + 50, 200, 50, "Play Again", FRUIT_THEME.primary);
    }

    void initializeFruits() {
        for (int i = 0; i < 10; ++i) {
            Fruit fruit = FRUITS[rand() % FRUITS.size()];
            int x = i * 80;
            falling_fruits.push_back(new FallingFruit(x, 0, fruit));
        }
        for (int i = 0; i < FRUITS.size(); ++i) {
            baskets.push_back(new Basket(250 + i * 100, 500, FRUITS[i]));
        }
    }

    void draw() {
        SDL_SetRenderDrawColor(renderer, FRUIT_THEME.background.r, FRUIT_THEME.background.g, FRUIT_THEME.background.b, 255);
        SDL_RenderClear(renderer);

        if (state == MENU) {
            drawMenu();
        } else if (state == PLAYING) {
            drawGame();
        } else if (state == GAME_OVER) {
            drawGameOver();
        }

        SDL_RenderPresent(renderer);
    }

    void drawMenu() {
        ui_manager->drawModernMenu(); // 使用 UIManager 繪製現代化菜單
        SDL_Surface* title_surface = TTF_RenderText_Solid(font, "Fruit Basket Sorter", {FRUIT_THEME.text.r, FRUIT_THEME.text.g, FRUIT_THEME.text.b});
        SDL_Texture* title_texture = SDL_CreateTextureFromSurface(renderer, title_surface);
        int title_width, title_height;
        SDL_QueryTexture(title_texture, NULL, NULL, &title_width, &title_height);
        SDL_Rect title_rect = {WIDTH / 2 - title_width / 2, HEIGHT / 4, title_width, title_height};
        SDL_RenderCopy(renderer, title_texture, NULL, &title_rect);
        SDL_FreeSurface(title_surface);
        SDL_DestroyTexture(title_texture);

        start_button->draw(renderer, font);
        quit_button->draw(renderer, font);
    }

    void drawGame() {
        for (auto& fruit : falling_fruits) {
            if (!fruit->isFalling()) {
                fruit->draw(renderer);
            }
        }
        for (auto& basket : baskets) {
            basket->draw(renderer);
        }

        SDL_Surface* score_surface = TTF_RenderText_Solid(font, ("Score: " + std::to_string(score)).c_str(), {FRUIT_THEME.text.r, FRUIT_THEME.text.g, FRUIT_THEME.text.b});
        SDL_Texture* score_texture = SDL_CreateTextureFromSurface(renderer, score_surface);
        int score_width, score_height;
        SDL_QueryTexture(score_texture, NULL, NULL, &score_width, &score_height);
        SDL_Rect score_rect = {10, 10, score_width, score_height};
        SDL_RenderCopy(renderer, score_texture, NULL, &score_rect);
        SDL_FreeSurface(score_surface);
        SDL_DestroyTexture(score_texture);
        drawPowerUps();
        drawCombo();
        drawTimer();
        effect_manager.draw(renderer);
    }

    void drawGameOver() {
        SDL_Surface* game_over_surface = TTF_RenderText_Solid(font, "Game Over!", {FRUIT_THEME.text.r, FRUIT_THEME.text.g, FRUIT_THEME.text.b});
        SDL_Texture* game_over_texture = SDL_CreateTextureFromSurface(renderer, game_over_surface);
        int game_over_width, game_over_height;
        SDL_QueryTexture(game_over_texture, NULL, NULL, &game_over_width, &game_over_height);
        SDL_Rect game_over_rect = {WIDTH / 2 - game_over_width / 2, HEIGHT / 4, game_over_width, game_over_height};
        SDL_RenderCopy(renderer, game_over_texture, NULL, &game_over_rect);
        SDL_FreeSurface(game_over_surface);
        SDL_DestroyTexture(game_over_texture);

        SDL_Surface* score_surface = TTF_RenderText_Solid(font, ("Final Score: " + std::to_string(score)).c_str(), {FRUIT_THEME.text.r, FRUIT_THEME.text.g, FRUIT_THEME.text.b});
        SDL_Texture* score_texture = SDL_CreateTextureFromSurface(renderer, score_surface);
        int score_width, score_height;
        SDL_QueryTexture(score_texture, NULL, NULL, &score_width, &score_height);
        SDL_Rect score_rect = {WIDTH / 2 - score_width / 2, HEIGHT / 3, score_width, score_height};
        SDL_RenderCopy(renderer, score_texture, NULL, &score_rect);
        SDL_FreeSurface(score_surface);
        SDL_DestroyTexture(score_texture);

        play_again_button->draw(renderer, font);
    }

    bool handleClick(int x, int y) {
        if (state == MENU) {
            if (start_button->isClicked(x, y)) {
                sound_manager->play("click");
                state = PLAYING;
                resetGame();
            } else if (quit_button->isClicked(x, y)) {
                return false;
            }
        } else if (state == PLAYING) {
            for (auto& basket : baskets) {
                if (basket->isClicked(x, y)) {
                    if (current_fruit_index < falling_fruits.size()) {
                        FallingFruit* current_fruit = falling_fruits[current_fruit_index];
                        if (basket->getFruit().type == current_fruit->getFruit().type) {
                            sound_manager->play("correct");
                            current_fruit->setFalling(true);
                            score++;
                            current_fruit_index++;
                            if (current_fruit_index >= falling_fruits.size()) {
                                all_fruits_falling = true;
                            }
                        } else {
                            sound_manager->play("wrong");
                        }
                    }
                }
            }
        } else if (state == GAME_OVER) {
            if (play_again_button->isClicked(x, y)) {
                sound_manager->play("click");
                state = PLAYING;
                resetGame();
            } else if (quit_button->isClicked(x, y)) {
                return false;
            }
        }
        return true;
    }

    void update(float dt) {
        if (state == PLAYING) {
            updateTimeAttack(dt);
            combo_system.update(dt);
            effect_manager.update(dt);
            powerup_manager.update(dt);
            for (auto& fruit : falling_fruits) {
                if (fruit->isFalling()) {
                    fruit->update(dt);
                }
            }
            if (all_fruits_falling && std::all_of(falling_fruits.begin(), falling_fruits.end(), [](FallingFruit* fruit) { return !fruit->isFalling(); })) {
                state = GAME_OVER;
            }
        }
    }

    void initializeAchievements() {
        achievements = {
            {"Fruit Master", "Score 1000 points", false, 0, 1000},
            {"Combo King", "Get a 10x combo", false, 0, 10},
            {"Speed Demon", "Complete level in 30 seconds", false, 0, 1}
        };
    }
    
    void checkAchievements() {
        for (auto& achievement : achievements) {
            if (!achievement.unlocked) {
                if (achievement.name == "Fruit Master" && score >= 1000) {
                    achievement.unlocked = true;
                }
                // Add more achievement checks
            }
        }
    }
    
    void updateTimeAttack(float dt) {
        if (current_mode == GameMode::TIME_ATTACK) {
            time_remaining -= dt;
            if (time_remaining <= 0) {
                state = GAME_OVER;
            }
        }
    }
    
    void increaseDifficulty() {
        difficulty_multiplier += 0.1f;
        // Adjust game parameters based on difficulty
    }
    
    void spawnPowerUp() {
        if (rand() % 100 < 5) { // 5% chance
            std::vector<std::string> types = {"double_points", "slow_motion", "multi_fruit"};
            powerup_manager.addPowerUp(types[rand() % types.size()]);
        }
    }

    void drawPowerUps() {
        // Draw active power-ups
    }
    
    void drawCombo() {
        // Draw combo multiplier
    }
    
    void drawTimer() {
        if (current_mode == GameMode::TIME_ATTACK) {
            // Draw remaining time
        }
    }

    // Add new member variables
    Difficulty difficulty;
    PlayerStats stats;
    DailyChallenge daily_challenge;
    std::map<std::string, Animation*> animations;
    float time_scale;
    bool tutorial_completed;
    std::string player_name;
    MenuState menu_state;
    std::vector<LevelData> levels;
    PlayerProfile player_profile;
    TutorialManager tutorial;
    
    // Add new methods
    void saveProgress() {
        nlohmann::json save_data;
        stats.serialize(save_data["stats"]);
        save_data["tutorial_completed"] = tutorial_completed;
        save_data["high_scores"] = high_score_manager.getHighScores();
        
        std::ofstream file("save.json");
        file << save_data.dump(4);
    }
    
    void loadProgress() {
        std::ifstream file("save.json");
        if (file.is_open()) {
            nlohmann::json save_data;
            file >> save_data;
            stats.deserialize(save_data["stats"]);
            tutorial_completed = save_data["tutorial_completed"];
            // Load high scores
        }
    }
    
    void updateDifficulty() {
        switch (difficulty) {
            case Difficulty::EASY:
                fall_speed_multiplier = 0.7f;
                score_multiplier = 1.0f;
                break;
            case Difficulty::HARD:
                fall_speed_multiplier = 1.3f;
                score_multiplier = 1.5f;
                break;
            case Difficulty::EXPERT:
                fall_speed_multiplier = 1.5f;
                score_multiplier = 2.0f;
                spawnSpecialFruits();
                break;
        }
    }
    
    void runTutorial() {
        if (!tutorial_completed) {
            // Show tutorial steps
            tutorial_steps = {
                "Welcome to Fruit Basket Sorter!",
                "Match fruits with their corresponding baskets",
                "Build combos for bonus points",
                "Watch out for special fruits!"
            };
            // ... tutorial implementation
        }
    }
    
    void spawnBonusFruits() {
        for (int i = 0; i < 5; i++) {
            // Spawn bonus fruits with special effects
        }
    }
    
    void showLeaderboard() {
        // Display top scores with player names
    }

    void initializeLevels() {
        levels = {
            {100, 1.0f, 10, 0.1f, {"double_points"}},
            {250, 1.2f, 15, 0.15f, {"double_points", "slow_motion"}},
            {500, 1.4f, 20, 0.2f, {"double_points", "slow_motion", "multi_fruit"}},
            // Add more levels...
        };
    }
    
    void updateLevelProgression() {
        if (score >= levels[current_level].required_score) {
            current_level++;
            updateDifficulty();
            spawnBonusFruits();
            effect_manager.addEffect("level_up", WIDTH/2, HEIGHT/2);
        }
    }
    
    void handleMenuState() {
        switch(menu_state) {
            case MenuState::MAIN:
                drawMainMenu();
                break;
            case MenuState::MODE_SELECT:
                drawModeSelect();
                break;
            case MenuState::DIFFICULTY_SELECT:
                drawDifficultySelect();
                break;
            // Add more menu states...
        }
    }
    
    void updateAchievements() {
        checkForNewAchievements();
        if (achievement_unlocked) {
            showAchievementPopup();
        }
    }
    
    void saveGameState() {
        nlohmann::json save_data;
        player_profile.serialize(save_data["profile"]);
        save_data["settings"] = {
            {"difficulty", static_cast<int>(difficulty)},
            {"sound_enabled", sound_enabled},
            {"tutorial_completed", tutorial_completed}
        };
        std::ofstream("save_data.json") << save_data.dump(4);
    }
    
    void loadGameState() {
        if (std::ifstream file("save_data.json"); file.is_open()) {
            nlohmann::json save_data;
            file >> save_data;
            player_profile = save_data["profile"];
            // Load other game state...
        }
    }
    
    void updateDailyChallenge() {
        if (daily_challenge.isActive()) {
            if (checkDailyChallengeComplete()) {
                awardDailyChallengeReward();
            }
        }
    }
    
    void drawLeaderboard() {
        // Draw top 10 scores
        const auto& scores = high_score_manager.getHighScores();
        int y_pos = 100;
        for (size_t i = 0; i < std::min(scores.size(), size_t(10)); ++i) {
            drawText(font, std::to_string(i+1) + ". " + std::to_string(scores[i]),
                    WIDTH/2, y_pos, FRUIT_THEME.text);
            y_pos += 40;
        }
    }
    
    void updatePowerUps() {
        for (auto& powerup : active_powerups) {
            powerup.update(dt);
            if (powerup.isActive()) {
                applyPowerUpEffect(powerup);
            }
        }
    }
    
    void checkCombo() {
        if (last_match_time > 0 && 
            current_time - last_match_time < COMBO_WINDOW) {
            current_combo++;
            score_multiplier = std::min(2.0f + current_combo * 0.5f, 5.0f);
        } else {
            current_combo = 0;
            score_multiplier = 1.0f;
        }
    }

public:
    void multiplyScore(float multiplier) {
        score *= multiplier;
    }
    
    void setTimeScale(float scale) {
        time_scale = scale;
    }
    
    void boostCombo() {
        combo_system.addBonus();
    }
    
    void startTutorial() {
        tutorial.startTutorial();
        state = GameState::TUTORIAL;
    }
    
    void setDifficulty(Difficulty diff) {
        difficulty = diff;
        updateDifficulty();
    }
    
    void saveProgress() {
        saveGameState();
        high_score_manager.saveHighScores();
    }

    // ... existing methods ...

    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    SoundManager* sound_manager;
    GameState state;
    int score;
    int current_fruit_index;
    bool all_fruits_falling;
    std::vector<FallingFruit*> falling_fruits;
    std::vector<Basket*> baskets;
    Button* start_button;
    Button* quit_button;
    Button* play_again_button;
    GameMode current_mode;
    ComboSystem combo_system;
    EffectManager effect_manager;
    HighScoreManager high_score_manager;
    PowerUpManager powerup_manager;
    std::vector<Achievement> achievements;
    float time_remaining;
    int level;
    float difficulty_multiplier;

private:
    // Add new member variables
    UIManager* ui_manager;
    bool dark_mode = true;
    float ui_scale = 1.0f;
    
    void createModernUI() {
        // Create modern buttons
        int button_width = 200 * ui_scale;
        int button_height = 50 * ui_scale;
        int start_y = HEIGHT / 2 - 100;
        
        menu_buttons = {
            new ModernButton(WIDTH/2 - button_width/2, start_y, 
                           button_width, button_height, 
                           "Play", MODERN_DARK_THEME.primary),
            new ModernButton(WIDTH/2 - button_width/2, start_y + 70, 
                           button_width, button_height, 
                           "Settings", MODERN_DARK_THEME.secondary),
            new ModernButton(WIDTH/2 - button_width/2, start_y + 140, 
                           button_width, button_height, 
                           "Quit", MODERN_DARK_THEME.error)
        };
    }

    void drawModernUI() {
        // Draw background with gradient
        drawGradientBackground();
        
        // Draw game title with shadow
        drawModernTitle();
        
        // Draw buttons with hover effects
        for (auto& button : menu_buttons) {
            button->draw(renderer, font);
        }
        
        // Draw modern HUD elements
        if (state == PLAYING) {
            drawModernHUD();
        }
    }

    void drawModernHUD() {
        // Draw score panel
        drawPanel(10, 10, 200, 50, "Score: " + std::to_string(score));
        
        // Draw combo meter
        drawComboMeter();
        
        // Draw power-up indicators
        drawPowerUpIndicators();
        
        // Draw progress bar
        drawProgressBar();
    }

    void drawGradientBackground() {
        // Implement vertical gradient background
        Color top_color = dark_mode ? DARK_PRIMARY : LIGHT_PRIMARY;
        Color bottom_color = dark_mode ? DARK_SECONDARY : LIGHT_SECONDARY;
        // Draw gradient...
    }

    void drawPanel(int x, int y, int w, int h, const std::string& text) {
        // Draw modern semi-transparent panel with blur
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 
            DARK_SECONDARY.r, DARK_SECONDARY.g, DARK_SECONDARY.b, 200);
        
        // Draw rounded rectangle
        drawRoundedRect(renderer, {x, y, w, h}, 10, DARK_SECONDARY);
        
        // Draw text
        drawText(font, text, x + w/2, y + h/2, TEXT_PRIMARY);
    }

public:
    void toggleDarkMode() {
        dark_mode = !dark_mode;
        updateTheme();
    }

    void updateTheme() {
        current_theme = dark_mode ? MODERN_DARK_THEME : MODERN_LIGHT_THEME;
        // Update UI elements...
    }
};

int main(int argc, char* args[]) {
    srand(static_cast<unsigned int>(time(0)));
    Game game;
    game.run();
    return 0;
}