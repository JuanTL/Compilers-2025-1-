//GRAMMAR
/*
<program> ::= <statement> | <statement> <program>
<statement> ::= <assign> | <command> | <if>
<assign> ::= "let" <ID> "=" <expression> ";"
<command> ::= <extract_frame> | <concatenate> | <extract_audio> | <play>
    <extract_frame> ::= "frame" <expression> <expression> "to" <string> ";"
    <concatenate> ::= "concat" <expression> <expression> "to" <string> ";"
    <extract_audio> ::= "audio" <expression> <expression> <expression> "to" <string> ";"
<play> ::= "play" <expression> ";" | "play" <expression> <expression> <expression> ";"    //Play all OR play from time X to time Y
<if> ::= "if" <condition> "then" <statement>
<condition> ::= <expression> "==" <expression>
<expression> ::= <term> | <term> "+" <expression> | <term> "*" <expression>
<term> ::= <number> | <string> | <time> | <ID>
<string> ::= "\"" <filename> "\""
<number> ::= <integer>
<time> ::= "\"" <integer> ":" <integer> "\""                                            //"Minutes:Seconds" position for time reference
<ID> ::= <alphabetic string>

#  - Single-line comment: # <text> (until end of line)
## - Multi-line comment: ## <text> ## (multi-line, ends at next ##)
*/

//EXAMPLE
/*
frame "video.mp4" 5 to "frame5.bmp";            Å® Extracts frame 5 as a bitmap.
concat "clip1.mp4" "clip2.mp4" to "output.mp4"; Å® Concatenates two clips.
audio "video.mp4" 10 20 to "audio.mp3";         Å® Extracts audio from 10s to 20s.
play "video.mp4";                               Å® Plays the video.
*/

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>


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
};

enum class TokenType { KEYWORD, STRING, NUMBER, TIME, SEMICOLON, TO, END };

struct Token {
    TokenType type;
    std::string value;
};


std::vector<Token> tokenize(const std::string& source) {
    std::vector<Token> tokens;
    size_t i = 0;

    while (i < source.length()) {
        if (std::isspace(source[i])) {
            i++;
            continue;
        }

        // Comments
        if (source[i] == '#') {
            i++; // Skip first '#'
            if (i < source.length() && source[i] == '#') { // Multi-line comment
                i++; // Skip second '#'
                bool foundEnd = false;
                while (i < source.length() - 1) {
                    if (source[i] == '#' && source[i + 1] == '#') {
                        i += 2; // Skip until reach '##'
                        foundEnd = true;
                        break;
                    }
                    i++;
                }
                if (!foundEnd) {
                    throw std::runtime_error("Unterminated multi-line comment");
                }
            }
            else { // Single-line comment
                while (i < source.length() && source[i] != '\n') {
                    i++;
                }
                //Loop to handle whitespace/newline
            }
            continue;
        }

        // Keyword or identifier
        if (std::isalpha(source[i])) {
            std::string word;
            while (i < source.length() && std::isalnum(source[i])) {
                word += source[i++];
            }
            if (word == "to") {
                tokens.push_back({ TokenType::TO, word });
            }
            else {
                tokens.push_back({ TokenType::KEYWORD, word });
            }
            continue;
        }

        // String ("TestVideo.mp4"), time("13:23")
        if (source[i] == '"') {
            std::string str;
            i++;
            while (i < source.length() && source[i] != '"') {
                str += source[i++];
            }
            i++; // Skip closing quote
            if (str.find(':') != std::string::npos) {
                tokens.push_back({ TokenType::TIME, str });
            }
            else {
                tokens.push_back({ TokenType::STRING, str });
            }
            continue;
        }

        // Number
        if (std::isdigit(source[i])) {
            std::string num;
            while (i < source.length() && std::isdigit(source[i])) {
                num += source[i++];
            }
            tokens.push_back({ TokenType::NUMBER, num });
            continue;
        }
        // Semicolon(end line)
        if (source[i] == ';') {
            tokens.push_back({ TokenType::SEMICOLON, ";" });
            i++;
            continue;
        }

        i++; //Skip to next character
    }
    tokens.push_back({ TokenType::END, "" });
    return tokens;
}



//ASTNode Docs 
//(Abstract syntax tree (AST) - DataStructure used to represent the syntactic structure of the code in a programming language.)
/*
//////////////////////////////////////////////////////////////
COMMANDS -> 
"play"   - 
"frame"  -
"concat" -
"audio"  -
"#"      -
//////////////////////////////////////////////////////////////
source1     - Main Input file for "play" and "frame"
source2     - Second input file for operations like "concat".
argStart    - Frame start time
argEnd      - Frame end time
destination - output filename
//////////////////////////////////////////////////////////////
*/
struct ASTNode {
    std::string command;
    std::string source1;
    std::string source2;
    TimePosition argStart;
    TimePosition argEnd;
    std::string destination;
};

