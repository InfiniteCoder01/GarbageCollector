#include <vector>

Image* NPCTex;
Image* santaTex;

Image* gunTex;
Image* magnifyingGlassTex;
Image* lightningRodTex;
Image* cristmasTreeTex;
Image* containerTex;
Image* presentTex;
Image* logTex;

Audio* windSound;
Audio* bossExplosionSound;
Audio* jingleBellsSong;

struct Object;
std::vector<Object> objectss[N_LEVELS];
std::vector<Object> objects;
int windDir = 1;

extern int playerW, playerH;
float getPlayerX();
float getPlayerY();

namespace particles {
void lightning(float x, float y);
void electricity(float x, float y, int count);
void leak(int x1, int y1, int w, int h, int count, bool affectPlayer = true);
void explode(float x, float y, Color color, int count = 100);
}  // namespace particles

struct Object {
  float x, y, deltaX = 0, deltaY = 0, temperature = 0, timer = 0;
  int w, h, frame = 0, nFrames;
  bool flip, remove = false;
  Image* content = nullptr;
  Image* image;

  Object() = default;
  Object(float x, float y, Image* image, bool flip = false, float deltaX = 0, float deltaY = 0) : x(x * tileSize), y(y * tileSize), image(image), flip(flip), deltaX(deltaX), deltaY(deltaY) {
    if (image == logTex) {
      w = tileSize;
      nFrames = 1;
    } else if (image == containerTex) {
      w = tileSize;
      nFrames = 1;
      timer = random(2.f, 5.f);
    } else if (image == cristmasTreeTex) {
      w = tileSize * 2;
      nFrames = 2;
    } else if (image == magnifyingGlassTex || image == gunTex) {
      w = tileSize * 0.7;
      nFrames = 1;
    } else if (image == NPCTex) {
      w = tileSize;
      nFrames = 2;
    } else if (image == presentTex) {
      w = tileSize;
      nFrames = 2;
      frame = rand() % nFrames;
    } else if (image == santaTex) {
      w = tileSize * 4;
      nFrames = 1;
    } else if (image == nullptr) {
      w = tileSize;
      h = tileSize;
      temperature = 1;
      return;
    }
    h = image->height * w * nFrames / image->width;
  }

  void update() {
    float gravity = 2;
    if (content == lightningRodTex && weather == Weather::THUNDERSTORM) timer -= deltaTime();
    if (image == cristmasTreeTex) {
      frame = weather == Weather::COLD;
      if (frame && !santaPresent) {
        playAudio(jingleBellsSong);
        objects.push_back(Object(-5, levelHeight - 1 - santaTex->height * 4.f / santaTex->width, santaTex));
        santaPresent = true;
      }
    } else if (image == NPCTex) {
      frame = 1;
    } else if (image == santaTex) {
      frame = 0;
      x += tileSize * 4 * deltaTime();
      return;
    } else if (image == nullptr) {
      if (temperature == 0) {
        if (!content) {
          particles::explode(x + w / 2, y + h / 2, red);
          playAudio(bossExplosionSound);
          content = gunTex;
          timer = 0;
          for (int x = 0; x < levelWidth; x++) {
            for (int y = 0; y < levelHeight; y++) {
              if (getTile(x, y) == '0') {
                setTile(x, y, ' ');
              }
            }
          }
        }
        timer += deltaTime();
        cameraX += sin(timer * 10) * tileSize;
        if (timer > 3) {
          remove = true;
        }
        return;
      }
      timer += deltaTime();
      gravity = 1;
      windDir = sign(x - getPlayerX());
      if (frame < 3) {
        if (timer >= 1.0f) deltaX = 0;
        if (timer > 2.0f) {
          frame++;
          timer = 0;
          flip = false;
          deltaX = (getPlayerX() - x) / tileSize;
          deltaY = -tileSize * 0.5;
        }
        if (weather == Weather::WIND && y < levelHeight * tileSize * 0.3) {
          deltaX += windDir * 0.3;
          if (!flip) {
            playAudio(windSound);
            flip = true;
          }
        }
      } else {
        deltaX += (sign(getPlayerX() - x) * 6 - deltaX) * deltaTime();
        deltaY = 0;
        if (timer > 5) {
          frame = 0;
          timer = 0;
        }
      }
    }
    if (content == lightningRodTex && timer < 0) {
      timer = random(2.f, 5.f);
      particles::lightning(x + tileSize * 8 / (float)TILE_SIZE, y + tileSize * 5 / (float)TILE_SIZE);
      particles::electricity(x + tileSize * 21 / (float)TILE_SIZE, y + tileSize * 29 / (float)TILE_SIZE, 50);
    }

    if (temperature > 1) {
      remove = true;
    }

    deltaY += tileSize * gravity * deltaTime();
    resolveCollisions(0, 1);
    x += deltaX * deltaTime() * tileSize;
    resolveCollisions(sign(deltaX), 0);
    y += deltaY * deltaTime() * tileSize;
    resolveCollisions(0, sign(deltaY));
  }

  void resolveCollisions(int dx, int dy) {
    float newX = x, newY = y;
    for (int x1 = x / tileSize; x1 < (x + w) / tileSize; x1++) {
      for (int y1 = y / tileSize; y1 < (y + h) / tileSize; y1++) {
        if (getTile(x1, y1) == '#' || (getTile(x1, y1) == 'T' && image != presentTex)) {
          newX = dx > 0 ? std::min(newX, (float)x1 * tileSize - w) : dx < 0 ? std::max(newX, (float)x1 * tileSize + tileSize) : newX;
          newY = dy > 0 ? std::min(newY, (float)y1 * tileSize - h) : dy < 0 ? std::max(newY, (float)y1 * tileSize + tileSize) : newY;
        } else if (getTile(x1, y1) == '_') {
          float level = std::min(std::max((int)((y1 + 1 - waterLevel) * tileSize), 0), tileSize);
          if (level > 0) {
            newY = dy > 0 ? std::min(newY, (float)y1 * tileSize + tileSize - level - h / 2) : newY;
          }
        }
      }
    }
    if (image != presentTex) {
      for (Object& object : objects) {
        if (&object == this) continue;
        if (x > object.x - w && x < object.x + object.w && y > object.y - h && y < object.y + object.h) {
          if (object.image == logTex) {
            newX = dx > 0 ? std::min(newX, object.x - w) : dx < 0 ? std::max(newX, object.x + object.w) : newX;
            newY = dy > 0 ? std::min(newY, object.y - h) : dy < 0 ? std::max(newY, object.y + object.h) : newY;
          }
        }
      }
    }
    if (x != newX) deltaX = 0;
    if (y != newY) deltaY = 0;
    x = newX;
    y = newY;
  }

  void draw(Window* window) {
    if (image == nullptr) {
      particles::leak(x, y, w, h, temperature * 10, false);
    } else {
      drawImage(window, image, x - cameraX, y - cameraY, w, h, flip, image->width / nFrames * frame, 0, image->width / nFrames, image->height);
      if (content) {
        drawImage(window, content, x - cameraX, y - cameraY, tileSize, tileSize, flip);
      }
    }
  }
};

void updateObjects(float scale) {
  for (Object& object : objects) {
    object.x *= scale;
    object.y *= scale;
    object.w *= scale;
    object.h *= scale;
    object.update();
  }
  objects.erase(std::remove_if(objects.begin(), objects.end(), [](const Object& object) { return object.remove; }), objects.end());
}

void drawObjects(Window* window) {
  for (Object& object : objects) {
    object.draw(window);
  }
}
