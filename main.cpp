#include <curl/curl.h>
#include <json/json.h>

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
// class CurlHandle {
//  private:
//   CURL* curl;

//  public:
//   CurlHandle() : curl(curl_easy_init()) {}
//   ~CurlHandle() {
//     if (curl) curl_easy_cleanup(curl);
//   }
//   CURL* get() { return curl; }
// };

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
    return (!BaseProgramDirectory.empty() || !ChildProgramDirectory.empty() || !ProgramAttributeDirectory.empty() || !Junks.empty()) ? true : false;
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
              << "> SUBJECT: " << subject << std::endl
              << "> Email Successfully Sent To: " << lead << std::endl
              << "=================================>  ECHOBLAST EMAIL SENDER  <=================================\n\n";
  }

  // #: 16
  void SMSSuccessMessage(std::string& sendername, std::string& lead) {
    std::cout << "=================================> ECHOBLAST SMS SENDER <================================="
              << "SENDER NAME: " << sendername << std::endl
              << "SMS Successfully Sent To: " << lead << std::endl
              << "=================================> ECHOBLAST SMS SENDER <=================================\n\n";
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

  void MakeEmailBody(std::string& EmailContentBody, bool useHTML, bool useAttachment, std::string boundary, std::string sendername, std::string username, std::string subject, std::string letter, std::string lead, std::string AttachmentFilePath = "") {
    if (boundary.empty()) {
      boundary = "----=_Part_001_" + std::to_string(time(nullptr));
    }
    std::string fromHeader = "From: " + sendername + " <" + username + ">\r\n";
    EmailContentBody = fromHeader;
    EmailContentBody += "To: " + lead + "\r\n";
    EmailContentBody += "Subject: " + subject + "\r\n";
    EmailContentBody += "MIME-Version: 1.0\r\n";
    EmailContentBody += "Content-Type: multipart/mixed; boundary=\"" + boundary + "\"\r\n\r\n";
    EmailContentBody += "--" + boundary + "\r\n";
    if (useHTML) {
      EmailContentBody += "Content-Type: text/html; charset=UTF-8\r\n\r\n";
    } else {
      EmailContentBody += "Content-Type: text/plain; charset=UTF-8\r\n\r\n";
    }
    EmailContentBody += letter + "\r\n";
    if (useAttachment && !AttachmentFilePath.empty()) {
      std::string encodedFile = base64Encode(AttachmentFilePath);
      std::string attachmentName = fs::path(AttachmentFilePath).filename().string();
      EmailContentBody += "--" + boundary + "\r\n";
      EmailContentBody += "Content-Type: application/octet-stream; name=\"" + attachmentName + "\"\r\n";
      EmailContentBody += "Content-Disposition: attachment; filename=\"" + attachmentName + "\"\r\n";
      EmailContentBody += "Content-Transfer-Encoding: base64\r\n\r\n";
      EmailContentBody += encodedFile + "\r\n";
    }
    EmailContentBody += "--" + boundary + "--\r\n";
  }

  // #: 1 Email Sender
  bool EmailSender(bool useHTML, bool useAttachment) {
    int smtpAssignCounter = 0;
    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, fs::path(EmailDataDirectory) / "smtps.txt") ||
        !ReadFileToVector(NameVectorObject.MailDataSetVector, fs::path(EmailDataDirectory) / "sendername.txt") ||
        !ReadFileToVector(SubjectVectorObject.MailDataSetVector, fs::path(EmailDataDirectory) / "subject.txt")) {
      std::cerr << "Email Assets Files Cannot Be Empty \n";
      sleep(1);
      return false;
    }

    std::string AttachmentFilePath;
    std::string LeadFileDirectory = fs::path(EmailDataDirectory) / "leads.txt";
    std::string LetterFileDirectory = fs::path(EmailDataDirectory) / "letter.txt";
    std::string SentEmail = fs::path(EmailDataDirectory) / "sent.txt";
    std::string FailedLeadsFile = fs::path(EmailJunkDirectory) / "failed.txt";
    std::string DeadSMTPsFile = fs::path(EmailJunkDirectory) / "deadsmtp.txt";
    std::string EmailContentBody, AttachmentFileName, CurrentSmtp, boundary = "----=_Part_001_2345_67890";
    long responseCode;
    if (useAttachment) {
      AttachmentFilePath = MakeAndGetDirectory(EmailDataDirectory, "Attachment");
      std::string tempstring;
      std::cout << "\033[2J\033[H";
      std::cout << "Provide Attachment filename with extension type (Example: filename.pdf): ";
      std::cin >> tempstring;
      AttachmentFileName = fs::path(AttachmentFilePath) / tempstring;
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
      std::cout << "\t\t[EMAIL SENDER INITIALIZED]\n\n";
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
      std::cout << "Total successfully sent email to " << SentCount << " Leads \n";
      curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    LeadFinalCleanUp(LeadFileDirectory, SentEmail, FailedLeadsFile);
    sleep(2);
    return true;
  }

  // #: 2 Email to SMS Sender
  bool EmailToSMSSender(std::string& LeadsFile) {
    int smtpAssignCounter = 0;
    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, SMSMailDataDirectory + "/smtps.txt") ||
        !ReadFileToVector(NameVectorObject.MailDataSetVector, SMSMailDataDirectory + "/sendername.txt")) {
      std::cout << "\033[2J\033[H";
      std::cout << "Error: Looks like smtp, sendername file has been moved or deleted.\n"
                << "Please create these files or restart program to automatically create them.\n";
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
      std::cout << "SMTP not found or finished....\n";
      return false;
    }

    int SentCount = 0, errorCount = 0, &port = SMTPAttributeObject.port;
    std::string& servername = SMTPAttributeObject.servername;
    std::string& username = SMTPAttributeObject.username;
    std::string& password = SMTPAttributeObject.password;
    std::string sendername, subject, letter;
    std::vector<std::string> LeadsVector;
    if (!ReadFileToVector(LeadsVector, LeadFileDirectory)) {
      std::cerr << "Error: failed to read " << LeadsFile << std::endl
                << "Retry program again\n";
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
    if (curl) {
      SetCurlForMail(curl, servername, port, username, password);
      std::cout << "\033[2J\033[H";
      std::cout << "Email to SMS Sender Initialized...\n";
      sleep(1);
      for (auto& lead : LeadsVector) {
        sendername = GetRandomDataFromVector(NameVectorObject.MailDataSetVector);
        recipients_list = curl_slist_append(recipients_list, lead.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, username.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients_list);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, letter.c_str());
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &response);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (res == CURLE_OK && responseCode == 250) {
          WriteToFile(SentEmail, lead);
          EmailSuccessMessage(sendername, subject, lead);
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
            if (!GetAndSetSMTP(SMTPVectorObject.MailDataSetVector, CurrentSmtp, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password)) {
              std::cout << "SMTP not found or finished....\n"
                        << "Total successfully sent email to " << SentCount << " Leads\n";
              curl_slist_free_all(recipients_list);
              recipients_list = nullptr;
              LeadFinalCleanUp(LeadFileDirectory, SentEmail, FailedLeadsFile);
              break;
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
          std::cout << "=============================    SMTP TEST    ============================= \n"
                    << smtp << std::endl
                    << "============================= TEST SUCCESSFUL ============================= \n\n";
          validsmtp.insert(smtp);
          totalvalid++;
        } else {
          std::cout << "=============================    SMTP TEST    ============================= \n"
                    << smtp << std::endl
                    << "=============================   TEST FAILED   ============================= \n\n";
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

// SMS GATEWAY SENDER
class SMSGatewaySenderProgram : public GlobalMethodClass {
 private:
  struct SMSDataStruct {
    std::vector<std::string> SMSDataVector;
    std::string auth_id, token, SenderNum, lead, message;
  };

 public:
  std::string SMSGatewaySenders = "SMS Gateway Senders";
  SMSGatewaySenderProgram() {
    SMSGatewaySenders = MakeAndGetDirectory("", SMSGatewaySenders);
  }

  std::string PrepareSMSProgramFiles(std::string SMSGatewaySenderName) {
    std::string ProgramDirectory = MakeAndGetDirectory(SMSGatewaySenders, SMSGatewaySenderName);
    std::vector<std::string> FilesVector = {"failedleads.txt", "sentleads.txt", "letter.txt", "leads.txt"};
    CreateSMSAttributeFiles(ProgramDirectory, FilesVector);
    return ProgramDirectory;
  }

  // #: 1
  bool PlivoGatewaySender(std::string& ProgramDirectory, std::string& auth_id, std::string& token, std::string& SenderNum) {
    SMSDataStruct SMSDataObject;
    std::string LetterFilePath = fs::path(ProgramDirectory) / "letter.txt";
    std::string FailedleadsPath = fs::path(ProgramDirectory) / "failedleads.txt";
    std::string SentleadsPath = fs::path(ProgramDirectory) / "sentleads.txt";
    std::string LeadsFilePath = fs::path(ProgramDirectory) / "leads.txt";
    std::string authentication = auth_id + ":" + token;
    if (!ReadFileToVector(SMSDataObject.SMSDataVector, LeadsFilePath) || !ReadFromFile(LetterFilePath, SMSDataObject.message)) {
      std::cerr << "Error: failed to find or read these directories: \n"
                << LeadsFilePath << std::endl
                << LetterFilePath << std::endl
                << "Please check files and retry program...\n";
      return false;
    }
    int errorCount = 0, successCount = 0;
    std::string url = "https://api.plivo.com/v1/Account/" + auth_id + "/Message/";
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
      for (auto& lead : SMSDataObject.SMSDataVector) {
        std::string encodedMessage = UrlEncode(SMSDataObject.message);
        std::string POSTDATA = "src=" + SenderNum + "&dst=" + lead + "&text=" + encodedMessage;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1L);
        curl_easy_setopt(curl, CURLOPT_USERNAME, auth_id.c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, authentication.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, POSTDATA.c_str());
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
          std::cerr << "Curl failed to process operation..." << std::endl;
          curl_easy_cleanup(curl);
          curl_easy_cleanup(curl);
          curl_global_cleanup();
          return false;
        }
        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode == 202) {
          SMSSuccessMessage(SenderNum, lead);
          WriteToFile(SentleadsPath, lead);
          successCount++;
          errorCount = 0;
        } else {
          if (errorCount <= 3) {
            std::cout << "Error: Failed to send SMS to " << lead << std::endl;
            WriteToFile(FailedleadsPath, lead);
            errorCount++;
          } else {
            std::cout << "Please check Plivo Account for troubleshoot..." << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
          }
        }
      }
      std::cout << "Total successfully sent SMS to " << successCount << " Leads \n";
      curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    LeadFinalCleanUp(LeadsFilePath, SentleadsPath, FailedleadsPath);
    return true;
  }

  // #: 2
  bool TwilioGatewaySender(std::string& ProgramDirectory, std::string& auth_id, std::string& token, std::string& SenderNum) {
    SMSDataStruct SMSDataObject;
    std::string LetterFilePath = fs::path(ProgramDirectory) / "letter.txt";
    std::string FailedleadsPath = fs::path(ProgramDirectory) / "failedleads.txt";
    std::string SentleadsPath = fs::path(ProgramDirectory) / "sentleads.txt";
    std::string LeadsFilePath = fs::path(ProgramDirectory) / "leads.txt";
    std::string authentication = auth_id + ":" + token;
    if (!ReadFileToVector(SMSDataObject.SMSDataVector, LeadsFilePath) || !ReadFromFile(LetterFilePath, SMSDataObject.message)) {
      std::cerr << "Error: failed to find or read these directories: \n"
                << LeadsFilePath << std::endl
                << LetterFilePath << std::endl
                << "Please check files and retry program...\n";
      return false;
    }
    int errorCount = 0, successCount = 0;
    std::string url = "https://api.twilio.com/2010-04-01/Accounts/" + auth_id + "/Messages.json";
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
      for (auto& lead : SMSDataObject.SMSDataVector) {
        std::string POSTDATA = "From=" + UrlEncode(SenderNum) + "&To=" + UrlEncode(lead) + "&Body=" + UrlEncode(SMSDataObject.message);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(curl, CURLOPT_USERPWD, authentication.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, POSTDATA.c_str());
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
          std::cerr << "Curl failed to process operation..." << std::endl
                    << curl_easy_strerror(res) << std::endl;
          curl_easy_cleanup(curl);
          curl_global_cleanup();
          return false;
        }
        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode == 201) {
          SMSSuccessMessage(SenderNum, lead);
          WriteToFile(SentleadsPath, lead);
          successCount++;
          errorCount = 0;
        } else {
          if (errorCount <= 3) {
            std::cout << "Failed to send SMS to " << lead << std::endl
                      << "Response code: " << responseCode << std::endl;
            WriteToFile(FailedleadsPath, lead);
            errorCount++;
          } else {
            std::cout << "Please check Twilio Account for troubleshoot..." << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
          }
        }
      }
      std::cout << "Total successfully sent SMS to " << successCount << " Leads \n";
      curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    LeadFinalCleanUp(LeadsFilePath, SentleadsPath, FailedleadsPath);
    return true;
  }

  // #: 3
  bool TrueDialogGatewaySender(std::string& ProgramDirectory, std::string& token, std::string& SenderNum) {
    SMSDataStruct SMSDataObject;
    std::string bearerToken = token;
    std::string LetterFilePath = fs::path(ProgramDirectory) / "letter.txt";
    std::string FailedleadsPath = fs::path(ProgramDirectory) / "failedleads.txt";
    std::string SentleadsPath = fs::path(ProgramDirectory) / "sentleads.txt";
    std::string LeadsFilePath = fs::path(ProgramDirectory) / "leads.txt";
    std::vector<std::string> FilesVector = {"failedleads.txt", "sentleads.txt"};
    CreateSMSAttributeFiles(ProgramDirectory, FilesVector);
    if (!ReadFileToVector(SMSDataObject.SMSDataVector, LeadsFilePath) || !ReadFromFile(LetterFilePath, SMSDataObject.message)) {
      std::cerr << "Error: Failed to read these directories:\n"
                << LeadsFilePath << "\n"
                << LetterFilePath << "\n";
      return false;
    }
    int batchsize, errorCount = 0, successCount = 0, interatorSize = 0;
    std::string url = "https://api.truedialog.com/api/v2.1/Message";
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
      std::cout << "True Dialog Gateway send sms in batch \n"
                << "Choose batch size (Recommended: 50/send): ";
      std::cin >> batchsize;
      interatorSize = SMSDataObject.SMSDataVector.size() / batchsize;
      for (int i = 0; i < interatorSize; i++) {
        std::string POSTDATA = "{ \"From\": \"" + SenderNum + "\", \"To\": [";
        for (int j = 0; j < batchsize; j++) {
          int index = i * batchsize + j;
          if (index >= SMSDataObject.SMSDataVector.size()) break;
          POSTDATA += "\"" + SMSDataObject.SMSDataVector[index] + "\",";
        }
        POSTDATA.pop_back();
        POSTDATA += "], \"Text\": \"" + SMSDataObject.message + "\" }";
        std::string responseBody;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, ("Authorization: Bearer " + bearerToken).c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, POSTDATA.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
          std::cerr << "Curl failed to process operation..." << std::endl
                    << curl_easy_strerror(res) << std::endl;
          curl_easy_cleanup(curl);
          curl_global_cleanup();
          return false;
        }
        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode == 201) {
          Json::CharReaderBuilder reader;
          Json::Value jsonData;
          std::istringstream sstream(responseBody);

          if (!Json::parseFromStream(reader, sstream, &jsonData, nullptr)) {
            std::cerr << "Error parsing JSON response!" << std::endl;
            return false;
          }
          const Json::Value& results = jsonData["results"];
          for (int j = 0; j < batchsize; j++) {
            int index = i * batchsize + j;
            std::string contact = SMSDataObject.SMSDataVector[index];
            std::string status = results[j]["status"].asString();

            if (status == "Success") {
              SMSSuccessMessage(SenderNum, contact);
              WriteToFile(SentleadsPath, contact);
              successCount++;
              errorCount = 0;
            } else {
              if (errorCount <= 6) {
                std::cout << "Please check TrueDialog Account for troubleshoot..." << std::endl;
                curl_easy_cleanup(curl);
                curl_global_cleanup();
                return false;
              }
              std::cout << "SMS Failed to send: " << contact << std::endl;
              WriteToFile(FailedleadsPath, contact);
              errorCount++;
            }
          }
        } else {
          std::cout << "Error: Operation failed, please try again \n";
          curl_easy_cleanup(curl);
          curl_global_cleanup();
          return false;
        }
      }
      std::cout << "Total successfully sent SMS to " << successCount << " Leads \n";
      curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    LeadFinalCleanUp(LeadsFilePath, SentleadsPath, FailedleadsPath);
    return true;
  }

  // #: 4
  bool ClickatellGatewaySender(std::string& ProgramDirectory, std::string& apiKey, std::string& SenderNum) {
    SMSDataStruct SMSDataObject;
    std::string bearerToken = apiKey;
    std::string LetterFilePath = fs::path(ProgramDirectory) / "letter.txt";
    std::string FailedleadsPath = fs::path(ProgramDirectory) / "failedleads.txt";
    std::string SentleadsPath = fs::path(ProgramDirectory) / "sentleads.txt";
    std::string LeadsFilePath = fs::path(ProgramDirectory) / "leads.txt";
    if (!ReadFileToVector(SMSDataObject.SMSDataVector, LeadsFilePath) || !ReadFromFile(LetterFilePath, SMSDataObject.message)) {
      std::cerr << "Error: Failed to read these directories:\n"
                << LeadsFilePath << "\n"
                << LetterFilePath << "\n";
      return false;
    }

    int batchsize, errorCount = 0, successCount = 0, interatorSize = 0;
    std::string url = "https://platform.clickatell.com/messages";
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
      std::cout << "Clickatell Gateway send SMS in batch \nChoose batch size (Recommended: 50/send): ";
      std::cin >> batchsize;
      interatorSize = SMSDataObject.SMSDataVector.size() / batchsize;

      for (int i = 0; i < interatorSize; i++) {
        std::string POSTDATA = "{ \"content\": \"" + SMSDataObject.message + "\", \"to\": [";
        for (int j = 0; j < batchsize; j++) {
          int index = i * batchsize + j;
          if (index >= SMSDataObject.SMSDataVector.size()) break;
          POSTDATA += "\"" + SMSDataObject.SMSDataVector[index] + "\",";
        }
        POSTDATA.pop_back();
        POSTDATA += "] }";

        std::string responseBody;
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + bearerToken).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, POSTDATA.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBody);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
          std::cerr << "Curl failed: " << curl_easy_strerror(res) << std::endl;
          curl_easy_cleanup(curl);
          curl_global_cleanup();
          return false;
        }

        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

        if (responseCode == 202) {
          Json::CharReaderBuilder reader;
          Json::Value jsonData;
          std::istringstream sstream(responseBody);

          if (!Json::parseFromStream(reader, sstream, &jsonData, nullptr)) {
            std::cerr << "Error parsing JSON response!" << std::endl;
            return false;
          }
          const Json::Value& messages = jsonData["messages"];

          for (int j = 0; j < messages.size(); j++) {
            std::string contact = messages[j]["to"].asString();
            std::string status = messages[j]["status"].asString();

            if (status == "QUEUED") {
              SMSSuccessMessage(SenderNum, contact);
              WriteToFile(SentleadsPath, contact);
              successCount++;
              errorCount = 0;
            } else {
              if (errorCount <= 6) {
                std::cout << "Check Clickatell Account for troubleshoot..." << std::endl;
                curl_easy_cleanup(curl);
                curl_global_cleanup();
                return false;
              }
              std::cout << "SMS Failed to send: " << contact << std::endl;
              WriteToFile(FailedleadsPath, contact);
              errorCount++;
            }
          }
        } else {
          std::cout << "Error: Operation failed, response code: " << responseCode << "\n";
          curl_easy_cleanup(curl);
          curl_global_cleanup();
          return false;
        }
      }

      std::cout << "Total successfully sent SMS to " << successCount << " Leads\n";
      curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    LeadFinalCleanUp(LeadsFilePath, SentleadsPath, FailedleadsPath);
    return true;
  }

  // #: 5
  bool MessageBirdGatewaySender(std::string& ProgramDirectory, std::string& apiKey, std::string& SenderNum) {
    SMSDataStruct SMSDataObject;
    std::string bearerToken = apiKey;
    std::string LetterFilePath = fs::path(ProgramDirectory) / "letter.txt";
    std::string FailedleadsPath = fs::path(ProgramDirectory) / "failedleads.txt";
    std::string SentleadsPath = fs::path(ProgramDirectory) / "sentleads.txt";
    std::string LeadsFilePath = fs::path(ProgramDirectory) / "leads.txt";
    if (!ReadFileToVector(SMSDataObject.SMSDataVector, LeadsFilePath) || !ReadFromFile(LetterFilePath, SMSDataObject.message)) {
      std::cerr << "Error: Failed to read these directories:\n"
                << LeadsFilePath << "\n"
                << LetterFilePath << "\n";
      return false;
    }
    int errorCount = 0, successCount = 0;
    std::string url = "https://rest.messagebird.com/messages";
    CURL* curl;
    CURLcode res;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
      for (auto& lead : SMSDataObject.SMSDataVector) {
        std::string POSTDATA = "{\"from\":\"" + SenderNum + "\",\"to\":\"" + lead + "\",\"body\":\"" + SMSDataObject.message + "\"}";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, NULL);
        std::string authHeader = "Authorization: AccessKey " + apiKey;
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, POSTDATA.c_str());

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
          std::cerr << "Curl failed to process operation..." << std::endl
                    << curl_easy_strerror(res) << std::endl;
          curl_easy_cleanup(curl);
          curl_global_cleanup();
          return false;
        }

        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

        if (responseCode == 200) {
          SMSSuccessMessage(SenderNum, lead);
          WriteToFile(SentleadsPath, lead);
          successCount++;
          errorCount = 0;
        } else {
          if (errorCount <= 3) {
            std::cout << "Failed to send SMS to " << lead << std::endl
                      << "Response code: " << responseCode << std::endl;
            WriteToFile(FailedleadsPath, lead);
            errorCount++;
          } else {
            std::cout << "Please check MessageBird Account for troubleshooting..." << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
          }
        }
      }
      std::cout << "Total successfully sent SMS to " << successCount << " Leads \n";
      curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    LeadFinalCleanUp(LeadsFilePath, SentleadsPath, FailedleadsPath);
    return true;
  }
};

