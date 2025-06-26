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
#include <unordered_set>
#include <set>

#include "../utils/GenericMethods.h"
#include "../services/curl_utils.h"

namespace fs = std::filesystem;

// Constructor implementation
EmailSenderProgram::EmailSenderProgram() {
    EmailSenderPrograms = MakeDirectory("", EmailSenderPrograms);
}

// PrepareEmailSenderDirectories implementation
bool EmailSenderProgram::PrepareEmailSenderDirectories() {
    ProgramAttributeVector = { "smtps.txt", "sendername.txt", "subject.txt", "letter.txt", "leads.txt", "sentleads.txt", "failedleads.txt", "deadsmtps.txt", "smtps_under_limit.txt" };
    if (!CreateProgramDirectories(EmailSenderPrograms, EmailProgramDirectory, EmailDataDirectory, EmailJunkDirectory)) return false;
    CreateProgramAttributeFiles(EmailDataDirectory, EmailJunkDirectory, ProgramAttributeVector);
    return true;
}

// PrepareSMTPLiveTesterDirectories implementation
bool EmailSenderProgram::PrepareSMTPLiveTesterDirectories() {
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

// PrepareEmailExtractorDirectories implementation
bool EmailSenderProgram::PrepareEmailExtractorDirectories() {
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

// PrepareDuplicateDirectories implementation
bool EmailSenderProgram::PrepareDuplicateDirectories() {
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
    int sentCount = 0, rateLimit = 0, rateCount = 0, retries = 0, sendspeed, userinput;
    size_t index = 0;
    bool flag = false;
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
            << "DO YOU WANT TO SET SEND RATE PER SMTP: (Y/N): ";
        std::string tmp;
        std::getline(std::cin >> std::ws, tmp);
        if (tmp.empty() || (tmp != "Y" && tmp != "y" && tmp != "N" && tmp != "n")) std::cerr << "INVALID RESPONSE, RATE LIMIT NOT SET.\n";
        if (tmp == "Y" || tmp == "y") {
            std::cout << "SET SEND RATE VALUE PER SMTP: ";
            if (!(std::cin >> rateLimit) || rateLimit < 0) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cerr << "INVALID INPUT, RATE LIMIT NOT SET.\n";
                rateLimit = 0;
            }
        }
        std::cout << "PLEASE HIT THE ENTER KEY ONCE AGAIN.";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    clearScreen();
    PromptSendDelayMessage();

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

    flag = (userinput >= 5);
    DelayHandler(sendspeed, userinput, flag);


    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "INITIALIZATION FAILED.\nPLEASE CHECK YOUR INTERNET OR VPN/PROXY IF IN USE.\n";
        curl_global_cleanup();
        return false;
    }

    SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
    clearScreenWithMessage("\t\t\t[CONSTANT EMAIL SENDER IN PROGRESS]\n");

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
            retries = 0;
            EMAILSUCCESSMESSAGE(sender, subject, lead, sentCount);
        }
        else {
            if (res == CURLE_LOGIN_DENIED || responseCode == 535 || responseCode == 530 || responseCode == 534) {
                std::cout << "\n\nCURRENT SMTP [" << CURRENTSMTP << "] IS DEAD.\n";
                deadSMTPs << CURRENTSMTP << '\n';
                POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
                std::cout << "\nLOADING NEW SMTP IN PROGRESS.\n";
                sleep(1);
                if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
                    SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
                    std::cout << "ALL SMTPS ARE USED UP OR UNAVAILABLE.\n"
                        << "LOAD MORE SMTP AND RETRY PROGRAM.\n\n";
                    break;
                }
                else {
                    std::cout << "NEW SMTP [" << CURRENTSMTP << "] IS LOADED.\n";
                    std::cout << "RETRYING TO SEND EMAIL TO " << lead << " WITH NEW SMTP.\n\n";
                    SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
                        SMTPAttributeObject.username, SMTPAttributeObject.password);
                }
            }
            else if ((res == CURLE_RECV_ERROR || res == CURLE_SEND_ERROR) &&
                (responseCode == 452 || responseCode == 421 || responseCode == 451 || responseCode == 550)) {
                if (retries >= 5) {
                    std::cout << "\n\nCURRENT SMTP [" << CURRENTSMTP << "] HITS LIMIT\n";
                    limitedSMTPs << CURRENTSMTP << '\n';
                    POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
                    std::cout << "\nLOADING NEW SMTP IN PROGRESS.\n";
                    sleep(1);
                    if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
                        std::cout
                            << "\nALL SMTP USED UP OR UNAVAILABLE\n"
                            << "LOAD MORE SMTP AND RETRY PROGRAM\n\n";
                        break;
                    }
                    else {
                        std::cout
                            << "NEW SMTP [" << CURRENTSMTP << "] IS LOADED.\n"
                            << "RETRYING TO SEND EMAIL TO " << lead << " WITH NEW SMTP.\n\n";
                        SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
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
                SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
            }
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

    DATACLEANUP(LEADSFILE, LEADCLEANUP);
    DATACLEANUP(SMTPFILES, SMTPCLEANUP);

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
    bool flag = false;
    size_t index = 0;
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
            << "DO YOU WANT TO SET SEND RATE PER SMTP: (Y/N): ";
        std::string tmp;
        std::getline(std::cin >> std::ws, tmp);
        if (tmp.empty() || (tmp != "Y" && tmp != "y" && tmp != "N" && tmp != "n")) std::cerr << "INVALID RESPONSE, RATE LIMIT NOT SET.\n";
        if (tmp == "Y" || tmp == "y") {
            std::cout << "SET SEND RATE VALUE PER SMTP: ";
            if (!(std::cin >> rateLimit) || rateLimit < 0) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cerr << "INVALID INPUT, RATE LIMIT NOT SET.\n";
                rateLimit = 0;
            }
        }
        std::cout << "PLEASE HIT THE ENTER KEY ONCE AGAIN.";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    clearScreen();
    PromptSendDelayMessage();

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
        std::cout << "PLEASE HIT THE ENTER KEY ONCE AGAIN.";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    flag = (userinput >= 5);
    DelayHandler(sendspeed, userinput, flag);


    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "INITIALIZATION FAILED.\nPLEASE CHECK YOUR INTERNET OR VPN/PROXY IF IN USE.\n";
        curl_global_cleanup();
        return false;
    }

    SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
    clearScreenWithMessage("\t\t\t[CONSTANT EMAIL SENDER IN PROGRESS]\n");

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
            retries = 0;
            EMAILSUCCESSMESSAGE(sender, subject, lead, sentCount);
        }
        else {
            if (res == CURLE_LOGIN_DENIED || responseCode == 535 || responseCode == 530 || responseCode == 534) {
                std::cout << "\n\nCURRENT SMTP [" << CURRENTSMTP << "] IS DEAD.\n";
                deadSMTPs << CURRENTSMTP << '\n';
                POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
                std::cout << "\nLOADING NEW SMTP IN PROGRESS.\n";
                sleep(1);
                if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
                    SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
                    std::cout << "ALL SMTPS ARE USED UP OR UNAVAILABLE.\n"
                        << "LOAD MORE SMTP AND RETRY PROGRAM.\n\n";
                    break;
                }
                else {
                    std::cout << "NEW SMTP [" << CURRENTSMTP << "] IS LOADED.\n";
                    std::cout << "RETRYING TO SEND EMAIL TO " << lead << " WITH NEW SMTP.\n\n";
                    SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
                        SMTPAttributeObject.username, SMTPAttributeObject.password);
                }
            }
            else if ((res == CURLE_RECV_ERROR || res == CURLE_SEND_ERROR) &&
                (responseCode == 452 || responseCode == 421 || responseCode == 451 || responseCode == 550)) {
                if (retries >= 5) {
                    std::cout << "\n\nCURRENT SMTP [" << CURRENTSMTP << "] HITS LIMIT\n";
                    limitedSMTPs << CURRENTSMTP << '\n';
                    POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
                    std::cout << "\nLOADING NEW SMTP IN PROGRESS.\n";
                    sleep(1);
                    if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
                        std::cout
                            << "\nALL SMTP USED UP OR UNAVAILABLE\n"
                            << "LOAD MORE SMTP AND RETRY PROGRAM\n\n";
                        break;
                    }
                    else {
                        std::cout
                            << "NEW SMTP [" << CURRENTSMTP << "] IS LOADED.\n"
                            << "RETRYING TO SEND EMAIL TO " << lead << " WITH NEW SMTP.\n\n";
                        SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
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
                SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password);
            }
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

    DATACLEANUP(LEADSFILE, LEADCLEANUP);
    DATACLEANUP(SMTPFILES, SMTPCLEANUP);

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
                "Best regards,\r\n" + smtp + "\r\n"
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
