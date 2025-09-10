#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <sys/ioctl.h>
#include <unistd.h>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <random>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <regex>
#include <functional>
#include <map>
#include <stdexcept>
#include <future>
#include <mutex>
#include "GenericMethods.h"

namespace fs = std::filesystem;



// 1. DIRECTORY AND FILE OPERATIONS
std::string GlobalMethodClass::MakeDirectory(const std::string& path, const std::string& folderName) {

    static std::set<std::string> createdFolders;
    fs::path fullPath = (path.empty()) ? fs::current_path() / folderName : fs::path(path) / folderName;
    if (createdFolders.count(fullPath.string()) && fs::exists(fullPath)) return fullPath.string();

    try {
        if (fs::create_directories(fullPath)) createdFolders.insert(fullPath.string());
        return fullPath.string();
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to create folder: " << e.what() << std::endl;
    }
    return "";
}

std::string GlobalMethodClass::MakeRootDirectory(const std::string& path) {
    // Get the user's home directory
    const char* home = std::getenv("HOME");   // works on Linux/macOS
#ifdef _WIN32
    if (!home) home = std::getenv("USERPROFILE"); // Windows fallback
#endif

    fs::path target;
    if (path == "Desktop") {
        target = fs::path(home) / "Desktop" / "ECHOBLAST";
    }
    else if (path == "Downloads") {
        target = fs::path(home) / "Downloads" / "ECHOBLAST";
    }
    else if (path == "Documents") {
        target = fs::path(home) / "Documents" / "ECHOBLAST";
    }

    try {
        if (!fs::exists(target)) {
            fs::create_directories(target);
            clearScreen();
            std::cout << "\033[92m\033[1mMain Directory is successfully created in " << path << std::endl;
        }
        else {
            clearScreen();
            std::cout << path << "directory chosen already exist.\n";
        }
    }
    catch (const fs::filesystem_error& e) {
        target = "";
        std::cout << "Program failed contact ECHOVSL\n";
        exit(1);
    }
    return target;
}


bool GlobalMethodClass::CreateProgramDirectories(std::string BaseProgramDirectory, std::string& ChildProgramDirectory, const std::string type, std::string& ProgramAttributeDirectory, std::string& Junks) {
    ChildProgramDirectory = GlobalMethodClass::MakeDirectory(BaseProgramDirectory, ChildProgramDirectory);
    ChildProgramDirectory = GlobalMethodClass::MakeDirectory(ChildProgramDirectory, type);
    ProgramAttributeDirectory = GlobalMethodClass::MakeDirectory(ChildProgramDirectory, ProgramAttributeDirectory);
    Junks = GlobalMethodClass::MakeDirectory(ChildProgramDirectory, Junks);
    return (!BaseProgramDirectory.empty() && !ChildProgramDirectory.empty() && !ProgramAttributeDirectory.empty() && !Junks.empty()) ? true : false;
}

void GlobalMethodClass::CreateProgramAttributeFiles(std::string& ProgramAttributeDirectory, std::string& Junks, std::vector<std::string>& ProgramAttributeVector) {
    std::fstream FileAccess;
    for (const auto& file : ProgramAttributeVector) {
        std::string filePath = (file == "failedleads.txt" || file == "smtps_under_limit.txt" || file == "deadsmtps.txt") ? (Junks + "/" + file) : (ProgramAttributeDirectory + "/" + file);
        FileAccess.open(filePath, std::ios::app);
        if (!FileAccess) {
            std::cerr << "Failed to create or open file: " << filePath << std::endl;
        }
        FileAccess.close();
    }
}

bool GlobalMethodClass::ReadFileToVector(std::vector<std::string>& FileVector, const std::string& filepath) {
    std::fstream FileData(filepath, std::ios::in);
    if (!FileData) {
        return false;
    }

    std::string line;
    while (std::getline(FileData, line)) {
        if (!line.empty()) {
            FileVector.emplace_back(std::move(line));
        }
    }
    FileData.close();
    return !FileVector.empty();
}

