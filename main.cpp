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
#include <limits>


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
  std::string MakeDirectory(const std::string& path, const std::string& folderName) {
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

  std::string GetRandomDataFromVector(const std::vector<std::string>& param) {
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> dis(0, param.size() - 1);
    return param[dis(gen)];
  }

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

  std::string base64Encode(const std::string& filePath) {
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

  void stripdata(std::string& server, std::string& port) {
    std::stringstream ss(server);
    std::getline(ss, server, ':');
    std::getline(ss, port);
  }

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


  bool CreateProgramDirectories(std::string BaseProgramDirectory, std::string& ChildProgramDirectory, std::string& ProgramAttributeDirectory, std::string& Junks) {
    ChildProgramDirectory = MakeDirectory(BaseProgramDirectory, ChildProgramDirectory);
    ProgramAttributeDirectory = MakeDirectory(ChildProgramDirectory, ProgramAttributeDirectory);
    Junks = MakeDirectory(ChildProgramDirectory, Junks);
    return (!BaseProgramDirectory.empty() && !ChildProgramDirectory.empty() && !ProgramAttributeDirectory.empty() && !Junks.empty()) ? true : false;
  }

  void CreateProgramAttributeFiles(std::string& ProgramAttributeDirectory, std::string& Junks, std::vector<std::string>& ProgramAttributeVector) {
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

  void EMAILSUCCESSMESSAGE(const std::string sendername, const std::string subject, const std::string lead, const int count) {
    const std::string green = "\033[92m";
    const std::string yellow = "\033[93m";
    const std::string red = "\033[91m";
    const std::string bold = "\033[1m";
    const std::string reset = "\033[0m";

    std::cout
      << green << "===============================   "
      << red << bold << "EMAIL SENT SUCCESSFUL  "
      << green << "===============================\n"
      << green << "COUNT: " << yellow << "[" << count << "]\n"
      << green << "DATE: " << yellow << GetCurrentDateTime() << "\n"
      << green << "SENDER NAME: " << yellow << sendername << "\n"
      << green << "SUBJECT TITLE: " << yellow << subject << "\n"
      << green << "RECIPIENT EMAIL: " << yellow << lead << "\n"
      << green << "===============================  "
      << red << bold << "ECHOBLAST EMAIL SENDER  "
      << reset << green << "===============================\n\n";
  }

  void SMTPSUCCESSMESSAGE(const std::string smtp) {
    const std::string green = "\033[92m";
    const std::string red = "\033[91m";
    const std::string reset = "\033[0m";
    const std::string yellow = "\033[93m";
    std::cout
      << green
      << "=============================  SMTP TEST SUCCESSFUL  =============================\n"
      << green << "DATE: " << yellow << GetCurrentDateTime() << "\n"
      << green << "SMTP: " << yellow << smtp << std::endl
      << green
      << "==============================  TELEGRAM: ECHOVSL  ===============================\n\n";
  }
  void SMTPFAILEDMESSAGE(const std::string smtp) {
    const std::string green = "\033[92m";
    const std::string red = "\033[91m";
    const std::string reset = "\033[0m";
    const std::string yellow = "\033[93m";

    std::cout
      << red
      << "=============================  SMTP TEST FAILED  =============================\n"
      << green << "DATE: " << yellow << GetCurrentDateTime() << "\n"
      << green << "SMTP: " << smtp << std::endl
      << red
      << "============================  TELEGRAM: ECHOVSL  =============================\n\n"
      << reset << green;
  }

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

  std::string GetCurrentDateTime() {
    time_t now = time(nullptr);
    struct tm tm = *gmtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S +0000", &tm);
    return std::string(buf);
  }

  bool isValidEmail(const std::string& email) {
    const std::regex pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return std::regex_match(email, pattern);
  }
  std::string GetDomain(const std::string& username, const std::string& servername) {
    const size_t at_pos = username.find('@');
    return (at_pos != std::string::npos) ? username.substr(at_pos + 1) : servername;
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
    ProgramAttributeVector = { "smtps.txt", "sendername.txt", "subject.txt", "letter.txt", "leads.txt", "sentleads.txt", "failedleads.txt", "deadsmtps.txt", "smtps_under_limit.txt" };
    if (!CreateProgramDirectories(EmailSenderPrograms, EmailProgramDirectory, EmailDataDirectory, EmailJunkDirectory)) return false;
    CreateProgramAttributeFiles(EmailDataDirectory, EmailJunkDirectory, ProgramAttributeVector);
    return true;
  }

  bool PrepareSMTPLiveTesterDirectories() {
    ProgramAttributeVector.clear();
    ProgramAttributeVector = { "Raw.txt", "Failed.txt", "Live.txt" };
    SMTPLiveTesterDirectory = MakeDirectory(EmailSenderPrograms, SMTPLiveTesterDirectory);
    clearScreen();
    for (auto& file : ProgramAttributeVector) {
      std::string filePath = fs::path(SMTPLiveTesterDirectory) / file;
      std::fstream CreateFile(filePath, std::ios::app);
      if (CreateFile) {
        CreateFile.close();
      }
    }
    return true;
  }

  bool PrepareEmailExtractorDirectories() {
    ProgramAttributeVector.clear();
    ProgramAttributeVector = { "Raw.txt", "Result.txt" };
    EmailExtractorDirectory = MakeDirectory(EmailSenderPrograms, EmailExtractorDirectory);
    for (auto& file : ProgramAttributeVector) {
      std::string FilePath = fs::path(EmailExtractorDirectory) / file;
      std::fstream CreateFile(FilePath, std::ios::app);
      if (!CreateFile) CreateFile.close();
    }
    return true;
  }

  bool PrepareDuplicateDirectories() {
    ProgramAttributeVector.clear();
    ProgramAttributeVector = { "Raw.txt", "Result.txt" };
    DuplicateRemoval = MakeDirectory(EmailSenderPrograms, DuplicateRemoval);
    for (auto& file : ProgramAttributeVector) {
      std::string FilePath = fs::path(DuplicateRemoval) / file;
      std::fstream CreateFile(FilePath, std::ios::app);
      if (!CreateFile) {
        std::cerr << "Failed to create " << file << std::endl;
        return false;
      }
      CreateFile.close();
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

  std::string GenerateMessageID(const std::string& username) {
    size_t at_pos = username.find('@');
    std::string domain = (at_pos != std::string::npos) ? username.substr(at_pos) : "admin@sendgrid.com";
    return GenerateUUID() + domain;
  }

  std::string StripHTML(const std::string& html) {
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
    std::string& EmailContentBody, bool useHTML, bool useAttachment,
    const std::string& servername, const std::string& sendername, const std::string& username,
    const std::string& subject, const std::string& letter, const std::string& lead,
    const std::string& AttachmentFilePath = "") {
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

  bool InsertVariableEmail(const std::string letter, const std::string& email, std::string& body) {
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

  int GenerateDelay() {
    static int delays[] = { 10, 11, 12, 13, 14, 15 };
    static int size = 6;
    static std::mt19937 rng(std::random_device{}());

    if (size == 0) {
      size = 6;
      std::shuffle(delays, delays + size, rng);
    }

    return delays[--size];
  }

  bool EmailSender(bool useHTML, bool useAttachment) {
    std::string LEADSFILE = fs::path(EmailDataDirectory) / "leads.txt";
    std::string letterPath = fs::path(EmailDataDirectory) / "letter.txt";
    std::string SENTLEADSFILE = fs::path(EmailDataDirectory) / "sentleads.txt";
    std::string FAILEDLEADFILE = fs::path(EmailJunkDirectory) / "failedleads.txt";
    std::string DEADSMTPFILE = fs::path(EmailJunkDirectory) / "deadsmtps.txt";
    std::string LIMITEDSMTP = fs::path(EmailJunkDirectory) / "smtps_under_limit.txt";
    std::string SMTPFILES = fs::path(EmailDataDirectory) / "smtps.txt";
    std::string SENDERNAMESFILE = fs::path(EmailDataDirectory) / "sendername.txt";
    std::string SUBJECTSFILE = fs::path(EmailDataDirectory) / "subject.txt";
    std::string letter, attachmentPath, emailBody, attachmentFileName, sender, subject, CURRENTSMTP;
    int sentCount = 0, rateLimit = 0, rateCount = 0, index = 0, retries = 0, sendspeed = 2;
    long responseCode;
    std::vector<std::string> leads;
    std::ofstream sentLeads(SENTLEADSFILE, std::ios::app), failedLeads(FAILEDLEADFILE, std::ios::app),
      deadSMTPs(DEADSMTPFILE, std::ios::app), limitedSMTPs(LIMITEDSMTP, std::ios::app);
    if (!sentLeads || !failedLeads || !deadSMTPs || !limitedSMTPs) {
      std::cerr << "Failed to open output files.\n";
      return false;
    }
    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, SMTPFILES) ||
      !ReadFileToVector(NameVectorObject.MailDataSetVector, SENDERNAMESFILE) ||
      !ReadFileToVector(SubjectVectorObject.MailDataSetVector, SUBJECTSFILE) ||
      !ReadFileToVector(leads, LEADSFILE)) {
      std::cerr << "FAILED TO READ EMAIL ATTRIBUTES\n";
      return false;
    }
    if (!FetchDataFromFile(letterPath, letter)) {
      std::cerr << "EMAIL LETTER NOT FOUND.\n";
      return false;
    }
    if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
      SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
      std::cerr << "NO SMTP FOUND INITIALLY.\n";
      return false;
    }
    if (useAttachment) {
      attachmentPath = MakeDirectory(EmailDataDirectory, "Attachment");
      clearScreen();
      std::cout << "Provide Attachment filename (filename.pdf): ";
      std::getline(std::cin >> std::ws, attachmentFileName);
      if (attachmentFileName.empty()) {
        std::cerr << "Invalid attachment filename. Using no attachment.\n";
        useAttachment = false;
      }
      else {
        attachmentFileName = fs::path(attachmentPath) / attachmentFileName;
      }
    }
    if (SMTPVectorObject.MailDataSetVector.size() > 1) {
      clearScreen();
      std::cout << "TOTAL " << SMTPVectorObject.MailDataSetVector.size() << " SMTPS DETECTED.\n"
        << "NOTE: SET SEND RATE TO ROTATE AND EXTEND SMTP LIFE.\n\n"
        << "DO SET SEND RATE PER SMTP: (Y/N): ";
      std::string tmp;
      std::getline(std::cin >> std::ws, tmp);
      if (tmp.empty() || (tmp != "Y" && tmp != "y" && tmp != "N" && tmp != "n")) {
        std::cerr << "INVALID RESPONSE, RATE LIMIT NOT SET.\n";
      }
      else if (tmp == "Y" || tmp == "y") {
        std::cout << "SET SEND RATE PER SMTP: ";
        if (!(std::cin >> rateLimit) || rateLimit < 0) {
          std::cin.clear();
          std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          std::cerr << "INVALID INPUT, RATE LIMIT NOT SET.\n";
          rateLimit = 0;
        }
      }
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    clearScreen();
    std::cout << "NOTE: BLAST DELAY PRESERVES SMTP LIFE.\n"
      << "NOTE: SYSTEM DEFINED DELAY RANGE 10-15 SECONDS SIMULTANEOUSLY.\n"
      << "NOTE: IF CUSTOM DELAY FALLS BELOW 5 SECONDS, PROGRAM FALLS BACK TO SYSTEM DEFINED DELAY.\n\n"
      << "SET CUSTOM DELAY (MIN 5): ";
    if (!(std::cin >> sendspeed) || sendspeed < 5) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cerr << "INVALID CHOICE, PROGRAM FALL BACK TO DEFAULT DELAY.\n";
      sendspeed = GenerateDelay();
      sleep(2);
    }
    else {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (!curl) {
      std::cerr << "INITIALIZATION FAILED.\nPLEASE CHECK YOUR INTERNET OR VPN/PROXY IF IN USE.\n";
      curl_global_cleanup();
      return false;
    }

    SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
      SMTPAttributeObject.username, SMTPAttributeObject.password);
    clearScreenWithMessage("\t\t\t[EMAIL SENDER INITIALIZED]\n");

    for (const auto& lead : leads) {
      if (sentCount > 0) sleep(sendspeed);

      if (rateLimit > 0 && rateCount >= rateLimit) {
        std::cout << "\n\nSEND RATE LIMIT SET IS REACHED FOR CURRENT SMTP\n[" << CURRENTSMTP << "]\n";
        sleep(2);
        std::cout << "LOADING NEW SMTP IN PROGRESS.\n";
        if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
          SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
          std::cout << "ALL SMTP USED UP OR UNAVAILABLE\n"
            << "LOAD MORE SMTP AND RETRY PROGRAM\n";
          if (sentCount <= 0) return false;
        }
        else {
          SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
            SMTPAttributeObject.username, SMTPAttributeObject.password);
          rateCount = 0;
        }
      }

      sender = GetRandomDataFromVector(NameVectorObject.MailDataSetVector);
      subject = GetRandomDataFromVector(SubjectVectorObject.MailDataSetVector);
      MakeEmailBody(emailBody, useHTML, useAttachment, SMTPAttributeObject.servername,
        sender, SMTPAttributeObject.username, subject, letter, lead, attachmentFileName);

      curl_slist* recipients = curl_slist_append(nullptr, lead.c_str());
      curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
      curl_easy_setopt(curl, CURLOPT_MAIL_FROM, SMTPAttributeObject.username.c_str());
      curl_easy_setopt(curl, CURLOPT_READDATA, &emailBody);
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);

      CURLcode res = curl_easy_perform(curl);
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

      if (res == CURLE_OK && responseCode == 250) {
        sentLeads << lead << '\n';
        sentCount++;
        rateCount++;
        retries = 0;
        EMAILSUCCESSMESSAGE(sender, subject, lead, sentCount);
      }
      else {
        if (res == CURLE_LOGIN_DENIED || responseCode == 535 || responseCode == 530 || responseCode == 534) {
          std::cout << "\n\nCURRENT SMTP [" << CURRENTSMTP << "] IS DEAD.\n";
          deadSMTPs << CURRENTSMTP << '\n';
          POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
          sleep(2);
          std::cout << "LOADING NEW SMTP IN PROGRESS.\n";
          if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
            SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
            std::cout << "ALL SMTPS ARE USED UP OR UNAVAILABLE.\n"
              << "LOAD MORE SMTP AND RETRY PROGRAM.\n";
            if (sentCount <= 0) return false;
          }
          else {
            SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
              SMTPAttributeObject.username, SMTPAttributeObject.password);
          }
        }
        else if ((res == CURLE_RECV_ERROR || res == CURLE_SEND_ERROR) &&
          (responseCode == 452 || responseCode == 421 || responseCode == 451 || responseCode == 550)) {
          if (retries >= 5) {
            std::cout << "\n\nCURRENT SMTP [" << CURRENTSMTP << "] IS HIT LIMIT\n";
            limitedSMTPs << CURRENTSMTP << '\n';
            POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
            sleep(2);
            std::cout << "LOADING NEW SMTP IN PROGRESS.\n";
            if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
              SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
              std::cout << "ALL SMTP USED UP OR UNAVAILABLE\n"
                << "LOAD MORE SMTP AND RETRY PROGRAM\n";
              if (sentCount <= 0) return false;
            }
            else {
              SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
                SMTPAttributeObject.username, SMTPAttributeObject.password);
              retries = 0;
            }
          }
          else {
            failedLeads << lead << '\n';
            retries++;
            continue;
          }
        }
        else {
          SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
            SMTPAttributeObject.username, SMTPAttributeObject.password);
        }
      }
      curl_slist_free_all(recipients);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    sentLeads.close();
    failedLeads.close();
    deadSMTPs.close();
    limitedSMTPs.close();
    DATACLEANUP(LEADSFILE, SENTLEADSFILE, FAILEDLEADFILE);
    DATACLEANUP(SMTPFILES, LIMITEDSMTP, DEADSMTPFILE);

    clearScreen();
    if (sentCount > 0) std::cout << "TOTAL SUCCESSFULLY SENT EMAIL TO " << sentCount << " LEADS \n";
    return sentCount > 0;
  }

  bool VariableEmailSender(bool useHTML, bool useAttachment) {
    std::string LEADSFILE = fs::path(EmailDataDirectory) / "leads.txt";
    std::string letterPath = fs::path(EmailDataDirectory) / "letter.txt";
    std::string SENTLEADSFILE = fs::path(EmailDataDirectory) / "sentleads.txt";
    std::string FAILEDLEADFILE = fs::path(EmailJunkDirectory) / "failedleads.txt";
    std::string DEADSMTPFILE = fs::path(EmailJunkDirectory) / "deadsmtps.txt";
    std::string LIMITEDSMTP = fs::path(EmailJunkDirectory) / "smtps_under_limit.txt";
    std::string SMTPFILES = fs::path(EmailDataDirectory) / "smtps.txt";
    std::string SENDERNAMESFILE = fs::path(EmailDataDirectory) / "sendername.txt";
    std::string SUBJECTSFILE = fs::path(EmailDataDirectory) / "subject.txt";
    std::string attachmentPath, emailBody, attachmentFileName, sender, subject, CURRENTSMTP, body;
    int sentCount = 0, rateLimit = 0, rateCount = 0, index = 0, retries = 0, sendspeed = 2;
    long responseCode;
    std::vector<std::string> leads;
    std::ofstream sentLeads(SENTLEADSFILE, std::ios::app), failedLeads(FAILEDLEADFILE, std::ios::app),
      deadSMTPs(DEADSMTPFILE, std::ios::app), limitedSMTPs(LIMITEDSMTP, std::ios::app);
    if (!sentLeads || !failedLeads || !deadSMTPs || !limitedSMTPs) {
      std::cerr << "Failed to open output files.\n";
      return false;
    }
    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, SMTPFILES) ||
      !ReadFileToVector(NameVectorObject.MailDataSetVector, SENDERNAMESFILE) ||
      !ReadFileToVector(SubjectVectorObject.MailDataSetVector, SUBJECTSFILE) ||
      !ReadFileToVector(leads, LEADSFILE)) {
      std::cerr << "FAILED TO READ EMAIL ATTRIBUTES\n";
      return false;
    }
    if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
      SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
      std::cerr << "NO SMTP FOUND INITIALLY.\n";
      return false;
    }
    if (useAttachment) {
      attachmentPath = MakeDirectory(EmailDataDirectory, "Attachment");
      clearScreen();
      std::cout << "Provide Attachment filename (e.g., file.pdf): ";
      std::getline(std::cin >> std::ws, attachmentFileName);
      if (attachmentFileName.empty()) {
        std::cerr << "Invalid attachment filename. Using no attachment.\n";
        useAttachment = false;
      }
      else {
        attachmentFileName = fs::path(attachmentPath) / attachmentFileName;
      }
    }
    if (SMTPVectorObject.MailDataSetVector.size() > 1) {
      clearScreen();
      std::cout << SMTPVectorObject.MailDataSetVector.size() << " SMTPS FOUND.\n"
        << "Set rate limit (Y/N): ";
      std::string tmp;
      std::getline(std::cin >> std::ws, tmp);
      if (tmp.empty() || (tmp != "Y" && tmp != "y" && tmp != "N" && tmp != "n")) {
        std::cerr << "Invalid input. Rate limit not set.\n";
      }
      else if (tmp == "Y" || tmp == "y") {
        std::cout << "Specify send rate limit: ";
        if (!(std::cin >> rateLimit) || rateLimit < 0) {
          std::cin.clear();
          std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          std::cerr << "Invalid rate limit. Rate limit not set.\n";
          rateLimit = 0;
        }
      }
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    clearScreen();
    std::cout << "BLAST DELAY PRESERVES SMTP LIFE.\n"
      << "SYSTEM DEFINED DELAY RANGE 10-15 SECONDS SIMULTANEOUSLY.\n"
      << "NOTE: IF CUSTOM DELAY FALLS BELOW 5 SECONDS, PROGRAM FALLS BACK TO SYSTEM DEFINED DELAY.\n"
      << "SET CUSTOM DELAY (MIN 5): ";
    if (!(std::cin >> sendspeed) || sendspeed < 5) {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      std::cerr << "Invalid delay. Using system-defined delay.\n";
      sendspeed = GenerateDelay();
    }
    else {
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    std::string letter;
    if (!FetchDataFromFile(letterPath, letter)) {
      std::cerr << "EMAIL LETTER NOT FOUND.\n";
      return false;
    }
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (!curl) {
      std::cerr << "INITIALIZATION FAILED.\nPLEASE CHECK YOUR INTERNET OR VPN/PROXY IF IN USE.\n";
      curl_global_cleanup();
      return false;
    }

    SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
      SMTPAttributeObject.username, SMTPAttributeObject.password);
    clearScreenWithMessage("\t\t\t[EMAIL SENDER INITIALIZED]\n");
    for (const auto& lead : leads) {
      if (sentCount > 0) sleep(sendspeed);
      if (rateLimit > 0 && rateCount >= rateLimit) {
        std::cout << "\n\nSEND RATE LIMIT SET IS REACHED FOR CURRENT SMTP\n[" << CURRENTSMTP << "]\n";
        sleep(2);
        std::cout << "LOADING NEW SMTP IN PROGRESS.\n";
        if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
          SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
          std::cout << "ALL SMTP USED UP OR UNAVAILABLE\n"
            << "LOAD MORE SMTP AND RETRY PROGRAM\n";
          if (sentCount <= 0) return false;
        }
        else {
          SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
            SMTPAttributeObject.username, SMTPAttributeObject.password);
          rateCount = 0;
        }
      }
      sender = GetRandomDataFromVector(NameVectorObject.MailDataSetVector);
      subject = GetRandomDataFromVector(SubjectVectorObject.MailDataSetVector);
      if (!InsertVariableEmail(letter, lead, body)) {
        std::cerr << "EMAIL VARIABLE NOT FOUND.\n"
          << "PLEASE USE CONSTANT EMAIL SENDER\n";
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        clearScreen();
        sleep(3);
        return false;
      }
      MakeEmailBody(emailBody, useHTML, useAttachment, SMTPAttributeObject.servername,
        sender, SMTPAttributeObject.username, subject, body, lead, attachmentFileName);
      curl_slist* recipients = curl_slist_append(nullptr, lead.c_str());
      curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
      curl_easy_setopt(curl, CURLOPT_MAIL_FROM, SMTPAttributeObject.username.c_str());
      curl_easy_setopt(curl, CURLOPT_READDATA, &emailBody);
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
      CURLcode res = curl_easy_perform(curl);
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

      if (res == CURLE_OK && responseCode == 250) {
        sentLeads << lead << '\n';
        sentCount++;
        rateCount++;
        retries = 0;
        EMAILSUCCESSMESSAGE(sender, subject, lead, sentCount);
      }
      else {
        if (res == CURLE_LOGIN_DENIED || responseCode == 535 || responseCode == 530 || responseCode == 534) {
          std::cout << "\n\nCURRENT SMTP [" << CURRENTSMTP << "] IS DEAD.\n";
          deadSMTPs << CURRENTSMTP << '\n';
          POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
          sleep(2);
          std::cout << "LOADING NEW SMTP IN PROGRESS.\n";
          if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
            SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
            std::cout << "ALL SMTPS ARE USED UP OR UNAVAILABLE.\n"
              << "LOAD MORE SMTP AND RETRY PROGRAM.\n";
            if (sentCount <= 0) return false;
          }
          else {
            SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
              SMTPAttributeObject.username, SMTPAttributeObject.password);
          }
        }
        else if ((res == CURLE_RECV_ERROR || res == CURLE_SEND_ERROR) &&
          (responseCode == 452 || responseCode == 421 || responseCode == 451 || responseCode == 550)) {
          if (retries >= 5) {
            std::cout << "\n\nCURRENT SMTP [" << CURRENTSMTP << "] IS HIT LIMIT\n";
            limitedSMTPs << CURRENTSMTP << '\n';
            POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
            sleep(2);
            std::cout << "LOADING NEW SMTP IN PROGRESS.\n";
            if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
              SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
              std::cout << "ALL SMTP USED UP OR UNAVAILABLE\n"
                << "LOAD MORE SMTP AND RETRY PROGRAM\n";
              if (sentCount <= 0) return false;
            }
            else {
              SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
                SMTPAttributeObject.username, SMTPAttributeObject.password);
              retries = 0;
            }
          }
          else {
            failedLeads << lead << '\n';
            retries++;
            continue;
          }
        }
        else {
          SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
            SMTPAttributeObject.username, SMTPAttributeObject.password);
          retries = 0;
        }
      }
      curl_slist_free_all(recipients);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    sentLeads.close();
    failedLeads.close();
    deadSMTPs.close();
    limitedSMTPs.close();
    DATACLEANUP(LEADSFILE, SENTLEADSFILE, FAILEDLEADFILE);
    DATACLEANUP(SMTPFILES, LIMITEDSMTP, DEADSMTPFILE);

    clearScreen();
    if (sentCount > 0) std::cout << "TOTAL SUCCESSFULLY SENT EMAIL TO " << sentCount << " LEADS \n";
    return sentCount > 0;
  }

  // #: 3 SMTP LIVE TESTER
  bool SMTPLiveTester() {
    int trials = 3;
    const std::string sendername = "ECHOVSL SENDER";
    CURL* curl = curl_easy_init();
    if (!curl) {
      std::cerr << "CURL initialization failed.\n";
      return false;
    }

    std::string SMTPS = fs::path(SMTPLiveTesterDirectory) / "Raw.txt",
      FAILEDSMTPS = fs::path(SMTPLiveTesterDirectory) / "Failed.txt",
      LIVESMTPS = fs::path(SMTPLiveTesterDirectory) / "Live.txt";

    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, SMTPS)) {
      std::cerr << "SMTP RAW FILE IS EMPTY.\n";
      curl_easy_cleanup(curl);
      return false;
    }

    std::string Email;
    clearScreen();
    std::cout << "\t\t[SMTP LIVE TEST PROGRAM IN PROCESS]\n\nProvide Email Address: ";
    std::cin >> Email;
    while (Email.empty() || !isValidEmail(Email)) {
      clearScreen();
      if (trials-- <= 0) {
        std::cerr << "Too many invalid attempts. Exiting program.\n";
        curl_easy_cleanup(curl);
        sleep(2);
        return false;
      }
      std::cout << "\t\t[SMTP LIVE TEST PROGRAM IN PROCESS]\n\n"
        << "Invalid Email Address.\nPlease enter a valid email address: ";
      std::cin >> Email;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::unordered_set<std::string> validsmtp, invalidsmtp;
    int totalvalid = 0;
    std::ofstream AvailableSMTPFile(LIVESMTPS, std::ios::app), UnavailableSMTPFile(FAILEDSMTPS, std::ios::app);

    if (!AvailableSMTPFile || !UnavailableSMTPFile) {
      std::cerr << "Failed to open output files.\n";
      curl_easy_cleanup(curl);
      curl_global_cleanup();
      return false;
    }
    clearScreenWithMessage("\t\t[SMTP LIVE TEST PROGRAM PROGRESSING]\n");
    for (const auto& smtp : SMTPVectorObject.MailDataSetVector) {
      std::stringstream ss(smtp);
      std::string servername, username, password;
      int port;
      std::getline(ss, servername, '|');
      ss >> port;
      ss.ignore();
      std::getline(ss, username, '|');
      std::getline(ss, password);

      struct curl_slist* recipients = curl_slist_append(nullptr, Email.c_str());
      const std::string data =
        "From: \"" + sendername + "\" <" + username + ">\r\n"
        "To: <" + Email + ">\r\n"
        "Subject: SMTP Connectivity Confirmation\r\n"
        "Date: " + GetCurrentDateTime() + "\r\n"
        "Message-ID: <" + std::to_string(time(nullptr)) + "." + GenerateUUID() + "@" + GetDomain(username, servername) + ">\r\n"
        "Reply-To: \"" + sendername + "\" <" + username + ">\r\n"
        "Return-Path: <" + username + ">\r\n"
        "MIME-Version: 1.0\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "Content-Transfer-Encoding: quoted-printable\r\n"
        "\r\n" +
        QuotedPrintableEncode(
          "Hello Friend,\r\n\r\n"
          "This is a protocol message test to agree the functionality of our email service.\r\n"
          "Best regards,\r\n" + sendername + "\r\n"
          "[Contact] TG: ECHOVSL\r\n"
        ) + "\r\n";

      SetCurlForMail(curl, servername, port, username, password);
      curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
      curl_easy_setopt(curl, CURLOPT_MAIL_FROM, username.c_str());
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
      curl_easy_setopt(curl, CURLOPT_READDATA, &data);

      long responseCode;
      CURLcode res = curl_easy_perform(curl);
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

      if (res == CURLE_OK && responseCode == 250) {
        SMTPSUCCESSMESSAGE(smtp);
        validsmtp.insert(smtp);
        AvailableSMTPFile << smtp << '\n';
        totalvalid++;
      }
      else {
        SMTPFAILEDMESSAGE(smtp);
        invalidsmtp.insert(smtp);
        UnavailableSMTPFile << smtp << '\n';
      }
      curl_slist_free_all(recipients);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    AvailableSMTPFile.close();
    UnavailableSMTPFile.close();

    clearScreen();
    std::cout << "\n" << totalvalid << " SMTP" << (totalvalid == 1 ? " IS" : "S ARE") << " TESTED VALID.\n\n";
    return true;
  }

  bool EmailExtractor() {
    std::string rawfile = fs::path(EmailExtractorDirectory) / "Raw.txt";
    std::string extracted = fs::path(EmailExtractorDirectory) / "Result.txt";

    std::ifstream RawFileAccess(rawfile);
    if (!RawFileAccess) {
      std::cerr << "FAILED TO OPEN RAW FILE\n";
      return false;
    }

    std::ofstream ExtractedFileAccess(extracted);
    if (!ExtractedFileAccess) {
      std::cerr << "FAILED TO OPEN RESULT FILE\n";
      return false;
    }

    std::regex EmailPattern(R"(([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}))");
    std::set<std::string> uniqueEmails;

    std::string line;
    while (std::getline(RawFileAccess, line)) {
      std::smatch match;
      auto searchStart = line.cbegin();
      while (std::regex_search(searchStart, line.cend(), match, EmailPattern)) {
        uniqueEmails.insert(match.str());
        searchStart = match.suffix().first;
      }
    }

    for (const auto& email : uniqueEmails) {
      ExtractedFileAccess << email << '\n';
    }

    std::cout << uniqueEmails.size() << " TOTAL EMAILS EXTRACTED\n";
    return true;
  }

  bool DuplicatesRemover() {
    clearScreen();
    std::cout << "\t\t[DATA DUPLICATE REMOVER IN PROGRESS]\n";
    const std::string f1 = "Raw.txt";
    const std::string f2 = "Result.txt";
    const std::string Raw = fs::path(DuplicateRemoval) / f1;
    const std::string Result = fs::path(DuplicateRemoval) / f2;
    int linecount = 0;
    int duplicatecount = 0;
    try {
      std::ifstream Filedata(Raw);
      if (!Filedata) {
        clearScreen();
        std::cout << "PROGRAM FAILED TO ACCESS " << Raw << std::endl;
        return false;
      }

      std::unordered_set<std::string> dataset;
      std::string line;
      while (std::getline(Filedata, line)) {
        if (line.empty() && linecount == 0) {
          std::cerr << "No data found to filter duplicates\n";
          return false;
        }
        if (line.empty() && linecount > 0) continue;
        dataset.insert(line);
        linecount++;

      }
      std::ofstream outFile(Result);
      if (!outFile) {
        clearScreen();
        std::cerr << "PROGRAM FAILED TO ACCESS " << f2 << std::endl;
        return false;
      }

      for (const auto& unique : dataset) {
        if (unique.empty()) continue;
        outFile << unique << std::endl;
        duplicatecount++;
      }
    }
    catch (const std::exception& error) {
      std::cerr << "PROGRAM FAILED BECAUSE " << error.what() << std::endl;
      return false;
    }
    clearScreen();
    std::cout << "TOTAL " << linecount - duplicatecount << " DUPLICATES MERGED.\n";
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
  success ?
    std::cout << programName << " PROGRAM COMPLETED.\n" :
    std::cout << programName << " PROGRAM FAILED.\n";

}

void DisplayStartMenu(int& option) {
  std::string optionStr;
  std::cout << "\033[91m"
    << "\033[1m"
    << "\t[ECHO MAILER V2 : TELEGRAM H4CKECHO]\n\n"
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
  clearScreen();
  std::cout << "Terminating program...\n";
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
    clearScreen();
    std::cout << "\033[91m"
      << "\033[1m"
      << "\t[ECHO MAILER V1 : TELEGRAM H4CKECHO]\n\n"
      << "\033[94m" << "[1] " << "\033[93m" << "CONSTANT SENDER\n"
      << "\033[94m" << "[2] " << "\033[93m" << "VARIABLE SENDER\n\n"
      << "\033[92m" << "CHOOSE SENDER TYPE: ";
    std::cin >> optionStr;
    if (optionStr != "1" && optionStr != "2") {
      std::cerr << "Invalid option selected. \n";
      sleep(1);
      clearScreen();
      DisplayStartMenu(option);
    }
    if (optionStr == "1") {
      if (!EmailProgramObject.PrepareEmailSenderDirectories()) {
        std::cerr << "Error: Program failed to create Email Sender directories\n";
        return 1;
      }
      clearScreenWithMessage("\t\t[EMAIL SENDER IN PROCESS]\n");
      UseHTML = askYesNoQuestion("ARE YOU SENDING HTML LETTER");
      UseAttachment = askYesNoQuestion("ARE YOU USING ATTACHMENT");

      bool success = EmailProgramObject.EmailSender(UseHTML, UseAttachment);
      handleProgramCompletion(success, "CONSTANT EMAIL SENDER");
    }
    else if (optionStr == "2") {
      if (!EmailProgramObject.PrepareEmailSenderDirectories()) {
        std::cerr << "Error: Program failed to create Email Sender directories\n";
        return 1;
      }
      clearScreenWithMessage("\t\t[EMAIL SENDER IN PROCESS]\n");
      UseHTML = askYesNoQuestion("ARE YOU SENDING HTML LETTER");
      UseAttachment = askYesNoQuestion("ARE YOU USING ATTACHMENT");
      bool success = EmailProgramObject.VariableEmailSender(UseHTML, UseAttachment);
      handleProgramCompletion(success, "VARIABLE EMAIL SENDER");
    }
    break;
  }

  case 2: {
    if (!EmailProgramObject.PrepareSMTPLiveTesterDirectories()) {
      std::cerr << "FAILED TO PREPARE PROGRAM FILES, TRY AGAIN.\n";
      break;
    }
    std::cout
      << "Make sure 'Raw.txt' is not empty.\n"
      << "Do you wanna continue (Y/N): ";
    std::cin >> optionStr;
    if (optionStr != "Y" && optionStr != "y") break;
    bool success = EmailProgramObject.SMTPLiveTester();
    handleProgramCompletion(success, "SMTP TEST PROGRAM");
    break;
  }

  case 3: {
    EmailProgramObject.PrepareEmailExtractorDirectories();
    clearScreenWithMessage("[EMAIL EXTRACTOR INITIALIZED]\n");
    std::cout << "PASTE THE DATA INSIDE  [rawfile.txt] LOCATED IN [Email Extractor] FOLDER\n";
    if (askYesNoQuestion("Continue extraction?:")) {
      bool success = EmailProgramObject.EmailExtractor();
      handleProgramCompletion(success, "Email Extraction");
    }
    break;
  }

  case 4: {
    clearScreen();
    if (!EmailProgramObject.PrepareDuplicateDirectories()) {
      std::cerr << "FAILED TO PREPARE PROGRAM FILES, TRY AGAIN.\n";
      break;
    }
    std::cout << "Make sure 'Raw.txt' is not empty.\n"
      << "Do you wanna continue (Y/N): ";
    std::cin >> optionStr;
    if (optionStr == "Y" || optionStr == "y") {
      bool success = EmailProgramObject.DuplicatesRemover();
      handleProgramCompletion(success, "DUPLICATE REMOVAL");
    }
    break;
  }

  default:
    clearScreen();
    std::cout << "Invalid option selected. \n";
    break;
  }
  GlobalMethodObject.prompt();
  TerminateProgram();
  std::cout << "\033[0m";
  return 0;
}