#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_map>

//GRAMMAR
/*
program        -> statement program'
program'       -> statement program' | ε
statement      -> assign | command | if_stmt
assign         -> let ID = expression ;
command        -> extract_frame | concatenate | extract_audio | play
extract_frame  -> frame expression expression to string ;
concatenate    -> concat expression expression to string ;
extract_audio  -> audio expression expression expression to string ;
play           -> play expression play_args
play_args      -> ; | expression expression ;
if_stmt        -> if condition then statement
condition      -> expression == expression
expression     -> term expression'
expression'    -> + term expression' | * term expression' | ε
term           -> number | string | time | ID
string         -> STRING
number         -> NUMBER
time           -> TIME
ID             -> IDENTIFIER
*/

//EXAMPLE
/*
frame "video.mp4" 5 to "frame5.bmp";              Extracts frame 5 as a bitmap.
concat "clip1.mp4" "clip2.mp4" to "output.mp4";   Concatenates two clips.
audio "video.mp4" 10 20 to "audio.mp3";           Extracts audio from 10s to 20s.
play "video.mp4";                                 Plays the video.
*/


//STRUCT TO HAVE CONSISTENT TIME INPUT (MINUTES:SECONDS)
struct TimePosition {
    int minutes;
    int seconds;
    TimePosition(int min = 0, int seg = 0) : minutes(min), seconds(seg) {
        normalize();
    }
    // Parse "MM:SS" format
    explicit TimePosition(const std::string& timeStr) {
        size_t colonPos = timeStr.find(':');
        //string::npos - When used as the value for a len parameter in string's member functions, means "until the end of the string".
        if (colonPos == std::string::npos) {
            throw std::invalid_argument("Invalid time format: " + timeStr);
        }
        minutes = std::stoi(timeStr.substr(0, colonPos));
        seconds = std::stoi(timeStr.substr(colonPos + 1));
        normalize();
    }
    void normalize() {
        if (seconds >= 60) {
            minutes += seconds / 60;
            seconds %= 60;
        }
        if (minutes < 0 || seconds < 0) {
            throw std::invalid_argument("Time cannot be negative");
        }
    }
    double toSeconds() const {
        return minutes * 60.0 + seconds;
    }
    std::string toString() const {
        return std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
    }
    TimePosition operator+(const TimePosition& other) const {
        return TimePosition(toSeconds() + other.toSeconds());
    }
    TimePosition operator*(int n) const {
        return TimePosition(toSeconds() * n);
    }
    bool operator==(const TimePosition& other) const {
        return toSeconds() == other.toSeconds();
    }

};

// Updated TokenType enum
enum class TokenType {
    ID, ASSIGN_OP, INT, ADD_OP, MUL_OP, PRINT_KEY, OPEN_PAR, CLOSE_PAR, EOP,
    KEYWORD, STRING, NUMBER, TIME, SEMICOLON, TO, LET, IF, THEN, EQUALS, END
};

std::string TokenTypeLiteral[] = {
    "ID", "ASSIGN_OP", "INT", "ADD_OP", "MUL_OP", "PRINT_KEY", "OPEN_PAR", "CLOSE_PAR", "EOP",
    "KEYWORD", "STRING", "NUMBER", "TIME", "SEMICOLON", "TO", "LET", "IF", "THEN", "EQUALS", "END"
};
struct Token {
    TokenType type;
    std::string value;
    int line;
    int charPos;
};

//STRUCT TO MARK ERRORS IN THE SCANNER
struct ScannerError {
    int line;
    int charPos;
    std::string type;
    std::string message;
};

