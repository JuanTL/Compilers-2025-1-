#pragma once

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


#include "Scanner.h"




// Represents a value in the language (number, string, or time).
struct Value {
    enum { NUMBER, STRING, TIME } type;
    int num;
    std::string str;
    TimePosition time;
    Value(int n = 0) : type(NUMBER), num(n), str(""), time(0, 0) {}
    Value(const std::string& s) : type(STRING), num(0), str(s), time(0, 0) {}
    Value(const TimePosition& t) : type(TIME), num(0), str(""), time(t) {}
};

struct ASTNode {
    std::string command; // let, frame, concat, audio, play, if
    std::string varName; // For let
    std::vector<Token> expr1;
    std::vector<Token> expr2;
    std::vector<Token> expr3;
    std::string destination; // Output file
    std::vector<ASTNode*> statements; // For program
    //ASTNode* thenStmt; // For if statements



    ASTNode(const std::string& cmd = "", const std::string& var = "",
        const std::vector<Token>& e1 = {}, const std::vector<Token>& e2 = {},
        const std::vector<Token>& e3 = {}, const std::string& dest = "")
        : command(cmd), varName(var), expr1(e1), expr2(e2), expr3(e3), destination(dest) {
    }



    // Copy constructor
    ASTNode(const ASTNode& other)
        : command(other.command), varName(other.varName),
        expr1(other.expr1), expr2(other.expr2), expr3(other.expr3),
        destination(other.destination) {
        for (const auto* stmt : other.statements) {
            statements.push_back(new ASTNode(*stmt)); // Deep copy
        }
    }

    ~ASTNode() {
        for (auto* stmt : statements) {
            delete stmt;
        }
    }
    // Copy assignment
    ASTNode& operator=(const ASTNode& other) {
        if (this != &other) {
            for (auto* stmt : statements) {
                delete stmt;
            }
            statements.clear();
            command = other.command;
            varName = other.varName;
            expr1 = other.expr1;
            expr2 = other.expr2;
            expr3 = other.expr3;
            destination = other.destination;
            for (const auto* stmt : other.statements) {
                statements.push_back(new ASTNode(*stmt));
            }
        }
        return *this;
    }

    // Move constructor
    ASTNode(ASTNode&& other) noexcept
        : command(std::move(other.command)), varName(std::move(other.varName)),
        expr1(std::move(other.expr1)), expr2(std::move(other.expr2)),
        expr3(std::move(other.expr3)), destination(std::move(other.destination)),
        statements(std::move(other.statements)) {
        other.statements.clear();
    }
    // Move assignment
    ASTNode& operator=(ASTNode&& other) noexcept {
        if (this != &other) {
            for (auto* stmt : statements) {
                delete stmt;
            }
            statements.clear();
            command = std::move(other.command);
            varName = std::move(other.varName);
            expr1 = std::move(other.expr1);
            expr2 = std::move(other.expr2);
            expr3 = std::move(other.expr3);
            destination = std::move(other.destination);
            statements = std::move(other.statements);
            other.statements.clear();
        }
        return *this;
    }
    // Constructor for ASTNode to initialize all fields

};

class Parser {
    std::vector<Token> tokens;
    size_t pos;
    std::unordered_map<std::string, Value> variables;
    std::vector<ScannerError> errors;

    //PANIC MODE FUNCTIONS

    // Check if token matches type without advancing
    bool check(TokenType type) const {
        return pos < tokens.size() && tokens[pos].type == type;
    }
    // Advance to next token
    void advance() { if (pos < tokens.size()) pos++; }
    // Synchronize to next statement or EOP
    void synchronize() {
        while (pos < tokens.size() && tokens[pos].type != TokenType::EOP) {
            if (tokens[pos].type == TokenType::SEMICOLON) {
                advance(); // Move past semicolon
                return;
            }
            if (tokens[pos].type == TokenType::LET || tokens[pos].type == TokenType::IF ||
                tokens[pos].type == TokenType::KEYWORD) {
                return; // Ready for next statement
            }
            advance();
        }
    }

    //PARSER FUNCTIONS
    bool expect(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        errors.push_back({ tokens[pos].line, tokens[pos].charPos, "UnexpectedToken",
                          "Expected " + TokenTypeLiteral[(int)type] + ", got " +
                          (pos < tokens.size() ? tokens[pos].value : "EOF") });
        synchronize();
        return false;
    }