void GlobalMethodClass::WriteDataToFile(const std::string FilePath, std::string data) {
    try {
        std::fstream FileAccess(FilePath, std::ios::app);
        if (!FileAccess) {
            std::cout << "Error: failed to access " << FilePath << std::endl;
            return;
        }
        FileAccess << data << std::endl;
        FileAccess.close();
    }
    catch (std::exception& error) {
        std::cerr << "Operation failed because " << error.what() << std::endl;
    }
}

bool GlobalMethodClass::FetchDataFromFile(const std::string FilePath, std::string& VariableStorage) {
    try {
        std::fstream FileAccess(FilePath, std::ios::in);
        if (!FileAccess) {
            std::cout << "Error: failed to access " << FilePath << std::endl;
            return false;
        }
        std::string line;
        while (std::getline(FileAccess, line)) {
            if (!line.empty()) {
                VariableStorage += line + "\n";
            }
        }
        FileAccess.close();
    }
    catch (std::exception& error) {
        std::cerr << "Operation failed because " << error.what() << std::endl;
    }
    return true;
}

void GlobalMethodClass::DATACLEANUP(const std::string header, const std::string& sourcePath, const std::vector<std::string>& excludePaths) {
    std::ifstream sourceFile(sourcePath);
    if (!sourceFile) {
        std::cerr << "[ERR]: CLEAN UP CANNOT READ SOURCE FILE" << std::endl;
        return;
    }

    std::unordered_set<std::string> entries;
    std::string line;

    while (std::getline(sourceFile, line)) {
        if (!line.empty()) {
            entries.insert(line);
        }
    }
    sourceFile.close();

    auto readExclusions = [](const std::string& path) -> std::unordered_set<std::string> {
        std::unordered_set<std::string> set;
        std::ifstream file(path);
        if (!file) {
            std::cerr << "[WAR]: CLEAN UP FAILED TO OPEN EXCLUDE FILES" << std::endl;
            return set;
        }
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                set.insert(line);
            }
        }
        return set;
        };

    std::vector<std::future<std::unordered_set<std::string>>> futures;
    for (const auto& path : excludePaths) {
        futures.push_back(std::async(std::launch::async, readExclusions, path));
    }

    for (auto& future : futures) {
        for (const auto& excludeEntry : future.get()) {
            entries.erase(excludeEntry);
        }
    }

    std::ofstream outFile(sourcePath, std::ios::trunc);
    if (!outFile) {
        std::cerr << "[ERR]: CLEAN UP FAILED TO WRITE TO SOURCE FILE" << std::endl;
        return;
    }

    for (const auto& entry : entries) {
        outFile << entry << '\n';
    }

    std::cout << header << "CLEAN UP COMPLETED. " << entries.size() << " REMAINING DATA RETURNED SOURCE FILE" << std::endl;
}
// void GlobalMethodClass::DATACLEANUP(const std::string& sourcePath, const std::vector<std::string>& excludePaths) {
//     std::ifstream sourceFile(sourcePath);
//     if (!sourceFile) {
//         std::cerr << "Error: Cannot open source file: " << sourcePath << std::endl;
//         return;
//     }

//     std::unordered_set<std::string> entries;
//     std::string line;

//     while (std::getline(sourceFile, line)) {
//         if (!line.empty()) {
//             entries.insert(line);
//         }
//     }
//     sourceFile.close();

//     for (const auto& path : excludePaths) {
//         std::ifstream excludeFile(path);
//         if (!excludeFile) {
//             std::cerr << "Warning: Cannot open exclude file: " << path << std::endl;
//             continue;
//         }

//         while (std::getline(excludeFile, line)) {
//             if (!line.empty()) {
//                 entries.erase(line);
//             }
//         }
//         excludeFile.close();
//     }

//     std::ofstream outFile(sourcePath, std::ios::trunc);
//     if (!outFile) {
//         std::cerr << "Error: Cannot write to source file: " << sourcePath << std::endl;
//         return;
//     }

//     for (const auto& entry : entries) {
//         outFile << entry << '\n';
//     }

