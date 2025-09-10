#include "email_program_header.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <random>
#include <filesystem>
#include <curl/curl.h>
#include <sstream>
#include <regex>
#include <mutex>
#include <future>
#include <unordered_set>
#include <set>
#include <thread>

#include "../utils/GenericMethods.h"
#include "../services/curl_utils.h"

namespace fs = std::filesystem;
std::string CHOOSEDIRECTORY() {
    GlobalMethodClass func;
    std::string res, path;
    bool validChoice = false;

    do {
        func.clearScreen();
        std::cout << "\033[91m\033[1m\tCHOOSE PROGRAM ROOT DIRECTORY\n\n"
            << "\033[94m[1] \033[93mDESKTOP\n"
            << "\033[94m[2] \033[93mDOWNLOADS\n"
            << "\033[94m[3] \033[93mDOCUMENTS\n"
            << "\033[92m\033[1mCHOOSE A DIRECTORY [0 END PROGRAM]: ";
        std::cin >> res;
        if (res == "0") {
            func.clearScreen();
            std::exit(0);
        }
        if (res == "1") {
            path = func.MakeRootDirectory("Desktop");
            validChoice = true;
            func.sleep(3);
        }
        else if (res == "2") {
            path = func.MakeRootDirectory("Downloads");
            validChoice = true;
            func.sleep(3);
        }
        else if (res == "3") {
            path = func.MakeRootDirectory("Documents");
            validChoice = true;
            func.sleep(3);
        }
        else {
            std::cout << "Invalid choice, please try again.\n";
        }

    } while (!validChoice);

    return path;
}

// Constructor implementation
EmailSenderProgram::EmailSenderProgram() {
    ParentEmailSenderDirectory = CHOOSEDIRECTORY();
    // ParentEmailSenderDirectory = MakeDirectory("", ParentEmailSenderDirectory);
}

// PrepareEmailSenderDirectories implementation
bool EmailSenderProgram::PrepareEmailSenderDirectories(const std::string type) {
    ProgramAttributeVector = { "smtps.txt", "sendername.txt", "subject.txt", "letter.txt", "leads.txt", "sentleads.txt", "failedleads.txt", "deadsmtps.txt", "smtps_under_limit.txt" };
    if (!CreateProgramDirectories(ParentEmailSenderDirectory, EmailProgramDirectory, type, EmailDataDirectory, EmailJunkDirectory)) return false;
    CreateProgramAttributeFiles(EmailDataDirectory, EmailJunkDirectory, ProgramAttributeVector);
    return true;
}

