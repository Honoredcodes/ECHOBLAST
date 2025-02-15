#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

class GlobalMethodClass {
 public:
  // #: 1
  std::string MakeAndGetDirectory(const std::string& folderName) {
    try {
      if (!fs::exists(folderName) && !fs::create_directories(folderName)) {
        std::cerr << "Error: Failed to make directory: " << folderName << std::endl;
        return "";
      }
      return folderName;
    } catch (const std::exception& e) {
      std::cerr << "Failed to create folder because: " << e.what() << std::endl;
    }
    return folderName;
  }

  // #: 2
  std::vector<std::string> ReadFileToVector(const std::string& filepath) {
    std::ifstream FileData(filepath);
    if (!FileData) {
      std::cerr << "Error: failed to access input file path \n";
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
};

class MainProgram {
  struct MailDataSet {
    std::vector<std::string> MailDataset;
    struct SMTPStruct {
      std::string servername, port, username, password;
    };
  };

  static GlobalMethodClass GlobalMethod;
  std::string DataSetDirectory = GlobalMethod.MakeAndGetDirectory("Data");
  MailDataSet SMTP, Name, Subject;

 public:
  // Main Program class constructor
  MainProgram() {
    DataSetDirectory = GlobalMethod.MakeAndGetDirectory("Data");
    if (!DataSetDirectory.empty()) {
      SMTP.MailDataset = GlobalMethod.ReadFileToVector(DataSetDirectory + "/smtp.txt");
      Name.MailDataset = GlobalMethod.ReadFileToVector(DataSetDirectory + "/name.txt");
      Subject.MailDataset = GlobalMethod.ReadFileToVector(DataSetDirectory + "/subject.txt");
    }
  }

  // #: 1
  void EmailSender(bool useHTML, bool useAttachment, std::string& filename) {}

  // #: 2
  void EmailSmsSender(std::string& filename) {}

  // #: 3
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

      // Filter data and remove duplicates
      while (std::getline(Filedata, line)) {
        if (!line.empty()) {
          dataset.insert(line);
        }
      }

      // Return filtered data back to file
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
      std::cout << "Using HTML Letter (Y/N): ";
      std::cin >> optionStr;
      UseHTML = (optionStr == "Y" || optionStr == "y") ? true : false;
      std::cout << "Using Attachment (Y/N): ";
      std::cin >> optionStr;
      UseAttachment = (optionStr == "Y" || optionStr == "y") ? true : false;
      std::cout << "Email lead filename (filename.txt): ";
      std::cin >> optionStr;
      ProgramObject.EmailSender(UseHTML, UseAttachment, optionStr);
      break;
    case 2:
      std::cout << "Email -> SMS lead filename (filename.txt): ";
      std::cin >> optionStr;
      ProgramObject.EmailSmsSender(optionStr);
      break;
    case 3:
      std::cout << "Email -> SMS lead filename (filename.txt): ";
      std::cin >> optionStr;
      ProgramObject.SmsGatewaySender();
      break;
    case 4:
      ProgramObject.SMTPLiveTester();
      break;
    case 5:
      std::cout << "Duplicate Filename (filename.txt): ";
      std::cin >> optionStr;
      ProgramObject.DuplicatesRemover(optionStr);
      break;
    default:
      break;
  }

  return 0;
}