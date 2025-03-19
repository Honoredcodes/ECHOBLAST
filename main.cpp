#include <curl/curl.h>
#include <json/json.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;
int main();

void sleep(const int& x) {
  std::this_thread::sleep_for(std::chrono::seconds(x));
}
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}
size_t readCallback(void* ptr, size_t size, size_t nmemb, void* userdata) {
  std::string* data = static_cast<std::string*>(userdata);
  if (data->empty()) return 0;  // No more data to send

  size_t toCopy = std::min(size * nmemb, data->size());
  memcpy(ptr, data->data(), toCopy);
  data->erase(0, toCopy);  // Remove sent data
  return toCopy;
}
void PrintTopMiddle(const std::string text) {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  int terminalWidth = w.ws_col;
  int textStartPos = (terminalWidth - text.length()) / 2;

  std::cout << "\033[1;1H";
  std::cout << std::string(textStartPos, ' ') << text
            << std::endl
            << std::endl;
}

class cURLHandler {
 private:
  CURL* curl;

 public:
  cURLHandler() {
    curl = curl_easy_init();
    CURLcode res;
    if (!curl) {
      throw std::runtime_error("Failed to initialize curl");
    }
  }

  ~cURLHandler() {
    if (curl) curl_easy_cleanup(curl);
  }
};

class GlobalMethodClass {
 public:
  // #: 1
  std::string MakeAndGetDirectory(const std::string& path, const std::string& folderName) {
    static std::set<std::string> createdFolders;
    std::string fullPath;
    if (path.empty()) {
      fullPath = "./" + folderName;
    } else {
      fullPath = path + "/" + folderName;
    }
    if (createdFolders.find(fullPath) != createdFolders.end()) {
      return fullPath;
    }
    try {
      if (fs::exists(fullPath)) {
        return fullPath;
      } else {
        fs::create_directories(fullPath);
        createdFolders.insert(fullPath);
        return fullPath;
      }
    } catch (const std::exception& e) {
      std::cerr << "Failed to create folder because: " << e.what() << std::endl;
    }
    return "";
  }