// PrepareSMTPLiveTesterDirectories implementation
bool EmailSenderProgram::PrepareSMTPLiveTesterDirectories() {
    ProgramAttributeVector.clear();
    ProgramAttributeVector = { "Raw.txt", "Failed.txt", "Live.txt" };
    SMTPLiveTesterDirectory = MakeDirectory(ParentEmailSenderDirectory, SMTPLiveTesterDirectory);
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

// PrepareEmailExtractorDirectories implementation
bool EmailSenderProgram::PrepareEmailExtractorDirectories() {
    ProgramAttributeVector.clear();
    ProgramAttributeVector = { "Raw.txt", "Result.txt" };
    EmailExtractorDirectory = MakeDirectory(ParentEmailSenderDirectory, EmailExtractorDirectory);
    for (auto& file : ProgramAttributeVector) {
        std::string FilePath = fs::path(EmailExtractorDirectory) / file;
        std::fstream CreateFile(FilePath, std::ios::app);
        if (!CreateFile) CreateFile.close();
    }
    return true;
}

// PrepareDuplicateDirectories implementation
bool EmailSenderProgram::PrepareDuplicateDirectories() {
    ProgramAttributeVector.clear();
    ProgramAttributeVector = { "Raw.txt", "Result.txt" };
    DuplicateRemoval = MakeDirectory(ParentEmailSenderDirectory, DuplicateRemoval);
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


template <typename T>
void CopyVector(std::vector<T>& source, std::vector<T>& destination) {
    destination = source;
    source.clear();
}

bool EmailSenderProgram::EmailSender(bool useHTML, bool useAttachment) {
    clearScreen();
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

    int sentCount = 0, rateLimit = 0, rateCount = 0, sendspeed, userinput;
    size_t index = 0;
    bool flag = false;

    std::vector<std::string> leads, tempvector;
    std::unordered_map<std::string, int> CoolDownMap;

    std::ofstream
        sentLeads(SENTLEADSFILE, std::ios::app), failedLeads(FAILEDLEADFILE, std::ios::app),
        deadSMTPs(DEADSMTPFILE, std::ios::app), limitedSMTPs(LIMITEDSMTP, std::ios::app);

    if (!sentLeads || !failedLeads || !deadSMTPs || !limitedSMTPs) {
        std::cerr << "Failed to open output files.\n";
        return false;
    }

    // Read data from files into vectors
    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, SMTPFILES) ||
        !ReadFileToVector(NameVectorObject.MailDataSetVector, SENDERNAMESFILE) ||
        !ReadFileToVector(SubjectVectorObject.MailDataSetVector, SUBJECTSFILE) ||
        !ReadFileToVector(leads, LEADSFILE)) {
        std::cerr << "FAILED TO READ EMAIL ATTRIBUTES\n";
        return false;
    }

    // Fetch letter content from file
    if (!FetchDataFromFile(letterPath, letter)) {
        std::cerr << "EMAIL LETTER NOT FOUND.\n";
        return false;
    }

    // Load initial SMTP settings
    if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
        SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
        std::cerr << "NO SMTP FOUND INITIALLY.\n";
        return false;
    }

    // Handle attachment if specified
    if (useAttachment) {
        attachmentPath = MakeDirectory(EmailDataDirectory, "Attachment");
        clearScreen();
        std::cout << "Provide Attachment filename (filename.pdf): ";
        std::getline(std::cin >> std::ws, attachmentFileName);
        if (!fs::exists(attachmentFileName)) {
            std::cerr << "Attachment file not found.\n";
            useAttachment = false;
        }
        else {
            attachmentFileName = fs::path(attachmentPath) / attachmentFileName;
        }
    }

    // Check if multiple SMTPs are available
    // If more than one SMTP is available, prompt user for send rate
    if (SMTPVectorObject.MailDataSetVector.size() > 1) {
        clearScreen();
        std::cout << "TOTAL " << SMTPVectorObject.MailDataSetVector.size() << " SMTPS DETECTED.\n"
            << "NOTE: SET SEND RATE TO ROTATE AND EXTEND SMTP LIFE.\n\n"
            << "DO YOU WANT TO SET SEND RATE PER SMTP: (Y/N): ";
        std::string tmp;
        std::getline(std::cin >> std::ws, tmp);
        while (tmp.empty() || (tmp != "Y" && tmp != "y" && tmp != "N" && tmp != "n")) {
            clearScreen();
            std::cin.clear();
            std::cerr << "INVALID RESPONSE, RATE LIMIT NOT SET.\n";
            std::cout << "DO YOU WANT TO SET SEND RATE PER SMTP: (Y/N): ";
            std::getline(std::cin >> std::ws, tmp);
        }

        if (tmp == "Y" || tmp == "y") {
            clearScreen();
            std::cout << "SET SEND RATE VALUE PER SMTP: ";
            if (!(std::cin >> rateLimit) || rateLimit < 0) {
                std::cin.clear();
                std::cerr << "INVALID INPUT, RATE LIMIT NOT SET.\n";
                rateLimit = 0;
            }
        }
        std::cout << "PLEASE HIT THE ENTER KEY ONCE AGAIN.";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    clearScreen();
    PromptSendDelayMessage();

    // Prompt user for send delay
    if (!(std::cin >> userinput) || userinput < 0) {
        do {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            clearScreen();
            std::cerr << "INVALID CHOICE, PLEASE MAKE THE SELECTION AGAIN.\n";
            sleep(1);
            clearScreen();
            PromptSendDelayMessage();
        } while (!(std::cin >> userinput) || userinput < 0);
    }
    else {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Set the flag based on user input for send delay
    flag = (userinput < 1);
    DelayHandler(sendspeed, userinput, flag);

    // Initialize CURL for sending emails
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "INITIALIZATION FAILED.\nPLEASE CHECK YOUR INTERNET OR VPN/PROXY IF IN USE.\n";
        curl_global_cleanup();
        return false;
    }

    // Set CURL options for sending emails
    SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
    clearScreenWithMessage("\t\t\t[CONSTANT EMAIL SENDER IN PROGRESS]\n");
    std::regex emailRegex(R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)");

    for (const auto& lead : leads) {
        if (sentCount > 0) sleep(sendspeed);

        if (!std::regex_match(lead, emailRegex)) {
            failedLeads << lead << '\n';
            continue;
        }

        int retries = 5;
        bool sentSuccessfully = false;

        while (retries > 0 && !sentSuccessfully) {
            // Handle rate limiting and switch SMTP if needed
            if (rateLimit > 0 && rateCount >= rateLimit) {
                std::cout << "\n\nSEND RATE LIMIT SET IS REACHED FOR CURRENT SMTP\n[" << CURRENTSMTP << "]\n";
                sleep(2);

                if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
                    SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
                    CopyVector(tempvector, SMTPVectorObject.MailDataSetVector);
                    std::cout << "\nALL SMTP USED UP OR UNAVAILABLE\nLOAD MORE SMTP AND RETRY PROGRAM\n\n";
                    break; // abort the loop entirely
                }
                std::cout << "NEW SMTP [" << CURRENTSMTP << "] IS LOADED.\n";
                std::cout << "RETRYING TO SEND EMAIL TO " << lead << " WITH NEW SMTP.\n\n";
                SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
                rateCount = 0;
            }

            // Generate new sender/subject, build email body
            sender = GetRandomDataFromVector(NameVectorObject.MailDataSetVector);
            subject = GetRandomDataFromVector(SubjectVectorObject.MailDataSetVector);
            MakeEmailBody(emailBody, useHTML, useAttachment, SMTPAttributeObject.servername, sender, SMTPAttributeObject.username, subject, letter, lead, attachmentFileName);

            // Send attempt
            CURLcode res = CURLE_GOT_NOTHING;
            curl_read_callback readCallback = nullptr;
            long responseCode = 0;
            PerformCurlSend(curl, res, SMTPAttributeObject.username, lead, &emailBody, readCallback, responseCode);

            if (res == CURLE_OK && responseCode == 250) {
                sentLeads << lead << '\n';
                sentCount++;
                rateCount++;
                EMAILSUCCESSMESSAGE(sender, subject, lead, sentCount);
                sentSuccessfully = true; // exit the retry loop
            }
            else {
                clearScreen();
                int tempcount = 20;
                std::cout << "\n============================= 🔔 ALERT: IMPORTANT MESSAGE =============================\n\n";
                while (tempcount--) {

                    if (tempcount < 16) system("play -n synth 0.2 sine 880 vol 0.5 > /dev/null 2>&1");
                    std::cout << "\r[🔔 ALERT] CHANGE VPN WITHIN " << tempcount << " SECONDS\n" << std::flush;
                    sleep(1);
                }

                clearScreen();
                std::cout << "RETRYING TO SEND EMAIL TO " << lead << " [" << (retries - 1) << " RETRIES LEFT]\n";
                retries--;

                if (!retries) {
                    std::cout << "[INFO] CURRENT SMTP [" << CURRENTSMTP << "] IS TROTTLED AND MOVED TO COOLING.\n";
                    tempvector.emplace_back(CURRENTSMTP);
                    POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
                    sleep(1);
                    bool smtploaded = LOADSMTP(
                        SMTPVectorObject.MailDataSetVector,
                        CURRENTSMTP,
                        SMTPAttributeObject.servername,
                        SMTPAttributeObject.port,
                        SMTPAttributeObject.username,
                        SMTPAttributeObject.password, index
                    );

                    if (!smtploaded) {
                        clearScreen();
                        std::cout << "FETCHING ALL SMTPS SENT TO COOLING DOWN.\n";
                        CopyVector(tempvector, SMTPVectorObject.MailDataSetVector);
                        smtploaded = LOADSMTP(
                            SMTPVectorObject.MailDataSetVector,
                            CURRENTSMTP,
                            SMTPAttributeObject.servername,
                            SMTPAttributeObject.port,
                            SMTPAttributeObject.username,
                            SMTPAttributeObject.password,
                            index
                        );
                    }

                    std::cout << "[INFO] NEW SMTP [" << CURRENTSMTP << "] IS LOADED.\nPROGRAM RETRYING SENDING TO " << lead << "\n";
                    SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
                    retries = 5; // Reset retries for the new SMTP
                }
            }
        }

        DelayHandler(sendspeed, userinput, flag);
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    sentLeads.close();
    failedLeads.close();
    deadSMTPs.close();
    limitedSMTPs.close();

    std::vector<std::string> LEADCLEANUP = { SENTLEADSFILE, FAILEDLEADFILE };
    std::vector<std::string> SMTPCLEANUP = { DEADSMTPFILE, LIMITEDSMTP };

    DATACLEANUP("LEADS ", LEADSFILE, LEADCLEANUP);
    DATACLEANUP("SMTP ", SMTPFILES, SMTPCLEANUP);

    LEADCLEANUP.clear();
    SMTPCLEANUP.clear();
    SMTPVectorObject.MailDataSetVector.clear();
    NameVectorObject.MailDataSetVector.clear();
    SubjectVectorObject.MailDataSetVector.clear();

    clearScreen();
    if (sentCount > 0) std::cout << "PROGRAM SUCCESSFULLY SENT EMAIL TO " << sentCount << " LEADS \n";
    return sentCount > 0;
}


