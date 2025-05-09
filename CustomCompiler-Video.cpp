
#include "Parser.h"


void read(std::string direc, std::string& out) {
    std::ifstream file(direc);

    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << direc << std::endl;
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Lee todo el archivo de una vez
    std::string line = buffer.str();
    out = line;

    scanAndLog(line);

    file.close();
}

int main() {

    /*
	// PREVIOUS TESTS
    std::string source;

    read("C:/Users/alumno-m/Desktop/input.txt", source);
    
    scanAndLog(source);
    */
	// --- TESTS ---

	//NO ERRORS
    std::string source0 = R"(
    frame "video.mp4" 10 to "frame10.bmp";
    concat "clip1.mp4" "clip2.mp4" to "output.mp4";
    audio "video.mp4" "00:10" "00:20" to "audio.mp3";
    play "output.mp4";
    
    )";
    //ERROR Semicolon
    std::string source1 = R"(
    let start = "00:10"
    frame "video.mp4" 5 to "frame5.bmp";
    play "video.mp4";
    )";
	//ERROR Command
    std::string source2 = R"(
    let file = "video";
    invalid "video.mp4"; # Unknown command
    concat file + ".mp4" "clip2.mp4" to "output.mp4";
    )";
    //ERROR Expression
    std::string source = R"(
    let duration = "00:05";
    audio "video.mp4" duration + + "00:10" to "audio.mp3"; # Invalid expression
    if duration == "00:05" then play "video.mp4";
    )";


    std::vector<ScannerError> errors;
    auto tokens = tokenize(source, errors);
    /*
    for (const auto& token : tokens) {
        std::cout << "Token: " << TokenTypeLiteral[(int)token.type] << " " << token.value << "\n";
    }
    */
    
    std::cout << "----------------------" << "\n";
    std::cout << "Token List size: " << tokens.size() << "\n";
    std::cout << "----------------------" << "\n";
    Parser parser(tokens);
    try {
        if (tokens.empty()) {
            std::cerr << "Error: No tokens generated from the source code.\n";
            return 1;
        }
        parser.parseAndExecute();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
    }
    return 0;
}