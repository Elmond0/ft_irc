
#include "../inc/Parser.hpp"
#include <cctype>

static std::string toUpper(const std::string& s)
{
    std::string result = s;
    for (std::size_t i = 0; i < result.size(); ++i)
        result[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[i])));
    return result;
}

static std::string nextToken(const std::string& line, std::size_t& pos)
{
    while (pos < line.size() && line[pos] == ' ')
        ++pos;
    if (pos >= line.size())
        return "";
    std::size_t start = pos;
    while (pos < line.size() && line[pos] != ' ')
        ++pos;
    return line.substr(start, pos - start);
}

IrcMessage parseMessage(const std::string& raw)
{
    IrcMessage msg;

    std::string line = raw;
    while (!line.empty() && (line[line.size() - 1] == '\r' || line[line.size() - 1] == '\n'))
        line.erase(line.size() - 1);

    if (line.empty())
        return msg;

    std::size_t pos = 0;

    if (line[0] == ':')
    {
        std::size_t spacePos = line.find(' ');
        if (spacePos == std::string::npos)
        {
            msg.prefix = line.substr(1);
            return msg;
        }
        msg.prefix = line.substr(1, spacePos - 1);
        pos = spacePos + 1;
    }

    std::string cmd = nextToken(line, pos);
    if (cmd.empty())
        return msg;
    msg.command = toUpper(cmd);

    while (pos < line.size())
    {
        while (pos < line.size() && line[pos] == ' ')
            ++pos;
        if (pos >= line.size())
            break;

        if (line[pos] == ':')
        {
            /* tutto il resto (spazi inclusi) e' UN solo parametro: il trailing */
            msg.trailing = line.substr(pos + 1);
            break;
        }

        std::string token = nextToken(line, pos);
        if (!token.empty())
            msg.params.push_back(token);
    }

    return msg;
}
