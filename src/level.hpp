#include <string>

enum class Weather {
  CLEAR,
  WIND,
  COLD,
  RAIN,
  THUNDERSTORM,
};

bool santaPresent;
float waterLevel, snowLevel;
const int TILE_SIZE = 32, N_LEVELS = 7;
int tileSize, levelIndex, nextLevelIndex, cameraX, cameraY;
Weather weather;
std::string level;
Image* tileset;

Weather weathers[N_LEVELS] = {Weather::CLEAR, Weather::CLEAR, Weather::RAIN, Weather::CLEAR, Weather::CLEAR, Weather::CLEAR, Weather::CLEAR};
std::string levels[N_LEVELS] = {
    /**********************************************/
    "0                                            F"
    "0                                            F"
    "0                                            F"
    "0                                            F"
    "0                                            F"
    "0                                            F"
    "0                                            F"
    "0                           H                F"
    "0                            PC M            F"
    "0                           TTTTTT           F"
    "##############################################",
    /**********************************************/
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B N           P      R                       F"
    "B            TTT                             F"
    "#################___________##################"
    "#################___________##################"
    "#################___________##################"
    "#################_____L_____##################"
    "##############################################",
    /**********************************************/
    "B                                           1F"
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B N             mP                           F"
    "B              TTTT                  ll      F"
    "##############################################",
    /**********************************************/
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B                                            F"
    "B N                P                         F"
    "B                 TTT            @l          F"
    "##############################################",
    /**********************************************/
    "B                       0F"
    "B                       0F"
    "B                       0F"
    "B                       0F"
    "B                       0F"
    "B                       0F"
    "B                       0F"
    "B                       0F"
    "B N             P       0F"
    "B              TTT    c 0F"
    "##########################",
    /**********************************************/
    "B                                           0F"
    "B                                           0F"
    "B                                           0F"
    "B                                           0F"
    "B                                           0F"
    "B                                           0F"
    "B                                           0F"
    "B                                           0F"
    "B N     P               b                   0F"
    "B      TTT                         G        0F"
    "##############################################",
    /*********************************/
    "B                        |"
    "B                        |"
    "B                        |"
    "B                        |"
    "B                        |"
    "B                        |"
    "B                        |"
    "B                        |"
    "B         PC M           |"
    "B        TTTTTT          |"
    "##########################",
};

// clang-format off
int levelWidth, levelHeight;
int NPCTextX = -1, NPCTextY = -1;
const std::string NPCDialogs[N_LEVELS] = {
    "Impossible message!",
    "Someone told me that logs float well!",
    "I heard that if you leave the\nlens in the sun, it can start a fire!",
    "The Void told me that if you type\n\"wlman [function name]\" into the computer,\nyou can find out how this function works!",
    "Merry Cristmas",
    "Wind will help you!",
    "Impossible message!",
};
// clang-format on

void loadLevel() {
  level = levels[levelIndex];
  levelWidth = level.find_first_of("F|") + 1;
  levelHeight = level.length() / levelWidth;
  weather = weathers[levelIndex];
  waterLevel = snowLevel = levelHeight - 1;
  santaPresent = false;
  NPCTextX = -1;
  NPCTextY = -1;
}

char getTile(int x, int y) {
  if (y < 0 || y >= levelHeight) return '#';
  if (x < 0 || x >= levelWidth) return ' ';
  return level[y * levelWidth + x];
}

void setTile(int x, int y, char tile) {
  if (x < 0 || x >= levelWidth || y < 0 || y >= levelHeight) return;
  level[y * levelWidth + x] = tile;
}

