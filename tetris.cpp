#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <cstring>
using namespace std;

const int width = 14;
const int height = 24;

int field[height][width] = {0};
int score = 0;

bool gameOver = false;
bool paused = false;


const char* tetromino[7] = {
    "..X...X...X...X.", // I
    "..X..XX...X.....", // T
    ".....XX..XX.....", // O
    "..X..XX..X......", // S
    ".X...XX...X.....", // Z
    ".X...X...XX.....", // J
    "..X...X..XX....."  // L
};

int currentPiece, currentRotation, currentX, currentY;

int getTetrominoBlock(int tet, int rot, int x, int y) {
    int pi = 0;
    switch (rot % 4) {
        case 0: pi = y * 4 + x; break;
        case 1: pi = 12 + y - (x * 4); break;
        case 2: pi = 15 - (y * 4) - x; break;
        case 3: pi = 3 - y + (x * 4); break;
    }
    return tetromino[tet][pi] == 'X' ? 1 : 0;
}

bool doesPieceFit(int tet, int rot, int posX, int posY) {
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            if (getTetrominoBlock(tet, rot, px, py)) {
                int fx = posX + px;
                int fy = posY + py;
                if (fx < 0 || fx >= width || fy < 0 || fy >= height) return false;
                if (field[fy][fx]) return false;
            }
        }
    }
    return true;
}



void clearScreen() {
    cout << "\033[2J\033[H";  //clears screen and move cursor to top-left
}

void getTerminalSize(int &rows, int &cols) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    rows = w.ws_row;
    cols = w.ws_col;
}

void enableAltScreen() {
    cout << "\033[?1049h";
}

void disableAltScreen() {
    cout << "\033[?1049l";
}

void draw() {
    int rows, cols;
    getTerminalSize(rows, cols);

    const int cellWidth = 2;
    const int fieldWidthChars = width * cellWidth + 2; // borders
    const int fieldHeightChars = height + 2;

    int startRow = (rows - fieldHeightChars) / 2;
    int startCol = (cols - fieldWidthChars) / 2;

    clearScreen();

    // print em empty lines above the field (push everything down so the title isn't cramped)
    for (int i = 0; i < startRow - 6; i++) cout << "\n";

    //TETRIS title lines
    const char* titleArt[] = {
        " _____  _____ _____  ____  _  ____ ",
        "/__ __\\/  __//__ __\\/  __\\/ \\/ ___\\",
        "  / \\  |  \\    / \\  |  \\/|| ||    \\",
        "  | |  |  /_   | |  |    /| |\\___ |",
        "  \\_/  \\____\\  \\_/  \\_/\\_\\\\_/\\____/",
        "                                   "
    };
    //  each line centered
    for (auto &line : titleArt) {
        int padding = startCol + (fieldWidthChars - (int)strlen(line)) / 2;
        cout << string(padding, ' ') << line << "\n";
    }

    cout << "\n\n"; // add two blank lines after the title before the field

    //draw the top border and rest as before...
    cout << string(startCol, ' ') << "+";
    for (int i = 0; i < width * cellWidth; i++) cout << "-";
    cout << "+  Score: " << score << "\n";

    for (int y = 0; y < height; y++) {
        cout << string(startCol, ' ') << "|";

        for (int x = 0; x < width; x++) {
            bool isCurrentPieceBlock = false;
            for (int px = 0; px < 4 && !isCurrentPieceBlock; px++) {
                for (int py = 0; py < 4; py++) {
                    if (getTetrominoBlock(currentPiece, currentRotation, px, py)) {
                        if (currentX + px == x && currentY + py == y) {
                            isCurrentPieceBlock = true;
                            break;
                        }
                    }
                }
            }

            if (isCurrentPieceBlock)
                cout << "██";
            else if (field[y][x])
                cout << "▓▓";
            else
                cout << "  ";
        }
        cout << "|";

        if (paused) {
            if (y == height/2 - 3) cout << "    *** PAUSED ***";
            if (y == height/2 - 1) cout << "    Controls:";
            if (y == height/2 + 0) cout << "    P - Resume";
            if (y == height/2 + 1) cout << "    Q - Quit";
        } else {
            if (y == height/2 - 5) cout << "   Controls:";
            if (y == height/2 - 4) cout << "   A/Left - Move Left";
            if (y == height/2 - 3) cout << "   D/Right - Move Right";
            if (y == height/2 - 2) cout << "   W/Up - Rotate";
            if (y == height/2 - 1) cout << "   S/Down - Soft Drop";
            if (y == height/2 + 0) cout << "   Space - Hard Drop";
            if (y == height/2 + 1) cout << "   P - Pause";
            if (y == height/2 + 2) cout << "   Q - Quit";
        }

        cout << "\n";
    }

    cout << string(startCol, ' ') << "+";
    for (int i = 0; i < width * cellWidth; i++) cout << "-";
    cout << "+\n";
}
int lockPiece() {
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            if (getTetrominoBlock(currentPiece, currentRotation, px, py)) {
                field[currentY + py][currentX + px] = 1;
            }
        }
    }

    int linesCleared = 0;
    for (int py = 0; py < 4; py++) {
        int line = currentY + py;
        if (line >= height) continue;
        bool fullLine = true;
        for (int x = 0; x < width; x++) {
            if (field[line][x] == 0) {
                fullLine = false;
                break;
            }
        }
        if (fullLine) {
            for (int y = line; y > 0; y--) {
                for (int x = 0; x < width; x++) {
                    field[y][x] = field[y - 1][x];
                }
            }
            for (int x = 0; x < width; x++) field[0][x] = 0;
            linesCleared++;
        }
    }
    return linesCleared;
}


