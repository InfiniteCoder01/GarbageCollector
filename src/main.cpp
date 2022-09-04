#include <mova.h>
#include <emscripten.h>
#include "level.hpp"
#include "objects.hpp"
#include "pc.hpp"
#include "player.hpp"
#include "particles.hpp"

int main(int argc, const char** argv) {
  EM_ASM(document.getElementsByTagName("head")[0].innerHTML += "<link href='https://fonts.googleapis.com/css?family=Press Start 2P' rel='stylesheet'>");
  Window* window = createWindow("GarbageCollector");
  __window = window;
  antialiasing(window, false);
  weatherLang::init();

  oneTex = loadImage("/Assets/Textures/One.png", window);
  zeroTex = loadImage("/Assets/Textures/Zero.png", window);

  playerTex = loadImage("/Assets/Textures/Robot.png", window);
  NPCTex = loadImage("/Assets/Textures/RobotNPC.png", window);
  santaTex = loadImage("/Assets/Textures/Santa.png", window);

  logTex = loadImage("/Assets/Textures/Log.png", window);
  presentTex = loadImage("/Assets/Textures/Present.png", window);
  containerTex = loadImage("/Assets/Textures/Container.png", window);
  cristmasTreeTex = loadImage("/Assets/Textures/CristmasTree.png", window);
  lightningRodTex = loadImage("/Assets/Textures/LightningRoad.png", window);
  magnifyingGlassTex = loadImage("/Assets/Textures/MagnifyingGlass.png", window);
  gunTex = loadImage("/Assets/Textures/Gun.png", window);

  tileset = loadImage("/Assets/Textures/Tileset.png", window);
  shootSound = loadAudio("Assets/Sound/Shoot.wav");
  deathSound = loadAudio("Assets/Sound/Death.wav");
  openBookSound = loadAudio("Assets/Sound/Book.mp3");
  shhhhSound = loadAudio("Assets/Sound/Shhhh.mp3");
  hoHoHoSound = loadAudio("Assets/Sound/HoHoHo.mp3");
  beepSound = loadAudio("Assets/Sound/Beep.wav");
  windSound = loadAudio("Assets/Sound/Wind.mp3");
  fixedLeaksSound = loadAudio("Assets/Sound/FixedLeaks.mp3");
  bossExplosionSound = loadAudio("Assets/Sound/BossExplosion.wav");
  jingleBellsSong = loadAudio("Assets/Sound/JingleBells.mp3");

  tileSize = windowHeight(window) / 11.25;
  initPC();
  float lightningTimer = 0;
  std::vector<Image*> prevInv;
  while (true) {
    bool left = nextLevelIndex < levelIndex;
    if (nextLevelIndex != levelIndex) {
      prevInv = inventory;
      levels[levelIndex] = level;
      objectss[levelIndex] = objects;
    } else {
      playAudio(deathSound);
    }
    levelIndex = nextLevelIndex;
    nextLevelIndex = -1;
    loadLevel();
    objects = objectss[levelIndex];
    for (const Object& object : objects) {
      if (object.image == santaTex) santaPresent = true;
    }
    inventory = prevInv;
    player.x = left ? levelWidth * tileSize - tileSize * 3 : tileSize * 2;
    player.y = tileSize;
    stopAudio(jingleBellsSong);

    while (nextLevelIndex == -1) {
      // Update sizes
      float scale = (int)(windowHeight(window) / 11.25) / (float)tileSize;
      tileSize = windowHeight(window) / 11.25;
      playerW = tileSize;
      playerH = playerW * player.nFrames * playerTex->height / playerTex->width;
      player.x *= scale;
      player.y *= scale;
      setFont(window, std::to_string((int)(tileSize * 0.3)) + "px 'Press Start 2P', cursive");

      // Update logic
      for (int x = 0; x < levelWidth; x++) {
        for (int y = 0; y < levelHeight; y++) {
          if (getTile(x, y) == 'L') {
            setTile(x, y, '_');
            objects.push_back(Object(x, y, logTex));
          } else if (getTile(x, y) == 'l') {
            setTile(x, y, ' ');
            for (int i = 0; i < 15; i++) objects.push_back(Object(x, y - i * (logTex->height / (float)logTex->width), logTex));
          } else if (getTile(x, y) == 'N') {
            setTile(x, y, ' ');
            objects.push_back(Object(x, y, NPCTex));
            objects.push_back(Object(x + 1.1, y, NPCTex, true));
          } else if (getTile(x, y) == 'm') {
            setTile(x, y, ' ');
            objects.push_back(Object(x, y, magnifyingGlassTex));
          } else if (getTile(x, y) == '@') {
            setTile(x, y, ' ');
            objects.push_back(Object(x, y, containerTex));
          } else if (getTile(x, y) == 'c') {
            setTile(x, y, ' ');
            objects.push_back(Object(x, y, cristmasTreeTex));
          } else if (getTile(x, y) == 'b') {
            setTile(x, y, ' ');
            objects.push_back(Object(x, y, nullptr));
          } else if (getTile(x, y) == 'G') {
            setTile(x, y, ' ');
            objects.push_back(Object(x, y, gunTex));
          } else if (getTile(x, y) == '1') {
            particles::leak(x * tileSize, y * tileSize, tileSize, tileSize, 3);
          } else if (getTile(x, y) == '0') {
            particles::leakWall(x, y, 3);
          }
        }
      }

      lightningTimer -= deltaTime();
      if (weather == Weather::COLD) {
        particles::snow(window);
      } else if (weather == Weather::RAIN) {
        particles::rain(window);
      } else if (weather == Weather::THUNDERSTORM) {
        particles::rain(window);
      }

      if (inPC || inManual) {
        if (isKeyPressed(Key::Escape)) {
          if (inPC) {
            closePC();
          } else {
            inManual = false;
          }
        }
        if (inPC) {
          updatePC();
        }
      } else {
        player.update(window);
        updateObjects(scale);
      }
      particles::update();

      // Draw
      if (weather == Weather::CLEAR) {
        clear(window, Color(171, 221, 247));
      } else if (weather == Weather::WIND) {
        clear(window, Color(171, 221, 247));
      } else if (weather == Weather::COLD) {
        clear(window, Color(198, 205, 207));
      } else if (weather == Weather::RAIN) {
        clear(window, Color(115, 140, 153));
      } else if (weather == Weather::THUNDERSTORM) {
        clear(window, Color(117, 117, 117));
        if (lightningTimer < 0.1) {
          if (lightningTimer <= 0) lightningTimer = random(10.f, 15.f);
          clear(window, white);
        }
      }
      if (inPC) {
        drawPC(window);
      } else if (inManual) {
        drawManual(window);
      } else {
        drawLevel(window);
        drawObjects(window);
        player.draw(window);
        particles::draw(window);
        drawText(window, 0, tileSize * 0.4, std::to_string((int)(1 / deltaTime())), black);
        if (NPCTextX != -1 && NPCTextY != -1) {
          int y = NPCTextY - std::count(NPCDialogs[levelIndex].begin(), NPCDialogs[levelIndex].end(), '\n') * tileSize * 0.5;
          std::stringstream ss(NPCDialogs[levelIndex]);
          std::string line;
          while (std::getline(ss, line, '\n')) {
            drawText(window, NPCTextX - cameraX, y - cameraY, line, black);
            y += tileSize * 0.5;
          }
        }
        int y = tileSize * 0.5;
        for (Image* item : inventory) {
          drawImage(window, item, 3, y, item->width * tileSize / item->height, tileSize);
          y += tileSize;
        }
        if (inventory.size() > 0) {
          if (inventory[0] == magnifyingGlassTex && weather == Weather::CLEAR) {
            particles::sun(window);
          } else if (inventory[0] == gunTex && isKeyHeld(Key::E)) {
            particles::bullets(window);
          }
        }
      }
      nextFrame();
    }
  }

  destroyImage(oneTex);
  destroyImage(zeroTex);

  destroyImage(playerTex);
  destroyImage(NPCTex);
  destroyImage(santaTex);

  destroyImage(logTex);
  destroyImage(presentTex);
  destroyImage(containerTex);
  destroyImage(cristmasTreeTex);
  destroyImage(lightningRodTex);
  destroyImage(magnifyingGlassTex);
  destroyImage(gunTex);

  destroyImage(tileset);
  destroyAudio(shootSound);
  destroyAudio(deathSound);
  destroyAudio(openBookSound);
  destroyAudio(shhhhSound);
  destroyAudio(hoHoHoSound);
  destroyAudio(beepSound);
  destroyAudio(windSound);
  destroyAudio(fixedLeaksSound);
  destroyAudio(bossExplosionSound);
  destroyAudio(jingleBellsSong);
  destroyWindow(window);
  return 0;
}
