# Basic-Tetris
An extremely backbones basic CLI version of the game Tetris. Centered in the terminal, with a bordered playfield, smooth controls, and a nice ASCII art title.

Note that this was made for Linux mainly, I'll try get one up for Windows if I can (the release should work on Windows, perhaps not the code).

---

## Download & Run

### **Windows**
1. Go to the [Releases](https://github.com/Official-User-Shabab/Basic-Tetris/releases/tag/v1.0) page.
2. Download `tetris.exe`.
3. Double-click `tetris.exe` to play.

### **Linux**
1. Go to the [Releases](https://github.com/Official-User-Shabab/Basic-Tetris/releases/tag/v1.0) page.
2. Download `tetris` (Linux binary).
3. Open a terminal in the download location.
4. Run:
   ```bash
   chmod +x tetris
   ./tetris

## Compile From Source

If you prefer to build it yourself:

### **Linux**

```bash
g++ -o tetris tetris.cpp
./tetris
```

### **Windows** (via MinGW on Linux)

```bash
x86_64-w64-mingw32-g++ -static -o tetris.exe tetris.cpp
```

## How to Play

**Controls**:

| Key           | Action         |
| ------------- | -------------- |
| **A** / **←** | Move Left      |
| **D** / **→** | Move Right     |
| **W** / **↑** | Rotate Piece   |
| **S** / **↓** | Soft Drop      |
| **Space**     | Hard Drop      |
| **P**         | Pause / Resume |
| **Q**         | Quit Game      |


## Terminal Tips

* For the best experience, **maximize your terminal** before starting.
* Works best on terminals that support **ANSI escape codes** (Linux terminal, Windows Terminal, Git Bash, etc.).
