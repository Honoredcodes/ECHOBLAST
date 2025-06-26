#include <iostream>
#include <mach-o/dyld.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>

#include "./Modules/email/email_program_header.h"
#include "./Modules/utils/GenericMethods.h"

GlobalMethodClass GlobalMethodObject;

bool setWorkingDirectoryToExecutablePath() {
  char path[PATH_MAX];
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) != 0) return false;
  char resolved[PATH_MAX];
  if (realpath(path, resolved) == nullptr)return false;
  if (chdir(dirname(resolved)) != 0) return false;
  return true;
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
  if (!GlobalMethodObject.ValidDigit(optionStr)) {
    do {
      GlobalMethodObject.clearScreen();
      std::cout << "Wrong input choice\nTry again with correct choice\n";
      GlobalMethodObject.sleep(1);
      DisplayStartMenu(option);
    } while (!GlobalMethodObject.ValidDigit(optionStr));
  }
  option = std::stoi(optionStr);
}

void TerminateProgram() {
  GlobalMethodObject.clearScreen();
  std::cout << "Terminating program...\n";
  GlobalMethodObject.sleep(1);
  GlobalMethodObject.clearScreen();
}

int main(void) {
  EmailSenderProgram EmailProgramObject;

  if (setWorkingDirectoryToExecutablePath()) {
    std::cerr << "PROGRAM SET TO SET WORKING REDIRECTORY\n";

  }

  std::string optionStr;
  bool UseAttachment, UseHTML;
  int option;

  GlobalMethodObject.clearScreen();
  DisplayStartMenu(option);

  switch (option) {
  case 0: {
    TerminateProgram();
    return 0;
  }

  case 1: {
    GlobalMethodObject.clearScreen();
    std::cout << "\033[91m"
      << "\033[1m"
      << "\t[ECHO MAILER V1 : TELEGRAM H4CKECHO]\n\n"
      << "\033[94m" << "[1] " << "\033[93m" << "CONSTANT SENDER\n"
      << "\033[94m" << "[2] " << "\033[93m" << "VARIABLE SENDER\n\n"
      << "\033[92m" << "CHOOSE SENDER TYPE: ";
    std::cin >> optionStr;
    if (optionStr != "1" && optionStr != "2") {
      std::cerr << "Invalid option selected. \n";
      GlobalMethodObject.sleep(1);
      GlobalMethodObject.clearScreen();
      DisplayStartMenu(option);
    }
    if (optionStr == "1") {
      if (!EmailProgramObject.PrepareEmailSenderDirectories()) {
        std::cerr << "Error: Program failed to create Email Sender directories\n";
        return 1;
      }
      GlobalMethodObject.clearScreenWithMessage("\t\t[EMAIL SENDER IN PROCESS]\n");
      UseHTML = GlobalMethodObject.askYesNoQuestion("ARE YOU SENDING HTML LETTER");
      UseAttachment = GlobalMethodObject.askYesNoQuestion("ARE YOU USING ATTACHMENT");

      bool success = EmailProgramObject.EmailSender(UseHTML, UseAttachment);
      GlobalMethodObject.handleProgramCompletion(success, "CONSTANT EMAIL SENDER");
    }
    else if (optionStr == "2") {
      if (!EmailProgramObject.PrepareEmailSenderDirectories()) {
        std::cerr << "Error: Program failed to create Email Sender directories\n";
        return 1;
      }
      GlobalMethodObject.clearScreenWithMessage("\t\t[EMAIL SENDER IN PROCESS]\n");
      UseHTML = GlobalMethodObject.askYesNoQuestion("ARE YOU SENDING HTML LETTER");
      UseAttachment = GlobalMethodObject.askYesNoQuestion("ARE YOU USING ATTACHMENT");
      bool success = EmailProgramObject.VariableEmailSender(UseHTML, UseAttachment);
      GlobalMethodObject.handleProgramCompletion(success, "VARIABLE EMAIL SENDER");
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
    GlobalMethodObject.handleProgramCompletion(success, "SMTP TEST PROGRAM");
    break;
  }

  case 3: {
    EmailProgramObject.PrepareEmailExtractorDirectories();
    GlobalMethodObject.clearScreenWithMessage("[EMAIL EXTRACTOR INITIALIZED]\n");
    std::cout << "PASTE THE DATA INSIDE  [rawfile.txt] LOCATED IN [Email Extractor] FOLDER\n";
    if (GlobalMethodObject.askYesNoQuestion("Continue extraction?:")) {
      bool success = EmailProgramObject.EmailExtractor();
      GlobalMethodObject.handleProgramCompletion(success, "Email Extraction");
    }
    break;
  }

  case 4: {
    GlobalMethodObject.clearScreen();
    if (!EmailProgramObject.PrepareDuplicateDirectories()) {
      std::cerr << "FAILED TO PREPARE PROGRAM FILES, TRY AGAIN.\n";
      break;
    }
    std::cout << "Make sure 'Raw.txt' is not empty.\n"
      << "Do you wanna continue (Y/N): ";
    std::cin >> optionStr;
    if (optionStr == "Y" || optionStr == "y") {
      bool success = EmailProgramObject.DuplicatesRemover();
      GlobalMethodObject.handleProgramCompletion(success, "DUPLICATE REMOVAL");
    }
    break;
  }

  default:
    GlobalMethodObject.clearScreen();
    std::cout << "Invalid option selected. \n";
    break;
  }
  GlobalMethodObject.prompt(main);
  TerminateProgram();
  std::cout << "\033[0m";
  return 0;
}