//     std::cout << "CLEAN UP COMPLETED. " << entries.size() << " REMAINING DATA RETURNED SOURCE FILE" << std::endl;
// }

// 2. SMTP && EMAIL PROCESSING
bool GlobalMethodClass::FETCHSMTP(std::vector<std::string>& Vector, std::string& CURRENTSMTP, std::string& servername, int& port, std::string& username, std::string& password) {
    if (Vector.empty()) return false;

    CURRENTSMTP = Vector.front();
    std::string portStr;
    std::stringstream ss(CURRENTSMTP);

    if (!std::getline(ss, servername, '|')) return false;
    if (!std::getline(ss, portStr, '|')) return false;
    if (!std::getline(ss, username, '|')) return false;
    if (!std::getline(ss, password)) return false;

    try {
        port = std::stoi(portStr);
    }
    catch (const std::exception& e) {
        return false;
    }
    return true;
}

bool GlobalMethodClass::LOADSMTP(std::vector<std::string>& Vector, std::string& CURRENTSMTP, std::string& servername, int& port, std::string& username, std::string& password, size_t& index) {
    if (Vector.empty()) return false;

    if (index >= Vector.size()) index = 0;

    CURRENTSMTP = Vector[index];
    std::string portStr;
    std::stringstream ss(CURRENTSMTP);
    if (!std::getline(ss, servername, '|')) return false;
    if (!std::getline(ss, portStr, '|')) return false;
    if (!std::getline(ss, username, '|')) return false;
    if (!std::getline(ss, password)) return false;

    try {
        port = std::stoi(portStr);
    }
    catch (const std::exception& e) {
        return false;
    }
    index++;
    return true;
}


void GlobalMethodClass::stripdata(std::string& server, std::string& port) {
    std::stringstream ss(server);
    std::getline(ss, server, ':');
    std::getline(ss, port);
}

bool GlobalMethodClass::isValidEmail(const std::string& email) {
    const std::regex pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return std::regex_match(email, pattern);
}

std::string GlobalMethodClass::GetDomain(const std::string& username, const std::string& servername) {
    const size_t at_pos = username.find('@');
    return (at_pos != std::string::npos) ? username.substr(at_pos + 1) : servername;
}

std::string GlobalMethodClass::GenerateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    const char* hex_chars = "0123456789abcdef";
    std::string uuid;
    for (int i = 0; i < 32; ++i) {
        uuid += hex_chars[dis(gen)];
        if (i == 7 || i == 11 || i == 15 || i == 19) {
            uuid += '-';
        }
    }
    return uuid;
}

std::string GlobalMethodClass::GenerateMessageID(const std::string& username) {
    size_t at_pos = username.find('@');
    std::string domain = (at_pos != std::string::npos) ? username.substr(at_pos) : "admin@sendgrid.com";
    return GenerateUUID() + domain;
}

std::string GlobalMethodClass::StripHTML(const std::string& html) {
    std::string result;
    bool inTag = false;

    for (char c : html) {
        if (c == '<') {
            inTag = true;
            if (!result.empty() && result.back() != ' ') {
                result += ' ';
            }
        }
        else if (c == '>') {
            inTag = false;
        }
        else if (!inTag) {
            result += c;
        }
    }

    result.erase(std::unique(result.begin(), result.end(),
        [](char a, char b) { return a == ' ' && b == ' '; }),
        result.end());
    result.erase(result.find_last_not_of(" \n\r\t") + 1);

    return result;
}

