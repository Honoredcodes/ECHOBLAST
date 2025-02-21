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
  bool ReadFileToVector(std::vector<std::string> FileVector, const std::string& filepath) {
    std::ifstream FileData(filepath);
    if (!FileData) {
      std::fstream FileData(filepath, std::ios::out);
      FileData.close();
      std::cout << "Path " << filepath << " not found \n"
                << "Path created, now try again.\n";
      return false;
    }
    std::string line;
    while (std::getline(FileData, line)) {
      if (!line.empty()) {
        FileVector.emplace_back(std::move(line));
      }
    }
    return true;
  }

  // #: 3
  std::string GetRandomDataFromVector(const std::vector<std::string>& param) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, param.size() - 1);
    return param[dis(gen)];
  }

  // #: 4
  bool GetAndSetSMTP(std::vector<std::string>& Vector, std::string& CurrentSmtp, std::string& servername, int& port, std::string& username, std::string& password) {
    int VectorSize = Vector.size();
    if (VectorSize <= 0) {
      return false;
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
  void LeadFinalCleanUp(std::string& leadfile, std::string sentleads, std::string failedleads) {
    std::ifstream leadfiledata(leadfile);
    std::ifstream sentleadsdata(sentleads);
    std::ifstream failedleadsdata(failedleads);
    if (!leadfiledata || !sentleadsdata || !failedleadsdata) {
      std::cerr << "Error: Unable to open one or more files.\n";
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
      std::cout << "Lead cleanup complete. Remaining leads: " << leadset.size() << std::endl;
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

  void stripdata(std::string& server, std::string& port) {
    std::stringstream ss(server);
    std::getline(ss, server, ':');
    std::getline(ss, port);
  }
};

class EmailSenderProgram : public GlobalMethodClass {
 public:
  struct MailDataSet {
    std::vector<std::string> MailDataSetVector;
    struct SMTPAttribute {
      int port;
      std::string servername, username, password;
    };
  };

  MailDataSet SMTPVectorObject, NameVectorObject, SubjectVectorObject;
  MailDataSet::SMTPAttribute SMTPAttributeObject;
  std::string DataDirectory;

  // Main Program class constructor
  EmailSenderProgram() {
    DataDirectory = MakeAndGetDirectory("", "Data");
  }

  void SetCurlForMail(CURL* curl, std::string servername, int port, std::string username, std::string password) {
    servername += ":" + std::to_string(port);
    curl_easy_setopt(curl, CURLOPT_URL, servername.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
  }

  void MakeEmailBody(std::string EmailContentBody, bool useHTML, bool useAttachment, std::string boundary, std::string sendername, std::string subject, std::string letter, std::string lead, std::string AttachmentFilePath = "") {
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

  // #: 1 Email Sender
  void EmailSender(bool useHTML, bool useAttachment, std::string LeadsFile) {
    std::string EmailDirectory = MakeAndGetDirectory(DataDirectory, "EmailData");
    std::string MailDataDirectory = MakeAndGetDirectory(EmailDirectory, "MailData");
    std::string JunkDirectory = MakeAndGetDirectory(EmailDirectory, "Junks");
    if (MailDataDirectory.empty()) {
      std::cout << "Please create the list of file assets for EmailSMSData\n"
                << "smtp.txt, name.txt, subject.txt, letter.txt\n";
      return;
    }
    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, MailDataDirectory + "/smtp.txt") ||
        !ReadFileToVector(NameVectorObject.MailDataSetVector, MailDataDirectory + "/name.txt") ||
        !ReadFileToVector(SubjectVectorObject.MailDataSetVector, MailDataDirectory + "/subject.txt")) {
      std::cout << "Error: failed to read some file data\n"
                << "Retry program again\n";
      return;
    }

    std::string LeadFileDirectory = fs::path(MailDataDirectory) / LeadsFile;
    std::string LetterFileDirectory = fs::path(MailDataDirectory) / "letter.txt";
    std::string SentEmail = fs::path(MailDataDirectory) / "sent.txt";
    std::string FailedLeadsFile = fs::path(JunkDirectory) / "failed.txt";
    std::string DeadSMTPsFile = fs::path(JunkDirectory) / "deadsmtp.txt";
    std::string AttachmentFilePath = MakeAndGetDirectory(MailDataDirectory, "Attachment");
    std::string EmailContentBody, AttachmentFileName, CurrentSmtp, boundary = "----=_Part_001_2345_67890";
    long responseCode;
    if (useAttachment) {
      std::string tempstring;
      std::cout << "Provide Attachment filename with extension type (Example: filename.pdf): ";
      std::cin >> tempstring;
      AttachmentFileName = fs::path(AttachmentFilePath) / tempstring;
    }
    if (!GetAndSetSMTP(SMTPVectorObject.MailDataSetVector, CurrentSmtp, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password)) {
      std::cout << "SMTP not found or finished....\n";
      return;
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
      return;
    }
    ReadFromFile(LetterFileDirectory, letter);
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
        MakeEmailBody(EmailContentBody, useHTML, useAttachment, boundary, sendername, subject, letter, lead, AttachmentFileName);
        recipients_list = curl_slist_append(recipients_list, lead.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, sendername.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients_list);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, EmailContentBody.c_str());
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &response);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (res == CURLE_OK && responseCode == 250) {
          WriteToFile(SentEmail, lead);
          printmessage(sendername, subject, lead);
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
              printmessage(sendername, subject, lead);
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
  }

  // #: 2 Email to SMS Sender
  void EmailToSMSSender(std::string& LeadsFile) {
    std::string EmailDirectory = MakeAndGetDirectory(DataDirectory, "EmailSMSData");
    std::string MailDataDirectory = MakeAndGetDirectory(EmailDirectory, "MailData");
    std::string JunkDirectory = MakeAndGetDirectory(EmailDirectory, "Junks");
    if (MailDataDirectory.empty()) {
      std::cout << "Please create the list of file assets for EmailSMSData\n"
                << "smtp.txt, name.txt, subject.txt, letter.txt\n";
      return;
    }
    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, MailDataDirectory + "/smtp.txt") ||
        !ReadFileToVector(NameVectorObject.MailDataSetVector, MailDataDirectory + "/name.txt")) {
      std::cout << "Error: failed to read some file data\n"
                << "Retry program again\n";
      return;
    }

    std::string LeadFileDirectory = fs::path(MailDataDirectory) / LeadsFile;
    std::string LetterFileDirectory = fs::path(MailDataDirectory) / "letter.txt";
    std::string SentEmail = fs::path(MailDataDirectory) / "sent.txt";
    std::string FailedLeadsFile = fs::path(JunkDirectory) / "failed.txt";
    std::string DeadSMTPsFile = fs::path(JunkDirectory) / "deadsmtp.txt";
    std::string CurrentSmtp;

    if (!GetAndSetSMTP(SMTPVectorObject.MailDataSetVector, CurrentSmtp, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password)) {
      std::cout << "SMTP not found or finished....\n";
      return;
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
      return;
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
      for (auto& lead : LeadsVector) {
        sendername = GetRandomDataFromVector(NameVectorObject.MailDataSetVector);
        recipients_list = curl_slist_append(recipients_list, lead.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, sendername.c_str());
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients_list);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, letter.c_str());
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_READDATA, &response);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (res == CURLE_OK && responseCode == 250) {
          WriteToFile(SentEmail, lead);
          printmessage(sendername, subject, lead);
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
              printmessage(sendername, subject, lead);
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
  }

  // #: 3 SMTP LIVE TESTER
  void SMTPLiveTester(std::string& filename) {
    CURL* curl;
    CURLcode res;
    int totalvalid = 0, totaldeadsmtp = 0;
    std::unordered_set<std::string> invalidsmtp, validsmtp;
    std::string servername, port, username, password;
    std::string CurrentSmtp, smtp, response, testemail, boundary = "----=_Part_001_2345_67890";
    std::string SMTPDataDirectory = MakeAndGetDirectory(DataDirectory, "SMTPTestData");
    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, SMTPDataDirectory + "/" + filename)) {
      std::cerr << "Error: failed to read " << filename << std::endl
                << "Retry program again\n";
      return;
    }

    std::cout << "Enter a valid email address: ";
    std::cin >> testemail;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    std::cout << "SMTP Live Tester Initialized...\n"
              << "Please wait while the program is testing the SMTPs...\n";

    while (true) {
      if (!GetAndSetSMTP(SMTPVectorObject.MailDataSetVector, CurrentSmtp, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password)) {
        std::cout << "Total SMTP tested: " << totalvalid + totaldeadsmtp << "\n"
                  << "Total good SMTPs: " << totalvalid << "\n"
                  << "Total dead SMTPs: " << totaldeadsmtp << "\n";
        break;
      }

      curl = curl_easy_init();
      if (!curl) {
        std::cerr << "Failed to initialize curl." << std::endl;
        break;
      }

      struct curl_slist* recipients = nullptr;
      recipients = curl_slist_append(recipients, testemail.c_str());

      response.clear();
      SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);

      std::string data =
          "Subject: SMTP Test OK\r\n\r\nTest email sent successfully.\r\n"
          "MIME-Version: 1.0\r\n"
          "Content-Type: multipart/mixed; boundary=\"" +
          boundary +
          "\"\r\n\r\n"
          "--" +
          boundary + "\r\n";

      curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
      curl_easy_setopt(curl, CURLOPT_MAIL_FROM, SMTPAttributeObject.username.c_str());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, writeCallback);
      curl_easy_setopt(curl, CURLOPT_READDATA, &response);
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

      res = curl_easy_perform(curl);

      if (res == CURLE_OK) {
        servername = SMTPAttributeObject.servername;
        port = SMTPAttributeObject.port;
        smtp = servername + "|" + port + "|" + SMTPAttributeObject.username + "|" + SMTPAttributeObject.password;

        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

        if (responseCode == 250) {
          validsmtp.insert(smtp);
          totalvalid++;
        } else {
          totaldeadsmtp++;
          invalidsmtp.insert(smtp);
        }
      } else {
        std::cout << "Failed to perform SMTP request: " << curl_easy_strerror(res) << std::endl;
      }
      curl_easy_cleanup(curl);
      curl_slist_free_all(recipients);
    }

    curl_global_cleanup();

    std::ofstream smtpfile(SMTPDataDirectory + "/smtp.txt", std::ios::out);
    std::ofstream invalidsmtpfile(SMTPDataDirectory + "/invalidsmtp.txt", std::ios::out);

    if (!smtpfile || !invalidsmtpfile) {
      std::cerr << "Error: failed to access output files." << std::endl;
      return;
    }

    for (const auto& smtp : validsmtp) {
      smtpfile << smtp << std::endl;
    }
    for (const auto& smtp : invalidsmtp) {
      invalidsmtpfile << smtp << std::endl;
    }

    smtpfile.close();
    invalidsmtpfile.close();
  }
};

