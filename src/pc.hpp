#include <vector>
#include <string>
#include <sstream>
#include "weather.hpp"

Window* __window;
Audio* beepSound;
float cursorTimer;
bool inPC, inManual;
std::string start = "> ";
std::string editedCode;
int cursor, cmdIdx, lineIndex;
std::map<std::string, std::vector<std::string>> codes;
std::vector<std::string> pcText;
std::vector<std::string> commands;
// clang-format off
std::vector<std::string> userManuals[] = {
  {
    "\\User manual for CodePC",
    "\\Press escape to exit",
    "",
    "CodePC is capable of",
    "executing code, written in",
    "Weather Lang, WL for short.",
    "To execute WL in CodePC, type",
    "wl [code].",
    "You can interact with game world",
    "in WL using functions.",
    "To execute function type:",
    "functionName([parameters]);",
    "Parameters are optional.",
    "For example, if you type",
    "\"wl beep();\" in CodePC's terminal,",
    "CodePC will beep.",
    "",
    "You can change weather in the",
    "world using weather function.",
    "Example:",
    "weather(\"Rain\");",
    "Weather can be \"Clear\",",
    "\"Wind\", \"Cold\", \"Rain\"",
    "or \"Thunderstorm\".",
    "\n",
    "You can press left and right",
    "arrows to move the cursor, hold",
    "ctrl and press the arrows to move",
    "cursor by one word at a time,",
    "press backspace or delete to",
    "erase, hold ctrl to do the same",
    "with words, press home to move the",
    "cursor to the start of the line,",
    "press end to move cursor to the end",
    "of the line. Press the up and down",
    "arrows to navigate previously typed",
    "commands.",
    "",
    "You can also clear CodePC's",
    "terminal using \"clear\" command.",
    "And you can try typing",
    "linux commands like ls, ps,",
    "vim or nano into CodePC,",
    "if you want.",
  }, {
    "Congrats for beating the game!",
    "And now I can tell you,",
    "that you can code anything",
    "with WL!",
    "There are variables:",
    "var a; var b = 2;",
    "a = b + 5; print(a, b);",
    "There is math:",
    "a = 5 * b; a = a + 3;",
    "a = a << b;",
    "[+ - & | ^ ~] [* / % && || !",
    "<< >> > < == !=] like in C++",
    "There are tables:",
    "var a = [\"k1\": 5, \"k2\":",
    "[\"1\": 3]];",
    "a[\"k1\"][1] = 4;",
    "print(a.k1);",
    "a.k1[1] = 5;",
    "print(a);",
    "Read about for, foreach, if,",
    "print, println, eval,",
    "fillRect and forever",
    "functions in wlman!",
    "\n",
    "Type wledit cmdname to open",
    "editor, hit escape to close it",
    "Type cmdname and hit enter to",
    "run your program! PageUp and",
    "PageDown supported!",
    "I'm TheVoid, game creator!",
    "Real nickname - InfiniteCoder",
    "I'm Dima from Minsk, 13 years old.",
    "Made for OLC CodeJam 2022.",
  },
};