std::vector<Token> tokenize(const std::string& source, std::vector<ScannerError>& errors) {
    std::vector<Token> tokens;
    size_t i = 0;
    int currentLine = 1;
    int charPosInLine = 1;

    while (i < source.length()) {
        if (source[i] == '\n') {
            currentLine++;
            charPosInLine = 1;
            i++;
            continue;
        }
        if (std::isspace(source[i])) {
            charPosInLine++;
            i++;
            continue;
        }

        // Comments
        if (source[i] == '#') {
            i++;
            charPosInLine++;
            if (i < source.length() && source[i] == '#') {
                i++;
                charPosInLine++;
                bool foundEnd = false;
                while (i < source.length() - 1) {
                    if (source[i] == '\n') {
                        currentLine++;
                        charPosInLine = 1;
                    }
                    else {
                        charPosInLine++;
                    }
                    if (source[i] == '#' && source[i + 1] == '#') {
                        i += 2;
                        charPosInLine += 2;
                        foundEnd = true;
                        break;
                    }
                    i++;
                }
                if (!foundEnd) {
                    errors.push_back({ currentLine, charPosInLine, "UnterminatedComment", "Unterminated multi-line comment" });
                }
            }
            else {
                while (i < source.length() && source[i] != '\n') {
                    i++;
                    charPosInLine++;
                }
            }
            continue;
        }
        /*
        // Keyword or identifier
        if (std::isalpha(source[i])) {
            std::string word;
            int startPos = charPosInLine;
            while (i < source.length() && std::isalnum(source[i])) {
                word += source[i++];
                charPosInLine++;
            }
            if (word == "print") {
                tokens.push_back({ TokenType::PRINT_KEY, word, currentLine, startPos });
            }
            else if (word == "to") tokens.push_back({ TokenType::TO,  word, currentLine, startPos });
            else if (word == "let" || word == "frame" || word == "concat" || word == "audio" || word == "play") {
                tokens.push_back({ TokenType::KEYWORD,  word, currentLine, startPos });
            }
            else if (word == "if") tokens.push_back({ TokenType::IF, word , currentLine, startPos });
            else if (word == "then") tokens.push_back({ TokenType::THEN,  word, currentLine, startPos });
            else tokens.push_back({ TokenType::ID,  word, currentLine, startPos });
            continue;
        }
        */
        // Identifiers and keywords
        if (std::isalpha(source[i])) {
            std::string word;
            int startPos = charPosInLine;
            while (i < source.length() && std::isalnum(source[i])) {
                word += source[i++];
                charPosInLine++;
            }
            if (word == "print") {
                tokens.push_back({ TokenType::PRINT_KEY, word, currentLine, startPos });
            }
            else if (word == "let") {
                tokens.push_back({ TokenType::LET, word, currentLine, startPos });
            }
            else if (word == "if") {
                tokens.push_back({ TokenType::IF, word, currentLine, startPos });
            }
            else if (word == "then") {
                tokens.push_back({ TokenType::THEN, word, currentLine, startPos });
            }
            else if (word == "to") {
                tokens.push_back({ TokenType::TO, word, currentLine, startPos });
            }
            else if (word == "frame" || word == "concat" || word == "audio" || word == "play") {
                tokens.push_back({ TokenType::KEYWORD, word, currentLine, startPos });
            }
            else {
                tokens.push_back({ TokenType::ID, word, currentLine, startPos });
            }
            continue;
        }
        if (source[i] == '"') {
            std::string str;
            int startPos = charPosInLine;
            i++;
            charPosInLine++;
            int startLine = currentLine;
            while (i < source.length() && source[i] != '"') {
                if (source[i] == '\n') {
                    currentLine++;
                    charPosInLine = 1;
                }
                else {
                    charPosInLine++;
                }
                str += source[i++];
            }
            if (i >= source.length()) {
                errors.push_back({ startLine, startPos, "UnclosedString", "Unclosed string literal" });
                continue;
            }
            i++;
            charPosInLine++;
            if (str.find(':') != std::string::npos) {
                try {
                    TimePosition time(str);
                    tokens.push_back({ TokenType::TIME, str, startLine, startPos });
                }
                catch (const std::exception& e) {
                    errors.push_back({ startLine, startPos, "InvalidTime", "Invalid time format: " + str });
                }
            }
            else {
                if (str.empty()) {
                    errors.push_back({ startLine, startPos, "EmptyString", "Empty string literal" });
                }
                else {
                    tokens.push_back({ TokenType::STRING, str, startLine, startPos });
                }
            }
            continue;
        }

        // Number (INT)
        if (std::isdigit(source[i])) {
            std::string num;
            int startPos = charPosInLine;
            while (i < source.length() && std::isdigit(source[i])) {
                num += source[i++];
                charPosInLine++;
            }
            tokens.push_back({ TokenType::INT, num, currentLine, startPos });
            continue;
        }

        // Operators
        if (source[i] == '=') {
            if (i + 1 < source.length() && source[i + 1] == '=') {
                tokens.push_back({ TokenType::EQUALS, "==", currentLine, charPosInLine });
                i += 2;
                charPosInLine += 2;
            }
            else {
                tokens.push_back({ TokenType::ASSIGN_OP, "=", currentLine, charPosInLine });
                i++;
                charPosInLine++;
            }
            continue;
        }
        if (source[i] == '+') {
            tokens.push_back({ TokenType::ADD_OP, "+", currentLine, charPosInLine });
            i++;
            charPosInLine++;
            continue;
        }
        if (source[i] == '*') {
            tokens.push_back({ TokenType::MUL_OP, "*", currentLine, charPosInLine });
            i++;
            charPosInLine++;
            continue;
        }
        if (source[i] == '(') {
            tokens.push_back({ TokenType::OPEN_PAR, "(", currentLine, charPosInLine });
            i++;
            charPosInLine++;
            continue;
        }
        if (source[i] == ')') {
            tokens.push_back({ TokenType::CLOSE_PAR, ")", currentLine, charPosInLine });
            i++;
            charPosInLine++;
            continue;
        }
        if (source[i] == ';') {
            tokens.push_back({ TokenType::SEMICOLON, ";", currentLine, charPosInLine });
            i++;
            charPosInLine++;
            continue;
        }
        if (source[i] == '$') {
            tokens.push_back({ TokenType::EOP, "$", currentLine, charPosInLine });
            i++;
            charPosInLine++;
            continue;
        }

        // Invalid character
        errors.push_back({ currentLine, charPosInLine, "InvalidCharacter", "Unexpected character: " + std::string(1, source[i]) });
        i++;
        charPosInLine++;
    }
    tokens.push_back({ TokenType::EOP, "", currentLine, charPosInLine });
    return tokens;
}
bool scanAndLog(const std::string& source) {
    std::cout << "INFO SCAN - Start scanningc\n";
    std::vector<ScannerError> errors;
    auto tokens = tokenize(source, errors);

    for (const auto& token : tokens) {
        if (token.type != TokenType::EOP || !token.value.empty()) {
            std::cout << "DEBUG SCAN - " << TokenTypeLiteral[(int)token.type]
                << " [ " << token.value << " ] found at ("
                << token.line << ":" << token.charPos << ")\n";
        }
    }

    if (errors.empty()) {
        std::cout << "INFO SCAN - Completed with 0 errors\n";
		return true;
    }
    else {
        std::cout << "INFO SCAN - Completed with " << errors.size() << " errors\n";
        for (const auto& err : errors) {
            std::cerr << "ERROR SCAN - Line " << err.line << ":" << err.charPos
                << ", type: " << err.type << " - " << err.message << "\n";
        }
		return false;
    }
}