    std::vector<Token> parseExpression() {
        std::vector<Token> expr;
        if (check(TokenType::OPEN_PAR)) {
            advance();
            expr = parseExpression();
            if (!expect(TokenType::CLOSE_PAR)) return {};
        }
        else {
            if (!(check(TokenType::INT) || check(TokenType::STRING) ||
                check(TokenType::TIME) || check(TokenType::ID))) {
                errors.push_back({ tokens[pos].line, tokens[pos].charPos, "InvalidExpression",
                                  "Expected number, string, time, or identifier" });
                synchronize();
                return {};
            }
            expr.push_back(tokens[pos++]);
        }
        while (pos < tokens.size() && (check(TokenType::ADD_OP) || check(TokenType::MUL_OP))) {
            expr.push_back(tokens[pos++]);
            if (check(TokenType::OPEN_PAR)) {
                advance();
                auto subExpr = parseExpression();
                if (!expect(TokenType::CLOSE_PAR)) return {};
                expr.insert(expr.end(), subExpr.begin(), subExpr.end());
            }
            else {
                if (!(check(TokenType::INT) || check(TokenType::STRING) ||
                    check(TokenType::TIME) || check(TokenType::ID))) {
                    errors.push_back({ tokens[pos].line, tokens[pos].charPos, "InvalidExpression",
                                      "Expected number, string, time, or identifier" });
                    synchronize();
                    return {};
                }
                expr.push_back(tokens[pos++]);
            }
        }
        return expr;
    }

    Value evaluate(const std::vector<Token>& expr) {
        if (expr.size() == 1) {
            if (expr[0].type == TokenType::INT) return Value(std::stoi(expr[0].value));
            if (expr[0].type == TokenType::STRING) return Value(expr[0].value);
            if (expr[0].type == TokenType::TIME) return Value(TimePosition(expr[0].value));
            if (expr[0].type == TokenType::ID && variables.count(expr[0].value)) return variables[expr[0].value];
            errors.push_back({ expr[0].line, expr[0].charPos, "UnknownIdentifier", "Unknown identifier: " + expr[0].value });
            throw std::runtime_error("Unknown identifier: " + expr[0].value);
        }

        Value result = evaluate({ expr[0] });
        for (size_t i = 1; i < expr.size(); i += 2) {
            Token op = expr[i];
            Value rhs = evaluate({ expr[i + 1] });
            if (op.type == TokenType::ADD_OP) {
                if (result.type == Value::STRING && rhs.type == Value::STRING) {
                    result.str += rhs.str;
                }
                else if (result.type == Value::TIME && rhs.type == Value::TIME) {
                    result.time = result.time + rhs.time;
                }
                else {
                    errors.push_back({ op.line, op.charPos, "TypeError", "Invalid + operands" });
                    throw std::runtime_error("Invalid + operands");
                }
            }
            else if (op.type == TokenType::MUL_OP) {
                if (result.type == Value::TIME && rhs.type == Value::NUMBER) {
                    result.time = result.time * rhs.num;
                }
                else if (result.type == Value::NUMBER && rhs.type == Value::TIME) {
                    result.time = rhs.time * result.num;
                }
                else {
                    errors.push_back({ op.line, op.charPos, "TypeError", "Multiplication only defined for time * number" });
                    throw std::runtime_error("Multiplication only defined for time * number");
                }
            }
        }
        return result;
    }

    ASTNode parseProgram() {
        ASTNode root{ "program" };
        while (pos < tokens.size() && tokens[pos].type != TokenType::EOP) {
            try {
                ASTNode* stmt = new ASTNode(parseStatement());
                root.statements.push_back(stmt);
            }
            catch (...) {
                synchronize();
            }
        }
        if (check(TokenType::EOP)) advance();
        return root;
    }