class Parser {
    std::vector<Token> tokens;
    size_t pos;

public:
    Parser(const std::vector<Token>& t) : tokens(t), pos(0) {}

    ASTNode parseStatement() {
        if (tokens[pos].type == TokenType::KEYWORD) {
            std::string cmd = tokens[pos].value;
            pos++;
            //Define how commands should be written HERE!
            if (cmd == "play") {
                if (tokens[pos].type == TokenType::STRING) {
                    ASTNode node{ "play", tokens[pos].value, "", TimePosition(), TimePosition(), "" };
                    pos++;
                    if (tokens[pos].type == TokenType::SEMICOLON) pos++;
                    return node;
                }
            }
            else if (cmd == "frame") {
                if (tokens[pos].type == TokenType::STRING &&
                    tokens[pos + 1].type == TokenType::NUMBER &&
                    tokens[pos + 2].type == TokenType::TO &&
                    tokens[pos + 3].type == TokenType::STRING) {
                    ASTNode node{ "frame", tokens[pos].value, "", TimePosition(std::stoi(tokens[pos + 1].value), 0), TimePosition(), tokens[pos + 3].value };
                    pos += 4;
                    if (tokens[pos].type == TokenType::SEMICOLON) pos++;
                    return node;
                }
            }
            else if (cmd == "concat") {
                if (tokens[pos].type == TokenType::STRING &&
                    tokens[pos + 1].type == TokenType::STRING &&
                    tokens[pos + 2].type == TokenType::TO &&
                    tokens[pos + 3].type == TokenType::STRING) {
                    ASTNode node{ "concat", tokens[pos].value, tokens[pos + 1].value, TimePosition(), TimePosition(), tokens[pos + 3].value };
                    pos += 4;
                    if (tokens[pos].type == TokenType::SEMICOLON) pos++;
                    return node;
                }
            }
            else if (cmd == "audio") {
                if (tokens[pos].type == TokenType::STRING &&
                    tokens[pos + 1].type == TokenType::TIME &&
                    tokens[pos + 2].type == TokenType::TIME &&
                    tokens[pos + 3].type == TokenType::TO &&
                    tokens[pos + 4].type == TokenType::STRING) {
                    ASTNode node{ "audio", tokens[pos].value, "", TimePosition(tokens[pos + 1].value), TimePosition(tokens[pos + 2].value), tokens[pos + 4].value };
                    pos += 5;
                    if (tokens[pos].type == TokenType::SEMICOLON) pos++;
                    return node;
                }
            }
        }
        throw std::runtime_error("Syntax error at position " + std::to_string(pos));
    }

    bool hasMoreStatements() { return tokens[pos].type != TokenType::END; }
};
//Define commands actions HERE!
void execute(const ASTNode& node) {
    if (node.command == "play") {
        //Defaults to VLC player as it is at the laboratory computers as well.
        std::string command = "vlc " + node.source1;
        std::cout << "Executing: " << command << "\n";
        //Execution via syscall
        system(command.c_str());
    }
    //Example using FFmpeg for audio extraction, probably should be used for Video as well.
    else if (node.command == "audio") {
        
        std::string command = "ffmpeg -i " + node.source1 + " -ss " + node.argStart.toString() +
            " -to " + node.argEnd.toString() + " -vn -acodec mp3 " + node.destination; 
        
        std::cout << "Executing: " << command << "\n";
        system(command.c_str());
    }
    // Add commands HERE!
}

int main() {
    std::string source = R"(
        # Extract a frame from video
        frame "video.mp4" 5 to "frame5.bmp";
        ## Multi-line comment Test
           Just another line
           And another... 
           Okay, because... why not?
           Next command a concatenation, btw.
        ##
        concat "clip1.mp4" "clip2.mp4" to "output.mp4";
        audio "video.mp4" "00:10" "00:20" to "audio.mp3";
        # Play the result
        play "output.mp4";
    )";
    auto tokens = tokenize(source);

    // Debug: Print tokens
    for (const auto& token : tokens) {
        std::cout << "Token: " << (int)token.type << " " << token.value << "\n";
    }

    Parser parser(tokens);
    while (parser.hasMoreStatements()) {
        try {
            auto ast = parser.parseStatement();
            execute(ast);
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
            break;
        }
    }

    return 0;
}