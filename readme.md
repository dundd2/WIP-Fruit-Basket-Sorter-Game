# Fruit Basket Sorter Game 🍎🍌🍇

**Fruit Basket Sorter Game** is an engaging and fast-paced game where players match falling fruits with their corresponding baskets. Featuring multiple game modes, achievement tracking, power-ups, and a modern UI, this game offers a variety of challenges and rewards to keep players entertained.

## 📸 Screenshots
![Screenshot1](path/to/screenshot1.png)  
![Screenshot2](path/to/screenshot2.png)  
![Screenshot3](path/to/screenshot3.png)  

## 🎯 Game Features

### Core Gameplay
- **Multiple Game Modes**: Choose from Classic, Time Attack, Zen Mode, and Challenge for different levels of difficulty and styles of play.
- **Dynamic Difficulty Scaling**: Adjusts difficulty based on player performance, ensuring a balanced challenge.
- **Modern UI Design**: Supports both dark and light themes with particle effects, animations, and a power-up system.
- **Achievement Tracking**: Unlock various achievements as you progress.
- **Daily Challenges**: Take on new challenges every day to earn extra rewards.
- **Leaderboard System**: Compete against other players for high scores on the global leaderboard.

### Technical Highlights
- **Modern C++17/20 Implementation**: Built with modern C++ standards for robust performance and reliability.
- **SDL2 for Graphics and Audio**: Utilizes SDL2 for handling graphics, audio, and input, ensuring smooth and responsive gameplay.
- **Advanced UI System**: Designed with user experience in mind, featuring custom widgets and responsive layouts.
- **JSON-based Save System**: Game data is saved in JSON format, making it easy to manage and extend.
- **Particle Effects and Animations**: Includes dynamic visual effects for an immersive experience.
- **Sophisticated Scoring and Combo System**: Earn combo bonuses and maximize your score.
- **Power-up System**: Collect and use power-ups to gain an edge during gameplay.
- **Achievement and Daily Challenge System**: Track achievements and complete daily challenges for continuous engagement.

## 🔧 Technologies & Skills Demonstrated

### Programming & Design
- **Object-Oriented Programming**: Organized code with a focus on reusability and modularity.
- **Design Patterns**: Implements common design patterns for better code structure.
- **Real-time Graphics & Audio Management**: Responsive graphics and sound powered by SDL2.
- **Event-driven Architecture**: Uses an event-driven system for efficient game state and UI management.
- **Performance Optimization**: Designed for optimal performance with modern C++ features.

### Game Development
- **SDL2 Framework**: Leveraging SDL2 for multimedia handling including graphics, audio, and user input.
- **Custom UI Widgets**: Includes custom buttons, progress bars, and dropdowns for a polished interface.
- **Responsive Layout**: Supports various screen sizes with dynamic layouts.
- **Theme Engine**: Customizable style settings, allowing users to switch themes seamlessly.

### Data Integration
- **File I/O and JSON Handling**: Data persistence using JSON for saving and loading game progress.
- **Achievement and Leaderboard System**: Tracks player progress and manages leaderboards.

## 🔨 Build Instructions

1. **Install Dependencies**:
   Ensure you have the following installed:
   - SDL2
   - SDL2_mixer
   - nlohmann/json

2. **Clone the Repository**:
   ```bash
   git clone https://github.com/dundd2/Fruit-Basket-Sorter-Game.git
   ```

3. **Build the Project**:
   You can build the project using CMake or your preferred build system.
   ```bash
   cd Fruit-Basket-Sorter-Game
   mkdir build && cd build
   cmake ..
   make
   ```

4. **Run the Game**:
   ```bash
   ./FruitBasketSorter
   ```

## 📦 Dependencies
- **SDL2**: For graphics, input, and basic window management.
- **SDL2_mixer**: For handling audio.
- **nlohmann/json**: For JSON data handling.
- **C++17 or Later**: The game is built with C++17/20 standards for modern functionality and performance.