    ASTNode parseStatement() {
        if (check(TokenType::LET)) {
            return parseAssign();
        }
        else if (check(TokenType::IF)) {
            return parseIfStmt();
        }
        else if (check(TokenType::KEYWORD)) {
            return parseCommand();
        }
        errors.push_back({ tokens[pos].line, tokens[pos].charPos, "InvalidStatement",
                          "Expected let, if, or command" });
        synchronize();
        return { "error" }; // Placeholder node
    }
    ASTNode parseAssign() {
        if (!expect(TokenType::LET)) return { "error" };
        std::string varName = tokens[pos].value;
        if (!expect(TokenType::ID)) return { "error" };
        if (!expect(TokenType::ASSIGN_OP)) return { "error" };
        auto expr = parseExpression();
        if (!expect(TokenType::SEMICOLON)) return { "error" };
        if (!expr.empty()) variables[varName] = evaluate(expr);
        return { "let", varName, expr, {}, {}, "" };
    }

    ASTNode parseCommand() {
        std::string cmd = tokens[pos].value;
        if (!expect(TokenType::KEYWORD)) return { "error" };
        if (cmd == "frame") {
            auto expr1 = parseExpression();
            if (expr1.empty()) return { "error" };
            auto expr2 = parseExpression();
            if (expr2.empty()) return { "error" };
            if (!expect(TokenType::TO)) return { "error" };
            std::string dest = tokens[pos].value;
            if (!expect(TokenType::STRING)) return { "error" };
            if (!expect(TokenType::SEMICOLON)) return { "error" };
            return { cmd, "", expr1, expr2, {}, dest };
        }
        else if (cmd == "concat") {
            auto expr1 = parseExpression();
            if (expr1.empty()) return { "error" };
            auto expr2 = parseExpression();
            if (expr2.empty()) return { "error" };
            if (!expect(TokenType::TO)) return { "error" };
            std::string dest = tokens[pos].value;
            if (!expect(TokenType::STRING)) return { "error" };
            if (!expect(TokenType::SEMICOLON)) return { "error" };
            return { cmd, "", expr1, expr2, {}, dest };
        }
        else if (cmd == "audio") {
            auto expr1 = parseExpression();
            if (expr1.empty()) return { "error" };
            auto expr2 = parseExpression();
            if (expr2.empty()) return { "error" };
            auto expr3 = parseExpression();
            if (expr3.empty()) return { "error" };
            if (!expect(TokenType::TO)) return { "error" };
            std::string dest = tokens[pos].value;
            if (!expect(TokenType::STRING)) return { "error" };
            if (!expect(TokenType::SEMICOLON)) return { "error" };
            return { cmd, "", expr1, expr2, expr3, dest };
        }
        else if (cmd == "play") {
            auto expr1 = parseExpression();
            if (expr1.empty()) return { "error" };
            if (check(TokenType::SEMICOLON)) {
                expect(TokenType::SEMICOLON);
                return { cmd, "", expr1, {}, {}, "" };
            }
            auto expr2 = parseExpression();
            if (expr2.empty()) return { "error" };
            auto expr3 = parseExpression();
            if (expr3.empty()) return { "error" };
            if (!expect(TokenType::SEMICOLON)) return { "error" };
            return { cmd, "", expr1, expr2, expr3, "" };
        }
        errors.push_back({ tokens[pos - 1].line, tokens[pos - 1].charPos, "UnknownCommand", "Unknown command: " + cmd });
        synchronize();
        return { "error" };
    }

    ASTNode parseIfStmt() {
        if (!expect(TokenType::IF)) return { "error" };
        auto expr1 = parseExpression();
        if (expr1.empty()) return { "error" };
        if (!expect(TokenType::EQUALS)) return { "error" };
        auto expr2 = parseExpression();
        if (expr2.empty()) return { "error" };
        if (!expect(TokenType::THEN)) return { "error" };
        ASTNode* stmt = new ASTNode(parseStatement());
        ASTNode node{ "if", "", expr1, expr2, {}, "" };
        node.statements.push_back(stmt);
        return node;
    }