  // #: 2
  bool ReadFileToVector(std::vector<std::string>& FileVector, const std::string& filepath) {
    std::fstream FileData(filepath, std::ios::in);
    if (!FileData) {
      std::cerr << "Program Failed To Access " << filepath << std::endl;
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
    static std::mt19937 gen(std::random_device{}());  // Static generator
    std::uniform_int_distribution<> dis(0, param.size() - 1);
    return param[dis(gen)];
  }

  // #: 4
  bool GetAndSetSMTP(std::vector<std::string>& Vector, std::string& CurrentSmtp, std::string& servername, int& port, std::string& username, std::string& password) {
    if (Vector.empty()) {
      return false;
    }
    const std::string SMTP = CurrentSmtp = Vector[0];
    std::stringstream ss(SMTP);
    std::getline(ss, servername, '|');
    ss >> port;
    ss.ignore();
    std::getline(ss, username, '|');
    std::getline(ss, password);
    for (int i = 0; i < Vector.size() - 1; i++) {
      Vector[i] = Vector[i + 1];
    }
    return true;
  }

  // #: 5
  void WriteToFile(const std::string FilePath, std::string data) {
    try {
      std::fstream FileAccess(FilePath, std::ios::app);
      if (!FileAccess) {
        std::cout << "Error: failed to access " << FilePath << std::endl;
        return;
      }
      FileAccess << data << std::endl;
      FileAccess.close();
    } catch (std::exception& error) {
      std::cerr << "Operation failed because " << error.what() << std::endl;
    }
  }

  // #: 6
  bool ReadFromFile(const std::string FilePath, std::string& VariableStorage) {
    try {
      std::fstream FileAccess(FilePath, std::ios::in);
      if (!FileAccess) {
        std::cout << "Error: failed to access " << FilePath << std::endl;
        return false;
      }
      std::string line;
      while (std::getline(FileAccess, line)) {
        if (!line.empty()) {
          VariableStorage += line;
        }
      }
      FileAccess.close();
    } catch (std::exception& error) {
      std::cerr << "Operation failed because " << error.what() << std::endl;
    }
    return true;
  }

  // #: 7
  void LeadFinalCleanUp(std::string& leadfile, std::string sentleads, std::string failedleads) {
    std::ifstream leadfiledata(leadfile);
    std::ifstream sentleadsdata(sentleads);
    std::ifstream failedleadsdata(failedleads);
    if (!leadfiledata || !sentleadsdata || !failedleadsdata) {
      std::cerr << "Error: Unable to open these files.\n"
                << leadfile << std::endl
                << sentleads << std::endl
                << failedleads << std::endl;
      return;
    }
    std::unordered_set<std::string> leadset;
    std::string line;

    while (std::getline(leadfiledata, line)) {
      if (!line.empty()) {
        leadset.insert(line);
      }
    }
    while (std::getline(sentleadsdata, line)) {
      if (!line.empty()) {
        leadset.erase(line);
      }
    }
    while (std::getline(failedleadsdata, line)) {
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
      std::cout << "Lead cleanup completed...\n"
                << "Remaining leads: " << leadset.size() << std::endl;
    }
    leadfileout.close();
    leadfiledata.close();
    sentleadsdata.close();
    failedleadsdata.close();
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
  bool DuplicatesRemover(std::string& filename) {
    try {
      std::ifstream Filedata(filename);
      if (!Filedata) {
        std::cout << "Error: failed to access " << filename;
        return false;
      }

      std::unordered_set<std::string> dataset;
      std::string line;
      while (std::getline(Filedata, line)) {
        if (!line.empty()) {
          dataset.insert(line);
        }
      }
      std::ofstream outFile(filename);
      if (!outFile) {
        std::cerr << "Error: failed to write to " << filename << std::endl;
        return false;
      }

      for (const auto& unique : dataset) {
        outFile << unique << std::endl;
      }
    } catch (const std::exception& error) {
      std::cerr << "Operation failed becase " << error.what() << std::endl;
    }
    return true;
  }

  // #: 10
  void stripdata(std::string& server, std::string& port) {
    std::stringstream ss(server);
    std::getline(ss, server, ':');
    std::getline(ss, port);
  }

  // #: 11
  void prompt() {
    std::string optionStr;
    std::cout << "Do you want to retry the program? (Y/N): ";
    std::cin >> optionStr;
    if (optionStr == "Y" || optionStr == "y") {
      main();
    } else {
      std::cout << "Program terminating...\n";
    }
    sleep(1);
    std::cout << "\033[2J\033[H";
  }

  // #: 12
  bool CreateProgramDirectories(std::string BaseProgramDirectory, std::string& ChildProgramDirectory, std::string& ProgramAttributeDirectory, std::string& Junks) {
    ChildProgramDirectory = MakeAndGetDirectory(BaseProgramDirectory, ChildProgramDirectory);
    ProgramAttributeDirectory = MakeAndGetDirectory(ChildProgramDirectory, ProgramAttributeDirectory);
    Junks = MakeAndGetDirectory(ChildProgramDirectory, Junks);
    return (!BaseProgramDirectory.empty() && !ChildProgramDirectory.empty() && !ProgramAttributeDirectory.empty() && !Junks.empty()) ? true : false;
  }

  // #: 13
  void CreateProgramAttributeFiles(std::string& ProgramAttributeDirectory, std::string& Junks, std::vector<std::string>& ProgramAttributeVector) {
    std::fstream FileAccess;
    for (const auto& file : ProgramAttributeVector) {
      std::string filePath = (file == "failed.txt" || file == "deadsmtp.txt") ? (Junks + "/" + file) : (ProgramAttributeDirectory + "/" + file);
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
  void EmailSuccessMessage(std::string& sendername, std::string& subject, std::string& lead) {
    std::cout << "=================================>   EMAIL SENT SUCCESSFUL  <=================================\n"
              << "> SENDER NAME: " << sendername << std::endl
              << "> SUBJECT TITLE: " << subject << std::endl
              << "> RECIPIENT EMAIL: " << lead << std::endl
              << "=================================>  ECHOBLAST EMAIL SENDER  <=================================\n\n";
  }

  // #: 16
  void SMSSuccessMessage(std::string& sendername, std::string& lead) {
    std::cout
        << "=================================>   MESSAGE SENT SUCCESSFUL   <=================================\n"
        << "SENDER NAME: " << sendername << std::endl
        << "RECIPIENT MAIL SMS: " << lead << std::endl
        << "=================================>  ECHOBLAST MAIL SMS SENDER  <=================================\n\n";
  }

  // #: 17
  std::string UrlEncode(const std::string& str) {
    std::ostringstream encoded;
    for (size_t i = 0; i < str.length(); ++i) {
      unsigned char c = str[i];
      if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
        encoded << c;
      } else {
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
    EmailSenderPrograms = MakeAndGetDirectory("", EmailSenderPrograms);
  }

  // EMAIL SENDER GLOBAL VARIABLES
  std::string EmailProgramDirectory = "Email Sender";
  std::string EmailDataDirectory = "Email Data";
  std::string EmailJunkDirectory = "Junks";
  std::vector<std::string> ProgramAttributeVector;

  // EMAIL TO SMS SENDER GLOBAL VARIABLES
  std::string SMSMailDirectory = "SMS Mail Sender";
  std::string SMSMailDataDirectory = "SMS Mail Data";
  std::string SMSMailJunkDirectory = "Junks";

  // SMTP LIVE TESTER GLOBAL VARIABLES
  std::string SMTPLiveTesterDirectory = "SMTP Live Tester";
  std::string EmailExtractorDirectory = "Email Extractor";

  bool PrepareEmailSenderDirectories() {
    ProgramAttributeVector = {"smtps.txt", "sendername.txt", "subject.txt", "letter.txt", "leads.txt", "sent.txt", "failed.txt", "deadsmtp.txt"};
    if (!CreateProgramDirectories(EmailSenderPrograms, EmailProgramDirectory, EmailDataDirectory, EmailJunkDirectory)) return false;
    CreateProgramAttributeFiles(EmailDataDirectory, EmailJunkDirectory, ProgramAttributeVector);
    return true;
  }

  bool PrepareEmailToSMSDirectories() {
    ProgramAttributeVector = {"smtps.txt", "sendername.txt", "letter.txt", "leads.txt", "sent.txt", "failed.txt", "deadsmtp.txt"};
    if (!CreateProgramDirectories(EmailSenderPrograms, SMSMailDirectory, SMSMailDataDirectory, SMSMailJunkDirectory)) return false;
    CreateProgramAttributeFiles(SMSMailDataDirectory, SMSMailJunkDirectory, ProgramAttributeVector);
    return true;
  }

  bool PrepareSMTPLiveTesterDirectories() {
    ProgramAttributeVector = {"smtps.txt", "failedsmtps.txt"};
    SMTPLiveTesterDirectory = MakeAndGetDirectory(EmailSenderPrograms, SMTPLiveTesterDirectory);
    for (auto& file : ProgramAttributeVector) {
      std::string filePath = fs::path(SMTPLiveTesterDirectory) / file;
      std::fstream FileAccess(filePath, std::ios::app);
      if (!FileAccess) {
        std::cerr << "Failed to create " << file << std::endl;
        return false;
      }
      FileAccess.close();
    }
    return true;
  }

  bool PrepareEmailExtractorDirectories() {
    ProgramAttributeVector = {"rawfile.txt", "extracted.txt"};
    EmailExtractorDirectory = MakeAndGetDirectory(EmailSenderPrograms, EmailExtractorDirectory);
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
    curl_easy_setopt(curl, CURLOPT_URL, ("smtp://" + servername + ":" + std::to_string(port)).c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
  }

  void MakeEmailBody(std::string& EmailContentBody, bool useHTML, bool useAttachment, std::string boundary,
                     std::string sendername, std::string username, std::string subject,
                     std::string letter, std::string lead, std::string AttachmentFilePath = "") {
    if (boundary.empty()) {
      boundary = "----=_Part_001_" + std::to_string(time(nullptr));
    }
    std::string altBoundary = "alt_" + boundary;

    // Email headers
    EmailContentBody = "From: " + sendername + " <" + username + ">\r\n";
    EmailContentBody += "To: " + lead + "\r\n";
    EmailContentBody += "Subject: " + subject + "\r\n";
    EmailContentBody += "Date: " + GetCurrentDateTime() + "\r\n";
    EmailContentBody += "MIME-Version: 1.0\r\n";

    if (useAttachment) {
      EmailContentBody += "Content-Type: multipart/mixed; boundary=\"" + boundary + "\"\r\n\r\n";
      EmailContentBody += "--" + boundary + "\r\n";
      EmailContentBody += "Content-Type: multipart/alternative; boundary=\"" + altBoundary + "\"\r\n\r\n";
    } else {
      EmailContentBody += "Content-Type: multipart/alternative; boundary=\"" + altBoundary + "\"\r\n\r\n";
    }

    // Plain text part
    EmailContentBody += "--" + altBoundary + "\r\n";
    EmailContentBody += "Content-Type: text/plain; charset=UTF-8\r\n";
    EmailContentBody += "Content-Disposition: inline\r\n\r\n";
    EmailContentBody += letter + "\r\n\r\n";

    // HTML part
    if (useHTML) {
      EmailContentBody += "--" + altBoundary + "\r\n";
      EmailContentBody += "Content-Type: text/html; charset=UTF-8\r\n";
      EmailContentBody += "Content-Disposition: inline\r\n\r\n";
      EmailContentBody += letter + "\r\n\r\n";
    }

    // Closing alternative section
    EmailContentBody += "--" + altBoundary + "--\r\n";

    // Attachment section
    if (useAttachment && !AttachmentFilePath.empty()) {
      std::string encodedFile = base64Encode(AttachmentFilePath);
      std::string attachmentName = std::filesystem::path(AttachmentFilePath).filename().string();

      EmailContentBody += "--" + boundary + "\r\n";
      EmailContentBody += "Content-Type: application/octet-stream; name=\"" + attachmentName + "\"\r\n";
      EmailContentBody += "Content-Disposition: attachment; filename=\"" + attachmentName + "\"\r\n";
      EmailContentBody += "Content-Transfer-Encoding: base64\r\n\r\n";
      EmailContentBody += encodedFile + "\r\n\r\n";
    }

    // Closing mixed boundary
    EmailContentBody += "--" + boundary + "--\r\n";
  }

  // #: 1 Email Sender
  bool EmailSender(bool useHTML, bool useAttachment) {
    std::string AttachmentFilePath;
    std::string LeadFileDirectory = fs::path(EmailDataDirectory) / "leads.txt";
    std::string LetterFileDirectory = fs::path(EmailDataDirectory) / "letter.txt";
    std::string SentEmail = fs::path(EmailDataDirectory) / "sent.txt";
    std::string FailedLeadsFile = fs::path(EmailJunkDirectory) / "failed.txt";
    std::string DeadSMTPsFile = fs::path(EmailJunkDirectory) / "deadsmtp.txt";
    std::string EmailContentBody, AttachmentFileName, CurrentSmtp, boundary = "----=_Part_001_" + std::to_string(time(nullptr));
    long responseCode;

    if (useAttachment) {
      AttachmentFilePath = MakeAndGetDirectory(EmailDataDirectory, "Attachment");
      std::string tempstring;
      std::cout << "\033[2J\033[H";
      std::cout << "Provide Attachment filename with extension type (Example: filename.pdf): ";
      std::cin >> tempstring;
      AttachmentFileName = fs::path(AttachmentFilePath) / tempstring;
    }

    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, fs::path(EmailDataDirectory) / "smtps.txt") ||
        !ReadFileToVector(NameVectorObject.MailDataSetVector, fs::path(EmailDataDirectory) / "sendername.txt") ||
        !ReadFileToVector(SubjectVectorObject.MailDataSetVector, fs::path(EmailDataDirectory) / "subject.txt")) {
      std::cerr << "Email Assets Files Cannot Be Empty \n";
      sleep(1);
      return false;
    }
    if (!GetAndSetSMTP(SMTPVectorObject.MailDataSetVector, CurrentSmtp, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password)) {
      std::cout << "SMTP not found or exhuasted.\n";
      return false;
    }
    int SentCount = 0, errorCount = 0, &port = SMTPAttributeObject.port;
    std::string& servername = SMTPAttributeObject.servername;
    std::string& username = SMTPAttributeObject.username;
    std::string& password = SMTPAttributeObject.password;
    std::string sendername, subject, letter;
    std::vector<std::string> LeadsVector;
    if (!ReadFileToVector(LeadsVector, LeadFileDirectory)) {
      std::cerr << "Program Failed To Access " << LeadFileDirectory << std::endl;
      return false;
    }
    ReadFromFile(LetterFileDirectory, letter);
    CURL* curl;
    CURLcode res;
    curl_slist* recipients_list = nullptr;
    std::string response;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
      std::cerr << "Failed to initialize, please check network and try again." << std::endl;
      return false;
    }
    if (curl) {
      SetCurlForMail(curl, servername, port, username, password);
      std::cout << "\033[2J\033[H";
      PrintTopMiddle("[EMAIL SENDER INITIALIZED]");
      sleep(1);
      for (auto& lead : LeadsVector) {
        sendername = GetRandomDataFromVector(NameVectorObject.MailDataSetVector);
        subject = GetRandomDataFromVector(SubjectVectorObject.MailDataSetVector);
        MakeEmailBody(EmailContentBody, useHTML, useAttachment, boundary, sendername, username, subject, letter, lead, AttachmentFileName);
        recipients_list = curl_slist_append(recipients_list, lead.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients_list);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, username.c_str());
        curl_easy_setopt(curl, CURLOPT_READDATA, &EmailContentBody);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);

        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (res == CURLE_OK && responseCode == 250) {
          WriteToFile(SentEmail, lead);
          EmailSuccessMessage(sendername, subject, lead);
          SentCount++;
          errorCount = 0;
        } else {
          errorCount++;
          if (errorCount <= 3) {
            WriteToFile(FailedLeadsFile, lead);
          } else if (errorCount >= 4 && errorCount <= 6) {
            WriteToFile(DeadSMTPsFile, CurrentSmtp);
            if (!GetAndSetSMTP(SMTPVectorObject.MailDataSetVector, CurrentSmtp, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password)) {
              std::cout << "SMTP not found or finished..\n"
                        << "Total successfully sent email to " << SentCount << " Leads\n";
              curl_slist_free_all(recipients_list);
              recipients_list = nullptr;
              LeadFinalCleanUp(LeadFileDirectory, SentEmail, FailedLeadsFile);
              sleep(2);
              return true;
            }

            SetCurlForMail(curl, servername, port, username, password);
            res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

            if (res == CURLE_OK && responseCode == 250) {
              WriteToFile(SentEmail, lead);
              EmailSuccessMessage(sendername, subject, lead);
              SentCount++;
              errorCount = 0;
            }
          } else {
            WriteToFile(FailedLeadsFile, lead);
            errorCount = 0;
          }
        }
        curl_slist_free_all(recipients_list);
        recipients_list = nullptr;
      }
      std::cout << "TOTAL SUCCESSFULLY SENT EMAIL TO " << SentCount << " LEADS \n";
      curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    LeadFinalCleanUp(LeadFileDirectory, SentEmail, FailedLeadsFile);
    sleep(2);
    return true;
  }

  // #: 2 Email to SMS Sender
  bool MailSMSSender(std::string& LeadsFile) {
    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, fs::path(SMSMailDataDirectory) / "smtps.txt") ||
        !ReadFileToVector(NameVectorObject.MailDataSetVector, fs::path(SMSMailDataDirectory) / "sendername.txt")) {
      std::cerr << "Mail SMS Asset Files Cannot Be Empty \n";
      sleep(1);
      return false;
    }
    std::string LeadFileDirectory = fs::path(SMSMailDataDirectory) / LeadsFile;
    std::string LetterFileDirectory = fs::path(SMSMailDataDirectory) / "letter.txt";
    std::string SentEmail = fs::path(SMSMailDataDirectory) / "sent.txt";
    std::string FailedLeadsFile = fs::path(SMSMailJunkDirectory) / "failed.txt";
    std::string DeadSMTPsFile = fs::path(SMSMailJunkDirectory) / "deadsmtp.txt";
    std::string CurrentSmtp;
    if (!GetAndSetSMTP(SMTPVectorObject.MailDataSetVector, CurrentSmtp, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password)) {
      std::cout << "SMTP not found or exhuasted.\n";
      return false;
    }
    int SentCount = 0, errorCount = 0, &port = SMTPAttributeObject.port;
    std::string& servername = SMTPAttributeObject.servername;
    std::string& username = SMTPAttributeObject.username;
    std::string& password = SMTPAttributeObject.password;
    std::string sendername, letter;
    std::vector<std::string> LeadsVector;
    std::cout << "This is the part to debug" << std::endl;
    std::cout << LeadFileDirectory << std::endl;
    if (!ReadFileToVector(LeadsVector, LeadFileDirectory)) {
      std::cerr << "Program Failed To Access " << LeadFileDirectory << std::endl;
      return false;
    }
    long responseCode;
    ReadFromFile(LetterFileDirectory, letter);

    CURL* curl;
    CURLcode res;
    curl_slist* recipients_list = nullptr;
    std::string response;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
      std::cerr << "Program failed to initilize sender, please check network connection /n";
      return false;
    }
    if (curl) {
      SetCurlForMail(curl, servername, port, username, password);
      std::cout << "\033[2J\033[H";
      std::cout << "\t\t[EMAIL TO SMS SENDER INITIALIZED]\n\n";
      sleep(1);

      for (auto& lead : LeadsVector) {
        const std::string data =
            "From: " + sendername + " <" + username + ">\r\n" +
            "To: " + lead + "\r\n" +
            "Subject: SMS Notification\r\n" +
            "MIME-Version: 1.0\r\n" +
            "Content-Type: text/plain; charset=UTF-8\r\n" +
            "\r\n" +
            letter +
            "\r\n";
        sendername = GetRandomDataFromVector(NameVectorObject.MailDataSetVector);
        recipients_list = curl_slist_append(recipients_list, lead.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients_list);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, username.c_str());
        curl_easy_setopt(curl, CURLOPT_READDATA, &data);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);

        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (res == CURLE_OK && responseCode == 250) {
          WriteToFile(SentEmail, lead);
          SMSSuccessMessage(sendername, lead);
          curl_slist_free_all(recipients_list);
          recipients_list = nullptr;
          SentCount++;
          errorCount = 0;
        } else {
          errorCount++;
          if (errorCount <= 3) {
            WriteToFile(FailedLeadsFile, lead);
          } else if (errorCount >= 4 && errorCount <= 6) {
            WriteToFile(DeadSMTPsFile, CurrentSmtp);
            if (!GetAndSetSMTP(SMTPVectorObject.MailDataSetVector, CurrentSmtp, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password) && SentCount > 0) {
              std::cout << "SMTP not found or finished....\n"
                        << "Total successfully sent email to " << SentCount << " Leads\n";
              curl_slist_free_all(recipients_list);
              recipients_list = nullptr;
              LeadFinalCleanUp(LeadFileDirectory, SentEmail, FailedLeadsFile);
              return true;
            } else {
              std::cout << "SMTP not found or finished....\n";
              curl_slist_free_all(recipients_list);
              recipients_list = nullptr;
              return false;
            }
            SetCurlForMail(curl, servername, port, username, password);
            res = curl_easy_perform(curl);
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

            if (res == CURLE_OK && responseCode == 250) {
              WriteToFile(SentEmail, lead);
              SMSSuccessMessage(sendername, lead);
              SentCount++;
              errorCount = 0;
            }
          }
        }
      }
      std::cout << "Total successfully sent email to " << SentCount << " Leads \n";
      curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    LeadFinalCleanUp(LeadFileDirectory, SentEmail, FailedLeadsFile);
    return true;
  }

  // #: 3 SMTP LIVE TESTER
  bool SMTPLiveTester() {
    CURL* curl;
    CURLcode res;
    int port, smtpAssignCounter = 0, totalvalid = 0, totaldeadsmtp = 0;
    std::unordered_set<std::string> invalidsmtp, validsmtp;
    std::string optionStr, filename, servername, username, password, response, TestEmailAddr, boundary = "----=_Part_001_2345_67890";
    std::string smtpsdir = fs::path(SMTPLiveTesterDirectory) / "smtps.txt", failedsmtpsdir = fs::path(SMTPLiveTesterDirectory) / "failedsmtps.txt";
    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, smtpsdir)) {
      std::cerr << "Error: failed to read data from " << smtpsdir << std::endl
                << "Please check if file exit and not empty\n";
      return false;
    }

    std::cout << "Enter a valid email address: ";
    std::cin >> TestEmailAddr;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::cout << "\033[2J\033[H"
              << "SMTP Live Tester Initialized...\n"
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
      recipients = curl_slist_append(recipients, TestEmailAddr.c_str());
      response.clear();
      const std::string data =
          "Subject: SMTP Test OK\r\n"
          "From: <" +
          username +
          ">\r\n"
          "To: <" +
          TestEmailAddr +
          ">\r\n"
          "MIME-Version: 1.0\r\n"
          "Content-Type: text/plain; charset=UTF-8\r\n"
          "Message-ID: <" +
          std::to_string(time(nullptr)) + "@" + smtp +
          ">\r\n"
          "\r\n"
          "Hello,\r\n" +
          smtp +
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
      if (res == CURLE_OK) {
        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode == 250) {
          std::cout
              << "\033[92m"
              << "=============================    SMTP TEST    =============================\n"
              << smtp << std::endl
              << "============================= TEST SUCCESSFUL =============================\n\n";
          validsmtp.insert(smtp);
          totalvalid++;
        } else {
          std::cout << "\033[31m"
                    << "=============================    SMTP TEST    =============================\n"
                    << smtp << std::endl
                    << "=============================   TEST FAILED   =============================\n\n"
                    << "\033[92m";
          totaldeadsmtp++;
          invalidsmtp.insert(smtp);
        }
      } else {
        std::cout << "Failed to perform SMTP request: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        curl_slist_free_all(recipients);
        curl_global_cleanup();
        return false;
      }
      curl_easy_cleanup(curl);
      curl_slist_free_all(recipients);
    }
    std::cout << "Total GOOD / LIVE smtps " << totalvalid << std::endl
              << "Total BAD / DEAD smtps " << totaldeadsmtp << std::endl;
    curl_global_cleanup();
    std::fstream smtpfile(smtpsdir, std::ios::out), invalidsmtpfile(failedsmtpsdir, std::ios::out);
    if (!smtpfile || !invalidsmtpfile) {
      std::cout << "Failed to open output files for writing results.\n";
      std::cout << "[VALID SMTPS]" << std::endl;
      for (const auto& smtp : validsmtp) {
        std::cout << smtp << std::endl;
      }
      std::cout << "\n";
      std::cout << "[INVALID SMTPS]" << std::endl;
      for (const auto& smtp : invalidsmtp) {
        std::cout << smtp << std::endl;
      }
      return true;
    }
    for (const auto& smtp : validsmtp) {
      smtpfile << smtp << std::endl;
    }
    for (const auto& smtp : invalidsmtp) {
      invalidsmtpfile << smtp << std::endl;
    }
    smtpfile.close();
    invalidsmtpfile.close();
    return true;
  }

  bool
  emailextractor() {
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
};