int main(void) {
  GlobalMethodClass GlobalMethodObject;
  EmailSenderProgram EmailProgramObject;
  std::string optionStr;
  bool UseAttachment, UseHTML;
  int option;
  std::cout << "\t\t[ECHOBLAST V1 : TELEGRAM H4CKECHO]\n";
  std::cout << "1. Email Sender \n"
            << "2. Email SMS Sender\n"
            << "3. SMTP Live Tester\n"
            << "4. Remove Duplicates\n"
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
      std::cout << "Email lead filename (Example: filename.txt): ";
      std::cin >> optionStr;
      EmailProgramObject.EmailSender(UseHTML, UseAttachment, optionStr);
      break;
    case 2:
      std::cout << "\t\t[EMAIL -> SMS SENDER INITIALIZED]\n\n"
                << "Phone Number lead filename (Example: filename.txt): ";
      std::cin >> optionStr;
      EmailProgramObject.EmailToSMSSender(optionStr);
      break;
    case 3:
      std::cout << "\t\t[SMTP LIVE TESTER INITIALIZED]\n\n";
      std::cout << "SMTP filename (Example: filename.txt): ";
      std::cin >> optionStr;
      EmailProgramObject.SMTPLiveTester(optionStr);
      break;
    case 4:
      std::cout << "Data to filter filename (Example: filename.txt): ";
      std::cin >> optionStr;
      GlobalMethodObject.DuplicatesRemover(optionStr);
      break;
    default:
      break;
  }

  return 0;
}