bool EmailSenderProgram::VariableEmailSender(bool useHTML, bool useAttachment) {
    clearScreen();
    std::string LEADSFILE = fs::path(EmailDataDirectory) / "leads.txt";
    std::string letterPath = fs::path(EmailDataDirectory) / "letter.txt";
    std::string SENTLEADSFILE = fs::path(EmailDataDirectory) / "sentleads.txt";
    std::string FAILEDLEADFILE = fs::path(EmailJunkDirectory) / "failedleads.txt";
    std::string DEADSMTPFILE = fs::path(EmailJunkDirectory) / "deadsmtps.txt";
    std::string LIMITEDSMTP = fs::path(EmailJunkDirectory) / "smtps_under_limit.txt";
    std::string SMTPFILES = fs::path(EmailDataDirectory) / "smtps.txt";
    std::string SENDERNAMESFILE = fs::path(EmailDataDirectory) / "sendername.txt";
    std::string SUBJECTSFILE = fs::path(EmailDataDirectory) / "subject.txt";
    std::string letter, body, attachmentPath, emailBody, attachmentFileName, sender, subject, CURRENTSMTP;

    int sentCount = 0, rateLimit = 0, rateCount = 0, retries = 0, sendspeed, userinput;
    size_t index = 0;
    bool flag = false;
    long responseCode;

    std::vector<std::string> leads, tempvector;
    std::unordered_map<std::string, int> CoolDownMap;

    std::ofstream
        sentLeads(SENTLEADSFILE, std::ios::app), failedLeads(FAILEDLEADFILE, std::ios::app),
        deadSMTPs(DEADSMTPFILE, std::ios::app), limitedSMTPs(LIMITEDSMTP, std::ios::app);

    if (!sentLeads || !failedLeads || !deadSMTPs || !limitedSMTPs) {
        std::cerr << "Failed to open output files.\n";
        return false;
    }

    // Read data from files into vectors
    if (!ReadFileToVector(SMTPVectorObject.MailDataSetVector, SMTPFILES) ||
        !ReadFileToVector(NameVectorObject.MailDataSetVector, SENDERNAMESFILE) ||
        !ReadFileToVector(SubjectVectorObject.MailDataSetVector, SUBJECTSFILE) ||
        !ReadFileToVector(leads, LEADSFILE)) {
        std::cerr << "FAILED TO READ EMAIL ATTRIBUTES\n";
        return false;
    }

    // Fetch letter content from file
    if (!FetchDataFromFile(letterPath, letter)) {
        std::cerr << "EMAIL LETTER NOT FOUND.\n";
        return false;
    }

    // Load initial SMTP settings
    if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
        SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
        std::cerr << "NO SMTP FOUND INITIALLY.\n";
        return false;
    }

    // Handle attachment if specified
    if (useAttachment) {
        attachmentPath = MakeDirectory(EmailDataDirectory, "Attachment");
        clearScreen();
        std::cout << "Provide Attachment filename (filename.pdf): ";
        std::getline(std::cin >> std::ws, attachmentFileName);
        if (!fs::exists(attachmentFileName)) {
            std::cerr << "Attachment file not found.\n";
            useAttachment = false;
        }
        else {
            attachmentFileName = fs::path(attachmentPath) / attachmentFileName;
        }
    }

    // Check if multiple SMTPs are available
    // If more than one SMTP is available, prompt user for send rate
    if (SMTPVectorObject.MailDataSetVector.size() > 1) {
        clearScreen();
        std::cout << "TOTAL " << SMTPVectorObject.MailDataSetVector.size() << " SMTPS DETECTED.\n"
            << "NOTE: SET SEND RATE TO ROTATE AND EXTEND SMTP LIFE.\n\n"
            << "DO YOU WANT TO SET SEND RATE PER SMTP: (Y/N): ";
        std::string tmp;
        std::getline(std::cin >> std::ws, tmp);
        while (tmp.empty() || (tmp != "Y" && tmp != "y" && tmp != "N" && tmp != "n")) {
            clearScreen();
            std::cin.clear();
            std::cerr << "INVALID RESPONSE, RATE LIMIT NOT SET.\n";
            std::cout << "DO YOU WANT TO SET SEND RATE PER SMTP: (Y/N): ";
            std::getline(std::cin >> std::ws, tmp);
        }

        if (tmp == "Y" || tmp == "y") {
            clearScreen();
            std::cout << "SET SEND RATE VALUE PER SMTP: ";
            if (!(std::cin >> rateLimit) || rateLimit < 0) {
                std::cin.clear();
                std::cerr << "INVALID INPUT, RATE LIMIT NOT SET.\n";
                rateLimit = 0;
            }
        }
        std::cout << "PLEASE HIT THE ENTER KEY ONCE AGAIN.";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    clearScreen();
    PromptSendDelayMessage();

    // Prompt user for send delay
    if (!(std::cin >> userinput) || userinput < 0) {
        do {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            clearScreen();
            std::cerr << "INVALID CHOICE, PLEASE MAKE THE SELECTION AGAIN.\n";
            sleep(1);
            clearScreen();
            PromptSendDelayMessage();
        } while (!(std::cin >> userinput) || userinput < 0);
    }
    else {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Set the flag based on user input for send delay
    flag = (userinput >= 5);
    DelayHandler(sendspeed, userinput, flag);

    // Initialize CURL for sending emails
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "INITIALIZATION FAILED.\nPLEASE CHECK YOUR INTERNET OR VPN/PROXY IF IN USE.\n";
        curl_global_cleanup();
        return false;
    }

    // Set CURL options for sending emails
    SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
    clearScreenWithMessage("\t\t\t[CONSTANT EMAIL SENDER IN PROGRESS]\n");

    // Iterate through each lead and send email
    for (const auto& lead : leads) {
        if (sentCount > 0) sleep(sendspeed);

        std::regex emailRegex(R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)");
        if (!std::regex_match(lead, emailRegex)) {
            failedLeads << lead << '\n';
            continue;
        }

        if (rateLimit > 0 && rateCount >= rateLimit) {
            std::cout << "\n\nSEND RATE LIMIT SET IS REACHED FOR CURRENT SMTP\n[" << CURRENTSMTP << "]\n";
            sleep(2);
            std::cout << "\nLOADING NEW SMTP IN PROGRESS.\n";
            if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
                SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
                CopyVector(tempvector, SMTPVectorObject.MailDataSetVector);
                std::cout << "\nALL SMTP USED UP OR UNAVAILABLE\n"
                    << "LOAD MORE SMTP AND RETRY PROGRAM\n\n";
                break;
            }
            else {
                std::cout << "NEW SMTP [" << CURRENTSMTP << "] IS LOADED.\n";
                std::cout << "RETRYING TO SEND EMAIL TO " << lead << " WITH NEW SMTP.\n\n";
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

        MakeEmailBody(emailBody, useHTML, useAttachment, SMTPAttributeObject.servername, sender, SMTPAttributeObject.username, subject, letter, lead, attachmentFileName);

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
            retries = 5;
            EMAILSUCCESSMESSAGE(sender, subject, lead, sentCount);
        }
        else {
            clearScreen();
            int tempcount = 20;

            std::cout << "============================= ALERT 🔔: URGENT MESSAGE, TACKE ACTION =============================\n\n";
            while (tempcount) {
                system("play -n synth 0.2 sine 880 vol 0.5 > /dev/null 2>&1");
                std::cout << "[INFO] PLEASE CHANGE VPN WITHIN " << tempcount << " SECONDS\n";
                sleep(1);
                clearScreen();
                tempcount--;
                std::cout << "============================= ALERT 🔔: URGENT MESSAGE, TACKE ACTION =============================\n\n";
            }

            retries--;

            if (retries <= 0) {
                clearScreen();
                std::cout << "[INFO] CURRENT SMTP [" << CURRENTSMTP << "] IS TROTTLED AND SENT TO COOLING.\n";
                tempvector.emplace_back(CURRENTSMTP);
                POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
                std::cout << "\n[INFO] PROGRAM LOADING A NEW SMTP TO KEEP PROGRESS.\n";
                sleep(1);
                bool smtploaded = LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index);
                if (!smtploaded) {
                    clearScreen();
                    std::cout << "FETCHING ALL SMTPS SENT TO COOLING DOWN. \n";
                    CopyVector(tempvector, SMTPVectorObject.MailDataSetVector);
                    smtploaded = LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index);
                }
                std::cout << "[INFO] NEW SMTP [" << CURRENTSMTP << "] IS LOADED.\n";
                SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
            }
            continue;
        }
        DelayHandler(sendspeed, userinput, flag);
        curl_slist_free_all(recipients);
    }

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    sentLeads.close();
    failedLeads.close();
    deadSMTPs.close();
    limitedSMTPs.close();

    std::vector<std::string> LEADCLEANUP = { SENTLEADSFILE, FAILEDLEADFILE };
    std::vector<std::string> SMTPCLEANUP = { DEADSMTPFILE, LIMITEDSMTP };

    DATACLEANUP("LEADS ", LEADSFILE, LEADCLEANUP);
    DATACLEANUP("SMTP ", SMTPFILES, SMTPCLEANUP);

    LEADCLEANUP.clear();
    SMTPCLEANUP.clear();
    SMTPVectorObject.MailDataSetVector.clear();
    NameVectorObject.MailDataSetVector.clear();
    SubjectVectorObject.MailDataSetVector.clear();

    clearScreen();
    if (sentCount > 0) std::cout << "PROGRAM SUCCESSFULLY SENT EMAIL TO " << sentCount << " LEADS \n";
    return sentCount > 0;
}

