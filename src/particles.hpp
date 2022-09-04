#include <list>

Image* oneTex;
Image* zeroTex;
Audio* shootSound;

namespace particles {
void fire(float x, float y, Color color, int count = 100);

struct Particle {
  float x, y, dx = 0, dy = 0, lifetime, lifetimeMax = -1, gravity = 0, incresement = 0, *level = nullptr;
  bool objectCollisive = false, remove = false;
  Image* texture;
  Color color;
  int w, h;

  void update() {
    if (lifetimeMax != -1) {
      lifetime -= deltaTime();
      if (lifetime < 0) {
        remove = true;
      }
    }
    if (weather == Weather::WIND && incresement != 10 && level != (float*)gunTex) {
      dx += tileSize * windDir * 0.05;
    }
    dy += tileSize * deltaTime() * gravity;
    x += dx * tileSize * deltaTime();
    y += dy * tileSize * deltaTime();
    char tile = getTile(x / tileSize, y / tileSize);
    if (tile == '#') {
      remove = true;
    }

    // Particles actions
    if (!level && incresement == 10) {
      float yellowToRed = (lifetime - lifetimeMax * 0.7) / (lifetimeMax * 0.3);
      float redToBlack = lifetime / (lifetimeMax * 0.7);
      int alpha = lifetime > lifetimeMax * 0.75 ? 255 - (lifetime - lifetimeMax * 0.75) / (lifetimeMax * 0.25) * 255 : lifetime / (lifetimeMax * 0.75) * 255;
      color = lifetime > lifetimeMax * 0.7 ? Color(255, yellowToRed * (230 - 114) + 114, yellowToRed * (87 - 51) + 51, alpha) : Color(redToBlack * 255, redToBlack * 114, redToBlack * 51, alpha);
    }

    if (tile == '_') {
      if (level && incresement && y + h > *level * tileSize) {
        *level -= incresement;
        remove = true;
      }
    }
    if (x < 0 || x > levelWidth * tileSize || y < 0 || y > levelHeight * tileSize) {
      remove = true;
    }

    if (objectCollisive) {
      for (Object& object : objects) {
        if (x > object.x && x < object.x + object.w && y > object.y && y < object.y + object.h) {
          if (object.image == logTex) {
            object.temperature += incresement;
            fire(x, y, red, 3);
            remove = true;
          } else if(object.image == nullptr) {
            if(level == (float*)gunTex) {
              object.temperature = std::max(object.temperature - 0.05f, 0.f);
              remove = true;
            }
          }
        }
      }
    }

    if ((texture == zeroTex || texture == oneTex) && incresement == 10) {
      if (x > player.x && x < player.x + playerW && y > player.y && y < player.y + playerH) {
        inventory.clear();
      }
    }
  }

