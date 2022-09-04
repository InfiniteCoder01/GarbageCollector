#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <algorithm>

namespace weatherLang {
struct Token;
using WFunction = void (*)(const std::vector<Token>& args);
std::map<std::string, WFunction> functions;

enum class TokenType { WORD, NUMBER, STRING, OPERATOR, CODE, TABLE, _EOF };
const std::string operators[] = {"<<", ">>", "&&", "||", "==", "!=", "!", "(", ")", "[", "]", ",", ".", ";", ":", "=", "+", "-", "*", "/", "%", "&", "|", "^", "~", ">", "<"};

std::string error;
std::ostringstream output;
std::vector<std::map<std::string, Token>> variables = {{}};

struct Token {
  TokenType type;
  std::string value;
  std::map<std::string, Token> table;

  Token() = default;
  Token(std::string value, TokenType type) : type(type), value(value) {}

  bool operator==(const Token& other) { return other.type == type && other.value == value; }
  bool operator!=(const Token& other) { return !operator==(other); }

  std::string toString() const {
    if (type == TokenType::TABLE) {
      std::string result = "[ ";
      for (const auto& entry : table) {
        result += '\"';
        result += entry.first;
        result += "\": ";
        result += entry.second.toString();
        result += ", ";
      }
      result.pop_back();
      result.pop_back();
      result += " ]";
      return result;
    }
    return value;
  }
};

Token errorToken = Token("ERROR", TokenType::_EOF);

bool match(const std::string& str, size_t& index, const std::string& prefix) {
  if (str.substr(index, prefix.length()) == prefix) {
    index += prefix.length();
    return true;
  }
  return false;
}

std::vector<Token> tokenize(const std::string& code) {
  std::vector<Token> tokens;
  size_t index = 0;
  while (index < code.length()) {
    if (isalpha(code[index]) || code[index] == '_') {
      Token word;
      while (isalpha(code[index]) || isdigit(code[index]) || code[index] == '_') {
        word.value += code[index];
        index++;
      }
      word.type = TokenType::WORD;
      tokens.push_back(word);
    } else if (isdigit(code[index])) {  // TODO: Floats
      Token number;
      while (isdigit(code[index])) {
        number.value += code[index];
        index++;
      }
      number.type = TokenType::NUMBER;
      tokens.push_back(number);
    } else if (code[index] == '\"') {
      index++;
      Token str;
      while (code[index] != '\"' && index < code.length()) {
        str.value += code[index];
        index++;
      }
      if (index >= code.length()) {
        error = "Unterminated string!";
        return tokens;
      }
      index++;
      str.type = TokenType::STRING;
      tokens.push_back(str);
    } else if (code[index] == '{') {
      int level = 1;
      index++;
      Token codeText;
      while (index < code.length()) {
        if (code[index] == '}') {
          level--;
          if (level == 0) break;
        } else if (code[index] == '{') {
          level++;
        }
        codeText.value += code[index];
        index++;
      }
      if (index >= code.length()) {
        error = "Unterminated code!";
        return tokens;
      }
      index++;
      codeText.type = TokenType::CODE;
      tokens.push_back(codeText);
    } else {
      bool found = false;
      for (const std::string& op : operators) {
        if (match(code, index, op)) {
          tokens.push_back(Token(op, TokenType::OPERATOR));
          found = true;
          break;
        }
      }
      if (!found) {
        index++;
      }
    }
  }
  tokens.push_back(Token("EOF", TokenType::_EOF));
  return tokens;
}

Token nextToken(const std::vector<Token>& tokens, size_t& index) {
  index++;
  return tokens[index];
}

void pushLayer() { variables.push_back(std::map<std::string, Token>()); }

void popLayer() { variables.pop_back(); }

bool createVar(const std::string& name, const Token& value) {
  if (variables[variables.size() - 1].count(name)) {
    error = "Variable '" + name + "' already exists!";
    return false;
  }
  variables[variables.size() - 1][name] = value;
  return true;
}

Token& findVar(std::string name) {
  for (int i = variables.size() - 1; i >= 0; i--) {
    if (variables[i].count(name)) {
      return variables[i][name];
    }
  }
  error = "Variable '" + name + "' does not exist!";
  return errorToken;
}

Token& findVarFromTable(const std::string& id, Token& token, const std::vector<Token>& tokens, size_t& index, bool create) {
  Token& res = findVar(id);
  Token* result = &res;
  if (result->type != TokenType::TABLE) {
    error = "'" + result->toString() + "' is not a table!";
    return errorToken;
  }
  while (token == Token("[", TokenType::OPERATOR) || token == Token(".", TokenType::OPERATOR)) {
    std::string key;
    if (token == Token("[", TokenType::OPERATOR)) {
      token = nextToken(tokens, index);
      if (token.type != TokenType::STRING && token.type != TokenType::NUMBER) {
        error = "Expected table key in quotes or number, got '" + token.toString() + "'!";
        return errorToken;
      }
      key = token.value;
      token = nextToken(tokens, index);
      if (token != Token("]", TokenType::OPERATOR)) {
        error = "Expected ']', got '" + token.toString() + "'!";
        return errorToken;
      }
    } else {
      token = nextToken(tokens, index);
      if (token.type != TokenType::WORD) {
        error = "Expected table key as word, got '" + token.toString() + "'!";
        return errorToken;
      }
      key = token.value;
    }
    token = nextToken(tokens, index);
    if (!result->table.count(key)) {
      if (create) {
        result->table[key] = Token("", TokenType::_EOF);
      } else {
        error = "Key '" + key + "' not found!";
        return errorToken;
      }
    }
    result = &result->table[key];
  }
  return *result;
}

Token eval(Token& token, const std::vector<Token>& tokens, size_t& index);
Token evalScalar(Token& token, const std::vector<Token>& tokens, size_t& index) {
  Token result = token;
  if (token == Token("(", TokenType::OPERATOR)) {
    token = nextToken(tokens, index);
    result = eval(token, tokens, index);
    if (token != Token(")", TokenType::OPERATOR)) {
      error = "Expected ')', got '" + token.toString() + "'!";
      return errorToken;
    }
    token = nextToken(tokens, index);
  } else if (token == Token("-", TokenType::OPERATOR)) {
    token = nextToken(tokens, index);
    Token n = evalScalar(token, tokens, index);
    if (n.type != TokenType::NUMBER) {
      error = "Cannot do math with non-number '" + n.toString() + "'!";
      return errorToken;
    }
    result = Token(std::to_string(-std::stoi(n.value)), TokenType::NUMBER);
  } else if (token == Token("!", TokenType::OPERATOR)) {
    token = nextToken(tokens, index);
    Token n = evalScalar(token, tokens, index);
    if (n.type != TokenType::NUMBER) {
      error = "Cannot do math with non-number '" + n.toString() + "'!";
      return errorToken;
    }
    result = Token(std::to_string((int)(!std::stoi(n.value))), TokenType::NUMBER);
  } else if (token.type == TokenType::WORD) {
    std::string id = token.value;
    token = nextToken(tokens, index);
    if (token == Token("[", TokenType::OPERATOR) || token == Token(".", TokenType::OPERATOR)) {
      result = findVarFromTable(id, token, tokens, index, false);
    } else {
      result = findVar(id);
    }
  } else if (token == Token("[", TokenType::OPERATOR)) {
    result.type = TokenType::TABLE;
    token = nextToken(tokens, index);
    while (token != Token("]", TokenType::OPERATOR)) {
      if (token.type != TokenType::STRING) {
        error = "Expected table key in quotes, got '" + token.toString() + "'!";
        return errorToken;
      }
      std::string key = token.value;
      token = nextToken(tokens, index);
      if (token != Token(":", TokenType::OPERATOR)) {
        error = "Expected ':', got '" + token.toString() + "'!";
        return errorToken;
      }
      token = nextToken(tokens, index);
      Token value = eval(token, tokens, index);
      result.table[key] = value;
      if (token == Token("]", TokenType::OPERATOR)) {
        break;
      }
      if (token != Token(",", TokenType::OPERATOR)) {
        error = "Expected ',' or ')', got '" + token.toString() + "'!";
        return errorToken;
      }
      token = nextToken(tokens, index);
    }
    token = nextToken(tokens, index);
  } else {
    token = nextToken(tokens, index);
  }
  if (token == Token("*", TokenType::OPERATOR) || token == Token("/", TokenType::OPERATOR) || token == Token("%", TokenType::OPERATOR) || token == Token("&&", TokenType::OPERATOR) || token == Token("||", TokenType::OPERATOR) || token == Token("<<", TokenType::OPERATOR) || token == Token(">>", TokenType::OPERATOR)) {
    std::string operation = token.value;
    token = nextToken(tokens, index);
    if (result.type != TokenType::NUMBER) {
      error = "Cannot do math with non-number '" + result.toString() + "'!";
      return errorToken;
    }
    Token bToken = evalScalar(token, tokens, index);
    if (bToken.type != TokenType::NUMBER) {
      error = "Cannot do math with non-number '" + bToken.toString() + "'!";
      return errorToken;
    }
    int a = std::stoi(result.value), b = std::stoi(bToken.value);
    result = Token("", TokenType::NUMBER);
    if (operation == "*") {
      result.value = std::to_string(a * b);
    } else if (operation == "/") {
      result.value = std::to_string(a / b);
    } else if (operation == "%") {
      result.value = std::to_string(a % b);
    } else if (operation == "&&") {
      result.value = std::to_string(a && b);
    } else if (operation == "||") {
      result.value = std::to_string(a || b);
    } else if (operation == "<<") {
      result.value = std::to_string(a << b);
    } else if (operation == ">>") {
      result.value = std::to_string(a >> b);
    }
  }
  return result;
}

Token eval(Token& token, const std::vector<Token>& tokens, size_t& index) {
  Token result = evalScalar(token, tokens, index);
  if (result == errorToken) return errorToken;
  if (token == Token("~", TokenType::OPERATOR)) {  // FIXME: ~
    token = nextToken(tokens, index);
    Token n = eval(token, tokens, index);
    if (n.type != TokenType::NUMBER) {
      error = "Cannot do math with non-number '" + n.toString() + "'!";
      return errorToken;
    }
    result = Token(std::to_string(~std::stoi(n.value)), TokenType::NUMBER);
  } else if (token == Token("+", TokenType::OPERATOR) || token == Token("-", TokenType::OPERATOR) || token == Token("&", TokenType::OPERATOR) || token == Token("|", TokenType::OPERATOR) || token == Token("^", TokenType::OPERATOR) || token == Token(">", TokenType::OPERATOR) || token == Token("<", TokenType::OPERATOR) || token == Token("==", TokenType::OPERATOR) || token == Token("!=", TokenType::OPERATOR)) {
    std::string operation = token.value;
    token = nextToken(tokens, index);
    if (result.type != TokenType::NUMBER) {
      error = "Cannot do math with non-number '" + result.toString() + "'!";
      return errorToken;
    }
    Token bToken = eval(token, tokens, index);
    if (bToken.type != TokenType::NUMBER) {
      error = "Cannot do math with non-number '" + bToken.toString() + "'!";
      return errorToken;
    }
    int a = std::stoi(result.value), b = std::stoi(bToken.value);
    result = Token("", TokenType::NUMBER);
    if (operation == "+") {
      result.value = std::to_string(a + b);
    } else if (operation == "-") {
      result.value = std::to_string(a - b);
    } else if (operation == "&") {
      result.value = std::to_string(a & b);
    } else if (operation == "|") {
      result.value = std::to_string(a | b);
    } else if (operation == "^") {
      result.value = std::to_string(a ^ b);
    } else if (operation == ">") {
      result.value = std::to_string(a > b);
    } else if (operation == "<") {
      result.value = std::to_string(a > b);
    } else if (operation == "==") {
      result.value = std::to_string(a == b);
    } else if (operation == "!=") {
      result.value = std::to_string(a != b);
    }
  }
  return result;
}

bool execute(const std::string& code) {
  error = "";
  std::vector<Token> tokens = tokenize(code);
  if (!error.empty()) return false;
  size_t index = 0;
  while (index < tokens.size()) {
    Token token = tokens[index];
    if (token.type == TokenType::WORD) {
      std::string id = token.value;
      token = nextToken(tokens, index);
      if (token == Token("(", TokenType::OPERATOR)) {
        token = nextToken(tokens, index);
        std::vector<Token> args;
        while (token != Token(")", TokenType::OPERATOR)) {
          args.push_back(eval(token, tokens, index));
          if (!error.empty()) return false;
          if (token == Token(")", TokenType::OPERATOR)) {
            break;
          }
          if (token != Token(",", TokenType::OPERATOR)) {
            error = "Expected ',' or ')', got '" + token.toString() + "'!";
            return false;
          }
          token = nextToken(tokens, index);
        }
        token = nextToken(tokens, index);
        if (token == Token(";", TokenType::OPERATOR)) {
          index++;
          if (functions.count(id)) {
            functions[id](args);
            if (!error.empty()) {
              return false;
            }
            continue;
          } else {
            error = "Function '" + id + "' not found!";
            return false;
          }
        } else {
          error = "Expected ';', got '" + token.toString() + "'!";
          return false;
        }
      } else if (token.type == TokenType::WORD) {
        std::string name = token.value;
        Token value = Token("", TokenType::_EOF);
        token = nextToken(tokens, index);
        if (id != "var") {
          error = "Only 'var', not '" + id + "' type supported!";
          return false;
        }
        if (token == Token("=", TokenType::OPERATOR)) {
          token = nextToken(tokens, index);
          value = eval(token, tokens, index);
          if (!error.empty()) return false;
        }
        if (token != Token(";", TokenType::OPERATOR)) {
          error = "Expected ';', got '" + token.toString() + "'!";
          return false;
        }
        index++;
        if (!createVar(name, value)) return false;
        continue;
      } else if (token == Token("=", TokenType::OPERATOR)) {
        token = nextToken(tokens, index);
        Token value = eval(token, tokens, index);
        if (!error.empty()) return false;
        if (token != Token(";", TokenType::OPERATOR)) {
          error = "Expected ';', got '" + token.toString() + "'!";
          return false;
        }
        index++;
        Token& var = findVar(id);
        if (var == errorToken) return false;
        var = value;
        continue;
      } else if (token == Token("[", TokenType::OPERATOR) || token == Token(".", TokenType::OPERATOR)) {
        Token& var = findVarFromTable(id, token, tokens, index, true);
        if (var == errorToken) return false;
        if (token != Token("=", TokenType::OPERATOR)) {
          error = "Expected '=', got '" + token.toString() + "'!";
          return false;
        }
        token = nextToken(tokens, index);
        var = eval(token, tokens, index);
        if (!error.empty()) return false;
        if (token != Token(";", TokenType::OPERATOR)) {
          error = "Expected ';', got '" + token.toString() + "'!";
          return false;
        }
        index++;
        continue;
      } else {
        error = "Expected '(', '=', '[', '.' or varname, got '" + token.toString() + "'!";
        return false;
      }
    } else if (token == Token("EOF", TokenType::_EOF)) {
      return true;
    }
    error = "Invalid expression: '" + token.toString() + "'!";
    return false;
  }
  error = "Impossible error!";
  return false;
}

bool isStringNumber(std::string str) {
  for (int i = 0; i < str.length(); i++) {
    if (str[i] < '0' || str[i] > '9') {
      return false;
    }
  }
  return true;
}

bool isTrue(Token value) { return value.value != "false" && !value.value.empty(); }

bool checkArgs(const std::string& format, const std::vector<Token>& args) {
  if (args.size() != format.length()) return false;
  for (int i = 0; i < format.length(); i++) {
    if (format[i] == 'w' && args[i].type != TokenType::WORD) return false;
    if (format[i] == 'n' && args[i].type != TokenType::NUMBER) return false;
    if (format[i] == 's' && args[i].type != TokenType::STRING) return false;
    if (format[i] == 'c' && args[i].type != TokenType::CODE) return false;
    if (format[i] == 't' && args[i].type != TokenType::TABLE) return false;
  }
  return true;
}

void weatherPrint(const std::vector<Token>& args) {
  for (int i = 0; i < args.size(); i++) {
    output << args[i].toString();
    if (i < args.size() - 1) output << ", ";
  }
}

void weatherPrintln(const std::vector<Token>& args) {
  weatherPrint(args);
  output << '\n';
}

void weatherIf(const std::vector<Token>& args) {
  if (!checkArgs("_c", args) && !checkArgs("_cc", args)) {
    error = "Usage: if(expr, {code}[, {else code}]);!";
    return;
  }
  pushLayer();
  if (isTrue(args[0])) {
    if (!execute(args[1].value)) return;
  } else if (args.size() == 3) {
    if (!execute(args[2].value)) return;
  }
  popLayer();
}

void weatherFor(const std::vector<Token>& args) {
  if (!checkArgs("snnnc", args)) {
    error = "Usage: for(\"varname\", from, to, step, {code});\nExample: for(\"i\", 0, 10, 1, { println(i); });!";
    return;
  }
  int from = std::stoi(args[1].value);
  int to = std::stoi(args[2].value);
  int step = std::stoi(args[3].value);
  for (int i = from; i != to; i += step) {
    pushLayer();
    if (!createVar(args[0].value, Token(std::to_string(i), TokenType::NUMBER))) return;
    if (!execute(args[4].value)) return;
    popLayer();
  }
}

void weatherForeach(const std::vector<Token>& args) {
  if (!checkArgs("tssc", args)) {
    error = "Usage: foreach(table, \"keyname\", \"valuename\", {code});!";
    return;
  }
  for (const auto& e : args[0].table) {
    pushLayer();
    if (isStringNumber(e.first)) {
      if (!createVar(args[1].value, Token(e.first, TokenType::NUMBER))) return;
    } else {
      if (!createVar(args[1].value, Token(e.first, TokenType::STRING))) return;
    }
    if (!createVar(args[2].value, e.second)) return;
    if (!execute(args[3].value)) return;
    popLayer();
  }
}

void weatherEval(const std::vector<Token>& args) {
  if (!checkArgs("s", args) && !checkArgs("c", args)) {
    error = "Usage: eval(\"code\");!";
    return;
  }
  execute(args[0].value);
}

void init() {
  functions["print"] = weatherPrint;
  functions["println"] = weatherPrintln;
  functions["if"] = weatherIf;
  functions["for"] = weatherFor;
  functions["foreach"] = weatherForeach;
  functions["eval"] = weatherEval;
}
}  // namespace weatherLang
