#include <algorithm>
#include <vector>

int playerW, playerH;
float playerSpeed = 0.015;  // 0.15
Image* playerTex;
Audio* deathSound;
Audio* shhhhSound;
Audio* hoHoHoSound;
Audio* openBookSound;
Audio* fixedLeaksSound;

std::vector<Image*> inventory;

struct Player {
  float x, y, deltaX, deltaY, visualAcceleration;
  int frame, jumps, nFrames = 2;
  int itemX, itemY, itemW, itemH;
  bool flip;

  void update(Window* window) {
    float prevDX = deltaX;
    deltaX = std::min(std::max(deltaX + (isKeyHeld(Key::D) - isKeyHeld(Key::A)) * tileSize * playerSpeed, tileSize * -0.15f), tileSize * 0.15f);
    deltaX *= 0.95;
    if (isKeyPressed(Key::Space) && jumps > 0) {
      jumps--;
      deltaY = tileSize * -0.4;
    }
    if (isKeyPressed(Key::R)) {
      nextLevelIndex = levelIndex;
    }

    deltaY += tileSize * 2 * deltaTime();
    resolveCollisions(0, 1);
    x += deltaX * deltaTime() * tileSize;
    resolveCollisions(sign(deltaX), 0);
    y += deltaY * deltaTime() * tileSize;
    resolveCollisions(0, sign(deltaY));
    flip = deltaX < 0 || deltaX == 0 && flip;
    if (abs(deltaX) < 1) {
      frame = 0;
    } else {
      frame = nFrames - 1;
    }
    cameraX = std::max(0, std::min(levelWidth * tileSize - windowWidth(window), (int)(x + playerW / 2 - windowWidth(window) / 2)));
    cameraY = std::max(0, std::min(levelHeight * tileSize - windowHeight(window), (int)(y + playerH / 2 - windowHeight(window) / 2)));
    visualAcceleration += (prevDX - deltaX - visualAcceleration) * deltaTime() * (abs(visualAcceleration) > abs(prevDX - deltaX) ? 8 : 1);
  }

  void resolveCollisions(int dx, int dy) {
    float newX = x, newY = y;
    for (int x1 = x / tileSize; x1 < (x + playerW) / tileSize; x1++) {
      for (int y1 = y / tileSize; y1 < (y + playerH) / tileSize; y1++) {
        if (getTile(x1, y1) == '#' || getTile(x1, y1) == '0' || getTile(x1, y1) == '|') {
          newX = dx > 0 ? std::min(newX, (float)x1 * tileSize - playerW) : dx < 0 ? std::max(newX, (float)x1 * tileSize + tileSize) : newX;
          newY = dy > 0 ? std::min(newY, (float)y1 * tileSize - playerH) : dy < 0 ? std::max(newY, (float)y1 * tileSize + tileSize) : newY;
          if (dy > 0) {
            jumps = 2;
          }
        } else if (getTile(x1, y1) == 'P') {
          if (isKeyPressed(Key::E)) {
            openPC();
          }
        } else if (getTile(x1, y1) == 'M') {
          if (isKeyPressed(Key::E)) {
            inManual = true;
            playAudio(openBookSound);
          }
        } else if (getTile(x1, y1) == 'F') {
          nextLevelIndex = levelIndex + 1;
        } else if (getTile(x1, y1) == 'B') {
          nextLevelIndex = levelIndex - 1;
        } else if (getTile(x1, y1) == '_') {
          jumps = 0;
          float level = std::min(std::max((int)((y1 + 1 - snowLevel) * tileSize), 0), tileSize);
          if (level > 0) {
            if (dy > 0) jumps = 1;
            newY = dy > 0 ? std::min(newY, (float)y1 * tileSize + tileSize - level - playerH) : newY;
          }
        }
      }
    }
    int i = 0;
    bool NPCTextPresent = (NPCTextX != -1 || NPCTextY != -1);
    for (Object& object : objects) {
      if (x > object.x - playerW && x < object.x + object.w && y > object.y - playerH && y < object.y + object.h) {
        if (object.image == logTex) {
          newX = dx > 0 ? std::min(newX, object.x - playerW) : dx < 0 ? std::max(newX, object.x + object.w) : newX;
          newY = dy > 0 ? std::min(newY, object.y - playerH) : dy < 0 ? std::max(newY, object.y + object.h) : newY;
          if (dy > 0) {
            jumps = 2;
          }
        } else if (object.image == NPCTex) {
          if (isKeyPressed(Key::E)) {
            if (NPCTextPresent) {
              NPCTextX = -1;
              NPCTextY = -1;
            } else {
              playAudio(shhhhSound);
              if (!object.flip) {
                NPCTextX = object.x;
                NPCTextY = object.y;
              } else {
                NPCTextX = objects[i - 1].x;
                NPCTextY = objects[i - 1].y;
              }
            }
          }
        } else if (object.image == nullptr) {
          nextLevelIndex = levelIndex;
        } else if (object.image == santaTex && (object.x + object.w / 2 > x && object.x + object.w / 2 < x + playerW) && object.timer != 100) {
          object.timer = 100;
          objects.push_back(Object((object.x + object.w / 2) / (float)tileSize, object.y / (float)tileSize, presentTex, false, 0, tileSize * -0.7));
          playAudio(hoHoHoSound);
        } else if (object.image == presentTex) {
          if (isKeyPressed(Key::E)) {
            object.remove = true;
            particles::explode(object.x + object.w / 2, object.y + object.h / 2, red);
            for (int x = 0; x < levelWidth; x++) {
              for (int y = 0; y < levelHeight; y++) {
                if (getTile(x, y) == '0') {
                  setTile(x, y, ' ');
                }
              }
            }
            playAudio(fixedLeaksSound);
          }
        } else if (!object.remove && object.image == magnifyingGlassTex) {
          if (isKeyPressed(Key::E)) {
            inventory.push_back(magnifyingGlassTex);
            object.remove = true;
          }
        } else if (!object.remove && object.image == gunTex) {
          if (isKeyPressed(Key::E)) {
            inventory.push_back(gunTex);
            object.remove = true;
          }
        }
      }
      i++;
    }
    if (x != newX) deltaX = 0;
    if (y != newY) deltaY = 0;
    x = newX;
    y = newY;
  }

  void draw(Window* window) {
    pushTransform(window);
    rotate(window, x - cameraX + playerW / 2, y - cameraY + playerH, visualAcceleration * 4.2);
    drawImage(window, playerTex, x - cameraX, y - cameraY, playerW, playerH, flip, frame * playerTex->width / nFrames, 0, playerTex->width / nFrames, playerTex->height);
    if (inventory.size() > 0) {
      itemX = playerW * 21 * nFrames / playerTex->width;
      itemY = y + playerH * 26 / playerTex->height - tileSize / 2;
      if (flip) {
        itemX = playerW / 2 - itemX;
      }
      itemX += x;
      itemW = inventory[0]->width * tileSize / 2 / inventory[0]->height;
      itemH = tileSize / 2;
      drawImage(window, inventory[0], itemX - cameraX, itemY - cameraY, itemW, itemH, flip);
    }
    popTransform(window);
  }
};

Player player;

float getPlayerX() { return player.x; }

float getPlayerY() { return player.y; }