std::map<std::string, std::vector<std::string>> wlman = { // for, if, print, println and eval
  {"beep", {
    "beep();",
    "CodePC will beep!"
  }}, {"weather", {
    "weather(\"type\");",
    "Set world weather to \"type\"!",
    "Type can be \"Clear\", \"Wind\", \"Cold\", \"Rain\"",
    "or \"Thunderstorm\".",
  }}, {"blockdata", {
    "blockdata(\"property\", value);",
    "Changes data of the nearest block, that is holding data.",
    "Any block holding data:",
    "Property \"type\" can be \"container\", \"MCU\".",
    "Containers:",
    "Property \"content\" can be \"null\", \"lightning rod\".",
  }}, {"for", {
    "Usage: for(\"varname\", from, to, step, {code});",
    "Example: for(\"i\", 0, 10, 1, { println(i); });",
  }}, {"foreach", {
    "Usage: foreach(table, \"keyname\", \"valuename\", {code});",
    "Example: var t = [ \"0\": 5, \"1\": 8 ];",
    "foreach(t, \"k\", \"v\", { println(k, v); });",
  }}, {"if", {
    "Usage: if(expr, {code}[, {else code}]);",
    "Example: var n = 1; if(n == 1, { println(\"n is 1\"); },",
    "{ println(\"n is not 1\"); });",
  }}, {"print", {
    "Usage: print(expr, expr...);",
    "Example: var a = 1; print(a); print(a, 2); print(a, 2, 3);",
  }}, {"println", {
    "Usage: println(expr, expr...);",
    "Example: var a = 1; println(a); println(a, 2); println(a, 2, 3);",
  }}, {"eval", {
    "Usage: eval(\"code\");",
    "Example: var a = \"println(1234);\"; eval(a);",
    "a = { print(\"Hello, World!\"); }; eval(a);",
  }}, {"fillRect", {
    "Usage: fillRect(x, y, w, h, r, g, b, a);",
  }}, {"forever", {
    "Usage: forever({code});",
    "variable dt contains deltaTime in microseconds",
    "to check is key pressed if(keys[0], {...});",
    "Tab = 0",
    "ArrowLeft = 1, ArrowRight = 2, ArrowUp, ArrowDown,",
    "PageUp, PageDown, Home, End,",
    "Insert, Delete, Backspace,",
    "Space, Enter, Escape,",
    "Apostrophe, Comma, Minus, Period, Slash, Semicolon, Equal,",
    "BracketLeft, Backslash, BracketRight, GraveAccent,",
    "CapsLock, ScrollLock, NumLock, PrintScreen,",
    "Pause,",
    "Numpad0 to Numpad9,",
    "NumpadDecimal, NumpadDivide, NumpadMultiply, NumpadSubtract, NumpadAdd, NumpadEnter, NumpadEqual,",
    "ShiftLeft, ControlLeft, AltLeft, MetaLeft, ShiftRight, ControlRight, AltRight, MetaRight, ContextMenu,",
    "Digit0 to Digit9,",
    "A to Z",
    "F1 to F12",
  }}
};
// clang-format on