std::string GlobalMethodClass::GetMimeType(const std::string& filepath) {
    static const std::map<std::string, std::string> mimeTypes = {
        {".txt", "text/plain"},
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".xml", "application/xml"},
        {".pdf", "application/pdf"},
        {".zip", "application/zip"},
        {".tar", "application/x-tar"},
        {".gz", "application/gzip"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".mp3", "audio/mpeg"},
        {".wav", "audio/wav"},
        {".mp4", "video/mp4"},
        {".avi", "video/x-msvideo"},
        {".mov", "video/quicktime"},
        {".doc", "application/msword"},
        {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
        {".xls", "application/vnd.ms-excel"},
        {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
        {".ppt", "application/vnd.ms-powerpoint"},
        {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"} };

    std::string extension = std::filesystem::path(filepath).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    auto it = mimeTypes.find(extension);
    return it != mimeTypes.end() ? it->second : "application/octet-stream";
}

void GlobalMethodClass::MakeEmailBody(
    std::string& EmailContentBody, bool useHTML, bool useAttachment,
    const std::string& servername, const std::string& sendername, const std::string& username,
    const std::string& subject, const std::string& letter, const std::string& lead,
    const std::string& AttachmentFilePath) {
    time_t now = time(nullptr);
    const std::string boundary = "----=_Part_" + std::to_string(now) + GenerateUUID();
    const std::string altBoundary = "----=_AltPart_" + GenerateUUID();
    const size_t at_pos = username.find('@');
    const std::string domain = (at_pos != std::string::npos) ? username.substr(at_pos + 1) : servername;

    EmailContentBody.clear();
    EmailContentBody.reserve(2048);

    EmailContentBody +=
        "From: \"" + sendername + "\" <" + username + ">\r\n"
        "To: <" + lead + ">\r\n"
        "Subject: " + subject + "\r\n"
        "Date: " + GetCurrentDateTime() + "\r\n"
        "Message-ID: <" + std::to_string(now) + "." + GenerateUUID() + "@" + domain + ">\r\n"
        "Reply-To: \"" + sendername + "\" <" + username + ">\r\n"
        "Return-Path: <" + username + ">\r\n"
        "MIME-Version: 1.0\r\n"
        "Content-Language: en-US\r\n";

    if (useAttachment) {
        EmailContentBody += "Content-Type: multipart/mixed; boundary=\"" + boundary + "\"\r\n\r\n";
        EmailContentBody += "--" + boundary + "\r\n";
    }

    EmailContentBody +=
        "Content-Type: multipart/alternative; boundary=\"" + altBoundary + "\"\r\n\r\n"
        "--" + altBoundary + "\r\n";

    if (useHTML) {
        EmailContentBody +=
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Content-Transfer-Encoding: quoted-printable\r\n\r\n" +
            QuotedPrintableEncode(letter) + "\r\n\r\n";
    }
    else {
        EmailContentBody +=
            "Content-Type: text/plain; charset=UTF-8\r\n"
            "Content-Transfer-Encoding: quoted-printable\r\n\r\n" +
            QuotedPrintableEncode(StripHTML(letter)) + "\r\n\r\n";
    }

    EmailContentBody += "--" + altBoundary + "--\r\n";

    if (useAttachment && !AttachmentFilePath.empty()) {
        const std::string attachmentName = std::filesystem::path(AttachmentFilePath).filename().string();
        const std::string mimeType = GetMimeType(AttachmentFilePath);
        const std::string encodedFile = base64Encode(AttachmentFilePath);

        EmailContentBody +=
            "--" + boundary + "\r\n"
            "Content-Type: " + mimeType + "; name=\"" + attachmentName + "\"\r\n"
            "Content-Disposition: attachment; filename=\"" + attachmentName + "\"\r\n"
            "Content-Transfer-Encoding: base64\r\n\r\n" +
            encodedFile + "\r\n\r\n";
        EmailContentBody += "--" + boundary + "--\r\n";
    }
}

bool GlobalMethodClass::InsertVariableEmail(const std::string letter, const std::string& email, std::string& body) {
    const std::string placeholder = "###";
    bool found = false;
    size_t pos = 0;
    body = letter;

    while ((pos = body.find(placeholder, pos)) != std::string::npos) {
        body.replace(pos, placeholder.length(), email);
        pos += email.length();
        found = true;
    }
    return found;
}

void GlobalMethodClass::EMAILSUCCESSMESSAGE(const std::string sendername, const std::string subject, const std::string lead, const int count) {
    const std::string green = "\033[92m";
    const std::string yellow = "\033[93m";
    const std::string red = "\033[91m";
    const std::string bold = "\033[1m";
    const std::string reset = "\033[0m";

    std::cout
        << green << "===============================   " << red << bold << "EMAIL SENT SUCCESSFUL  " << green << "===============================\n"
        << green << "COUNT: " << yellow << "[" << count << "]\n"
        << green << "DATE: " << yellow << GetCurrentDateTime() << "\n"
        << green << "SENDER NAME: " << yellow << sendername << "\n"
        << green << "SUBJECT TITLE: " << yellow << subject << "\n"
        << green << "RECIPIENT EMAIL: " << yellow << lead << "\n"
        << green << "===============================  " << red << bold << "ECHOBLAST EMAIL SENDER  " << green << "===============================\n\n"
        << reset;
}

void GlobalMethodClass::SMTP_RESPONSE_MESSAGE(const std::string smtp, bool flag, const int& count) {
    const std::string green = "\033[92m";
    const std::string red = "\033[91m";
    const std::string reset = "\033[0m";
    const std::string yellow = "\033[93m";
    int x = static_cast<int>(flag);
    switch (x) {
    case 0:
        std::cout
            << red << "================================== SMTP RESPONSE FAILED =================================\n"
            << red << "COUNT:" << yellow << "[" << count << "]\n"
            << red << "DATE: " << yellow << GetCurrentDateTime() << "\n"
            << red << "SMTP: " << yellow << smtp << std::endl
            << red << "==================================  TELEGRAM: ECHOVSL   =================================\n\n"
            << green;
        break;
    case 1:
        std::cout
            << green << "=================================  SMTP RESPONSE SUCCESS =================================\n"
            << green << "COUNT:" << yellow << "[" << count << "]\n"
            << green << "DATE: " << yellow << GetCurrentDateTime() << "\n"
            << green << "SMTP: " << yellow << smtp << std::endl
            << green << "==================================  TELEGRAM: ECHOVSL  ===================================\n\n"
            << green;
        break;
    }
}

// 3. DATA ENCODING AND PROCESSING
std::string GlobalMethodClass::QuotedPrintableEncode(const std::string& input) {
    std::ostringstream out;
    int lineLength = 0;

    auto flushChar = [&](char c) {
        if (lineLength + 1 >= 76) {
            out << "=\r\n";
            lineLength = 0;
        }
        out << c;
        lineLength++;
        };

    auto flushEncoded = [&](unsigned char c) {
        if (lineLength + 3 >= 76) {
            out << "=\r\n";
            lineLength = 0;
        }
        char buf[4];
        snprintf(buf, sizeof(buf), "=%02X", c);
        out << buf;
        lineLength += 3;
        };

    for (size_t i = 0; i < input.length(); ++i) {
        unsigned char c = input[i];

        bool isLastCharOnLine =
            (i + 1 == input.length()) || (input[i + 1] == '\n' || input[i + 1] == '\r');

        if ((c >= 33 && c <= 60) || (c >= 62 && c <= 126)) {
            flushChar(c);
        }
        else if (c == ' ' || c == '\t') {
            if (isLastCharOnLine) {
                flushEncoded(c);
            }
            else {
                flushChar(c);
            }
        }
        else if (c == '\r' || c == '\n') {
            out << c;
            lineLength = 0;
        }
        else {
            flushEncoded(c);
        }
    }

    return out.str();
}

std::string GlobalMethodClass::base64Encode(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    std::ostringstream fileStream;
    fileStream << file.rdbuf();
    std::string fileContent = fileStream.str();
    static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    int val = 0, valb = -6;
    for (unsigned char c : fileContent) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (encoded.size() % 4) {
        encoded.push_back('=');
    }
    return encoded;
}

std::string GlobalMethodClass::UrlEncode(const std::string& str) {
    std::ostringstream encoded;
    for (size_t i = 0; i < str.length(); ++i) {
        unsigned char c = str[i];
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        }
        else {
            encoded << '%' << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        }
    }
    return encoded.str();
}

// 4. UTILITY FUNCTIONS
std::string GlobalMethodClass::GetRandomDataFromVector(const std::vector<std::string>& param) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(0, param.size() - 1);
    return param[dis(gen)];
}

std::string GlobalMethodClass::GetCurrentDateTime() {
    time_t now = time(nullptr);
    struct tm tm = *gmtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S +0000", &tm);
    return std::string(buf);
}

void GlobalMethodClass::POPDATA(std::vector<std::string>& Vec, std::string data) {
    try {
        auto find = std::find(Vec.begin(), Vec.end(), data);
        if (find != Vec.end()) {
            Vec.erase(find);
            Vec.shrink_to_fit();
        }
    }
    catch (std::exception& e) {
        return;
    }
}

bool GlobalMethodClass::ValidDigit(const std::string& param) {
    char c;
    for (size_t i = 0; i < param.length(); i++) {
        c = param[i];
        if (!isdigit(c))
            return false;
    }
    return true;
}

// 5. CONSOLE AND USER INTERACTION
void GlobalMethodClass::clearScreen() {
    std::cout << "\033[2J\033[1;1H"; // ANSI escape code to clear the screen
}
void GlobalMethodClass::clearScreenWithMessage(const std::string& message) {
    std::cout << "\033[2J\033[1;1H" << message << "\n";
}

bool GlobalMethodClass::askYesNoQuestion(const std::string& question) {
    std::string optionStr;
    std::cout << question << " (Y/N): ";
    std::cin >> optionStr;
    return (optionStr == "Y" || optionStr == "y");
}

void GlobalMethodClass::handleProgramCompletion(bool success, const std::string& programName) {
    sleep(1);
    success ?
        std::cout << programName << " PROGRAM COMPLETED.\n" :
        std::cout << programName << " PROGRAM FAILED.\n";

}

void GlobalMethodClass::PrintTopMiddle(const std::string text) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int terminalWidth = w.ws_col;
    int textStartPos = (terminalWidth - text.length()) / 2;
    std::cout << "\033[1;1H";
    std::cout << std::string(textStartPos, ' ') << text << std::endl << std::endl;
}

