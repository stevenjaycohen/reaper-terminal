#pragma once
#include <map>
#include <string>

struct ReaperTerminalContext {
  std::string workingDirectory;
  std::string projectFile;
  std::string resourceDirectory;
  std::string reaperVersion;
  std::string extensionVersion = "0.2.0";
  std::string startupCommand;
};

std::map<std::string,std::string> terminalEnvironment(const ReaperTerminalContext& context);
bool launchNativeTerminal(const ReaperTerminalContext& context,std::string& error);
