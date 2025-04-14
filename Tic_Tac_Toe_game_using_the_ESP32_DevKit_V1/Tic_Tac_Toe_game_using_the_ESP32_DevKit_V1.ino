#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define CELL_W 53
#define CELL_H 42
#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     2

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Buttons
#define BTN_UP     13
#define BTN_DOWN   12
#define BTN_LEFT   14
#define BTN_RIGHT  27
#define BTN_SELECT 26

int board[3][3];  // 0 = empty, 1 = X, 2 = O
int cursorRow = 0, cursorCol = 0;
int currentPlayer = 1;
bool gameOver = false;
bool vsAI = false;

void drawBoard();
void drawSymbol(int row, int col, int player);
void drawCursor();
void clearCursor();
bool checkWin(int player);
bool checkDraw();
void showGameOver(const char* msg);
void resetGame();
void aiMove();
void modeSelectScreen();

void setup() {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(true);
  
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);

  modeSelectScreen();
  resetGame();
}

void loop() {
  if (gameOver) return;

  delay(100); // debounce

  if (digitalRead(BTN_UP) == LOW && cursorRow > 0) {
    clearCursor();
    cursorRow--;
    drawCursor();
  }

  if (digitalRead(BTN_DOWN) == LOW && cursorRow < 2) {
    clearCursor();
    cursorRow++;
    drawCursor();
  }

  if (digitalRead(BTN_LEFT) == LOW && cursorCol > 0) {
    clearCursor();
    cursorCol--;
    drawCursor();
  }

  if (digitalRead(BTN_RIGHT) == LOW && cursorCol < 2) {
    clearCursor();
    cursorCol++;
    drawCursor();
  }

  if (digitalRead(BTN_SELECT) == LOW && board[cursorRow][cursorCol] == 0) {
    board[cursorRow][cursorCol] = currentPlayer;
    drawSymbol(cursorRow, cursorCol, currentPlayer);

    if (checkWin(currentPlayer)) {
      char msg[20];
      sprintf(msg, "Player %d Wins!", currentPlayer);
      showGameOver(msg);
      return;
    } else if (checkDraw()) {
      showGameOver("Draw!");
      return;
    }

    currentPlayer = 3 - currentPlayer;

    if (vsAI && currentPlayer == 2) {
      delay(300);
      aiMove();
      if (checkWin(currentPlayer)) {
        showGameOver("AI Wins!");
        return;
      } else if (checkDraw()) {
        showGameOver("Draw!");
        return;
      }
      currentPlayer = 1;
    }
    drawCursor();
  }
}

void drawBoard() {
  tft.fillScreen(ST77XX_BLACK);
  tft.drawLine(CELL_W, 0, CELL_W, 128, ST77XX_WHITE);
  tft.drawLine(2 * CELL_W, 0, 2 * CELL_W, 128, ST77XX_WHITE);
  tft.drawLine(0, CELL_H, 160, CELL_H, ST77XX_WHITE);
  tft.drawLine(0, 2 * CELL_H, 160, 2 * CELL_H, ST77XX_WHITE);
  drawCursor();
}

void drawSymbol(int row, int col, int player) {
  int x = col * CELL_W + CELL_W / 2;
  int y = row * CELL_H + CELL_H / 2;

  if (player == 1) {
    tft.drawLine(x - 10, y - 10, x + 10, y + 10, ST77XX_RED);
    tft.drawLine(x + 10, y - 10, x - 10, y + 10, ST77XX_RED);
  } else {
    tft.drawCircle(x, y, 12, ST77XX_BLUE);
  }
}

void drawCursor() {
  int x = cursorCol * CELL_W;
  int y = cursorRow * CELL_H;
  tft.drawRect(x + 1, y + 1, CELL_W - 2, CELL_H - 2, ST77XX_GREEN);
}

void clearCursor() {
  int x = cursorCol * CELL_W;
  int y = cursorRow * CELL_H;
  tft.drawRect(x + 1, y + 1, CELL_W - 2, CELL_H - 2, ST77XX_BLACK);
}

bool checkWin(int p) {
  for (int i = 0; i < 3; i++) {
    if ((board[i][0] == p && board[i][1] == p && board[i][2] == p) ||
        (board[0][i] == p && board[1][i] == p && board[2][i] == p))
      return true;
  }
  if ((board[0][0] == p && board[1][1] == p && board[2][2] == p) ||
      (board[0][2] == p && board[1][1] == p && board[2][0] == p))
    return true;
  return false;
}

bool checkDraw() {
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      if (board[i][j] == 0)
        return false;
  return true;
}

void showGameOver(const char* msg) {
  tft.fillRect(0, 70, 128, 30, ST77XX_BLACK);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(2);
  tft.setCursor(10, 75);
  tft.println(msg);
  gameOver = true;
  delay(3000);
  resetGame();
}

void resetGame() {
  memset(board, 0, sizeof(board));
  cursorRow = cursorCol = 0;
  currentPlayer = 1;
  gameOver = false;
  drawBoard();
}

void modeSelectScreen() {
  bool selected = false;
  int mode = 0; // 0 = 2P, 1 = AI

  while (!selected) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.setCursor(10, 40);
    tft.println("Select Mode:");

    tft.setCursor(10, 70);
    tft.setTextColor(mode == 0 ? ST77XX_GREEN : ST77XX_WHITE);
    tft.println("> 2 Player");

    tft.setCursor(10, 100);
    tft.setTextColor(mode == 1 ? ST77XX_GREEN : ST77XX_WHITE);
    tft.println("> Vs AI");

    delay(100);

    if (digitalRead(BTN_UP) == LOW || digitalRead(BTN_DOWN) == LOW)
      mode = 1 - mode;

    if (digitalRead(BTN_SELECT) == LOW) {
      selected = true;
      vsAI = (mode == 1);
    }
  }
}

void aiMove() {
  // 1. Win if possible
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      if (board[r][c] == 0) {
        board[r][c] = 2;
        if (checkWin(2)) {
          drawSymbol(r, c, 2);
          return;
        }
        board[r][c] = 0;
      }

  // 2. Block player win
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      if (board[r][c] == 0) {
        board[r][c] = 1;
        if (checkWin(1)) {
          board[r][c] = 2;
          drawSymbol(r, c, 2);
          return;
        }
        board[r][c] = 0;
      }

  // 3. Center
  if (board[1][1] == 0) {
    board[1][1] = 2;
    drawSymbol(1, 1, 2);
    return;
  }

  // 4. Corners / others
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      if (board[r][c] == 0) {
        board[r][c] = 2;
        drawSymbol(r, c, 2);
        return;
      }
}
