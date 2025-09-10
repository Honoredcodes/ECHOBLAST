#ifndef EMAIL_SENDER_PROGRAM_H
#define EMAIL_SENDER_PROGRAM_H

#include <string>
#include <vector>
#include <filesystem>
#include "GenericMethods.h"

namespace fs = std::filesystem;

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
    std::string ParentEmailSenderDirectory = "Main";

    EmailSenderProgram();
    ~EmailSenderProgram() = default;

    // EMAIL SENDER GLOBAL VARIABLES
    std::string EmailProgramDirectory = "Email Sender";
    std::string EmailDataDirectory = "Data";
    std::string EmailJunkDirectory = "Trash";
    std::vector<std::string> ProgramAttributeVector;

    // SMTP LIVE TESTER GLOBAL VARIABLES
    std::string SMTPLiveTesterDirectory = "Smtp Tester";
    std::string EmailExtractorDirectory = "Email Extractor";
    std::string DuplicateRemoval = "Duplicate Remover";


    bool PrepareEmailSenderDirectories(const std::string type);
    bool PrepareSMTPLiveTesterDirectories();
    bool PrepareEmailExtractorDirectories();
    bool PrepareDuplicateDirectories();

    bool EmailSender(bool useHTML, bool useAttachment);
    bool VariableEmailSender(bool useHTML, bool useAttachment);
    bool SMTPLiveTester();
    bool EmailExtractor();
    bool DuplicatesRemover();
};

#endif 