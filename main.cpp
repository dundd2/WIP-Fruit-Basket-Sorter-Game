#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <chrono>

// Basic game constants
const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 20;

// Game objects
struct Fruit {
    std::string type;
    char symbol;
};

struct Basket {
    int x;
    std::string type;
    char symbol;
};

class Game {
private:
    bool running;
    int score;
    std::vector<Fruit> fruits;
    std::vector<Basket> baskets;
    Fruit* currentFruit;
    int fruitY;

    void initializeFruits() {
        fruits = {
            {"apple", 'A'},
            {"banana", 'B'},
            {"orange", 'O'},
            {"grape", 'G'}
        };
    }

    void initializeBaskets() {
        int spacing = SCREEN_WIDTH / fruits.size();
        for (size_t i = 0; i < fruits.size(); i++) {
            Basket basket;
            basket.x = i * spacing + spacing / 2;
            basket.type = fruits[i].type;
            basket.symbol = fruits[i].symbol;
            baskets.push_back(basket);
        }
    }

    void spawnFruit() {
        if (!currentFruit) {
            currentFruit = new Fruit(fruits[rand() % fruits.size()]);
            fruitY = 0;
        }
    }

    void drawGame() {
        system("clear"); // Use "cls" for Windows
        
        // Draw score
        std::cout << "Score: " << score << "\n\n";

        // Draw game area
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                if (currentFruit && y == fruitY && x == SCREEN_WIDTH/2) {
                    std::cout << currentFruit->symbol;
                } else if (y == SCREEN_HEIGHT-1) {
                    bool isBasket = false;
                    for (const auto& basket : baskets) {
                        if (x == basket.x) {
                            std::cout << basket.symbol;
                            isBasket = true;
                            break;
                        }
                    }
                    if (!isBasket) std::cout << "-";
                } else {
                    std::cout << " ";
                }
            }
            std::cout << "\n";
        }

        // Draw controls
        std::cout << "\nControls: [1-4] to select basket, [Q] to quit\n";
    }

public:
    Game() : running(true), score(0), currentFruit(nullptr), fruitY(0) {
        initializeFruits();
        initializeBaskets();
    }

    void run() {
        while (running) {
            spawnFruit();
            drawGame();

            // Handle input
            if (_kbhit()) {
                char input = _getch();
                if (input >= '1' && input <= '4') {
                    int basketIndex = input - '1';
                    if (basketIndex < baskets.size()) {
                        if (currentFruit->type == baskets[basketIndex].type) {
                            score += 10;
                        } else {
                            score -= 5;
                        }
                        delete currentFruit;
                        currentFruit = nullptr;
                    }
                } else if (input == 'q' || input == 'Q') {
                    running = false;
                }
            }

            // Update fruit position
            if (currentFruit) {
                fruitY++;
                if (fruitY >= SCREEN_HEIGHT-1) {
                    delete currentFruit;
                    currentFruit = nullptr;
                    score -= 5;
                }
            }

            // Game speed
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        std::cout << "\nGame Over! Final Score: " << score << "\n";
    }

    ~Game() {
        delete currentFruit;
    }
};

int main() {
    srand(time(0));
    Game game;
    game.run();
    return 0;
}