    std::string exprToString(const std::vector<Token>& expr) const {
        std::string result;
        for (const auto& token : expr) {
            result += token.value + " ";
        }
        return result.empty() ? "" : result.substr(0, result.size() - 1);
    }
    void printAST(const ASTNode& node, std::ofstream& out, const std::string& parent = "") const {
        // Generate unique node ID to avoid name clashes
        static int nodeCounter = 0;
        std::string nodeId = "node_" + std::to_string(nodeCounter++);
        
        // Create the main command node
        std::string commandName = node.command;
        if (node.command == "error") {
            commandName = "ERROR";
        }
        
        // Write main command node
        if (parent.empty()) {
            out << nodeId << " = Node(\"" << commandName << "\")\n";
        }
        else {
            out << nodeId << " = Node(\"" << commandName << "\", parent=" << parent << ")\n";
        }
        

        if (node.command == "let") {
 
            std::string varNodeId = "node_" + std::to_string(nodeCounter++);
            std::string exprNodeId = "node_" + std::to_string(nodeCounter++);
            
            out << varNodeId << " = Node(\"var: " << node.varName << "\", parent=" << nodeId << ")\n";
            out << exprNodeId << " = Node(\"expr: " << exprToString(node.expr1) << "\", parent=" << nodeId << ")\n";
        }
        else if (node.command == "if") {

            std::string expr1NodeId = "node_" + std::to_string(nodeCounter++);
            std::string expr2NodeId = "node_" + std::to_string(nodeCounter++);
            
            out << expr1NodeId << " = Node(\"left: " << exprToString(node.expr1) << "\", parent=" << nodeId << ")\n";
            out << expr2NodeId << " = Node(\"right: " << exprToString(node.expr2) << "\", parent=" << nodeId << ")\n";
        }
        else if (node.command == "frame" || node.command == "concat") {

            std::string expr1NodeId = "node_" + std::to_string(nodeCounter++);
            std::string expr2NodeId = "node_" + std::to_string(nodeCounter++);
            std::string destNodeId = "node_" + std::to_string(nodeCounter++);
            
            out << expr1NodeId << " = Node(\"arg1: " << exprToString(node.expr1) << "\", parent=" << nodeId << ")\n";
            out << expr2NodeId << " = Node(\"arg2: " << exprToString(node.expr2) << "\", parent=" << nodeId << ")\n";
            out << destNodeId << " = Node(\"dest: " << node.destination << "\", parent=" << nodeId << ")\n";
        }
        else if (node.command == "audio") {

            std::string expr1NodeId = "node_" + std::to_string(nodeCounter++);
            std::string expr2NodeId = "node_" + std::to_string(nodeCounter++);
            std::string expr3NodeId = "node_" + std::to_string(nodeCounter++);
            std::string destNodeId = "node_" + std::to_string(nodeCounter++);
            
            out << expr1NodeId << " = Node(\"arg1: " << exprToString(node.expr1) << "\", parent=" << nodeId << ")\n";
            out << expr2NodeId << " = Node(\"arg2: " << exprToString(node.expr2) << "\", parent=" << nodeId << ")\n";
            out << expr3NodeId << " = Node(\"arg3: " << exprToString(node.expr3) << "\", parent=" << nodeId << ")\n";
            out << destNodeId << " = Node(\"dest: " << node.destination << "\", parent=" << nodeId << ")\n";
        }
        else if (node.command == "play") {

            std::string expr1NodeId = "node_" + std::to_string(nodeCounter++);
            out << expr1NodeId << " = Node(\"arg1: " << exprToString(node.expr1) << "\", parent=" << nodeId << ")\n";
            

            if (!node.expr2.empty()) {
                std::string expr2NodeId = "node_" + std::to_string(nodeCounter++);
                std::string expr3NodeId = "node_" + std::to_string(nodeCounter++);
                
                out << expr2NodeId << " = Node(\"arg2: " << exprToString(node.expr2) << "\", parent=" << nodeId << ")\n";
                out << expr3NodeId << " = Node(\"arg3: " << exprToString(node.expr3) << "\", parent=" << nodeId << ")\n";
            }
        }
        
        // Recursively print child statements
        for (size_t i = 0; i < node.statements.size(); ++i) {
            printAST(*node.statements[i], out, nodeId);
        }
    }

public:
    Parser(const std::vector<Token>& t) : tokens(t), pos(0) {}
    /*
    void parseAndExecute() {
        if (!errors.empty()) {
            for (const auto& err : errors) {
                std::cerr << "Error at line " << err.line << ", col " << err.charPos << ": " << err.type << " - " << err.message << "\n";
            }
            errors.clear();
        }
        ASTNode program = parseProgram();
        if (!errors.empty()) {
            for (const auto& err : errors) {
                std::cerr << "Error at line " << err.line << ", col " << err.charPos << ": " << err.type << " - " << err.message << "\n";
            }
        }
        else {
            // Generate AST.txt if parsing succeeded
            std::ofstream out("AST.py");
            if (out.is_open()) {
                out << "from anytree import Node\n\n";
                printAST(program, out);
                out.close();
            }
            //execute(program);
        }
    }
    */
    void parseAndExecute() {
        if (!errors.empty()) {
            for (const auto& err : errors) {
                std::cerr << "Error at line " << err.line << ", col " << err.charPos << ": "
                    << err.type << " - " << err.message << "\n";
            }
            errors.clear();
        }

        ASTNode program = parseProgram();

        if (!errors.empty()) {
            for (const auto& err : errors) {
                std::cerr << "Error at line " << err.line << ", col " << err.charPos << ": "
                    << err.type << " - " << err.message << "\n";
            }
        }
        else {
            // Write AST visualization (tree form)
            std::ofstream treeOut("AST.py");
            if (treeOut.is_open()) {
                treeOut << "from anytree import Node\n\n";
                printAST(program, treeOut);
                treeOut.close();
            }

            // Write executable Python code
            std::ofstream pyOut("generated_video_script.py");
            if (pyOut.is_open()) {
                translateToPython(program, pyOut);
                pyOut.close();
                std::cout << "Generated Python script: generated_video_script.py\n";
            }
        }
    }