int main(void) {
  std::cout << "\033[2J\033[H"
            << "\033[92m"
            << "\033[1m";
  GlobalMethodClass GlobalMethodObject;
  EmailSenderProgram EmailProgramObject;
  std::string optionStr;
  std::string optionArr[3];
  std::string ProgramDirectory;
  bool UseAttachment, UseHTML;
  int option;
  std::cout << "\t\t[ECHOBLAST V1 : TELEGRAM H4CKECHO]\n\n"
            << "1. Email Sender \n"
            << "2. Email -> SMS Sender\n"
            << "3. SMTP Live Tester\n"
            << "4. Remove Duplicates\n"
            << "5. Email Extractor\n"
            << "Choose a program: ";
  std::cin >> option;
  switch (option) {
    case 1:

      // #: 1 EMAIL SENDER
      if (!EmailProgramObject.PrepareEmailSenderDirectories()) {
        std::cerr << "Error: Program failed to create Email Sender directories\n";
        return 1;
      }
      std::cout << "\033[2J\033[H"
                << "\t\t[EMAIL SENDER INITIALIZED]\n\n"
                << "Are you using HTML Letter (Y/N): ";
      std::cin >> optionStr;
      UseHTML = (optionStr == "Y" || optionStr == "y") ? true : false;

      std::cout << "Are you using Attachment (Y/N): ";
      std::cin >> optionStr;
      UseAttachment = (optionStr == "Y" || optionStr == "y") ? true : false;
      if (EmailProgramObject.EmailSender(UseHTML, UseAttachment)) {
        sleep(1);
        std::cout << "\n\n"
                  << "Email Sender program completed.\n";
      } else {
        sleep(1);
        std::cout << "\n\n"
                  << "Email Sender program failed.\n";
      }

      GlobalMethodObject.prompt();
      break;

    case 2:
      //  #: 2 EMAIL TO SMS SENDER
      if (!EmailProgramObject.PrepareEmailToSMSDirectories()) {
        std::cerr << "Error: Program failed to create Email to SMS directories\n";
        return 1;
      }
      std::cout << "\033[2J\033[H"
                << "\t\t[EMAIL -> SMS SENDER INITIALIZED]\n\n"
                << "Phone Number lead filename (Example: filename.txt): ";
      std::cin >> optionStr;

      if (EmailProgramObject.MailSMSSender(optionStr)) {
        sleep(1);
        std::cout << "\033[2J\033[H";
        std::cout << "Email to SMS Sender program completed.\n";
      } else {
        sleep(1);
        std::cout << "\033[2J\033[H";
        std::cout << "Email to SMS Sender program failed.\n";
      }
      GlobalMethodObject.prompt();
      break;

    case 3:
      EmailProgramObject.PrepareSMTPLiveTesterDirectories();
      std::cout << "\033[2J\033[H"
                << "\t\t[SMTP LIVE TESTER INITIALIZED]\n\n";
      if (EmailProgramObject.SMTPLiveTester()) {
        sleep(1);
        std::cout
            << "SMTP Live Tester program completed.\n";
      } else {
        sleep(1);
        std::cout
            << "SMTP Live Tester program failed.\n";
      }
      GlobalMethodObject.prompt();
      break;

    case 4:
      std::cout << "\033[2J\033[H"
                << "Data to filter filename (Example: filename.txt): ";
      std::cin >> optionStr;

      if (GlobalMethodObject.DuplicatesRemover(optionStr)) {
        sleep(1);
        std::cout << "\033[2J\033[H"
                  << "Duplicates removed successfully.\n";
      } else {
        sleep(1);
        std::cout << "\033[2J\033[H"
                  << "Duplicates removal failed.\n";
      }
      GlobalMethodObject.prompt();
      break;

    case 5:
      EmailProgramObject.PrepareEmailExtractorDirectories();
      std::cout << "\033[2J\033[H"
                << "[EMAIL EXTRACTOR INITIALIZED] \n\n";
      std::cout << "Copy and paste raw file inside rawfile.txt in Email Extractor Directory \n"
                << "Continue extraction (Y/N) ?: ";
      std::cin >> optionStr;
      if (optionStr == "Y" || optionStr == "y") {
        if (!EmailProgramObject.emailextractor()) {
          sleep(1);
          std::cout << "Email Extraction is completed...\n";
          GlobalMethodObject.prompt();
        } else {
          sleep(1);
          std::cout << "Email Extraction failed... \n";
          GlobalMethodObject.prompt();
        }
      }
      break;

    default:
      std::cout << "\033[2J\033[H"
                << "Invalid option selected. \n";
      GlobalMethodObject.prompt();
      break;
  }
  std::cout
      << " \033[0m"
      << "\033[2J\033[H";
  return 0;
}