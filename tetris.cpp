#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

using namespace std;

const int WIDTH = 20;
const int HEIGHT = 25;
const string EMPTY = "  ";


const string COLORS[7] = {"🟦", "🟨", "🟪", "🟩", "🟥", "🟧", "🟫"};

const string GHOST_COLORS[7] = {"⬜", "⬜", "⬜", "⬜", "⬜", "⬜", "⬜"};
const int tetrominoes[7][4][4] = {
    
    {
        {1, 1, 1, 1}, 
        {0, 0, 0, 0}, 
        {0, 0, 0, 0}, 
        {0, 0, 0, 0}
    },
    
    {
        {1, 1, 0, 0}, 
        {1, 1, 0, 0}, 
        {0, 0, 0, 0}, 
        {0, 0, 0, 0}
    },
    
    {
        {0, 1, 0, 0}, 
        {1, 1, 1, 0}, 
        {0, 0, 0, 0}, 
        {0, 0, 0, 0}
    },
    
    {
        {0, 1, 1, 0}, 
        {1, 1, 0, 0}, 
        {0, 0, 0, 0}, 
        {0, 0, 0, 0}
    },
  
    {
        {1, 1, 0, 0}, 
        {0, 1, 1, 0}, 
        {0, 0, 0, 0}, 
        {0, 0, 0, 0}
    },

    {
        {1, 0, 0, 0}, 
        {1, 1, 1, 0}, 
        {0, 0, 0, 0}, 
        {0, 0, 0, 0}
    },
 
    {
        {0, 0, 1, 0}, 
        {1, 1, 1, 0}, 
        {0, 0, 0, 0}, 
        {0, 0, 0, 0}
    }
};

class Tetromino {
public:
    int shape[4][4];
    int x, y;
    int type;
    string color;
    string ghostColor;

    Tetromino(int pieceType) : x(WIDTH / 2 - 2), y(0), type(pieceType), color(COLORS[pieceType]), ghostColor(GHOST_COLORS[pieceType]) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                shape[i][j] = tetrominoes[pieceType][i][j];
    }

    void rotate() {
        int temp[4][4] = {{0}};
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                temp[j][3 - i] = shape[i][j];
        
        int oldX = x;
        int wallKickOffsets[5] = {0, -1, 1, -2, 2};
        for (int offset : wallKickOffsets) {
            x = oldX + offset;
            if (canMoveToNewPosition(temp)) {
                for (int i = 0; i < 4; i++)
                    for (int j = 0; j < 4; j++)
                        shape[i][j] = temp[i][j];
                return;
            }
        }
        x = oldX; 
    }

    bool canMoveToNewPosition(int newShape[4][4]) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (newShape[i][j]) {
                    int newX = x + j;
                    int newY = y + i;
                    if (newX < 0 || newX >= WIDTH || newY >= HEIGHT) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
};


class GameBoard {
public:
    string grid[HEIGHT][WIDTH];
    int score;
    int highScore;
    int level;
    int linesCleared;

    GameBoard() : score(0), highScore(0), level(1), linesCleared(0) {
        for (int i = 0; i < HEIGHT; i++)
            for (int j = 0; j < WIDTH; j++)
                grid[i][j] = EMPTY;
    }

    void draw(Tetromino &current, Tetromino &next) {
        cout << "\033[H\033[J"; 
        
        int ghostY = getGhostY(current);
        
        for (int i = 0; i < HEIGHT; i++) {
            cout << "|";
            for (int j = 0; j < WIDTH; j++) {
                string displayChar = grid[i][j];
                bool isGhostPosition = false;
                for (int ti = 0; ti < 4; ti++) {
                    for (int tj = 0; tj < 4; tj++) {
                        if (current.shape[ti][tj] && (i == ghostY + ti) && (j == current.x + tj)) {
                            isGhostPosition = true;
                        }
                    }
                }
                
                bool isActualPosition = false;
                for (int ti = 0; ti < 4; ti++) {
                    for (int tj = 0; tj < 4; tj++) {
                        if (current.shape[ti][tj] && (i == current.y + ti) && (j == current.x + tj)) {
                            displayChar = current.color;
                            isActualPosition = true;
                        }
                    }
                }
                
                if (isGhostPosition && !isActualPosition) {
                    displayChar = current.ghostColor;
                }
                
                cout << displayChar;
            }
            cout << "|\n";
        }
        cout << " ";
        for (int i = 0; i < WIDTH; i++) cout << "--";
        cout << "\n";
        cout << "Score: " << score << "  Level: " << level << "  Lines: " << linesCleared  << " High Score: " << highScore << "\n";
        cout << "[<-] Left  [->] Right  [W] Rotate  [Space] Hard Drop\n";
        cout << "[P] Pause  [R] Restart  [X] Exit\n";

        showNextPiece(next);
    }

