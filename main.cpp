#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <json/json.h>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

int main();
bool ValidDigit(const std::string& param);
void clearScreenWithMessage(const std::string& message);
void clearScreen() {
  std::cout << "\033[2J\033[H";
}
void sleep(const int& x) {
  std::this_thread::sleep_for(std::chrono::seconds(x));
}

size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}
size_t readCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
  std::string* data = static_cast<std::string*>(userdata);
  if (data->empty()) return 0;

  size_t toCopy = std::min(size * nmemb, data->size());
  memcpy(ptr, data->data(), toCopy);
  data->erase(0, toCopy);
  return toCopy;
}
void PrintTopMiddle(const std::string text) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int terminalWidth = w.ws_col;
  int textStartPos = (terminalWidth - text.length()) / 2;
  std::cout << "\033[1;1H";
  std::cout << std::string(textStartPos, ' ') << text << std::endl
    << std::endl;
}

void POPDATA(std::vector<std::string>& Vec, std::string data) {
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

bool ValidDigit(const std::string& param) {
  char c;
  for (size_t i = 0; i < param.length(); i++) {
    c = param[i];
    if (!isdigit(c))
      return false;
  }
  return true;
}

class GlobalMethodClass {
public:
  // #: 1
  std::string MakeDirectory(const std::string& path, const std::string& folderName) {
    static std::set<std::string> createdFolders;
    fs::path fullPath = (path.empty()) ? fs::current_path() / folderName : fs::path(path) / folderName;
    if (createdFolders.count(fullPath.string())) {
      return fullPath.string();
    }
    try {
      if (fs::create_directories(fullPath)) {
        createdFolders.insert(fullPath.string());
      }
      return fullPath.string();
    }
    catch (const std::exception& e) {
      std::cerr << "Failed to create folder: " << e.what() << std::endl;
    }
    return "";
  }

  // #: 2
  bool ReadFileToVector(std::vector<std::string>& FileVector, const std::string& filepath) {
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

  // #: 3
  std::string GetRandomDataFromVector(const std::vector<std::string>& param) {
    static std::mt19937 gen(std::random_device{}()); // Static generator
    std::uniform_int_distribution<> dis(0, param.size() - 1);
    return param[dis(gen)];
  }

  // #: 4
  bool FETCHSMTP(std::vector<std::string>& Vector, std::string& CURRENTSMTP, std::string& servername, int& port, std::string& username, std::string& password) {
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
      std::cerr << "Port failed to parse because " << e.what() << std::endl;
      return false;
    }
    return true;
  }

  // THIS IS A TEMP FUNCTION TO FETCH SMTP IN NEXT LINES
  bool LOADSMTP(std::vector<std::string>& Vector, std::string& CURRENTSMTP, std::string& servername, int& port, std::string& username, std::string& password, int& index) {
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
      std::cerr << "Port failed to parse because " << e.what() << std::endl;
      return false;
    }
    return true;
  }

  // #: 5
  void WriteDataToFile(const std::string FilePath, std::string data) {
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

  // #: 6
  bool FetchDataFromFile(const std::string FilePath, std::string& VariableStorage) {
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

  // ENCODING HTML
  std::string QuotedPrintableEncode(const std::string& input) {
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

  // #: 7
  void DATACLEANUP(const std::string leadfile, const std::string sentleads, const std::string failedleads) {
    std::ifstream newfile(leadfile);
    std::ifstream sentfiles(sentleads);
    std::ifstream failedfiles(failedleads);
    if (!newfile || !sentfiles || !failedfiles) {
      std::cerr << "Error: Unable to open these files.\n"
        << leadfile << std::endl
        << sentleads << std::endl
        << failedleads << std::endl;
      return;
    }
    std::unordered_set<std::string> leadset;
    std::string line;

    while (std::getline(newfile, line)) {
      if (!line.empty()) {
        leadset.insert(line);
      }
    }
    while (std::getline(sentfiles, line)) {
      if (!line.empty()) {
        leadset.erase(line);
      }
    }
    while (std::getline(failedfiles, line)) {
      if (!line.empty()) {
        leadset.erase(line);
      }
    }
    std::ofstream leadfileout(leadfile, std::ios::out);
    if (!leadfileout) {
      std::cerr << "Error: Failed to clean up " << leadfile << std::endl;
      return;
    }

    if (!leadset.empty()) {
      for (const auto& lead : leadset) {
        leadfileout << lead << std::endl;
      }
      std::cout << "LEAD CLEANUP COMPLETED.\n"
        << leadset.size() << " REMAINING LEADS UNSENT." << std::endl;
    }
    leadfileout.close();
    newfile.close();
    sentfiles.close();
    failedfiles.close();
  }

  // #: 8
  std::string base64Encode(const std::string& filePath) {
    // Open the file in binary mode
    std::ifstream file(filePath, std::ios::binary);
    std::ostringstream fileStream;
    fileStream << file.rdbuf();
    std::string fileContent = fileStream.str();

    // Base64 encode the file content
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

  // #: 9


  // #: 10
  void stripdata(std::string& server, std::string& port) {
    std::stringstream ss(server);
    std::getline(ss, server, ':');
    std::getline(ss, port);
  }

  // #: 11
  void prompt() {
    std::string optionStr;
    std::cout << std::endl
      << "Do you want to retry the program? (Y/N): ";
    std::cin >> optionStr;
    if (optionStr == "Y" || optionStr == "y") {
      main();
    }
    clearScreen();
  }

  // #: 12
  bool CreateProgramDirectories(std::string BaseProgramDirectory, std::string& ChildProgramDirectory, std::string& ProgramAttributeDirectory, std::string& Junks) {
    ChildProgramDirectory = MakeDirectory(BaseProgramDirectory, ChildProgramDirectory);
    ProgramAttributeDirectory = MakeDirectory(ChildProgramDirectory, ProgramAttributeDirectory);
    Junks = MakeDirectory(ChildProgramDirectory, Junks);
    return (!BaseProgramDirectory.empty() && !ChildProgramDirectory.empty() && !ProgramAttributeDirectory.empty() && !Junks.empty()) ? true : false;
  }

  // #: 13
  void CreateProgramAttributeFiles(std::string& ProgramAttributeDirectory, std::string& Junks, std::vector<std::string>& ProgramAttributeVector) {
    std::fstream FileAccess;
    for (const auto& file : ProgramAttributeVector) {
      std::string filePath = (file == "failedleads.txt" || file == "smtp_under_limit.txt" || file == "deadsmtp.txt") ? (Junks + "/" + file) : (ProgramAttributeDirectory + "/" + file);
      FileAccess.open(filePath, std::ios::app);
      if (!FileAccess) {
        std::cerr << "Failed to create or open file: " << filePath << std::endl;
      }
      FileAccess.close();
    }
  }

  // #: 14
  bool CreateSMSAttributeFiles(std::string& GatewaySenderDirectory, std::vector<std::string>& FilesVector) {
    std::fstream FileAccess;
    for (const auto& file : FilesVector) {
      std::string filePath = GatewaySenderDirectory + "/" + file;
      FileAccess.open(filePath, std::ios::app);
      if (!FileAccess) {
        std::cerr << "Failed to create " << file << std::endl;
        return false;
      }
      FileAccess.close();
    }
    return true;
  }

  // #: 15
  void EMAILSUCCESSMESSAGE(const std::string sendername, const std::string subject, const std::string lead, const int count) {
    std::cout
      << "\033[92m"
      << "=================================>   "
      << "\033[91m"
      << "\033[1m"
      << "EMAIL SENT SUCCESSFUL  "
      << "\033[92m"
      << "<=================================\n"

      << "\033[92m" << "> COUNT: "
      << "\033[93m" << "[" << count << "]\n"

      << "\033[92m" << "> SENDER NAME: "
      << "\033[93m" << sendername << "\n"

      << "\033[92m" << "> SUBJECT TITLE: "
      << "\033[93m" << subject << "\n"

      << "\033[92m" << "> RECIPIENT EMAIL: "
      << "\033[93m" << lead << "\n"

      << "\033[92m"
      << "=================================>  "
      << "\033[91m"
      << "\033[1m"
      << "ECHOBLAST EMAIL SENDER  "
      << "\033[92m"
      << "<=================================\n\n";
  }

  //SMTP PRINT RESPONSE
  void SMTPSUCCESSMESSAGE(const std::string smtp) {
    std::cout
      << "\033[92m"
      << "=============================    SMTP TEST    =============================\n"
      << smtp << std::endl
      << "============================= TEST SUCCESSFUL =============================\n\n";
  }
  void SMTPFAILEDMESSAGE(const std::string smtp) {
    std::cout << "\033[31m"
      << "=============================    SMTP TEST    =============================\n"
      << smtp << std::endl
      << "=============================   TEST FAILED   =============================\n\n"
      << "\033[92m";
  }

  // #: 17
  std::string UrlEncode(const std::string& str) {
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

  // #: 18
  std::string GetCurrentDateTime() {
    time_t now = time(nullptr);
    struct tm tm = *gmtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S +0000", &tm);
    return std::string(buf);
  }
};

class EmailSenderProgram : public GlobalMethodClass {
private:
  struct MailDataSet {
    std::vector<std::string> MailDataSetVector;
    struct SMTPAttribute {
      int port;
      std::string servername, username, password;
    };
  };

public:
  MailDataSet SMTPVectorObject, NameVectorObject, SubjectVectorObject;
  MailDataSet::SMTPAttribute SMTPAttributeObject;
  std::string EmailSenderPrograms = "Email Sender Programs";

  // Main Program class constructor
  EmailSenderProgram() {
    EmailSenderPrograms = MakeDirectory("", EmailSenderPrograms);
  }

  // EMAIL SENDER GLOBAL VARIABLES
  std::string EmailProgramDirectory = "Email Sender";
  std::string EmailDataDirectory = "Email Data";
  std::string EmailJunkDirectory = "Junks";
  std::vector<std::string> ProgramAttributeVector;

  // SMTP LIVE TESTER GLOBAL VARIABLES
  std::string SMTPLiveTesterDirectory = "SMTP Live Tester";
  std::string EmailExtractorDirectory = "Email Extractor";
  std::string DuplicateRemoval = "Duplicate Remover";

  bool PrepareEmailSenderDirectories() {
    ProgramAttributeVector = { "smtps.txt", "sendername.txt", "subject.txt", "letter.txt", "leads.txt", "sentleads.txt", "failedleads.txt", "deadsmtp.txt", "smtp_under_limit.txt" };
    if (!CreateProgramDirectories(EmailSenderPrograms, EmailProgramDirectory, EmailDataDirectory, EmailJunkDirectory)) return false;
    CreateProgramAttributeFiles(EmailDataDirectory, EmailJunkDirectory, ProgramAttributeVector);
    return true;
  }

  bool PrepareSMTPLiveTesterDirectories() {
    ProgramAttributeVector = { "smtps.txt", "Failed.txt", "Live.txt" };
    SMTPLiveTesterDirectory = MakeDirectory(EmailSenderPrograms, SMTPLiveTesterDirectory);
    clearScreen();
    std::cout << "Please wait, program checking files.\n";
    for (auto& file : ProgramAttributeVector) {
      std::string filePath = fs::path(SMTPLiveTesterDirectory) / file;
      std::fstream FileAccess(filePath, std::ios::app);
      if (!FileAccess) {
        return false;
      }
      FileAccess.close();
    }
    ProgramAttributeVector.clear();
    return true;
  }

  bool PrepareEmailExtractorDirectories() {
    ProgramAttributeVector = { "rawfile.txt", "extracted.txt" };
    EmailExtractorDirectory = MakeDirectory(EmailSenderPrograms, EmailExtractorDirectory);
    for (auto& file : ProgramAttributeVector) {
      std::string FilePath = fs::path(EmailExtractorDirectory) / file;
      std::fstream FileAccess(FilePath, std::ios::out);
      if (!FileAccess) {
        std::cerr << "Failed to create " << file << std::endl;
        return false;
      }
      FileAccess.close();
    }
    return true;
  }
  bool PrepareDuplicateDirectories() {
    ProgramAttributeVector = { "Unfiltered.txt", "Filtered.txt" };
    EmailExtractorDirectory = MakeDirectory(EmailSenderPrograms, DuplicateRemoval);
    for (auto& file : ProgramAttributeVector) {
      std::string FilePath = fs::path(EmailExtractorDirectory) / file;
      std::fstream FileAccess(FilePath, std::ios::out);
      if (!FileAccess) {
        std::cerr << "Failed to create " << file << std::endl;
        return false;
      }
      FileAccess.close();
    }
    return true;
  }

  void SetCurlForMail(CURL*& curl, std::string servername, int port, std::string username, std::string password) {
    std::string protocol = (port == 465) ? "smtps://" : "smtp://";
    std::string url = protocol + servername + ":" + std::to_string(port);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L); // for debugging
  }

  std::string GenerateUUID() {
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

  // Helper function to generate Message-ID
  std::string GenerateMessageID(const std::string& username) {
    size_t at_pos = username.find('@');
    std::string domain = (at_pos != std::string::npos) ? username.substr(at_pos) : "example.com";
    return GenerateUUID() + domain;
  }

  std::string StripHTML(const std::string& html) {
    std::string result;
    bool inTag = false;

    for (char c : html) {
      if (c == '<') {
        inTag = true;
        // Add space when closing tags to maintain word separation
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

    // Collapse multiple spaces and trim
    result.erase(std::unique(result.begin(), result.end(),
      [](char a, char b) { return a == ' ' && b == ' '; }),
      result.end());
    result.erase(result.find_last_not_of(" \n\r\t") + 1);

    return result;
  }

  // Helper function to get MIME type from filename
  std::string GetMimeType(const std::string& filepath) {
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

  void MakeEmailBody(
    std::string& EmailContentBody, bool useHTML, bool useAttachment, std::string boundary,
    const std::string& servername, const std::string& sendername, const std::string& username,
    const std::string& subject, const std::string& letter, const std::string& lead,
    const std::string& AttachmentFilePath = "") {

    std::string tmp;
    time_t now = time(nullptr);

    if (boundary.empty()) {
      boundary = "----=_Part_" + std::to_string(now) + GenerateUUID();
    }

    const std::string altBoundary = "----=_AltPart_" + GenerateUUID();
    const size_t at_pos = username.find('@');
    const std::string domain = (at_pos != std::string::npos) ? username.substr(at_pos + 1) : servername;

    EmailContentBody.clear();
    EmailContentBody.reserve(2048);

    EmailContentBody +=
      "From: " + sendername + " <" + username + ">\r\n"
      "To: " + lead + "\r\n"
      "Subject: " + subject + "\r\n"
      "Date: " + GetCurrentDateTime() + "\r\n"
      "Message-ID: <" + std::to_string(now) + "." + GenerateUUID() + "@" + domain + ">\r\n"
      "Reply-To: " + sendername + " <" + username + ">\r\n"
      "Return-Path: <" + username + ">\r\n"
      "User-Agent: Apple Mail (2.3654.120.0.1.13)\r\n"
      "X-Mailer: Apple Mail (2.3654.120.0.1.13)\r\n"
      "Mime-Version: 1.0 (Apple Message framework v3654.120.0.1.13)\r\n"
      "X-Priority: 3 (Normal)\r\n"
      "Priority: normal\r\n"
      "Content-Language: en-US\r\n";

    if (useAttachment) {
      EmailContentBody += "Content-Type: multipart/mixed; boundary=\"" + boundary + "\"\r\n\r\n";
      EmailContentBody += "--" + boundary + "\r\n";
    }

    EmailContentBody +=
      "Content-Type: multipart/alternative; boundary=\"" + altBoundary + "\"\r\n\r\n"
      "--" + altBoundary + "\r\n";

    if (useHTML) {
      tmp = QuotedPrintableEncode(letter);
      EmailContentBody +=
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Transfer-Encoding: quoted-printable\r\n\r\n" +
        tmp + "\r\n\r\n";
    }
    else {
      EmailContentBody +=
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "Content-Transfer-Encoding: quoted-printable\r\n\r\n" +
        StripHTML(letter) + "\r\n\r\n";
    }

    EmailContentBody += "--" + altBoundary + "--\r\n";

    if (useAttachment) {
      if (!AttachmentFilePath.empty()) {
        const std::string attachmentName = std::filesystem::path(AttachmentFilePath).filename().string();
        const std::string mimeType = GetMimeType(AttachmentFilePath);
        const std::string encodedFile = base64Encode(AttachmentFilePath);

        EmailContentBody +=
          "--" + boundary + "\r\n"
          "Content-Type: " + mimeType + "; name=\"" + attachmentName + "\"\r\n"
          "Content-Disposition: attachment; filename=\"" + attachmentName + "\"\r\n"
          "Content-Transfer-Encoding: base64\r\n\r\n" +
          encodedFile + "\r\n\r\n";
      }
      EmailContentBody += "--" + boundary + "--\r\n";
    }
  }

  // # EMAIL SENDER MODULE

  bool EmailSender(bool useHTML, bool useAttachment) {
    clearScreen();
    bool success = false;
    std::string LEADSFILE = fs::path(EmailDataDirectory) / "leads.txt";
    std::string letterPath = fs::path(EmailDataDirectory) / "letter.txt";
    std::string SENTLEADSFILE = fs::path(EmailDataDirectory) / "sentleads.txt";
    std::string FAILEDLEADFILE = fs::path(EmailJunkDirectory) / "failedleads.txt";
    std::string DEADSMTPFILE = fs::path(EmailJunkDirectory) / "deadsmtp.txt";
    std::string LIMITEDSMTP = fs::path(EmailJunkDirectory) / "smtp_under_limit.txt";
    std::string SMTPFILES = fs::path(EmailDataDirectory) / "smtps.txt";
    std::string SENDERNAMESFILE = fs::path(EmailDataDirectory) / "sendername.txt";
    std::string SUBJECTSFILE = fs::path(EmailDataDirectory) / "subject.txt";
    std::string attachmentPath, emailBody, attachmentFileName, sender, subject, CURRENTSMTP, boundary;
    int sentCount = 0, errorCount = 0, rateLimit = 0, rateCount = 0, index = 0, retries = 0;
    int sendspeed;
    long responseCode;

    if (useAttachment) {
      attachmentPath = MakeDirectory(EmailDataDirectory, "Attachment");
      std::cout << "\033[2J\033[HProvide Attachment filename (e.g., file.pdf): ";
      std::string filename;
      std::cin >> filename;
      attachmentFileName = fs::path(attachmentPath) / filename;
    }

    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, SMTPFILES) ||
      !ReadFileToVector(NameVectorObject.MailDataSetVector, SENDERNAMESFILE) ||
      !ReadFileToVector(SubjectVectorObject.MailDataSetVector, SUBJECTSFILE)) {
      std::cerr << "Email Assets Files Cannot Be Empty\n";
      sleep(1);
      return false;
    }

    if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
      std::cerr << "SMTP NOT FOUND OR FINISHED.\n";
      return false;
    }

    std::string tmp;

    if (!ReadFileToVector(NameVectorObject.MailDataSetVector, fs::path(EmailDataDirectory) / "sendername.txt") ||
      !ReadFileToVector(SubjectVectorObject.MailDataSetVector, fs::path(EmailDataDirectory) / "subject.txt")) return false;

    std::vector<std::string> leads;
    if (!ReadFileToVector(leads, LEADSFILE)) {
      std::cerr << "FAILED TO ACCESS LEADS FILE.\n";
      return false;
    }

    clearScreen();
    int totalSMTP = SMTPVectorObject.MailDataSetVector.size();
    if (totalSMTP > 1) {
      clearScreen();
      std::cout << totalSMTP << " SMTPS FOUND.\n"
        << "Do you wanna set rate limit (Y/N): ";
      std::cin >> tmp;
      if (tmp == "Y" || tmp == "y") {
        std::cout << "Specify send rate limit : ";
        std::cin >> rateLimit;
      }
    }

    clearScreen();
    std::cout << "[SEND SPEED LIMIT PRESERVES SMTP LIFE]\n"
      << "[SPEED LIMIT WORKS IN SECONDS]\n"
      << "Set speed delay limit (default: 2): ";
    std::cin >> sendspeed;
    sendspeed = (sendspeed > 2) ? sendspeed : 2;

    std::string letter;
    FetchDataFromFile(letterPath, letter);

    CURL* curl;
    CURLcode res;
    curl_slist* recipients = nullptr;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) return false;

    SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
    clearScreenWithMessage("\t\t\t[EMAIL SENDER INITIALIZED]\n");

    for (const auto& lead : leads) {
      if (sentCount > 0) {
        sleep(sendspeed);
      }

      if (rateLimit > 0 && rateCount >= rateLimit) {
        if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
          std::cout << "ALL SMTP USED UP OR UNAVAILABLE\n";
          std::cout << "LOAD MORE SMTP AND RETRY PROGRAM\n";
          if (sentCount <= 0) {
            return false;
          }
          break;
        }
        SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
        rateCount = 0;
      }

      sender = GetRandomDataFromVector(NameVectorObject.MailDataSetVector);
      subject = GetRandomDataFromVector(SubjectVectorObject.MailDataSetVector);

      MakeEmailBody(emailBody, useHTML, useAttachment, boundary, SMTPAttributeObject.servername, sender, SMTPAttributeObject.username, subject, letter, lead, attachmentFileName);

      recipients = curl_slist_append(nullptr, lead.c_str());
      curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
      curl_easy_setopt(curl, CURLOPT_MAIL_FROM, SMTPAttributeObject.username.c_str());
      curl_easy_setopt(curl, CURLOPT_READDATA, &emailBody);
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
      res = curl_easy_perform(curl);
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

      if (res == CURLE_OK && responseCode == 250) {
        success = true;
        retries = 0;
        WriteDataToFile(SENTLEADSFILE, lead);
        sentCount++;
        rateCount++;
        EMAILSUCCESSMESSAGE(sender, subject, lead, sentCount);
      }
      else {
        if (res == CURLE_LOGIN_DENIED && responseCode == 535 || responseCode == 530 || responseCode == 534) {
          WriteDataToFile(DEADSMTPFILE, CURRENTSMTP);
          POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
          if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
            std::cout << "ALL SMTP USED UP OR UNAVAILABLE\n"
              << "LOAD MORE SMTP AND RETRY PROGRAM\n"
              << "LOGIN DENIED\n";
            break;
          }
        }
        else if (res == CURLE_RECV_ERROR || res == CURLE_SEND_ERROR && responseCode == 452 || responseCode == 421 || responseCode == 451 || responseCode == 550) {
          if (retries > 5) {
            WriteDataToFile(LIMITEDSMTP, CURRENTSMTP);
            POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
            if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
              std::cout << "ALL SMTP USED UP OR UNAVAILABLE\n"
                << "LOAD MORE SMTP AND RETRY PROGRAM\n"
                << "RATE LIMIT HIT\n";
              break;
            }
          }
          else {
            WriteDataToFile(FAILEDLEADFILE, lead);
            retries++;
            continue;
          }
        }
        SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
      }
      curl_slist_free_all(recipients);
      recipients = nullptr;
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    DATACLEANUP(LEADSFILE, SENTLEADSFILE, FAILEDLEADFILE);
    DATACLEANUP(SMTPFILES, LIMITEDSMTP, DEADSMTPFILE);
    if (!success) {
      return false;
    }
    std::cout << "TOTAL SUCCESSFULLY SENT EMAIL TO " << sentCount << " LEADS \n";
    return true;
  }

  // #: 3 SMTP LIVE TESTER
  bool SMTPLiveTester() {
    CURL* curl;
    CURLcode res;
    long responseCode;
    int port, smtpAssignCounter = 0, totalvalid = 0, totaldeadsmtp = 0;
    std::unordered_set<std::string> invalidsmtp, validsmtp;
    std::string optionStr, filename, servername, username, password, response, TESTEMAIL, boundary = "----=_Part_001_2345_67890";
    std::string SMTPS = fs::path(SMTPLiveTesterDirectory) / "smtps.txt", FAILEDSMTPS = fs::path(SMTPLiveTesterDirectory) / "failedsmtps.txt", LIVESMTPS = fs::path(SMTPLiveTesterDirectory) / "Live.txt";

    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, SMTPS)) {
      std::cerr << "SMTP NOT FOUND IN FILE [smtps.txt]\n";
      return false;
    }

    std::cout << "Enter a valid email address: ";
    std::cin >> TESTEMAIL;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    clearScreen();
    std::cout << "SMTP Live Tester Initialized...\n"
      << "Please wait while the program is testing the SMTPs...\n\n";
    sleep(1);

    for (auto& smtp : SMTPVectorObject.MailDataSetVector) {
      std::stringstream ss(smtp);
      std::getline(ss, servername, '|');
      ss >> port;
      ss.ignore();
      std::getline(ss, username, '|');
      std::getline(ss, password);
      curl = curl_easy_init();
      if (!curl) {
        std::cerr << "Failed to initialize, please check network and try again." << std::endl;
        return false;
      }
      struct curl_slist* recipients = nullptr;
      recipients = curl_slist_append(recipients, TESTEMAIL.c_str());
      response.clear();
      const std::string data =
        "Subject: SMTP Test OK\r\n"
        "From: <" +
        username +
        ">\r\n"
        "To: <" +
        TESTEMAIL +
        ">\r\n"
        "MIME-Version: 1.0\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "Message-ID: <" +
        std::to_string(time(nullptr)) + "@" + username +
        ">\r\n"
        "\r\n"
        "Hello,\r\n" +
        username +
        "\r\n"
        "This is a test email to check SMTP configuration.\r\n"
        "\r\n"
        "Best regards,\r\n"
        "Your SMTP Test Response\r\n"
        "\r\n"
        ".\r\n";
      SetCurlForMail(curl, servername, port, username, password);
      curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
      curl_easy_setopt(curl, CURLOPT_MAIL_FROM, username.c_str());
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
      curl_easy_setopt(curl, CURLOPT_READDATA, &data);
      res = curl_easy_perform(curl);
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
      if (res == CURLE_OK && responseCode == 250) {
        SMTPSUCCESSMESSAGE(smtp);
        validsmtp.insert(smtp);
        totalvalid++;
      }
      else {
        SMTPFAILEDMESSAGE(smtp);
        invalidsmtp.insert(smtp);
      }
      curl_slist_free_all(recipients);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    std::cout << totalvalid << " LIVE SMTPS SUCCESSFULLY TESTED." << std::endl;

    std::fstream AvailableSMTPFile(LIVESMTPS, std::ios::app), UnavailableSMTPFile(FAILEDSMTPS, std::ios::app);
    if (!AvailableSMTPFile || !UnavailableSMTPFile) {
      std::cout << "[VALID SMTPS]" << std::endl;
      for (const auto& smtp : validsmtp) {
        std::cout << smtp << std::endl;
      }
      std::cout << "\n";
      return true;
    }
    for (const auto& smtp : validsmtp) {
      AvailableSMTPFile << smtp << std::endl;
    }
    for (const auto& smtp : invalidsmtp) {
      UnavailableSMTPFile << smtp << std::endl;
    }
    AvailableSMTPFile.close();
    UnavailableSMTPFile.close();
    return true;
  }

  bool EmailExtractor() {
    std::string rawfile = fs::path(EmailExtractorDirectory) / "rawfile.txt";
    std::string extracted = fs::path(EmailExtractorDirectory) / "extracted.txt";
    std::ifstream RawFileAccess(rawfile);
    if (!RawFileAccess) {
      std::cerr << "Failed to open rawfile.txt ";
      return false;
    }
    std::string rawcontent((std::istreambuf_iterator<char>(RawFileAccess)), std::istreambuf_iterator<char>());
    RawFileAccess.close();
    std::regex EmailPattern(R"(([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}))");
    std::set<std::string> emails;
    std::smatch match;
    auto searchStart = rawcontent.cbegin();
    while (std::regex_search(searchStart, rawcontent.cend(), match, EmailPattern)) {
      emails.insert(match.str());
      searchStart = match.suffix().first;
    }
    int totalemails = emails.size();
    std::fstream ExtractedFileAccess(extracted, std::ios::out);
    if (!ExtractedFileAccess) {
      std::cerr << "Failed to access extracted.txt \n\n";
      return false;
    }
    for (auto& email : emails) {
      ExtractedFileAccess << email << std::endl;
    }
    ExtractedFileAccess.close();
    std::cout << "Total email extracted " << totalemails << std::endl;
    return true;
  }

  bool DuplicatesRemover() {
   
    const std::string filename_1 = "Unfiltered.txt";
    const std::string filename_2 = "Filtered.txt";

    const std::string AssetFilePath = fs::path(DuplicateRemoval) / filename_1;
    const std::string CleanedFilePath = fs::path(DuplicateRemoval) / filename_2;
    try {
      std::ifstream Filedata(AssetFilePath);
      if (!Filedata) {
        std::cout << "Error: failed to access " << filename_1 << std::endl;
        return false;
      }

      std::unordered_set<std::string> dataset;
      std::string line;
      while (std::getline(Filedata, line)) {
        if (!line.empty()) {
          dataset.insert(line);
        }
      }
      std::ofstream outFile(filename_2);
      if (!outFile) {
        std::cerr << "Error: failed to write to " << filename_2 << std::endl;
        return false;
      }

      for (const auto& unique : dataset) {
        outFile << unique << std::endl;
      }
    }
    catch (const std::exception& error) {
      std::cerr << "Operation failed becase " << error.what() << std::endl;
    }
    return true;
  }
};

void clearScreenWithMessage(const std::string& message) {
  clearScreen();
  std::cout << message << "\n";
}

bool askYesNoQuestion(const std::string& question) {
  std::string optionStr;
  std::cout << question << " (Y/N): ";
  std::cin >> optionStr;
  return (optionStr == "Y" || optionStr == "y");
}

void handleProgramCompletion(bool success, const std::string& programName) {
  sleep(1);
  if (success) {
    std::cout << programName << " PROGRAM COMPLETED.\n";
  }
  else {
    std::cout << programName << " PROGRAM FAILED.\n";
  }
}

void DisplayStartMenu(int& option) {
  std::string optionStr;
  std::cout << "\033[91m"
    << "\033[1m"
    << "\t[ECHO MAILER V1 : TELEGRAM H4CKECHO]\n\n"
    << "\033[94m" << "[1] " << "\033[93m" << "EMAIL SENDER\n"
    << "\033[94m" << "[2] " << "\033[93m" << "SMTP MAIL TESTER\n"
    << "\033[94m" << "[3] " << "\033[93m" << "EMAIL LEAD EXTRACTOR\n"
    << "\033[94m" << "[4] " << "\033[93m" << "DATA DUPLICATE REMOVAL\n\n"
    << "\033[92m" << "\033[1m" << "CHOOSE A PROGRAM [0 END PROGRAM]: ";
  std::cin >> optionStr;
  if (!ValidDigit(optionStr)) {
    do {
      clearScreen();
      std::cout << "Wrong input choice\nTry again with correct choice\n";
      sleep(1);
      DisplayStartMenu(option);
    } while (!ValidDigit(optionStr));
  }
  option = std::stoi(optionStr);
}

void TerminateProgram() {
  std::cout
    << "\033[2J\033[H"
    << "Terminating program...\n";
  sleep(1);
  clearScreen();
}

int main(void) {
  GlobalMethodClass GlobalMethodObject;
  EmailSenderProgram EmailProgramObject;
  std::string optionStr;
  bool UseAttachment, UseHTML;
  int option;

  clearScreen();
  DisplayStartMenu(option);

  switch (option) {
  case 0: {
    TerminateProgram();
    return 0;
  }

  case 1: {
    if (!EmailProgramObject.PrepareEmailSenderDirectories()) {
      std::cerr << "Error: Program failed to create Email Sender directories\n";
      return 1;
    }
    clearScreenWithMessage("\t\t[EMAIL SENDER IN PROCESS]\n");
    UseHTML = askYesNoQuestion("ARE YOU SENDING HTML LETTER");
    UseAttachment = askYesNoQuestion("ARE YOU USING ATTACHMENT");

    bool success = EmailProgramObject.EmailSender(UseHTML, UseAttachment);
    handleProgramCompletion(success, "EMAIL SENDER");
    GlobalMethodObject.prompt();
    TerminateProgram();
    break;
  }

  case 2: {
    if (!EmailProgramObject.PrepareSMTPLiveTesterDirectories()) {
      std::cout << "Program runtime error, please restart program.\n";
      GlobalMethodObject.prompt();
    }
    std::cout << "Please make sure program files are not empty\n"
      << "Do you wanna continue (Y/N): ";
    std::cin >> optionStr;
    if (optionStr != "Y" || optionStr != "y") {
      clearScreen();
      main();
    }
    clearScreenWithMessage("\t\t[SMTP LIVE TESTER INITIALIZED]");
    bool success = EmailProgramObject.SMTPLiveTester();
    handleProgramCompletion(success, "SMTP Live Tester");
    GlobalMethodObject.prompt();
    TerminateProgram();
    break;
  }

  case 3: {
    EmailProgramObject.PrepareEmailExtractorDirectories();
    clearScreenWithMessage("[EMAIL EXTRACTOR INITIALIZED]\n");
    std::cout << "PASTE THE DATA INSIDE  [rawfile.txt] LOCATED IN [Email Extractor] FOLDER\n";
    if (askYesNoQuestion("Continue extraction?:")) {
      bool success = EmailProgramObject.EmailExtractor();
      handleProgramCompletion(success, "Email Extraction");
      GlobalMethodObject.prompt();
    }
    TerminateProgram();
    break;
  }

  case 4: {
    clearScreen();
    if (!EmailProgramObject.PrepareDuplicateDirectories()) {
      std::cerr << "Failed to prepare this program files.\n";
      break;
    }
    std::cout << "Copy and paste unfiltered data in unfiltered.txt\nDo you wanna continue? (Y/N): ";
    std::cin >> optionStr;
    if (optionStr == "Y" || optionStr == "y") {
      bool success = EmailProgramObject.DuplicatesRemover();
      handleProgramCompletion(success, "Duplicates removal");
    }
    GlobalMethodObject.prompt();
    break;
  }

  default:
    clearScreen();
    std::cout << "Invalid option selected. \n";
    GlobalMethodObject.prompt();
    TerminateProgram();
    break;
  }

  std::cout << "\033[0m";
  return 0;
}