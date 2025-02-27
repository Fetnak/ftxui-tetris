#include "t_config/t_yaml_parser.hpp"

#include <string>

namespace Tetris::Config
{
template <typename T>
void YAMLParser::decodeOptionalScalar(
    const YAML::Node &node, const std::string &name, T &value, std::function<bool(T)> validate
)
{
    if (!node[name].IsDefined() || !node[name].IsScalar() || !validate(node[name].as<T>()))
    {
        return;
    }

    value = node[name].as<T>();
}

template <typename T>
void YAMLParser::decodeOptionalVector(
    const YAML::Node &node, const std::string &name, std::vector<T> &value, std::function<bool(T)> validate
)
{
    if (!node[name].IsDefined() || !node[name].IsSequence() || !node[name].size())
    {
        return;
    }

    value.clear();

    for (YAML::const_iterator it = node[name].begin(); it != node[name].end(); ++it)
    {
        if (!validate(it->as<T>()))
        {
            continue;
        }

        value.push_back(it->as<T>());
    }
}

void YAMLParser::decodeOptionalString(const YAML::Node &node, const std::string &name, std::string &value)
{
    if (!node[name].IsDefined() || node[name].IsNull() || node[name].as<std::string>().empty())
    {
        return;
    }

    value = node[name].as<std::string>();
}

void YAMLParser::loadData(Config &config, Controls &controls)
{
    std::fstream fs;
    fs.open(YAMLParser::fileName, std::ios::out | std::ios::app);
    fs.close();

    YAML::Node data = YAML::LoadFile(YAMLParser::fileName);

    config   = data["config"].as<Config>();
    controls = data["controls"].as<Controls>();
}

void YAMLParser::saveData(const Config &config, const Controls &controls)
{
    YAML::Node data;

    data["config"]   = config;
    data["controls"] = controls;

    std::ofstream fout(YAMLParser::fileName);
    fout << data;
}
} // namespace Tetris::Config

