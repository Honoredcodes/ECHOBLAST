#include <curl/curl.h>

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

void sleep(const int& x) {
  std::this_thread::sleep_for(std::chrono::seconds(x));
}
size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

class GlobalMethodClass {
 public:
  // #: 1
  std::string MakeAndGetDirectory(const std::string& path, const std::string& folderName) {
    static std::set<std::string> createdFolders;
    std::string fullPath;
    if (path.empty()) {
      fullPath = fs::current_path().string() + "/" + folderName;
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
  std::vector<std::string> ReadFileToVector(const std::string& filepath) {
    std::ifstream FileData(filepath);
    if (!FileData) {
      std::fstream FileData(filepath, std::ios::out);
      FileData.close();
      std::cout << "Path " << filepath << " not found \n"
                << "Path created, now try again.";
      return {};
    }
    std::string line;
    std::vector<std::string> FileVector;
    while (std::getline(FileData, line)) {
      if (!line.empty()) {
        FileVector.emplace_back(std::move(line));
      }
    }
    return FileVector;
  }

  // #: 3
  std::string GetRandomDataFromVector(const std::vector<std::string>& param) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, param.size() - 1);
    return param[dis(gen)];
  }

  // #: 4
  std::string GetAndSetSMTP(std::vector<std::string>& Vector, std::string& servername, int& port, std::string& username, std::string& password) {
    int VectorSize = Vector.size();
    if (VectorSize <= 0) {
      return "";
    }
    const std::string SMTP = Vector[0];
    std::stringstream ss(SMTP);
    std::getline(ss, servername, '|');
    ss >> port;
    ss.ignore();
    std::getline(ss, username, '|');
    std::getline(ss, password);
    for (int i = 0; i < VectorSize - 1; i++) {
      Vector[i] = Vector[i + 1];
    }
    return SMTP;
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
  void ReadFromFile(const std::string FilePath, std::string& VariableStorage) {
    try {
      std::fstream FileAccess(FilePath, std::ios::in);
      if (!FileAccess) {
        std::cout << "Error: failed to access " << FilePath << std::endl;
        return;
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
  }

  // #: 7
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
};

class MainProgram : public GlobalMethodClass {
 public:
  struct MailDataSet {
    std::vector<std::string> MailDataSetVector;
    struct SMTPAttribute {
      int port;
      std::string servername, username, password;
    };
  };

  std::string CurrentSMTP, DataDirectory, JunkDirectory, MailDataDirectory;
  MailDataSet SMTPVectorObject, NameVectorObject, SubjectVectorObject;
  MailDataSet::SMTPAttribute SMTPAttributeObject;

  // Main Program class constructor
  MainProgram() {
    DataDirectory = MakeAndGetDirectory("", "Data");
    MailDataDirectory = MakeAndGetDirectory(DataDirectory, "MailData");
    JunkDirectory = MakeAndGetDirectory(DataDirectory, "Junks");
    if (!MailDataDirectory.empty()) {
      SMTPVectorObject.MailDataSetVector = ReadFileToVector(MailDataDirectory + "/smtp.txt");
      NameVectorObject.MailDataSetVector = ReadFileToVector(MailDataDirectory + "/name.txt");
      SubjectVectorObject.MailDataSetVector = ReadFileToVector(MailDataDirectory + "/subject.txt");
    }
  }

  void SetCurlForMail(CURL* curl, std::string servername, int port, std::string username, std::string password) {
    servername += ":" + std::to_string(port);
    curl_easy_setopt(curl, CURLOPT_URL, servername.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
  }

  bool MakeEmailBody(std::string EmailContentBody, bool useHTML, bool useAttachment, std::string boundary, std::string sendername, std::string subject, std::string letter, std::string lead, std::string AttachmentFilePath = "") {
    if (useHTML) {
      EmailContentBody = "From: " + sendername +
                         "\r\n"
                         "To: " +
                         lead +
                         "\r\n"
                         "Subject: " +
                         subject +
                         "\r\n"
                         "MIME-Version: 1.0\r\n"
                         "Content-Type: multipart/mixed; boundary=\"" +
                         boundary + "\"\r\n\r\n";
      EmailContentBody += "--" + boundary + "\r\n";
      EmailContentBody += "Content-Type: text/html; charset=UTF-8\r\n\r\n" + letter + "\r\n";
    } else {
      EmailContentBody = "From: " + sendername +
                         "\r\n"
                         "To: " +
                         lead +
                         "\r\n"
                         "Subject: " +
                         subject +
                         "\r\n"
                         "MIME-Version: 1.0\r\n"
                         "Content-Type: multipart/mixed; boundary=\"" +
                         boundary + "\"\r\n\r\n";
      EmailContentBody += "--" + boundary + "\r\n";
      EmailContentBody += "Content-Type: text/plain; charset=UTF-8\r\n\r\n" + letter + "\r\n";
    }
    if (useAttachment) {
      std::string encodedFile = base64Encode(AttachmentFilePath);
      EmailContentBody += "--" + boundary + "\r\n";
      EmailContentBody += "Content-Type: application/octet-stream; name=\"" + AttachmentFilePath + "\"\r\n";
      EmailContentBody += "Content-Disposition: attachment; filename=\"" + AttachmentFilePath + "\"\r\n";
      EmailContentBody += "Content-Transfer-Encoding: base64\r\n\r\n" + encodedFile + "\r\n";
    }
    EmailContentBody += "--" + boundary + "--\r\n";
  }

  void printmessage(std::string& sendername, std::string& subject, std::string& lead) {
    std::cout << "=================================> ECHOBLAST EMAIL SENDER <================================="
              << "SENDER NAME: " << sendername << std::endl
              << "SUBJECT: " << subject << std::endl
              << "Email Successfully Sent: " << lead << std::endl
              << "=================================> ECHOBLAST EMAIL SENDER <=================================\n\n";
  }
  
  // #: 1
  void EmailSender(bool useHTML, bool useAttachment, std::string LeadsFile) {
    std::string LeadFileDirectory = fs::path(MailDataDirectory) / LeadsFile;
    std::string LetterFileDirectory = fs::path(MailDataDirectory) / "letter.txt";
    std::string SubjectFileDirectory = fs::path(MailDataDirectory) / "subject.txt";
    std::string SenderNameFileDirectory = fs::path(MailDataDirectory) / "name.txt";
    std::string SuccessfullySentEmail = fs::path(MailDataDirectory) / "sent.txt";
    std::string FailedLeadFile = fs::path(JunkDirectory) / "fails.txt";
    std::string UsedSMTPFile = fs::path(JunkDirectory) / "UsedSMTPS.txt";
    std::string AttachmentFilePath = MakeAndGetDirectory(MailDataDirectory, "Attachment");
    std::string EmailContentBody, AttachmentFileName = "", CurrentUsedSMTP = "", boundary = "----=_Part_001_2345_67890";
    long responseCode;
    if (useAttachment) {
      std::string tempstring;
      std::cout << "Provide Attachment filename with extension type (Example: filename.pdf): ";
      std::cin >> tempstring;
      AttachmentFileName = AttachmentFilePath + tempstring;
    }
    CurrentUsedSMTP = GetAndSetSMTP(SMTPVectorObject.MailDataSetVector, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
    int SentCount = 0, &port = SMTPAttributeObject.port;
    std::string& servername = SMTPAttributeObject.servername;
    std::string& username = SMTPAttributeObject.username;
    std::string& password = SMTPAttributeObject.password;
    std::string sendername, subject, letter;
    std::vector<std::string> LeadsVector = ReadFileToVector(LeadFileDirectory);
    CURL* curl;
    CURLcode res;
    curl_slist* recipients_list = nullptr;
    std::string response;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
      SetCurlForMail(curl, servername, port, username, password);
      for (auto& lead : LeadsVector) {
        sendername = GetRandomDataFromVector(NameVectorObject.MailDataSetVector);
        subject = GetRandomDataFromVector(SubjectVectorObject.MailDataSetVector);
        ReadFromFile(LetterFileDirectory, letter);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, sendername.c_str());
        recipients_list = curl_slist_append(recipients_list, lead.c_str());
        MakeEmailBody(EmailContentBody, useHTML, useAttachment, boundary, sendername, subject, letter, lead, AttachmentFileName);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients_list);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, EmailContentBody.c_str());
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &response);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (res == CURLE_OK && responseCode == 250) {
          WriteToFile(SuccessfullySentEmail, lead);
          printmessage(sendername, subject, lead);
          SentCount++;
          sleep(1);
        } else if (responseCode == 421 || responseCode == 451 || responseCode == 500 || responseCode == 502 || responseCode == 503 || responseCode == 521 || responseCode == 554) {
          WriteToFile(UsedSMTPFile, CurrentUsedSMTP);
          CurrentUsedSMTP = GetAndSetSMTP(SMTPVectorObject.MailDataSetVector, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
          if (CurrentUsedSMTP.empty()) {
            std::cout << "SMTP not found or finished....\nTotal successfully sent email to " << SentCount << " Leads \n ";
            curl_slist_free_all(recipients_list);
            break;
          }
          SetCurlForMail(curl, servername, port, username, password);
          res = curl_easy_perform(curl);
          curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
          if (res == CURLE_OK && responseCode == 250) {
            WriteToFile(SuccessfullySentEmail, lead);
            printmessage(sendername, subject, lead);
            SentCount++;
          }
        } else if (responseCode == 550 || responseCode == 551 || responseCode == 552 || responseCode == 553 || responseCode == 554) {
          std::cout << "Failed to send message to " << lead << std::endl;
          WriteToFile(FailedLeadFile, lead);
        }
        curl_slist_free_all(recipients_list);
      }
      std::cout << "Total successfully sent email to " << SentCount << " Leads \n";
      curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
  }

  // #: 2
  void SmsGatewaySender() {}

  // #: 4
  void SMTPLiveTester() {}

  //  #: 5
  void DuplicatesRemover(std::string& filename) {
    try {
      std::ifstream Filedata(filename);
      if (!Filedata) {
        std::cout << "Error: failed to access " << filename;
        return;
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
        return;
      }

      for (const auto& unique : dataset) {
        outFile << unique << std::endl;
      }
    } catch (const std::exception& error) {
      std::cerr << "Operation failed becase " << error.what() << std::endl;
    }
  }
};

int main(void) {
  MainProgram ProgramObject;
  std::string optionStr;
  bool UseAttachment, UseHTML;
  int option;
  std::cout << "\t\t[ECHOBLAST V1 : TELEGRAM H4CKECHO]\n";
  std::cout << "1. Email Sender \n"
            << "2. Email to SMS Sender"
            << "3. SMS Gateway Sender\n"
            << "4. SMTP Live Tester\n"
            << "5. Remove Duplicates\n"
            << "Choose a program: ";
  std::cin >> option;
  switch (option) {
    case 1:
      std::cout << "\t\t[EMAIL SENDER INITIALIZED]\n\n"
                << "Are you using HTML Letter (Y/N): ";
      std::cin >> optionStr;
      UseHTML = (optionStr == "Y" || optionStr == "y") ? true : false;
      std::cout << "Are you using Attachment (Y/N): ";
      std::cin >> optionStr;
      UseAttachment = (optionStr == "Y" || optionStr == "y") ? true : false;
      std::cout << "Email lead filename (filename.txt): ";
      std::cin >> optionStr;
      ProgramObject.EmailSender(UseHTML, UseAttachment, optionStr);
      break;
    case 2:
      std::cout << "Email -> SMS lead filename (filename.txt): ";
      std::cin >> optionStr;
      ProgramObject.SmsGatewaySender();
      break;
    case 3:
      ProgramObject.SMTPLiveTester();
      break;
    case 4:
      std::cout << "Duplicate Filename (filename.txt): ";
      std::cin >> optionStr;
      ProgramObject.DuplicatesRemover(optionStr);
      break;
    default:
      break;
  }

  return 0;
}