  void draw(Window* window) {
    if (texture) {
      drawImage(window, texture, x - cameraX, y - cameraY, w, h);
    } else {
      fillRect(window, x - cameraX, y - cameraY, w, h, color);
    }
  }
};

std::list<Particle> particles;

void fire(float x, float y, Color color, int count) {
  for (int i = 0; i < count; i++) {
    float angle = random((float)M_PI * 0.95f, (float)M_PI * 1.05f);
    float velocity = random(0.1f, 0.5f);
    particles.push_back(Particle{
        .x = x,
        .y = y,
        .dx = sin(angle) * velocity,
        .dy = cos(angle) * velocity,
        .lifetime = 0.3,
        .lifetimeMax = 0.3,
        .gravity = -0.4,
        .incresement = 10,
        .w = tileSize / 16,
        .h = tileSize / 16,
    });
  }
}

void rain(Window* window) {
  static float timer = 0;
  timer -= deltaTime();
  if (timer <= 0) {
    timer = random(0.01f, 0.02f);
    for (int i = 0; i < random(5, 10) * (weather == Weather::THUNDERSTORM ? 2 : 1); i++) {
      float x = random(cameraX - tileSize * 10, cameraX + windowWidth(window) + tileSize * 10), y = random(-5, -15);
      particles.push_back(Particle{
          .x = x,
          .y = y,
          .dx = 0,
          .dy = random(10.f, 15.f),
          .gravity = 0.5,
          .incresement = (weather == Weather::THUNDERSTORM ? 0.004f : 0.002f),
          .level = &waterLevel,
          .color = Color(75, 69, 110),
          .w = tileSize / 32,
          .h = tileSize / 8,
      });
    }
  }
}

void snow(Window* window) {
  static float timer = 0;
  timer -= deltaTime();
  if (timer <= 0) {
    timer = random(0.01f, 0.02f);
    for (int i = 0; i < random(10, 20); i++) {
      float x = random(cameraX - tileSize * 20, cameraX + windowWidth(window) + tileSize * 20), y = random(-5, -15);
      particles.push_back(Particle{
          .x = x,
          .y = y,
          .dx = 0,
          .dy = random(2.f, 3.f),
          .incresement = 0.002,
          .level = &snowLevel,
          .color = white,
          .w = tileSize / 16,
          .h = tileSize / 16,
      });
    }
  }
}

void sun(Window* window) {
  static float timer = 0;
  timer -= deltaTime();
  if (timer <= 0) {
    timer = 0.01f;

    for (int i = 0; i < 5; i++) {
      float x = random(cameraX, cameraX + windowWidth(window) * 0.35), y = cameraY;
      float dx = player.itemX + (player.flip ? player.itemW - player.itemW / 1.35 : player.itemW / 1.35) - x;
      float dy = player.itemY + player.itemH / 3 - y;
      float len = sqrt(dx * dx + dy * dy);
      len /= 16;
      dx /= len;
      dy /= len;

      particles.push_back(Particle{
          .x = x,
          .y = y,
          .dx = dx,
          .dy = dy,
          .incresement = 0.003,
          .objectCollisive = true,
          .color = Color(255, 200, 0),
          .w = tileSize / 16,
          .h = tileSize / 16,
      });
    }
  }
}

void leak(int x1, int y1, int w, int h, int count, bool affectPlayer) {
  static float timer = 0;
  timer -= deltaTime();
  if (timer <= 0) {
    timer = 0.03f;

    for (int i = 0; i < count; i++) {
      float x = random(x1, x1 + w), y = random(y1, y1 + h);
      Image* tex = rand() % 2 ? oneTex : zeroTex;

      particles.push_back(Particle{
          .x = x,
          .y = y,
          .gravity = 1,
          .incresement = affectPlayer ? 10.f : 0.f,
          .texture = tex,
          .w = tileSize / 24 * tex->width,
          .h = tileSize / 24 * tex->height,
      });
    }
  }
}

void leakWall(int x1, int y1, int count, bool affectPlayer = true) {
  static float timer = 0;
  timer -= deltaTime();
  if (timer <= 0) {
    timer = 0.03f;

    for (int i = 0; i < count; i++) {
      float x = random(x1 * tileSize, x1 * tileSize + tileSize), y = random(y1 * tileSize, y1 * tileSize + tileSize);
      Image* tex = rand() % 2 ? oneTex : zeroTex;

      particles.push_back(Particle{
          .x = x,
          .y = y,
          .lifetime = 0.3,
          .lifetimeMax = 0.3,
          .incresement = affectPlayer ? 10.f : 0.f,
          .texture = tex,
          .w = tileSize / 24 * tex->width,
          .h = tileSize / 24 * tex->height,
      });
    }
  }
}

void lightning(float x1, float y1) {
  for (int i = cameraY; i < y1; i += tileSize / 16) {
    float x = x1, y = i;

    particles.push_back(Particle{
        .x = x,
        .y = y,
        .lifetime = 0.1,
        .lifetimeMax = 0.1,
        .color = white,
        .w = tileSize / 16,
        .h = tileSize / 16,
    });
  }
}

void electricity(float x, float y, int count) {
  for (int i = 0; i < count; i++) {
    float angle = i * M_PI * 2 / count;
    float velocity = 0.5;
    particles.push_back(Particle{
        .x = x,
        .y = y,
        .dx = sin(angle) * velocity,
        .dy = cos(angle) * velocity,
        .lifetime = 1.3,
        .lifetimeMax = 1.3,
        .incresement = 0.5,
        .objectCollisive = true,
        .color = Color(255, 200, 0),
        .w = tileSize / 16,
        .h = tileSize / 16,
    });
  }
}

void explode(float x, float y, Color color, int count) {
  for (int i = 0; i < count; i++) {
    float angle = random(0.f, (float)M_PI * 2.f);
    float velocity = random(0.5f, 1.5f);
    particles.push_back(Particle{
        .x = x,
        .y = y,
        .dx = sin(angle) * velocity,
        .dy = cos(angle) * velocity,
        .lifetime = 1.3,
        .lifetimeMax = 1.3,
        .level = (float*)gunTex,
        .color = color,
        .w = tileSize / 16,
        .h = tileSize / 16,
    });
  }
}

void bullets(Window* window) {
  static float timer = 0;
  timer -= deltaTime();
  if (timer <= 0) {
    timer = 0.3f;
    playAudio(shootSound);

    float x = player.itemX + (player.flip ? 0 : player.itemW), y = player.itemY + player.itemH / 2;
    float dx = player.flip ? -1 : 1;
    dx *= tileSize * 0.1f;

    particles.push_back(Particle{
        .x = x,
        .y = y,
        .dx = dx,
        .dy = 0,
        .level = (float*)gunTex,
        .objectCollisive = true,
        .color = black,
        .w = tileSize / 16,
        .h = tileSize / 16,
    });
  }
}

void update() {
  for (Particle& particle : particles) {
    particle.update();
  }
  particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& particle) { return particle.remove; }), particles.end());
}

void draw(Window* window) {
  for (Particle& particle : particles) {
    particle.draw(window);
  }
}
}  // namespace particles
