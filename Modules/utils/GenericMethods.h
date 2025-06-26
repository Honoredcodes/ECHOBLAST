#ifndef GLOBAL_METHOD_CLASS_H
#define GLOBAL_METHOD_CLASS_H
#include <string>
#include <functional>

class GlobalMethodClass {
public:
    // 1. Directory and File Operations
    std::string MakeDirectory(const std::string& path, const std::string& folderName);
    bool CreateProgramDirectories(std::string BaseProgramDirectory, std::string& ChildProgramDirectory, std::string& ProgramAttributeDirectory, std::string& Junks);
    void CreateProgramAttributeFiles(std::string& ProgramAttributeDirectory, std::string& Junks, std::vector<std::string>& ProgramAttributeVector);
    bool ReadFileToVector(std::vector<std::string>& FileVector, const std::string& filepath);
    void WriteDataToFile(const std::string FilePath, std::string data);
    bool FetchDataFromFile(const std::string FilePath, std::string& VariableStorage);
    void DATACLEANUP(const std::string& sourcePath, const std::vector<std::string>& excludePaths);

    // 2. SMTP and Email Processing
    bool FETCHSMTP(std::vector<std::string>& Vector, std::string& CURRENTSMTP, std::string& servername, int& port, std::string& username, std::string& password);
    bool LOADSMTP(std::vector<std::string>& Vector, std::string& CURRENTSMTP, std::string& servername, int& port, std::string& username, std::string& password, size_t& index);
    void stripdata(std::string& server, std::string& port);
    bool isValidEmail(const std::string& email);
    std::string GetDomain(const std::string& username, const std::string& servername);
    std::string GenerateUUID();
    std::string GenerateMessageID(const std::string& username);
    std::string StripHTML(const std::string& html);
    std::string GetMimeType(const std::string& filepath);
    void MakeEmailBody(std::string& EmailContentBody, bool useHTML, bool useAttachment, const std::string& servername, const std::string& sendername, const std::string& username, const std::string& subject, const std::string& letter, const std::string& lead, const std::string& AttachmentFilePath = "");
    bool InsertVariableEmail(const std::string letter, const std::string& email, std::string& body);
    void EMAILSUCCESSMESSAGE(const std::string sendername, const std::string subject, const std::string lead, const int count);
    void SMTP_RESPONSE_MESSAGE(const std::string smtp, bool flag, const int& count);

    // 3. Data Encoding and Processing
    std::string QuotedPrintableEncode(const std::string& input);
    std::string base64Encode(const std::string& filePath);
    std::string UrlEncode(const std::string& str);

    // 4. Utility Functions
    std::string GetRandomDataFromVector(const std::vector<std::string>& param);
    std::string GetCurrentDateTime();
    void POPDATA(std::vector<std::string>& Vec, std::string data);
    bool ValidDigit(const std::string& param);
    int GenerateDelay();
    void DelayHandler(int& sendspeed, int res, bool flag);

    // 5. Console and User Interaction
    void clearScreen();
    void clearScreenWithMessage(const std::string& message);
    void handleProgramCompletion(bool success, const std::string& programName);
    bool askYesNoQuestion(const std::string& question);
    void PrintTopMiddle(const std::string text);
    void sleep(const int& x);
    void prompt(std::function<int()> main);
    void PromptSendDelayMessage();
};

#endif // GLOBAL_METHOD_CLASS_H