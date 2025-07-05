#include <iostream>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <string>

#include "./Modules/email/email_program_header.h"
#include "./Modules/utils/GenericMethods.h"

GlobalMethodClass GlobalMethodObject;

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

bool setWorkingDirectoryToExecutablePath() {
  char path[PATH_MAX];

#ifdef __APPLE__
  uint32_t size = sizeof(path);
  if (_NSGetExecutablePath(path, &size) != 0)
    return false;
#elif defined(__linux__)
  ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
  if (count == -1)
    return false;
  path[count] = '\0';
#else
  return false; // Unsupported OS
#endif

  char resolved[PATH_MAX];
  if (realpath(path, resolved) == nullptr)
    return false;

  if (chdir(dirname(resolved)) != 0)
    return false;

  return true;
}

void DisplayStartMenu(int& option) {
  GlobalMethodObject.clearScreen();
  std::string optionStr;
  std::cout << "\033[91m\033[1m\t[ECHO MAILER V2 : TELEGRAM H4CKECHO]\n\n"
    << "\033[94m[1] \033[93mEMAIL SENDER\n"
    << "\033[94m[2] \033[93mSMTP MAIL TESTER\n"
    << "\033[94m[3] \033[93mEMAIL LEAD EXTRACTOR\n"
    << "\033[94m[4] \033[93mDATA DUPLICATE REMOVAL\n\n"
    << "\033[92m\033[1mCHOOSE A PROGRAM [0 END PROGRAM]: ";
  std::cin >> optionStr;
  if (!GlobalMethodObject.ValidDigit(optionStr)) {
    GlobalMethodObject.clearScreen();
    std::cout << "Invalid input. Try again.\n";
    GlobalMethodObject.sleep(2);
    DisplayStartMenu(option);
  }
  else {
    option = std::stoi(optionStr);
  }
}

void TerminateProgram() {
  GlobalMethodObject.clearScreen();
  std::cout << "Terminating program...\n";
  GlobalMethodObject.sleep(1);
  GlobalMethodObject.clearScreen();
}

void HandleEmailSender(EmailSenderProgram& emailProgram, const std::string& senderType) {
  if (!emailProgram.PrepareEmailSenderDirectories(senderType)) {
    std::cerr << "ERROR: FAILED TO CREATE EMAIL SENDER DIRECTORIES\n";
    return;
  }
  GlobalMethodObject.clearScreenWithMessage("\t\t[EMAIL SENDER IN PROCESS]\n");
  bool useHTML = GlobalMethodObject.askYesNoQuestion("ARE YOU SENDING HTML LETTER");
  bool useAttachment = GlobalMethodObject.askYesNoQuestion("ARE YOU USING ATTACHMENT");
  bool success = (senderType == "CONSTANT")
    ? emailProgram.EmailSender(useHTML, useAttachment)
    : emailProgram.VariableEmailSender(useHTML, useAttachment);
  GlobalMethodObject.handleProgramCompletion(success, senderType + " EMAIL SENDER");
}

int main() {
  EmailSenderProgram emailProgram;
  if (setWorkingDirectoryToExecutablePath()) {
    std::cerr << "PROGRAM SET TO WORKING DIRECTORY\n";
  }

  int option;
  GlobalMethodObject.clearScreen();
  DisplayStartMenu(option);

  switch (option) {
  case 0:
    TerminateProgram();
    break;
  case 1: {
    GlobalMethodObject.clearScreen();
    std::cout << "\033[91m\033[1m\t[ECHO MAILER V1 : TELEGRAM H4CKECHO]\n\n"
      << "\033[94m[1] \033[93mCONSTANT SENDER\n"
      << "\033[94m[2] \033[93mVARIABLE SENDER\n\n"
      << "\033[92mCHOOSE SENDER TYPE: ";
    std::string senderType;
    std::cin >> senderType;
    if (senderType == "1") {
      HandleEmailSender(emailProgram, "CONSTANT");
    }
    else if (senderType == "2") {
      HandleEmailSender(emailProgram, "VARIABLE");
    }
    else {
      std::cerr << "Invalid sender type.\n";
      GlobalMethodObject.sleep(1);
      GlobalMethodObject.clearScreen();
      DisplayStartMenu(option);
    }
    break;
  }
  case 2: {
    if (!emailProgram.PrepareSMTPLiveTesterDirectories()) {
      std::cerr << "FAILED TO PREPARE PROGRAM FILES, TRY AGAIN.\n";
      break;
    }
    std::cout << "MAKE SURE, 'Raw.txt' IS NOT EMPTY.\n";
    if (GlobalMethodObject.askYesNoQuestion("DO YOU WANNA CONTINUE")) {
      bool success = emailProgram.SMTPLiveTester();
      GlobalMethodObject.handleProgramCompletion(success, "SMTP TEST PROGRAM");
    }
    break;
  }
  case 3: {
    emailProgram.PrepareEmailExtractorDirectories();
    GlobalMethodObject.clearScreenWithMessage("[EMAIL EXTRACTOR INITIALIZED]\n");
    std::cout << "PASTE THE DATA INSIDE [rawfile.txt] LOCATED IN [Email Extractor] FOLDER\n";
    if (GlobalMethodObject.askYesNoQuestion("Continue extraction?")) {
      bool success = emailProgram.EmailExtractor();
      GlobalMethodObject.handleProgramCompletion(success, "Email Extraction");
    }
    break;
  }
  case 4: {
    if (!emailProgram.PrepareDuplicateDirectories()) {
      std::cerr << "FAILED TO PREPARE PROGRAM FILES, TRY AGAIN.\n";
      break;
    }
    std::cout << "Make sure 'Raw.txt' is not empty.\n";
    if (GlobalMethodObject.askYesNoQuestion("Do you wanna continue")) {
      bool success = emailProgram.DuplicatesRemover();
      GlobalMethodObject.handleProgramCompletion(success, "DUPLICATE REMOVAL");
    }
    break;
  }
  default:
    GlobalMethodObject.clearScreen();
    std::cerr << "Invalid option selected.\n";
    break;
  }

  TerminateProgram();
  std::cout << "\033[0m";
  return 0;
}