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
        std::cerr << "Failed to load input files.\n";
        return false;
    }
    if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
        SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
        std::cerr << "No valid SMTP found.\n";
        return false;
    }
    if (useAttachment) {
        attachmentPath = MakeDirectory(EmailDataDirectory, "Attachment");
        clearScreen();
        std::cout << "Provide Attachment filename (e.g., file.pdf): ";
        std::cin >> attachmentFileName;
        attachmentFileName = fs::path(attachmentPath) / attachmentFileName;
    }
    if (SMTPVectorObject.MailDataSetVector.size() > 1) {
        clearScreen();
        std::cout << SMTPVectorObject.MailDataSetVector.size() << " SMTPS FOUND.\n"
            << "Set rate limit (Y/N): ";
        std::string tmp;
        std::cin >> tmp;
        if (tmp == "Y" || tmp == "y") {
            std::cout << "Specify send rate limit: ";
            std::cin >> rateLimit;
        }
    }
    clearScreen();
    std::cout << "[SEND SPEED LIMIT PRESERVES SMTP LIFE]\n"
        << "Set speed delay limit (default: 2): ";
    std::cin >> sendspeed;
    sendspeed = std::max(sendspeed, 2);
    std::string letter;
    if (!FetchDataFromFile(letterPath, letter)) {
        std::cerr << "Failed to load email template.\n";
        return false;
    }
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL initialization failed.\n";
        curl_global_cleanup();
        return false;
    }

    SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
        SMTPAttributeObject.username, SMTPAttributeObject.password);
    clearScreenWithMessage("\t\t\t[EMAIL SENDER INITIALIZED]\n");
    for (const auto& lead : leads) {
        if (sentCount > 0) sleep(sendspeed);
        if (rateLimit > 0 && rateCount >= rateLimit) {
            if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
                SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
                std::cout
                    << "ALL SMTP USED UP OR UNAVAILABLE\n"
                    << "LOAD MORE SMTP AND RETRY PROGRAM\n";
                break;
            }
            SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
                SMTPAttributeObject.username, SMTPAttributeObject.password);
            rateCount = 0;
        }
        sender = GetRandomDataFromVector(NameVectorObject.MailDataSetVector);
        subject = GetRandomDataFromVector(SubjectVectorObject.MailDataSetVector);
        if (!InsertVariableEmail(letter, lead, body)) {
            std::cerr
                << "EMAIL VARIABLE NOT FOUND.\n"
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
                cout << "\n\nCURRENT SMTP [" << SMTPAttributeObject.username << "] IS DEAD\n";
                deadSMTPs << CURRENTSMTP << '\n';
                POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
                sleep(2);
                cout << "LOADING NEW SMTP IN PROGRESS.\n";
                if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
                    SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
                    std::cout
                        << "ALL SMTPS ARE USED UP OR UNAVAILABLE\n"
                        << "LOAD MORE SMTP AND RETRY PROGRAM\n";
                    break;
                }
            }
            else if ((res == CURLE_RECV_ERROR || res == CURLE_SEND_ERROR) &&
                (responseCode == 452 || responseCode == 421 || responseCode == 451 || responseCode == 550)) {
                if (retries >= 5) {
                    limitedSMTPs << CURRENTSMTP << '\n';
                    std::cout << "SMTP [" << SMTPAttributeObject.username << "] IS UNDER LIMIT\n";
                    POPDATA(SMTPVectorObject.MailDataSetVector, CURRENTSMTP);
                    sleep(2);
                    std::cout << "LOADING NEW SMTP IN PROGRESS.\n";
                    if (!LOADSMTP(SMTPVectorObject.MailDataSetVector, CURRENTSMTP, SMTPAttributeObject.servername,
                        SMTPAttributeObject.port, SMTPAttributeObject.username, SMTPAttributeObject.password, index)) {
                        std::cout
                            << "ALL SMTP USED UP OR UNAVAILABLE\n"
                            << "LOAD MORE SMTP AND RETRY PROGRAM\n";
                        break;
                    }
                    retries = 0;
                }
                else {
                    failedLeads << lead << '\n';
                    retries++;
                    continue;
                }
            }
            SetCurlForMail(curl, SMTPAttributeObject.servername, SMTPAttributeObject.port,
                SMTPAttributeObject.username, SMTPAttributeObject.password);
        }
        curl_slist_free_all(recipients);
        recipients = nullptr;
    }

    // Cleanup
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    sentLeads.close();
    failedLeads.close();
    deadSMTPs.close();
    limitedSMTPs.close();
    DATACLEANUP(LEADSFILE, SENTLEADSFILE, FAILEDLEADFILE);
    DATACLEANUP(SMTPFILES, LIMITEDSMTP, DEADSMTPFILE);

    clearScreen();
    std::cout << "TOTAL SUCCESSFULLY SENT EMAIL TO " << sentCount << " LEADS \n";
    return sentCount > 0;
}