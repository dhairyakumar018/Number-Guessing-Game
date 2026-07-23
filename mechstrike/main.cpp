/*
=====================================================================
    MechStrike: A Console-Based Tactical Battle Simulator
---------------------------------------------------------------------
    A SIMULTANEOUS-TURN tactical battle game written in pure C++.
    - No external libraries, no GUI. Console only.
    - Standard libraries used: iostream, fstream, cstdlib, ctime,
      string, cmath, limits.

    Each round, you and the enemy choose actions at the SAME time.
    Both actions are then resolved together, so you must PREDICT
    what the enemy will do (dodge its shot, cut off its retreat,
    or fire at the cell it is about to occupy).

    Controls:
        W / A / S / D  -> Move Up / Left / Down / Right
        F              -> Fire (hits if enemy is in range AFTER moves)
        Q              -> Quit current battle

    Compile:  g++ main.cpp -o mechstrike
    Run:      ./mechstrike      (Linux)
              mechstrike.exe    (Windows)
=====================================================================
*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cmath>
#include <limits>

using namespace std;

// ------------------------- Constants -------------------------------
const int GRID_ROWS      = 8;     // battlefield height
const int GRID_COLS      = 8;     // battlefield width
const int OBSTACLE_COUNT = 8;     // number of obstacles placed
const char CELL_EMPTY    = '.';
const char CELL_OBSTACLE = '#';
const char CELL_PLAYER   = 'P';
const char CELL_ENEMY    = 'E';
const string STATS_FILE  = "stats.txt";

// ========================= Robot Class =============================
// Represents a combat robot (used for BOTH player and enemy).
// Demonstrates: encapsulation, constructors, member functions.
class Robot {
private:
    string name;
    int health;
    int maxHealth;
    int attackPower;
    int attackRange;   // Manhattan-distance attack range
    int row, col;      // current position on the grid

public:
    Robot(string n, int hp, int atk, int range)
        : name(n), health(hp), maxHealth(hp),
          attackPower(atk), attackRange(range), row(0), col(0) {}

    // ---- Getters ----
    string getName()  const { return name; }
    int getHealth()   const { return health; }
    int getMaxHealth()const { return maxHealth; }
    int getAttack()   const { return attackPower; }
    int getRange()    const { return attackRange; }
    int getRow()      const { return row; }
    int getCol()      const { return col; }

    // ---- Setters / actions ----
    void setPosition(int r, int c) { row = r; col = c; }

    void takeDamage(int dmg) {
        health -= dmg;
        if (health < 0) health = 0;
    }

    bool isAlive() const { return health > 0; }

    // Manhattan distance to another robot
    int distanceTo(const Robot &other) const {
        return abs(row - other.getRow()) + abs(col - other.getCol());
    }

    bool inRangeOf(const Robot &other) const {
        return distanceTo(other) <= attackRange;
    }
};

// ======================= Battlefield Class =========================
// Manages the 2D grid, obstacles, rendering and position validation.
// Demonstrates: 2D arrays, composition, const-correctness.
class Battlefield {
private:
    char grid[GRID_ROWS][GRID_COLS];

public:
    Battlefield() { clear(); }

    void clear() {
        for (int r = 0; r < GRID_ROWS; r++)
            for (int c = 0; c < GRID_COLS; c++)
                grid[r][c] = CELL_EMPTY;
    }

    bool isInside(int r, int c) const {
        return r >= 0 && r < GRID_ROWS && c >= 0 && c < GRID_COLS;
    }

    bool isWalkable(int r, int c) const {
        return isInside(r, c) && grid[r][c] == CELL_EMPTY;
    }

    void setCell(int r, int c, char ch) {
        if (isInside(r, c)) grid[r][c] = ch;
    }

    char getCell(int r, int c) const {
        return isInside(r, c) ? grid[r][c] : CELL_OBSTACLE;
    }

    // Place obstacles at random empty cells
    void placeObstacles(int count) {
        int placed = 0;
        while (placed < count) {
            int r = rand() % GRID_ROWS;
            int c = rand() % GRID_COLS;
            if (grid[r][c] == CELL_EMPTY) {
                grid[r][c] = CELL_OBSTACLE;
                placed++;
            }
        }
    }

    // Draw the battlefield along with both robots' status
    void render(const Robot &player, const Robot &enemy) const {
        cout << "\n";
        for (int r = 0; r < GRID_ROWS; r++) {
            cout << "   ";
            for (int c = 0; c < GRID_COLS; c++) {
                cout << grid[r][c] << ' ';
            }
            cout << "\n";
        }
        cout << "\n   " << player.getName() << " HP: "
             << player.getHealth() << "/" << player.getMaxHealth()
             << "   |   " << enemy.getName() << " HP: "
             << enemy.getHealth() << "/" << enemy.getMaxHealth() << "\n";
    }
};

// ===================== Statistics Module ===========================
// Handles saving and loading of game results using file handling.
// File format (one line per game):  RESULT,MOVES
class Statistics {
public:
    static void saveResult(const string &result, int moves) {
        ofstream out(STATS_FILE.c_str(), ios::app);
        if (out.is_open()) {
            out << result << "," << moves << "\n";
            out.close();
            cout << "   Result saved to statistics.\n";
        } else {
            cout << "   Warning: could not save statistics.\n";
        }
    }

    static void display() {
        ifstream in(STATS_FILE.c_str());
        int total = 0, wins = 0, losses = 0, draws = 0;
        long totalMoves = 0;

        if (in.is_open()) {
            string line;
            while (getline(in, line)) {
                if (line.empty()) continue;
                size_t comma = line.find(',');
                if (comma == string::npos) continue;
                string result = line.substr(0, comma);
                int moves = atoi(line.substr(comma + 1).c_str());
                total++;
                totalMoves += moves;
                if (result == "WIN") wins++;
                else if (result == "LOSS") losses++;
                else if (result == "DRAW") draws++;
            }
            in.close();
        }

        cout << "\n   ========= GAME STATISTICS =========\n";
        if (total == 0) {
            cout << "   No games played yet. Go fight!\n";
        } else {
            cout << "   Total Games : " << total << "\n";
            cout << "   Wins        : " << wins << "\n";
            cout << "   Losses      : " << losses << "\n";
            cout << "   Draws       : " << draws << "\n";
            cout << "   Win Rate    : " << (wins * 100 / total) << "%\n";
            cout << "   Avg. Rounds : " << (totalMoves / total) << "\n";
        }
        cout << "   ===================================\n";
    }
};

// ====================== Action Structure ===========================
// One declared action for one robot during a round. Both robots
// declare an Action first; the Game then resolves them TOGETHER.
struct Action {
    enum Type { HOLD, MOVE, ATTACK, QUIT };
    Type type;
    int destR, destC;      // destination cell (== current cell unless MOVE)
    string label;          // human-readable description for the reveal

    Action() : type(HOLD), destR(0), destC(0), label("hold position") {}
};

// ========================= AI Module ===============================
// Rule-based decision making for the enemy robot:
//   Rule 1: If player is in attack range      -> ATTACK
//   Rule 2: If own health is low (< 30%)      -> RETREAT (move away)
//   Rule 3: Otherwise                          -> CHASE  (move closer)
//
// IMPORTANT: the AI only DECIDES here. It decides using the state at
// the START of the round, so it cannot see what you just typed --
// exactly like you cannot see its choice. The Game resolves both
// declared actions simultaneously.
class EnemyAI {
public:
    static string directionName(int dr, int dc) {
        if (dr == -1) return "up";
        if (dr ==  1) return "down";
        if (dc == -1) return "left";
        if (dc ==  1) return "right";
        return "nowhere";
    }

    static Action decide(const Robot &enemy, const Robot &player,
                         const Battlefield &field) {
        Action act;
        act.destR = enemy.getRow();
        act.destC = enemy.getCol();

        // Rule 1: player currently in range -> fire at their position
        if (enemy.inRangeOf(player)) {
            act.type  = Action::ATTACK;
            act.label = "FIRE at your position";
            return act;
        }

        // Rule 2 / Rule 3: decide direction (retreat or chase)
        bool retreat = (enemy.getHealth() * 100 / enemy.getMaxHealth()) < 30;

        int bestR = enemy.getRow();
        int bestC = enemy.getCol();
        int bestDist = player.distanceTo(enemy);

        // Try the four possible moves and pick the best one
        int dr[] = { -1, 1, 0, 0 };
        int dc[] = { 0, 0, -1, 1 };

        for (int i = 0; i < 4; i++) {
            int nr = enemy.getRow() + dr[i];
            int nc = enemy.getCol() + dc[i];
            if (!field.isWalkable(nr, nc)) continue;

            int dist = abs(nr - player.getRow()) + abs(nc - player.getCol());

            if (retreat) {
                if (dist > bestDist) { bestDist = dist; bestR = nr; bestC = nc; }
            } else {
                if (dist < bestDist) { bestDist = dist; bestR = nr; bestC = nc; }
            }
        }

        if (bestR != enemy.getRow() || bestC != enemy.getCol()) {
            act.type  = Action::MOVE;
            act.destR = bestR;
            act.destC = bestC;
            act.label = (retreat ? "RETREAT " : "ADVANCE ")
                        + directionName(bestR - enemy.getRow(),
                                        bestC - enemy.getCol());
        } else {
            act.type  = Action::HOLD;
            act.label = "HOLD position";
        }
        return act;
    }
};

// ===================== Game Controller =============================
// Runs the main game loop. Each round BOTH robots declare an action,
// then the round is resolved simultaneously:
//   1) Movement phase : both moves happen at once.
//        - Same destination cell, or swapping cells -> CLASH,
//          both robots bounce back to where they were.
//        - Stepping into the cell the other robot just LEFT is legal.
//   2) Attack phase   : shots resolve on POST-move positions.
//        - Dodge out of range and the shot misses.
//        - Fire where the enemy is ABOUT to be and it hits.
//        - Both robots can destroy each other the same round (DRAW).
class Game {
private:
    Battlefield field;
    Robot player;
    Robot enemy;
    int moveCount;

    void placeRobots() {
        // Player at top-left area, enemy at bottom-right area
        player.setPosition(1, 1);
        enemy.setPosition(GRID_ROWS - 2, GRID_COLS - 2);
        field.setCell(1, 1, CELL_EMPTY);                       // ensure free
        field.setCell(GRID_ROWS - 2, GRID_COLS - 2, CELL_EMPTY);
        field.setCell(player.getRow(), player.getCol(), CELL_PLAYER);
        field.setCell(enemy.getRow(), enemy.getCol(), CELL_ENEMY);
    }

    // Validate the player's command and build their DECLARED action.
    // Nothing is executed here -- the round resolves later, together
    // with the enemy's declaration.
    // Returns true if valid, false if input invalid (round not used).
    bool buildPlayerAction(char cmd, Action &act) {
        act.destR = player.getRow();
        act.destC = player.getCol();

        int dr = 0, dc = 0;

        switch (cmd) {
            case 'w': case 'W': dr = -1; act.label = "MOVE up";    break;
            case 's': case 'S': dr =  1; act.label = "MOVE down";  break;
            case 'a': case 'A': dc = -1; act.label = "MOVE left";  break;
            case 'd': case 'D': dc =  1; act.label = "MOVE right"; break;
            case 'f': case 'F':
                // Firing is ALWAYS allowed -- even if the enemy is out
                // of range right now, it may walk INTO range this round.
                act.type  = Action::ATTACK;
                act.label = "FIRE";
                return true;
            case 'q': case 'Q':
                act.type = Action::QUIT;
                return true;
            default:
                cout << "   >> Invalid command! Use W, A, S, D, F, or Q.\n";
                return false;
        }

        // Movement validation (against static terrain only)
        int nr = player.getRow() + dr;
        int nc = player.getCol() + dc;

        if (!field.isInside(nr, nc)) {
            cout << "   >> You cannot move outside the battlefield!\n";
            return false;
        }
        if (field.getCell(nr, nc) == CELL_OBSTACLE) {
            cout << "   >> An obstacle blocks your path!\n";
            return false;
        }
        // NOTE: moving toward the enemy's CURRENT cell is allowed.
        // If it moves away this round you slip in behind it;
        // if it stays, you clash and bounce back.

        act.type  = Action::MOVE;
        act.destR = nr;
        act.destC = nc;
        return true;
    }

    // Resolve one simultaneous round from two declared actions.
    void resolveRound(const Action &pAct, const Action &eAct) {
        // ---- Reveal phase: both choices become visible at once ----
        cout << "\n   You declared   : " << pAct.label << "\n";
        cout << "   Enemy declared : " << eAct.label << "\n";

        int pOldR = player.getRow(), pOldC = player.getCol();
        int eOldR = enemy.getRow(),  eOldC = enemy.getCol();

        int pNewR = pAct.destR, pNewC = pAct.destC;
        int eNewR = eAct.destR, eNewC = eAct.destC;

        // ---- Movement phase (simultaneous) ----
        bool sameCell = (pNewR == eNewR && pNewC == eNewC);
        bool swapped  = (pNewR == eOldR && pNewC == eOldC &&
                         eNewR == pOldR && eNewC == pOldC);

        if (sameCell || swapped) {
            cout << "   >> CLASH! Both robots collided and bounced back!\n";
            pNewR = pOldR; pNewC = pOldC;
            eNewR = eOldR; eNewC = eOldC;
        }

        field.setCell(pOldR, pOldC, CELL_EMPTY);
        field.setCell(eOldR, eOldC, CELL_EMPTY);
        player.setPosition(pNewR, pNewC);
        enemy.setPosition(eNewR, eNewC);
        field.setCell(pNewR, pNewC, CELL_PLAYER);
        field.setCell(eNewR, eNewC, CELL_ENEMY);

        // ---- Attack phase (simultaneous, on POST-move positions) ----
        // Damage is applied AFTER both hit checks, so the two shots
        // land at the same instant: both robots can die in one round.
        bool playerHits = (pAct.type == Action::ATTACK) && player.inRangeOf(enemy);
        bool enemyHits  = (eAct.type == Action::ATTACK) && enemy.inRangeOf(player);

        if (pAct.type == Action::ATTACK) {
            if (playerHits)
                cout << "   >> Your shot HITS " << enemy.getName() << " for "
                     << player.getAttack() << " damage!\n";
            else
                cout << "   >> Your shot MISSES -- " << enemy.getName()
                     << " is out of range! (Range: " << player.getRange()
                     << ", Distance: " << player.distanceTo(enemy) << ")\n";
        }
        if (eAct.type == Action::ATTACK) {
            if (enemyHits)
                cout << "   >> " << enemy.getName() << "'s shot HITS you for "
                     << enemy.getAttack() << " damage!\n";
            else
                cout << "   >> " << enemy.getName()
                     << "'s shot MISSES -- you dodged out of range!\n";
        }

        if (playerHits) enemy.takeDamage(player.getAttack());
        if (enemyHits)  player.takeDamage(enemy.getAttack());
    }

public:
    Game()
        : player("Player", 100, 20, 2),
          enemy("Enemy", 100, 15, 2),
          moveCount(0) {}

    void run() {
        field.clear();
        field.placeObstacles(OBSTACLE_COUNT);
        placeRobots();

        cout << "\n   ====== BATTLE START! Defeat the enemy robot! ======\n";
        cout << "   (Both robots act at the SAME time -- predict its move!)\n";

        bool quitGame = false;

        while (player.isAlive() && enemy.isAlive() && !quitGame) {
            field.render(player, enemy);
            cout << "\n   Declare your action! (W/A/S/D = Move, F = Fire, Q = Quit): ";

            char cmd;
            cin >> cmd;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            Action pAct;
            if (!buildPlayerAction(cmd, pAct)) {
                continue;   // invalid action -> ask again, no round used
            }
            if (pAct.type == Action::QUIT) {
                quitGame = true;
                break;
            }

            // Enemy decides from the SAME start-of-round state,
            // with no knowledge of what you declared.
            Action eAct = EnemyAI::decide(enemy, player, field);

            resolveRound(pAct, eAct);
            moveCount++;
        }

        // ------- Battle result -------
        field.render(player, enemy);

        if (quitGame) {
            cout << "\n   You fled the battle! Counted as a LOSS.\n";
            Statistics::saveResult("LOSS", moveCount);
        } else if (!player.isAlive() && !enemy.isAlive()) {
            cout << "\n   *** MUTUAL DESTRUCTION! Both robots fired their\n";
            cout << "       final shots at the same instant. It's a DRAW! ***\n";
            cout << "   Total rounds: " << moveCount << "\n";
            Statistics::saveResult("DRAW", moveCount);
        } else if (!enemy.isAlive()) {
            cout << "\n   *** You defeated the enemy robot! YOU WIN! ***\n";
            cout << "   Total rounds: " << moveCount << "\n";
            Statistics::saveResult("WIN", moveCount);
        } else {
            cout << "\n   *** Your robot was destroyed! YOU LOSE! ***\n";
            cout << "   Total rounds: " << moveCount << "\n";
            Statistics::saveResult("LOSS", moveCount);
        }
    }
};

// ======================= Menu Functions ============================
void showInstructions() {
    cout << "\n   ============ INSTRUCTIONS ============\n";
    cout << "   You control the robot 'P' on the grid.\n";
    cout << "   Defeat the enemy robot 'E' before it destroys you!\n\n";
    cout << "   Symbols:\n";
    cout << "     P  - Your robot\n";
    cout << "     E  - Enemy robot\n";
    cout << "     #  - Obstacle (blocks movement)\n";
    cout << "     .  - Empty cell\n\n";
    cout << "   Controls (then press Enter):\n";
    cout << "     W - Move up        A - Move left\n";
    cout << "     S - Move down      D - Move right\n";
    cout << "     F - Fire           Q - Quit the battle (counts as a loss)\n\n";
    cout << "   SIMULTANEOUS ROUNDS:\n";
    cout << "     - You and the enemy declare actions at the SAME time,\n";
    cout << "       then both actions happen together.\n";
    cout << "     - Moving to the same cell (or swapping cells) = CLASH:\n";
    cout << "       both robots bounce back to where they were.\n";
    cout << "     - Shots resolve AFTER movement: dodge out of range to\n";
    cout << "       avoid damage, or fire where the enemy is ABOUT to be!\n";
    cout << "     - Both robots can be destroyed in the same round (DRAW).\n\n";
    cout << "   Tips:\n";
    cout << "     - Attack range is Manhattan distance (steps), not diagonal.\n";
    cout << "     - Use obstacles to control the enemy's approach.\n";
    cout << "     - The enemy fires when you START a round in its range --\n";
    cout << "       predict it and step out before the shot lands!\n";
    cout << "     - The enemy retreats when its health is low - chase it!\n";
    cout << "   ======================================\n";
}

int readMenuChoice() {
    int choice;
    cout << "\n   Enter your choice: ";
    while (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "   Invalid input! Enter a number (1-4): ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return choice;
}

// =========================== main ==================================
int main() {
    srand(static_cast<unsigned int>(time(0)));   // seed random generator

    bool running = true;
    while (running) {
        cout << "\n   =================================\n";
        cout << "      MECHSTRIKE - BATTLE SIM\n";
        cout << "   =================================\n";
        cout << "   1. New Game\n";
        cout << "   2. View Statistics\n";
        cout << "   3. Instructions\n";
        cout << "   4. Exit\n";

        int choice = readMenuChoice();

        switch (choice) {
            case 1: {
                Game game;
                game.run();
                break;
            }
            case 2:
                Statistics::display();
                break;
            case 3:
                showInstructions();
                break;
            case 4:
                cout << "\n   Thanks for playing MechStrike. Goodbye!\n\n";
                running = false;
                break;
            default:
                cout << "   Invalid choice! Please enter 1-4.\n";
        }
    }
    return 0;
}