int main(void) {
  std::cout << "\033[2J\033[H";
  GlobalMethodClass GlobalMethodObject;
  EmailSenderProgram EmailProgramObject;
  SMSGatewaySenderProgram SMSGatewaySenderProgram;
  std::string optionStr;
  std::string optionArr[3];
  std::string ProgramDirectory;
  bool UseAttachment, UseHTML;
  int option;
  std::cout << "\t\t[ECHOBLAST V1 : TELEGRAM H4CKECHO]\n\n"
            << "1. Email Sender \n"
            << "2. Email -> SMS Sender\n"
            << "3. SMS Gateway Sender\n"
            << "4. SMTP Live Tester\n"
            << "5. Remove Duplicates\n"
            << "6. Email Extractor\n"
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
        std::cout << "\033[2J\033[H"
                  << "Email Sender program completed.\n";
      } else {
        sleep(1);
        std::cout << "\033[2J\033[H"
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

      if (EmailProgramObject.EmailToSMSSender(optionStr)) {
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
      std::cout << "\033[2J\033[H";
      std::cout << "\t\t[SMS GATEWAY SENDER INITIALIZED]\n\n"
                << "1. Plivo SMS Gateway ($0.005/SMS)\n"
                << "2. Twilio SMS Gateway ($0.0079/SMS)\n"
                << "3. TrueDialog SMS Gateway ($0.007/SMS)\n"
                << "4. Clickatell SMS Gateway ($0.0075/SMS)\n"
                << "5. Message Bird SMS Gateway($0.0067/SMS)\n"
                << "Choose a SMS Gateway: ";
      std::cin >> option;
      switch (option) {
        case 1:
          ProgramDirectory = SMSGatewaySenderProgram.PrepareSMSProgramFiles("PlivoGatewaySender");
          std::cout << "\033[2J\033[H";
          std::cout << "\t\t[PLIVO SMS GATEWAY]\n\n";
          std::cout << "Plivo Auth ID: ";
          std::cin >> optionArr[0];
          std::cout << "Plivo Auth Token: ";
          std::cin >> optionArr[1];
          std::cout << "Plivo Sender Number: ";
          std::cin >> optionArr[2];
          std::cout << "\033[2J\033[H";
          if (SMSGatewaySenderProgram.PlivoGatewaySender(ProgramDirectory, optionArr[0], optionArr[1], optionArr[2])) {
            sleep(1);
            std::cout << "\033[2J\033[H";
            std::cout << "Plivo SMS Gateway Sender program completed.\n";
            GlobalMethodObject.prompt();
          } else {
            sleep(1);
            std::cout << "\033[2J\033[H";
            std::cout << "Plivo SMS Gateway Sender program failed.\n";
            GlobalMethodObject.prompt();
          }
          break;

        case 2:
          ProgramDirectory = SMSGatewaySenderProgram.PrepareSMSProgramFiles("TwilioGatewaySender");
          std::cout << "\033[2J\033[H";
          std::cout << "\t\t[TWILIO SMS GATEWAY]\n\n";
          std::cout << "Twilio Account SID: ";
          std::cin >> optionArr[0];
          std::cout << "Twilio Auth Token: ";
          std::cin >> optionArr[1];
          std::cout << "Twilio Sender Number: ";
          std::cin >> optionArr[2];
          std::cout << "\033[2J\033[H";
          if (SMSGatewaySenderProgram.TwilioGatewaySender(ProgramDirectory, optionArr[0], optionArr[1], optionArr[2])) {
            sleep(1);
            std::cout << "Twilio SMS Gateway Sender program completed.\n";
            GlobalMethodObject.prompt();
          } else {
            sleep(1);
            std::cout << "Twilio SMS Gateway Sender program failed.\n";
            GlobalMethodObject.prompt();
          }
          break;
        case 3:
          ProgramDirectory = SMSGatewaySenderProgram.PrepareSMSProgramFiles("TrueDialogGatewaySender");
          std::cout << "\033[2J\033[H";
          std::cout << "\t\t[TRUE DIALOG SMS GATEWAY]\n\n";
          std::cout << "TrueDialog Bearer Token: ";
          std::cin >> optionArr[0];
          std::cout << "TrueDialog Sender Number: ";
          std::cin >> optionArr[1];
          std::cout << "\033[2J\033[H";
          if (SMSGatewaySenderProgram.TrueDialogGatewaySender(ProgramDirectory, optionArr[0], optionArr[1])) {
            sleep(1);
            std::cout << "\033[2J\033[H";
            std::cout << "TrueDialog SMS Gateway Sender program completed.\n";
            GlobalMethodObject.prompt();
          } else {
            sleep(1);
            std::cout << "\033[2J\033[H";
            std::cout << "TrueDialog SMS Gateway Sender program failed.\n";
            GlobalMethodObject.prompt();
          }
          break;
        case 4:
          ProgramDirectory = SMSGatewaySenderProgram.PrepareSMSProgramFiles("ClickatellGatewaySender");
          std::cout << "\033[2J\033[H";
          std::cout << "\t\t[CLICKATELL SMS GATEWAY]\n\n";
          std::cout << "Clickatell API Key: ";
          std::cin >> optionArr[0];
          std::cout << "Clickatell Sender Number: ";
          std::cin >> optionArr[1];
          std::cout << "\033[2J\033[H";
          if (SMSGatewaySenderProgram.ClickatellGatewaySender(ProgramDirectory, optionArr[0], optionArr[1])) {
            sleep(1);
            std::cout << "\033[2J\033[H";
            std::cout << "Clickatell SMS Gateway Sender program completed.\n";
            GlobalMethodObject.prompt();
          } else {
            sleep(1);
            std::cout << "\033[2J\033[H";
            std::cout << "Clickatell SMS Gateway Sender program failed.\n";
            GlobalMethodObject.prompt();
          }
          break;

        case 5:
          ProgramDirectory = SMSGatewaySenderProgram.PrepareSMSProgramFiles("MessageBirdGatewaySender");
          std::cout << "\033[2J\033[H";
          std::cout << "\t\t[MESSAGE BIRD SMS GATEWAY]\n\n";
          std::cout << "Message Bird API Key: ";
          std::cin >> optionArr[0];
          std::cout << "Message Bird Sender Number: ";
          std::cin >> optionArr[1];
          std::cout << "\033[2J\033[H";
          if (SMSGatewaySenderProgram.MessageBirdGatewaySender(ProgramDirectory, optionArr[0], optionArr[1])) {
            sleep(1);
            std::cout << "\033[2J\033[H";
            std::cout << "Message Bird SMS Gateway Sender program completed.\n";
            GlobalMethodObject.prompt();
          } else {
            sleep(1);
            std::cout << "\033[2J\033[H";
            std::cout << "Message Bird SMS Gateway Sender program failed.\n";
            GlobalMethodObject.prompt();
          }
          break;
        default:
          std::cout << "Invalid option selected. \n";
          GlobalMethodObject.prompt();
          break;
      }
      break;
    case 4:
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

    case 5:
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
    case 6:
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
  return 0;
}