void drawTile(Window* window, int x, int y, int sheetX, int sheetY, int w = 1, int h = 1) { drawImage(window, tileset, x * tileSize - cameraX, y * tileSize - cameraY, w * tileSize, h * tileSize, false, sheetX * TILE_SIZE, sheetY * TILE_SIZE, w * TILE_SIZE, h * TILE_SIZE); }
void drawQuater(Window* window, int x, int y, int sx, int sy, int qx, int qy) {  // Thanks Kofybreak (Youtube) for idea to split tiles into quaters
  int type = 0, dx = qx * 2 - 1, dy = qy * 2 - 1;
  char tile = getTile(x, y);
  if (getTile(x + dx, y) != tile) {
    type = (getTile(x, y + dy) != tile ? 0 : 3);
  } else if (getTile(x, y + dy) != tile) {
    type = 1;
  } else {
    type = (getTile(x + dx, y + dy) == tile ? 4 : 2);
  }
  if (type == 4) {
    drawImage(window, tileset, x * tileSize + qx * tileSize / 2 - cameraX, y * tileSize + qy * tileSize / 2 - cameraY, tileSize / 2, tileSize / 2, false, sx * TILE_SIZE + qx * TILE_SIZE / 2, sy * TILE_SIZE + TILE_SIZE + qy * TILE_SIZE / 2, TILE_SIZE / 2, TILE_SIZE / 2);
  } else {
    drawImage(window, tileset, x * tileSize + qx * tileSize / 2 - cameraX, y * tileSize + qy * tileSize / 2 - cameraY, tileSize / 2, tileSize / 2, qx, sx * TILE_SIZE + type * TILE_SIZE / 2, sy * TILE_SIZE + qy * TILE_SIZE / 2, TILE_SIZE / 2, TILE_SIZE / 2);
  }
}

void drawPatch(Window* window, int x, int y, int sx, int sy) {
  drawQuater(window, x, y, sx, sy, 0, 0);
  drawQuater(window, x, y, sx, sy, 1, 0);
  drawQuater(window, x, y, sx, sy, 0, 1);
  drawQuater(window, x, y, sx, sy, 1, 1);
}

void drawLevel(Window* window) {
  for (int layer = 0; layer < 3; layer++) {
    for (int x = cameraX / tileSize; x < cameraX / tileSize + windowWidth(window) / tileSize + 2; x++) {
      for (int y = cameraY / tileSize; y < cameraY / tileSize + windowHeight(window) / tileSize + 2; y++) {
        if (layer == 0) {
          if (getTile(x, y) == '_') {
            float wlevel = std::min(std::max((int)((y + 1 - waterLevel) * tileSize), 0), tileSize);
            float slevel = std::min(std::max((int)((y + 1 - snowLevel) * tileSize), 0), tileSize);
            fillRect(window, x * tileSize - cameraX - tileSize * 3 / TILE_SIZE, y * tileSize - cameraY + tileSize - wlevel, tileSize + tileSize * 6 / TILE_SIZE, wlevel, Color(75, 69, 110));
            fillRect(window, x * tileSize - cameraX - tileSize * 3 / TILE_SIZE, y * tileSize - cameraY + tileSize - slevel, tileSize + tileSize * 6 / TILE_SIZE, slevel, white);
          }
        } else if (layer == 1) {
          if (getTile(x, y) == '#') {
            drawPatch(window, x, y, 8, (weather == Weather::COLD || (snowLevel < levelHeight - 1) ? 2 : 0));
          } else if (getTile(x, y) == 'P') {
            drawTile(window, x, y, 0, 0);
          } else if (getTile(x, y) == 'C') {
            drawTile(window, x, y, 0, 2);
          } else if (getTile(x, y) == 'M') {
            drawTile(window, x, y, 0, 1);
          } else if (getTile(x, y) == 'T') {
            drawTile(window, x, y, 2 + (getTile(x - 1, y) == 'T') - (getTile(x + 1, y) == 'T'), 3);
          } else if (getTile(x, y) == '-') {
            drawTile(window, x, y, 4, 3);
          }
        } else if (layer == 2) {
          if (getTile(x, y) == 'H') {
            drawText(window, x * tileSize - cameraX, y * tileSize - cameraY, "Press E to interact!", black);
          } else if (getTile(x, y) == 'R') {
            drawText(window, x * tileSize - cameraX, y * tileSize - cameraY, "Press R to restart!", black);
          }
        }
      }
    }
  }
}

int sign(float n) { return n > 0 ? 1 : n < 0 ? -1 : 0; }
int random(int from, int to) { return rand() % (to - from) + from; }
float random(float from, float to) { return rand() % (int)((to - from) * 1000) / 1000.0 + from; }