void GlobalMethodClass::sleep(const int& x) {
    std::this_thread::sleep_for(std::chrono::seconds(x));
}

void GlobalMethodClass::prompt(std::function<int()> main) {
    std::string optionStr;
    std::cout << std::endl
        << "\nGOTO MENU? (Y/N): ";
    std::cin >> optionStr;
    if (optionStr == "Y" || optionStr == "y") {
        main();
    }
    clearScreen();
}

// 6. DELAY HANDLING
// This function shuffles and generates a random delay from a predefined set of delays, 
// ensure randomness and returns the next delay in the sequence.
int GlobalMethodClass::GenerateDelay() {
    static int delays[] = { 2, 3, 4, 5 };
    static const int delaysCount = sizeof(delays) / sizeof(delays[0]);
    static int currentIndex = delaysCount;
    static std::mt19937 rng(std::random_device{}());
    if (currentIndex == delaysCount) {
        std::shuffle(delays, delays + delaysCount, rng);
        currentIndex = 0;
    }

    return delays[currentIndex++];
}

void GlobalMethodClass::DelayHandler(int& sendspeed, int res, bool flag) {
    sendspeed = (flag) ? res : GenerateDelay();
}

void GlobalMethodClass::PromptSendDelayMessage() {
    std::cout << "NOTE: BLAST DELAY PRESERVES SMTP LIFE.\n"
        << "NOTE: SYSTEM DEFINED DELAY RANGE 1-5 SECONDS SIMULTANEOUSLY.\n"
        << "NOTE: IF CUSTOM DELAY FALLS BELOW 5 SECONDS, PROGRAM FALLS BACK TO SYSTEM DEFINED DELAY.\n\n"
        // << "SET CUSTOM DELAY VALUE (MINIMUM 5)\n"
        << "ENTER 0 FOR SYSTEM DEFAULT DELAY: ";
}