    void showNextPiece(Tetromino &next) {
        cout << "\nNext Piece:\n";
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                cout << (next.shape[i][j] ? next.color : "  ");
            }
            cout << "\n";
        }
    }

    bool canMove(Tetromino &t, int dx, int dy) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                if (t.shape[i][j]) {
                    int newX = t.x + j + dx;
                    int newY = t.y + i + dy;
                    if (newX < 0 || newX >= WIDTH || newY >= HEIGHT || (newY >= 0 && grid[newY][newX] != EMPTY))
                        return false;
                }
        return true;
    }

    void placeTetromino(Tetromino &t) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                if (t.shape[i][j])
                    grid[t.y + i][t.x + j] = t.color;
        clearLines();
    }

    void clearLines() {
        int fullLines = 0;
        for (int i = 0; i < HEIGHT; i++) {
            bool full = true;
            for (int j = 0; j < WIDTH; j++) {
                if (grid[i][j] == EMPTY) {
                    full = false;
                    break;
                }
            }
            if (full) {
                fullLines++;
               
                for (int k = i; k > 0; k--)
                    for (int j = 0; j < WIDTH; j++)
                        grid[k][j] = grid[k - 1][j];
                
                for (int j = 0; j < WIDTH; j++)
                    grid[0][j] = EMPTY;
            }
        }
        score += (fullLines == 1 ? 100 : fullLines == 2 ? 300 : fullLines == 3 ? 500 : fullLines == 4 ? 800 : 0);
        linesCleared += fullLines;
        highScore = max(score, highScore);
        if (linesCleared >= level * 10)
            level++;
    }

    void clearBoard() {
        for (int i = 0; i < HEIGHT; i++)
            for (int j = 0; j < WIDTH; j++)
                grid[i][j] = EMPTY;
        score = 0;
        highScore = max(score, highScore);
        level = 1;
        linesCleared = 0;
    }

    int getGhostY(Tetromino &t) {
        int ghostY = t.y;
        while (canMove(t, 0, ghostY - t.y + 1)) {
            ghostY++;
        }
        return ghostY;
    }
};

class TetrisGame {
public:
    GameBoard board;
    Tetromino *currentPiece;
    Tetromino *nextPiece;
    bool gameOver;
    bool isPaused;

    TetrisGame() : gameOver(false), isPaused(false) {
        srand(time(0));
        currentPiece = new Tetromino(rand() % 7);
        nextPiece = new Tetromino(rand() % 7);
    }

    ~TetrisGame() {
        delete currentPiece;
        delete nextPiece;
    }

    void run() {
        while (!gameOver) {
            handleInput();
            if (!isPaused) {
                update();
            }
            board.draw(*currentPiece, *nextPiece);
            usleep(200000 / board.level);
        }
        cout << "Game Over!" << endl;
    }

private:
    char getInput() {
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        int bytesAvailable;
        ioctl(STDIN_FILENO, FIONREAD, &bytesAvailable);
        char key = 0;
        if (bytesAvailable > 0) {
            read(STDIN_FILENO, &key, 1);
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        return key;
    }

    void handleInput() {
        char key = getInput();
        if (key == 'p' || key == 'P') {
            isPaused = !isPaused;
            if (isPaused) {
                cout << "Game Paused. Press 'p' to resume...\n";
            }
        }
        if (isPaused)
            return;
            if (key == '\033') { 
                getInput();
                char arrowKey = getInput();
                
            if (arrowKey == 'D') { 
                if (board.canMove(*currentPiece, -1, 0))
                    currentPiece->x--;
            } else if (arrowKey == 'C') {
                if (board.canMove(*currentPiece, 1, 0))
                    currentPiece->x++;
            } else if (arrowKey == 'B') { 
                if (board.canMove(*currentPiece, 0, 1))
                    currentPiece->y++;
            }
        } else if (key == 'w' || key == 'W') {
            currentPiece->rotate();
        } else if (key == ' ') {
            hardDrop();
        } else if (key == 'r' || key == 'R') {
            restartGame();
        } else if (key == 'x' || key == 'X') {
            exit(0);
        }
    }

    void update() {
        if (!board.canMove(*currentPiece, 0, 1)) {
            board.placeTetromino(*currentPiece);
            delete currentPiece;
            currentPiece = nextPiece;
            nextPiece = new Tetromino(rand() % 7);
            if (!board.canMove(*currentPiece, 0, 0))
                gameOver = true;
        } else {
            currentPiece->y++;
        }
    }

    void hardDrop() {
        while (board.canMove(*currentPiece, 0, 1)) {
            currentPiece->y++;
            board.score++;
        }
        update();
    }

    void restartGame() {
        board.clearBoard();
        delete currentPiece;
        delete nextPiece;
        currentPiece = new Tetromino(rand() % 7);
        nextPiece = new Tetromino(rand() % 7);
        gameOver = false;
        isPaused = false;
    }
};

int main() {
    TetrisGame game;
    game.run();
    return 0;
}