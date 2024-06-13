#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <map>

// Function to trim whitespace from both ends of a string
std::string trim(const std::string& str) {
    const auto strBegin = str.find_first_not_of(" \t\n\r");
    if (strBegin == std::string::npos)
        return ""; // No content

    const auto strEnd = str.find_last_not_of(" \t\n\r");
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

// Function to parse the content
std::vector<std::map<std::string, std::map<std::string, std::vector<std::string>>>> parseContent(const std::string& content) {
    std::regex serverRegex(R"(server\s*{(.*?)}\s*)", std::regex::dotall);
    std::regex mainRegex(R"(main\s*{(.*?)}\s*)", std::regex::dotall);
    std::regex locationRegex(R"(location\s+(\S+)\s*{(.*?)}\s*)", std::regex::dotall);
    std::regex keyValueRegex(R"((\w+)\s+([^;]+);)");

    std::vector<std::map<std::string, std::map<std::string, std::vector<std::string>>>> servers;

    auto server_begin = std::sregex_iterator(content.begin(), content.end(), serverRegex);
    auto server_end = std::sregex_iterator();

    for (std::sregex_iterator i = server_begin; i != server_end; ++i) {
        std::smatch serverMatch = *i;
        std::string serverBlock = serverMatch[1].str();
        std::map<std::string, std::map<std::string, std::vector<std::string>>> serverData;

        // Parse the main block
        std::smatch mainMatch;
        if (std::regex_search(serverBlock, mainMatch, mainRegex)) {
            std::string mainBlock = mainMatch[1].str();
            std::map<std::string, std::vector<std::string>> mainData;

            auto keyValue_begin = std::sregex_iterator(mainBlock.begin(), mainBlock.end(), keyValueRegex);
            auto keyValue_end = std::sregex_iterator();

            for (std::sregex_iterator j = keyValue_begin; j != keyValue_end; ++j) {
                std::smatch keyValueMatch = *j;
                std::string key = keyValueMatch[1].str();
                std::string valueStr = keyValueMatch[2].str();
                std::vector<std::string> values;
                std::istringstream valueStream(valueStr);
                std::string value;
                while (valueStream >> value) {
                    values.push_back(value);
                }
                mainData[key] = values;
            }
            serverData["main"] = mainData;
        }

        // Parse the location blocks
        auto location_begin = std::sregex_iterator(serverBlock.begin(), serverBlock.end(), locationRegex);
        auto location_end = std::sregex_iterator();

        for (std::sregex_iterator j = location_begin; j != location_end; ++j) {
            std::smatch locationMatch = *j;
            std::string locationPath = locationMatch[1].str();
            std::string locationBlock = locationMatch[2].str();
            std::map<std::string, std::vector<std::string>> locationData;
            locationData["path"].push_back(locationPath);

            auto keyValue_begin = std::sregex_iterator(locationBlock.begin(), locationBlock.end(), keyValueRegex);
            auto keyValue_end = std::sregex_iterator();

            for (std::sregex_iterator k = keyValue_begin; k != keyValue_end; ++k) {
                std::smatch keyValueMatch = *k;
                std::string key = keyValueMatch[1].str();
                std::string valueStr = keyValueMatch[2].str();
                std::vector<std::string> values;
                std::istringstream valueStream(valueStr);
                std::string value;
                while (valueStream >> value) {
                    values.push_back(value);
                }
                locationData[key] = values;
            }
            serverData["location"].insert({ locationPath, locationData });
        }
        servers.push_back(serverData);
    }

    return servers;
}

int main() {
    std::string content = R"(
server
{
    main  # this part will collect general setting of server
    {
        key1  value0;    # the key1 is keywork  
        key2  value0 value1;
        key3  value0;
        
    }
    location reletive_location
    {
        key3  value0;
        key4  value0 value1;
        key5  value0;
    }
    location reletive_location
    {
        key3  value0;
        key4  value0 value1;
        key5  value0;
    }
}
server
{
    main  # this part will collect general setting of server
    {
        key1  value0;    # the key1 is keywork  
        key2  value0 value1;
        key3  value0;
        
    }
    location reletive_location
    {
        key3  value0;
        key4  value0 value1;
        key5  value0;
    }
    location reletive_location
    {
        key3  value0;
        key4  value0 value1;
        key5  value0;
    }
}
)";

    auto parsedData = parseContent(content);

    for (const auto& server : parsedData) {
        std::cout << "Server:" << std::endl;
        for (const auto& main : server.at("main")) {
            std::cout << "  Main:" << std::endl;
            std::cout << "    " << main.first << ":";
            for (const auto& val : main.second) {
                std::cout << " " << val;
            }
            std::cout << std::endl;
        }
        for (const auto& loc : server.at("location")) {
            std::cout << "  Location " << loc.first << ":" << std::endl;
            for (const auto& kv : loc.second) {
                std::cout << "    " << kv.first << ":";
                for (const auto& val : kv.second) {
                    std::cout << " " << val;
                }
                std::cout << std::endl;
            }
        }
    }

    return 0;
}