    // Video Operations Python

    void translateToPython(const ASTNode& node, std::ofstream& out) {
        if (node.command == "program") {
            out << "import ffmpeg\n";
            out << "import subprocess\n\n";
            for (const auto* stmt : node.statements) {
                translateToPython(*stmt, out);
                out << "\n";
            }
            return;
        }

        if (node.command == "play") {
            std::string file = exprToString(node.expr1);
            out << "subprocess.run([\"vlc\", \"" << file << "\"";
            if (!node.expr2.empty()) {
                std::string start = exprToString(node.expr2);
                std::string end = exprToString(node.expr3);
                out << ", \"--start-time\", \"" << start << "\", \"--stop-time\", \"" << end << "\"";
            }
            out << "])\n";
        }
        else if (node.command == "frame") {
            std::string input = exprToString(node.expr1);
            std::string frameNum = exprToString(node.expr2);
            out << "ffmpeg.input(\"" << input << "\")"
                << ".filter(\"select\", \"eq(n\\\\," << frameNum << ")\")"
                << ".output(\"" << node.destination << "\", vframes=1).run()\n";
        }
        else if (node.command == "concat") {
            std::string input1 = exprToString(node.expr1);
            std::string input2 = exprToString(node.expr2);
            std::string dest = node.destination;

            out << "# Convert inputs\n";
            out << "ffmpeg.input(\"" << input1 << "\").output(\"converted_0.mp4\", vcodec='libx264', acodec='aac').run()\n";
            out << "ffmpeg.input(\"" << input2 << "\").output(\"converted_1.mp4\", vcodec='libx264', acodec='aac').run()\n\n";

            out << "# Write concat file list\n";
            out << "with open('files.txt', 'w') as f:\n";
            out << "    f.write(\"file 'converted_0.mp4'\\n\")\n";
            out << "    f.write(\"file 'converted_1.mp4'\\n\")\n\n";

            out << "# Concatenate with concat demuxer\n";
            out << "subprocess.run(['ffmpeg', '-f', 'concat', '-safe', '0', '-i', 'files.txt', '-c', 'copy', '" << dest << "'])\n";
        }


        else if (node.command == "audio") {
            std::string input = exprToString(node.expr1);
            std::string start = exprToString(node.expr2);
            std::string end = exprToString(node.expr3);
            std::string dest = node.destination;

            out << "ffmpeg.input(\"" << input << "\", ss=\"" << start << "\", to=\"" << end << "\")"
                << ".output(\"" << dest << "\", vn=None, acodec='mp3').run()\n";
        }
        else if (node.command == "if") {
            std::string cond1 = exprToString(node.expr1);
            std::string cond2 = exprToString(node.expr2);
            out << "if " << cond1 << " == " << cond2 << ":\n";
            for (const auto* stmt : node.statements) {
                out << "    ";
                translateToPython(*stmt, out);
            }
        }
    }
};