void drawBorder(Window* window, int x, int y) {
  drawImage(window, tileset, 0, 0, tileSize, tileSize, false, x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
  drawImage(window, tileset, 0, tileSize, tileSize, windowHeight(window) - tileSize * 2, false, x * TILE_SIZE, y * TILE_SIZE + TILE_SIZE, TILE_SIZE, TILE_SIZE);
  drawImage(window, tileset, 0, windowHeight(window) - tileSize, tileSize, tileSize, false, x * TILE_SIZE, y * TILE_SIZE + TILE_SIZE * 2, TILE_SIZE, TILE_SIZE);
  drawImage(window, tileset, tileSize, 0, windowWidth(window) - tileSize * 2, tileSize, false, x * TILE_SIZE + TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
  drawImage(window, tileset, tileSize, windowHeight(window) - tileSize, windowWidth(window) - tileSize * 2, tileSize, false, x * TILE_SIZE + TILE_SIZE, y * TILE_SIZE + TILE_SIZE * 2, TILE_SIZE, TILE_SIZE);
  drawImage(window, tileset, windowWidth(window) - tileSize, 0, tileSize, tileSize, false, x * TILE_SIZE + TILE_SIZE * 2, y * TILE_SIZE, TILE_SIZE, TILE_SIZE);
  drawImage(window, tileset, windowWidth(window) - tileSize, tileSize, tileSize, windowHeight(window) - tileSize * 2, false, x * TILE_SIZE + TILE_SIZE * 2, y * TILE_SIZE + TILE_SIZE, TILE_SIZE, TILE_SIZE);
  drawImage(window, tileset, windowWidth(window) - tileSize, windowHeight(window) - tileSize, tileSize, tileSize, false, x * TILE_SIZE + TILE_SIZE * 2, y * TILE_SIZE + TILE_SIZE * 2, TILE_SIZE, TILE_SIZE);
  drawImage(window, tileset, tileSize, tileSize, windowWidth(window) - tileSize * 2, windowHeight(window) - tileSize * 2, false, x * TILE_SIZE + TILE_SIZE, y * TILE_SIZE + TILE_SIZE, TILE_SIZE, TILE_SIZE);
}

std::vector<std::string>& split(const std::string& str, const std::string& delimiter, std::vector<std::string>& strings, const std::string& prefix = "") {
  if (str.empty()) return strings;
  std::string::size_type pos = 0;
  std::string::size_type prev = 0;
  while ((pos = str.find(delimiter, prev)) != std::string::npos) {
    strings.push_back(prefix + str.substr(prev, pos - prev));
    prev = pos + 1;
  }
  strings.push_back(prefix + str.substr(prev));
  return strings;
}

void drawPC(Window* window) {
  if ((pcText.size() * 0.4 + 1) * tileSize > windowHeight(window)) {
    pcText.erase(pcText.begin());
  }
  drawBorder(window, 1, 0);
  for (int i = 0; i < pcText.size(); i++) {
    if (pcText[i][0] == '\\') {
      drawText(window, tileSize * 0.6, (i * 0.4 + 1) * tileSize, pcText[i].substr(1), red);
    } else {
      drawText(window, tileSize * 0.6, (i * 0.4 + 1) * tileSize, pcText[i], Color(9, 255, 0));
    }
  }
  if (cursorTimer < 0.5) {
    if (!start.empty()) lineIndex = pcText.size() - 1;
    int cursorX = pcText[lineIndex].empty() ? 0 : textWidth(window, pcText[lineIndex].substr(0, cursor));
    int cursorW = pcText[lineIndex].empty() ? textWidth(window, "_") : textWidth(window, pcText[lineIndex].substr(std::min(cursor, (int)pcText[lineIndex].size() - 1), 1));
    fillRect(window, tileSize * 0.6 + cursorX, ((lineIndex - 1) * 0.4 + 1) * tileSize + tileSize * 0.4 - tileSize / 8, cursorW, tileSize / 8, Color(9, 255, 0));
  }
}

void drawManual(Window* window) {
  drawBorder(window, 4, 0);
  drawImage(window, tileset, (windowWidth(window) - tileSize) / 2, 0, tileSize, tileSize, false, TILE_SIZE * 7, 0, TILE_SIZE, TILE_SIZE);
  drawImage(window, tileset, (windowWidth(window) - tileSize) / 2, tileSize, tileSize, windowHeight(window) - tileSize * 2, false, TILE_SIZE * 7, TILE_SIZE, TILE_SIZE, TILE_SIZE);
  drawImage(window, tileset, (windowWidth(window) - tileSize) / 2, windowHeight(window) - tileSize, tileSize, tileSize, false, TILE_SIZE * 7, TILE_SIZE * 2, TILE_SIZE, TILE_SIZE);
  int offsetX = 0, offsetY = 0;
  std::vector<std::string> userManual = userManuals[levelIndex > 0];
  for (int i = 0; i < userManual.size(); i++) {
    if (userManual[i] == "\n") {
      offsetX = windowWidth(window) / 2;
      offsetY = (i + 1) * 0.4 * tileSize;
    } else if (userManual[i][0] == '\\') {
      drawText(window, (windowWidth(window) / 2 - textWidth(window, userManual[i].substr(1))) / 2 + offsetX, (i * 0.4 + 1) * tileSize - offsetY, userManual[i].substr(1), black);
    } else {
      drawText(window, tileSize * 0.6 + offsetX, (i * 0.4 + 1) * tileSize - offsetY, userManual[i], black);
    }
  }
}

void modifyText() {
  cursorTimer = 0;
  cmdIdx = commands.size();
}

void openPC() {
  inPC = true;
  cursor = start.length();
  pcText.clear();
  if (levelIndex == 3) {
    pcText.push_back("CodePC v1.1");
    pcText.push_back("Introduced the 'blockdata' function");
  } else {
    pcText.push_back("CodePC v1.0");
    pcText.push_back("Press escape to exit");
  }
  pcText.push_back(start);
  modifyText();
}

void closePC() {
  if (start.empty()) {
    start = "> ";
    codes[editedCode] = pcText;
    pcText.clear();
    pcText.push_back(start);
    cursor = start.length();
  } else {
    inPC = false;
  }
}

void command(std::string cmd) {
  if (cmd == "clear") {
    pcText.clear();
  } else if (cmd == "ls") {
    pcText.push_back("\\No filesystem in this PC!");
  } else if (cmd == "ps") {
    pcText.push_back("\\No OS is running!");
  } else if (cmd == "vim") {
    pcText.push_back("\\Read the manual, Linux user!");
  } else if (cmd == "nano") {
    pcText.push_back("Please, read the manual, Linux beginner!");
  } else if (cmd.substr(0, 3) == "wl ") {
    if (weatherLang::execute(cmd.substr(3))) {
      split(weatherLang::output.str(), "\n", pcText);
    } else {
      split(weatherLang::error, "\n", pcText, "\\");
    }
    weatherLang::output.str("");
    weatherLang::output.clear();
  } else if (cmd.substr(0, 6) == "wlman ") {
    if (wlman.count(cmd.substr(6))) {
      pcText.insert(pcText.end(), wlman[cmd.substr(6)].begin(), wlman[cmd.substr(6)].end());
    } else {
      pcText.push_back("\\function \"" + cmd.substr(6) + "\" not found!");
    }
  } else if (cmd.substr(0, 7) == "wledit ") {
    editedCode = cmd.substr(7);
    if (!codes.count(editedCode)) codes[editedCode] = {""};
    pcText = codes[editedCode];
    lineIndex = 0;
    start = "";
  } else if (codes.count(cmd)) {
    std::vector<std::string> code = codes[cmd];
    std::ostringstream text;
    std::copy(code.begin(), code.end(), std::ostream_iterator<std::string>(text, "\n"));
    if (weatherLang::execute(text.str())) {
      split(weatherLang::output.str(), "\n", pcText);
    } else {
      split(weatherLang::error, "\n", pcText, "\\");
    }
    weatherLang::output.str("");
    weatherLang::output.clear();
  } else {
    pcText.push_back("\\Invalid command: " + cmd);
  }
}

bool isID(char ch) { return isalpha(ch) || isdigit(ch) || ch == '_'; }
bool wordToLeft() { return isKeyHeld(Key::ControlLeft) && cursor > start.length() && isID(pcText[lineIndex][cursor - 1]); }
bool wordToRight() { return isKeyHeld(Key::ControlLeft) && cursor < pcText[lineIndex].length() && isID(pcText[lineIndex][cursor]); }

void updatePC() {  // FIXME: cmd history broken after opening editor
  cursorTimer += deltaTime();
  std::string& line = pcText[lineIndex];
  // Editing
  if (isKeyRepeated(Key::Backspace)) {
    if (start.empty() && cursor == 0) {
      if (lineIndex > 0) {
        cursor = pcText[lineIndex - 1].length();
        pcText[lineIndex - 1] += pcText[lineIndex];
        pcText.erase(pcText.begin() + lineIndex);
        lineIndex--;
      }
    } else {
      do {
        if (cursor > start.length()) {
          line.erase(line.begin() + cursor - 1);
          cursor--;
          modifyText();
        }
      } while (wordToLeft());
    }
  } else if (isKeyRepeated(Key::Delete)) {
    do {
      if (start.empty() && cursor == line.size()) {
        if (lineIndex < pcText.size() - 1) {
          pcText[lineIndex] += pcText[lineIndex + 1];
          pcText.erase(pcText.begin() + lineIndex + 1);
        }
      } else if (cursor < line.size()) {
        line.erase(line.begin() + cursor);
        modifyText();
      }
    } while (wordToRight());
  } else if (getCharPressed()) {
    line.insert(line.begin() + cursor, getCharPressed());
    cursor++;
    modifyText();
  }
  // Navigation
  else if (isKeyRepeated(Key::ArrowLeft)) {
    do {
      if (start.empty() && cursor == 0 && lineIndex > 0) {
        cursor = pcText[lineIndex - 1].length();
        lineIndex--;
      } else {
        cursor = std::max(cursor - 1, (int)start.length());
      }
      modifyText();
    } while (wordToLeft());
  } else if (isKeyRepeated(Key::ArrowRight)) {
    do {
      if (start.empty() && cursor == line.length() && lineIndex < pcText.size() - 1) {
        cursor = 0;
        lineIndex++;
      } else {
        cursor = std::min(cursor + 1, (int)line.length());
      }
      modifyText();
    } while (wordToRight());
  } else if (isKeyRepeated(Key::Home)) {
    cursor = start.length();
  } else if (isKeyRepeated(Key::End)) {
    cursor = line.length();
  }

  // Out of line
  else if (isKeyRepeated(Key::Enter)) {
    if (start.empty()) {
      std::string newline = line.substr(cursor);
      line = line.substr(0, cursor);
      lineIndex++;
      cursor = 0;
      pcText.insert(pcText.begin() + lineIndex, newline);
    } else {
      commands.push_back(line);
      cmdIdx = commands.size();
      command(line.substr(start.length()));
      pcText.push_back(start);
      cursor = start.length();
    }
    modifyText();
  } else if (isKeyRepeated(Key::ArrowUp)) {
    if (start.empty()) {
      lineIndex = std::max(lineIndex - 1, 0);
      cursor = std::min(cursor, (int)pcText[lineIndex].length());
      modifyText();
    } else {
      if (cmdIdx > 0) {
        cmdIdx--;
        line = commands[cmdIdx];
        cursor = std::min(cursor, (int)line.length());
      }
    }
  } else if (isKeyRepeated(Key::ArrowDown)) {
    if (start.empty()) {
      lineIndex = std::min(lineIndex + 1, (int)pcText.size() - 2);
      cursor = std::min(cursor, (int)pcText[lineIndex].length());
      modifyText();
    } else {
      if (cmdIdx < commands.size() - 1) {
        cmdIdx++;
        line = commands[cmdIdx];
        cursor = std::min(cursor, (int)line.length());
      }
    }
  } else if (isKeyRepeated(Key::PageUp)) {
    if (start.empty()) {
      lineIndex = std::max(lineIndex - 30, 0);
      cursor = std::min(cursor, (int)pcText[lineIndex].length());
      modifyText();
    }
  } else if (isKeyRepeated(Key::PageDown)) {
    if (start.empty()) {
      lineIndex = std::min(lineIndex + 30, (int)pcText.size() - 2);
      cursor = std::min(cursor, (int)pcText[lineIndex].length());
      modifyText();
    }
  }

  if (cursorTimer > 1) {
    cursorTimer = 0;
  }
}

namespace pcFunctions {
void beep(const std::vector<weatherLang::Token>& args) { playAudio(beepSound); }

void weatherF(const std::vector<weatherLang::Token>& args) {
  if (!weatherLang::checkArgs("s", args)) {
    weatherLang::error = "Usage: weather(\"Weather name\");";
    return;
  }
  if (args[0].value == "Clear") {
    weather = Weather::CLEAR;
  } else if (args[0].value == "Wind") {
    weather = Weather::WIND;
  } else if (args[0].value == "Cold") {
    weather = Weather::COLD;
  } else if (args[0].value == "Rain") {
    weather = Weather::RAIN;
  } else if (args[0].value == "Thunderstorm") {
    weather = Weather::THUNDERSTORM;
  } else {
    weatherLang::error = "Invalid weather: '" + args[0].value + "'!";
    return;
  }
  pcText.push_back("Weather is now '" + args[0].value + "'!");
}

void blockdata(const std::vector<weatherLang::Token>& args) {
  if (!weatherLang::checkArgs("s_", args)) {
    weatherLang::error = "Usage: blockdata(\"property\", value);";
    return;
  }
  for (Object& object : objects) {
    if (object.image == containerTex) {
      if (args[0].value == "type") {
        weatherLang::error = "Type changing is not supported yet!";
        return;
      } else if (args[0].value == "content") {
        if (args[1].type != weatherLang::TokenType::STRING) {
          weatherLang::error = "Container's content should be a string!";
          return;
        }
        if (args[1].value == "null") {
          object.content = nullptr;
        } else if (args[1].value == "lightning rod") {
          object.content = lightningRodTex;
        } else {
          weatherLang::error = "Invalid container's content '" + args[1].value + "'!";
          return;
        }
        pcText.push_back("Container's content is now " + args[1].value);
      } else {
        weatherLang::error = "Unknown property '" + args[0].value + "'!";
        return;
      }
      return;
    }
  }
  weatherLang::error = "No data holding blocks was found!";
}

void fillRectF(const std::vector<weatherLang::Token>& args) {
  if (!weatherLang::checkArgs("nnnnnnnn", args)) {
    weatherLang::error = "Usage: fillRect(x, y, w, h, r, g, b, a);";
    return;
  }
  fillRect(__window, std::stoi(args[0].value), std::stoi(args[1].value), std::stoi(args[2].value), std::stoi(args[3].value), Color(std::stoi(args[4].value), std::stoi(args[5].value), std::stoi(args[6].value), std::stoi(args[7].value)));
}

void foreverF(const std::vector<weatherLang::Token>& args) {
  if (!weatherLang::checkArgs("c", args)) {
    weatherLang::error = "Usage: forever({code});";
    return;
  }
  weatherLang::execute("var deltaTime; var keys = [];");
  while (!isKeyPressed(Key::Escape)) {
    weatherLang::execute("deltaTime = " + std::to_string((int)(deltaTime() * 1000000)) + ';');
    for(int key = 0; key < (int)Key::Unknown; key++) {
      weatherLang::execute("keys[" + std::to_string(key) + "] = " + std::to_string((int)isKeyHeld((Key)key)) + ';');
    }
    weatherLang::execute(args[0].value);
    nextFrame();
  }
}
}  // namespace pcFunctions

void initPC() {
  weatherLang::functions["beep"] = pcFunctions::beep;
  weatherLang::functions["weather"] = pcFunctions::weatherF;
  weatherLang::functions["blockdata"] = pcFunctions::blockdata;
  weatherLang::functions["fillRect"] = pcFunctions::fillRectF;
  weatherLang::functions["forever"] = pcFunctions::foreverF;
}