// #: 3 SMTP LIVE TESTER
bool EmailSenderProgram::SMTPLiveTester() {
    int trials = 3, count = 1;
    bool flag = false;
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
                "Big regards,\r\n" + smtp + "\r\n"
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
            flag = true;
            SMTP_RESPONSE_MESSAGE(smtp, flag, count);
            validsmtp.insert(smtp);
            AvailableSMTPFile << smtp << '\n';
            totalvalid++;
        }
        else {
            flag = false;
            SMTP_RESPONSE_MESSAGE(smtp, flag, count);
            invalidsmtp.insert(smtp);
            UnavailableSMTPFile << smtp << '\n';
        }
        count++;
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


bool EmailSenderProgram::EmailExtractor() {
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
    clearScreen();
    char alphaOrderChoice;
    std::cout << "Sort emails alphabetically? (y/n): ";
    std::cin >> alphaOrderChoice;
    std::cout << "\t\t[EMAIL EXTRACTOR && SORTER]\n\n";
    clearScreenWithMessage("PLEASE WAIT, EXTRACTING AND SORTING...");
    std::regex EmailPattern(
        R"(([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}))"
    );

    std::set<std::string> sortedSet;
    std::unordered_set<std::string> unsortedSet;

    bool useSorted = (alphaOrderChoice == 'y' || alphaOrderChoice == 'Y');

    std::string line;
    while (std::getline(RawFileAccess, line)) {
        std::smatch match;
        auto searchStart = line.cbegin();
        while (std::regex_search(searchStart, line.cend(), match, EmailPattern)) {
            (useSorted ? (void)sortedSet.insert(match.str()) : (void)unsortedSet.insert(match.str()));
            searchStart = match.suffix().first;
        }
    }

    if (useSorted) {
        for (const auto& email : sortedSet) {
            ExtractedFileAccess << email << '\n';
        }
        std::cout << sortedSet.size() << " TOTAL EMAILS EXTRACTED\n";
    }
    else {
        for (const auto& email : unsortedSet) {
            ExtractedFileAccess << email << '\n';
        }
        std::cout << unsortedSet.size() << " TOTAL EMAILS EXTRACTED\n";
    }

    return true;
}

bool EmailSenderProgram::DuplicatesRemover() {
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