void setTerminalRawMode(termios& original) {
    termios raw;
    tcgetattr(STDIN_FILENO, &original);
    raw = original;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void restoreTerminal(termios& original) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}
int kbhit() {
    timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}
char getch() {
    char c = 0;
    if (read(STDIN_FILENO, &c, 1) < 0) return 0;
    return c;
}


// helper to read single char or arrow keys mapped to w,a,s,d
char getInputKey() {
    char c = getch();
    if (c == '\033') { //wscape sequence for arrows
        if (kbhit() && getch() == '[') {
            if (kbhit()) {
                char arrow = getch();
                switch (arrow) {
                    case 'A': return 'w'; // Up = rotate
                    case 'B': return 's'; // Down= soft drop
                    case 'C': return 'd'; // Right = right
                    case 'D': return 'a'; // Left arrow = left
                }
            }
        }
        return 0;
    } else {
        return c;
    }
}

void resetDropTimer(int &speedCounter) {
    speedCounter = 0;
}



// main function fr

int main() {
    srand(time(0));
    termios originalTermios;
    setTerminalRawMode(originalTermios);
    enableAltScreen();

    currentPiece = rand() % 7;
    currentRotation = 0;
    currentX = width / 2 - 2;
    currentY = 0;

    int speed = 20;
    int speedCounter = 0;
    int lines = 0;

    while (!gameOver) {
        usleep(50000);
        speedCounter++;

        if (!paused) {
            bool forceDown = (speedCounter >= speed);

            while (kbhit()) {
                char key = getInputKey();
                switch (key) {
                    case 'a': case 'A':
                        if (doesPieceFit(currentPiece, currentRotation, currentX - 1, currentY))
                            currentX--;
                        break;
                    case 'd': case 'D':
                        if (doesPieceFit(currentPiece, currentRotation, currentX + 1, currentY))
                            currentX++;
                        break;
                    case 'w': case 'W':
                        if (doesPieceFit(currentPiece, currentRotation + 1, currentX, currentY))
                            currentRotation = (currentRotation + 1) % 4;
                        break;
                    case 's': case 'S':
                        if (doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1))
                            currentY++;
                        break;
                    case ' ':
                        while (doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1))
                            currentY++;
                        forceDown = true;
                        break;
                    case 'p': case 'P':
                        paused = true;
                        break;
                    case 'q': case 'Q':
                        gameOver = true;
                        break;
                }
            }

            if (forceDown) {
                if (doesPieceFit(currentPiece, currentRotation, currentX, currentY + 1))
                    currentY++;
                else {
                    int cleared = lockPiece();
                    if (cleared > 0) {
                        lines += cleared;
                        score += cleared * 100;
                        if (lines % 5 == 0 && speed > 5)
                            speed--;
                    }
                    currentPiece = rand() % 7;
                    currentRotation = 0;
                    currentX = width / 2 - 2;
                    currentY = 0;

                    if (!doesPieceFit(currentPiece, currentRotation, currentX, currentY))
                        gameOver = true;
                }
                speedCounter = 0;
            }
        } else {
            // paused: only process keys for pause/unpause/quit
            while (kbhit()) {
                char key = getInputKey();
                if (key == 'p' || key == 'P') {
                    paused = false;
                    resetDropTimer(speedCounter);
                }
                else if (key == 'q' || key == 'Q') gameOver = true;
            }
        }

        draw();
    }


    disableAltScreen();
    restoreTerminal(originalTermios);

    cout << "\nGame Over! Your score: " << score << "\nPress ENTER to exit...";
    cin.ignore();

    return 0;
}
