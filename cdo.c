#include <iostream>
#include <string>
#include <curl/curl.h>
#include <regex>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <stdexcept>
#include <vector>
#include <ctime>

// TODOLIST:
// TODO: better err handling
// TODO: cmd validation, we validate nothing, nothing is valid
// TODO: this is rough and dirty. there is zero async req handling
// TODO: deal /w security and execution
// TODO: potential memory leaks, need to handle them
// TODO: better input handling than the basic regex mess
// TODO: move all quotes and language strings to a file
// TODO: there is no system prompt
// TODO: need to have system prompt be a part of the config file
// TODO: system prompt should tell GPT to wrap cmd
//          -cmd-/:cmd:/@cmd@/*cmd* dunno, char needs to be non volatile to shellcmd
//          this way, each can be parsed, put into arr, and multiple
//          lines can be sent in 1 request, then iter on each cmd
// TODO: user needs to be able to run in free access mode/intervention
//          intervention: require each returned list of cmds to be y/n
//          free access: "It's turbo time!" Guardrails off, good luck.
// TODO: need to specify to LLM/GPT which OS it is running on. Win/Mac/Linux(distro)/BSD
// TODO: add ability to use local/self hosted LLM trained on terminal cmds

// rando commando quote
std::string getRandomQuote() {
    std::vector<std::string> quotes = {
        "Let off some steam Bennett.",
        "I eat Green Berets for breakfast and right now, I'm very hungry!",
        "Remember Sully, when I promised to kill you last? I lied.",
        "Don't disturb my friend. He's dead tired.",
        "You're a funny guy, Sully. I like you. That's why I'm going to kill you last."
    };

    // cur_time for rand
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    int randomIndex = std::rand() % quotes.size();
    return quotes[randomIndex];
}

// check input
bool isInputValid(const std::string& input) {
    // only alpha and special
    std::regex pattern("^[a-zA-Z0-9_\\-./\\s]*$");
    return std::regex_match(input, pattern);
}

// executes cmd
void executeCommand(const std::string& command) {
    std::cout << "Executing: " << command << std::endl;
    int exitCode = std::system(command.c_str());
    if (exitCode != 0) {
        std::cerr << "Cmd exec failed. " << exitCode << std::endl;
    }
    std::cout << std::endl;
}

// read config
bool readConfig(const std::string& filePath, std::string& apiKey, std::string& apiUrl, std::string& caCert, std::string& logFilePath) {
    std::ifstream file(filePath);
    if (!file) {
        std::cerr << "unable to open: " << filePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        // skip the blank lines and lines commented out /w #
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line.find("apiKey=") == 0) {
            apiKey = line.substr(7);
        } else if (line.find("apiUrl=") == 0) {
            apiUrl = line.substr(7);
        } else if (line.find("caCert=") == 0) {
            caCert = line.substr(7);
        } else if (line.find("logFile=") == 0) {
            logFilePath = line.substr(8);
        }
    }

    file.close();

    // set default log path if not provided
    if (logFilePath.empty()) {
        logFilePath = "/var/log/commando.log";
    }

    return true;
}

// writes the log messages
void logMessage(const std::string& logFilePath, const std::string& message) {
    std::ofstream logFile(logFilePath, std::ios_base::app);
    if (!logFile) {
        std::cerr << "Failed to open  log: " << logFilePath << std::endl;
        return;
    }
    logFile << message << std::endl;
}

// send cmd to api and get the response
std::string getApiResponse(const std::string& command, const std::string& apiKey, const std::string& apiUrl, const std::string& caCert, const std::string& logFilePath) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL init failed." << std::endl;
        logMessage(logFilePath, "CURL init fail.");
        return "";
    }

    // curl opts
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, ("prompt=" + command).c_str());
    curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
    curl_easy_setopt(curl, CURLOPT_CAINFO, caCert.c_str());

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](char* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append(ptr, size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // do network request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "Failed to execute the network request: " << curl_easy_strerror(res) << std::endl;
        logMessage(logFilePath, "Failed to execute the network request: " + std::string(curl_easy_strerror(res)));
        // clean up and close
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        // TODO: should throw an err or handle exception
        throw std::runtime_error("Network request failed.");
    }

    // clean up and close
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return response;
}

// let out some steam bennet
int main(int argc, char* argv[]) {
    std::string randomQuote = getRandomQuote();
    std::string input;

    std::string apiKey;
    std::string apiUrl;
    std::string caCert;
    std::string logFilePath;

    // parse command line option
    int opt;
    std::string configFilePath;

    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
            case 'c':
                configFilePath = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -c <config_file_path>" << std::endl;
                return 1;
        }
    }

    // check if we have config file provided
    if (configFilePath.empty()) {
        std::cerr << "Usage: " << argv[0] << " -c <config_file_path>" << std::endl;
        return 1;
    }

    // read config from specified file
    if (readConfig(configFilePath, apiKey, apiUrl, caCert, logFilePath)) {
        while (true) {
            std::cout << "cdo: ";
            if (!std::getline(std::cin, input)) {
                std::cout << "I'll be back." << std::endl;
                break;
            }

            if (input == "exit") {
                std::cout << "I'll be back Bennett!" << std::endl;
                break;
            } else if (input.empty()) {
                continue; // empty, skip to next
            } else if (input == "RUN!" || input == "GO!") {
                std::cout << "GET TO THE CHOPPER!" << std::endl;
                continue;
            }

            // validate input
            if (!isInputValid(input)) {
                std::cerr << "Invalid input. Please use only a-z 0-9 and special char." << std::endl;
                logMessage(logFilePath, "Invalid characters: " + input);
                continue;
            }

            try {
                // send input to api and read response
                std::string apiResponse = getApiResponse(input, apiKey, apiUrl, caCert, logFilePath);

                // validate and sanitize the response
                if (apiResponse.empty()) {
                    std::cerr << "API return empty." << std::endl;
                    logMessage(logFilePath, "API returned empty response for: " + input);
                    continue;
                }

                // execute returned cmd
                executeCommand(apiResponse);
            } catch (const std::exception& e) {
                std::cerr << "Error occurred: " << e.what() << std::endl;
                logMessage(logFilePath, "Error occurred: " + std::string(e.what()));
            }
        }
    } else {
        std::cerr << "Failed to read API config " << configFilePath << ". Exiting..." << std::endl;
        logMessage(logFilePath, "Failed to read API config " + configFilePath);
        return 1;
    }

    return 0;
}