namespace YAML
{

YAML::Node convert<Config>::encode(const Config &config)
{
    YAML::Node node;

    node["easyMode"]                = (bool)config.isEasyMode;
    node["level"]                   = config.level;
    node["updatesPerSecond"]        = config.updatesPerSecond;
    node["currentTheme"]            = config.themes.size() ? config.themes[config.currentTheme].name : "Default";
    node["forceBackgroundColor"]    = (bool)config.withBackgroundColor;
    node["storeDelayMs"]            = config.storeDelay;
    node["softDropDelayMs"]         = config.softDropDelay;
    node["softDropGravityMsPerRow"] = config.softDropGravity;
    node["comboDelayMs"]            = config.comboDelay;
    node["debug"]                   = (bool)config.isDebug;
    node["themes"]                  = config.themes;

    return node;
}

bool convert<Config>::decode(const YAML::Node &node, Config &config)
{
    if (!node.IsMap())
    {
        return true;
    }
    std::function<bool(int)>    basicIntValidator         = [](int value) -> bool { return value > 0; };
    std::function<bool(double)> basicDoubleValidator      = [](double value) -> bool { return value > 0; };
    std::function<bool(int)>    levelValidator            = [](int value) -> bool { return value > 0 && value <= 15; };
    std::function<bool(double)> updatesPerSecondValidator = [](double value) -> bool { return value > 1; };

    YAMLParser::decodeOptionalScalar(node, "debug", config.isDebug);
    YAMLParser::decodeOptionalScalar(node, "easyMode", config.isEasyMode);
    YAMLParser::decodeOptionalScalar(node, "forceBackgroundColor", config.withBackgroundColor);
    YAMLParser::decodeOptionalScalar(node, "storeDelayMs", config.storeDelay, basicIntValidator);
    YAMLParser::decodeOptionalScalar(node, "softDropDelayMs", config.softDropDelay, basicIntValidator);
    YAMLParser::decodeOptionalScalar(node, "comboDelayMs", config.comboDelay, basicIntValidator);
    YAMLParser::decodeOptionalScalar(node, "updatesPerSecond", config.updatesPerSecond, updatesPerSecondValidator);
    YAMLParser::decodeOptionalScalar(node, "softDropGravityMsPerRow", config.softDropGravity, basicDoubleValidator);
    YAMLParser::decodeOptionalScalar(node, "level", config.level, levelValidator);

    config.themes = node["themes"].as<std::vector<Theme>>();

    std::string currentTheme;

    YAMLParser::decodeOptionalString(node, "currentTheme", currentTheme);

    std::transform(currentTheme.begin(), currentTheme.end(), currentTheme.begin(), ::tolower);

    for (int i = 0; i < (int)config.themes.size(); i++)
    {
        std::string themeName = config.themes[i].name;

        std::transform(themeName.begin(), themeName.end(), themeName.begin(), ::tolower);

        if (currentTheme == themeName)
        {
            config.currentTheme = i;

            break;
        }
    }

    config.applyTheme();

    return true;
}

YAML::Node convert<Controls>::encode(const Controls &controls)
{
    YAML::Node node;

    node["moveLeft"]    = controls.moveLeft;
    node["moveRight"]   = controls.moveRight;
    node["rotateLeft"]  = controls.rotateLeft;
    node["rotateRight"] = controls.rotateRight;
    node["swapHold"]    = controls.swapHold;
    node["softDrop"]    = controls.softDrop;
    node["hardDrop"]    = controls.hardDrop;
    node["forfeit"]     = controls.forfeit;
    node["retry"]       = controls.retry;

    return node;
}

YAML::Node convert<ftxui::Color>::encode(const ftxui::Color &color)
{
    YAML::Node node;

    node = Tetris::Renderer::DataTransformer::toString(color);

    return node;
}

bool convert<ftxui::Color>::decode(const YAML::Node &node, ftxui::Color &color)
{
    if (node.IsMap() && node["r"] && node["r"].IsScalar() && node["g"] && node["g"].IsScalar() && node["b"]
        && node["b"].IsScalar())
    {
        color = ftxui::Color::RGB(node["r"].as<int>(), node["g"].as<int>(), node["b"].as<int>());

        return true;
    }

    if (!node.IsScalar())
    {
        return false;
    }

    std::string value = node.as<std::string>();

    std::regex rgbPattern(
        "\\s*rgb\\((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|\\d{1,2})\\s*,\\s*(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|\\d{1,2})\\s*,"
        "\\s*(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|\\d{1,2})\\)\\s*",
        std::regex_constants::icase
    );

    std::regex hexPattern("\\s*#([A-Fa-f0-9]{2})([A-Fa-f0-9]{2})([A-Fa-f0-9]{2})\\s*");

    std::smatch match;

    if (std::regex_match(value, match, rgbPattern))
    {
        color = ftxui::Color::RGB(std::stoi(match[1].str()), std::stoi(match[2].str()), std::stoi(match[3].str()));

        return true;
    }
    else if (std::regex_match(value, match, hexPattern))
    {
        color = ftxui::Color::RGB(
            std::stoi(match[1].str(), nullptr, 16),
            std::stoi(match[2].str(), nullptr, 16),
            std::stoi(match[3].str(), nullptr, 16)
        );

        return true;
    }

    return false;
}

bool convert<Controls>::decode(const YAML::Node &node, Controls &controls)
{
    if (!node.IsMap())
    {
        return true;
    }

    std::function<bool(std::string)> basicStringValidator = [](std::string value) -> bool { return !value.empty(); };

    YAMLParser::decodeOptionalString(node, "moveLeft", controls.moveLeft);
    YAMLParser::decodeOptionalString(node, "moveRight", controls.moveRight);
    YAMLParser::decodeOptionalString(node, "rotateLeft", controls.rotateLeft);
    YAMLParser::decodeOptionalString(node, "rotateRight", controls.rotateRight);
    YAMLParser::decodeOptionalString(node, "swapHold", controls.swapHold);
    YAMLParser::decodeOptionalString(node, "softDrop", controls.softDrop);
    YAMLParser::decodeOptionalString(node, "hardDrop", controls.hardDrop);
    YAMLParser::decodeOptionalString(node, "forfeit", controls.forfeit);
    YAMLParser::decodeOptionalString(node, "retry", controls.retry);

    return true;
}

YAML::Node convert<Theme>::encode(const Theme &theme)
{
    YAML::Node node;

    node["name"] = theme.name;

    for (int i = 0; i < (int)theme.mainColors.size(); i++)
    {
        node["mainColorsGradient"].push_back(theme.mainColors[i]);
    }

    for (int i = 0; i < (int)theme.gameOverColors.size(); i++)
    {
        node["gameOverColorsGradient"].push_back(theme.gameOverColors[i]);
    }

    node["mainColor"]       = theme.mainColor;
    node["gameOverColor"]   = theme.gameOverColor;
    node["backgroundColor"] = theme.backgroundColor;

    node["valueColor"] = theme.valueColor;
    node["trueColor"]  = theme.trueColor;
    node["falseColor"] = theme.falseColor;

    node["IColor"] = theme.IColor;
    node["OColor"] = theme.OColor;
    node["TColor"] = theme.TColor;
    node["JColor"] = theme.JColor;
    node["LColor"] = theme.LColor;
    node["SColor"] = theme.SColor;
    node["ZColor"] = theme.ZColor;

    return node;
}

bool convert<Theme>::decode(const YAML::Node &node, Theme &theme)
{
    if (!node.IsMap())
    {
        return true;
    }

    std::function<bool(std::string)> basicStringValidator = [](std::string value) -> bool { return !value.empty(); };

    YAMLParser::decodeOptionalString(node, "name", theme.name);

    YAMLParser::decodeOptionalVector(node, "mainColorsGradient", theme.mainColors);
    YAMLParser::decodeOptionalVector(node, "gameOverColorsGradient", theme.gameOverColors);

    YAMLParser::decodeOptionalScalar(node, "mainColor", theme.mainColor);
    YAMLParser::decodeOptionalScalar(node, "backgroundColor", theme.backgroundColor);
    YAMLParser::decodeOptionalScalar(node, "gameOverColor", theme.gameOverColor);
    YAMLParser::decodeOptionalScalar(node, "valueColor", theme.valueColor);
    YAMLParser::decodeOptionalScalar(node, "trueColor", theme.trueColor);
    YAMLParser::decodeOptionalScalar(node, "falseColor", theme.falseColor);
    YAMLParser::decodeOptionalScalar(node, "IColor", theme.IColor);
    YAMLParser::decodeOptionalScalar(node, "OColor", theme.OColor);
    YAMLParser::decodeOptionalScalar(node, "TColor", theme.TColor);
    YAMLParser::decodeOptionalScalar(node, "JColor", theme.JColor);
    YAMLParser::decodeOptionalScalar(node, "LColor", theme.LColor);
    YAMLParser::decodeOptionalScalar(node, "SColor", theme.SColor);
    YAMLParser::decodeOptionalScalar(node, "ZColor", theme.ZColor);

    return true;
}

YAML::Node convert<std::vector<Theme>>::encode(const std::vector<Theme> &themes)
{
    YAML::Node node(YAML::NodeType::Sequence);

    for (int i = 0; i < (int)themes.size(); i++)
    {
        node.push_back(themes[i]);
    }

    return node;
}

bool convert<std::vector<Theme>>::decode(const YAML::Node &node, std::vector<Theme> &themes)
{
    if (!node.IsSequence() || !node.size())
    {
        return true;
    }

    themes.clear();

    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
    {
        themes.push_back(it->as<Theme>());
    }

    if (themes.empty())
    {
        Theme theme;
        theme.setDefault();

        themes.push_back(theme);
    }

    return true;
}

} // namespace YAML