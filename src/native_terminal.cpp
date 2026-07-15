#include "native_terminal.h"

std::map<std::string,std::string> terminalEnvironment(const ReaperTerminalContext& c){
  return {
    {"REAPER_TERMINAL","1"},
    {"REAPER_TERMINAL_VERSION",c.extensionVersion},
    {"REAPER_VERSION",c.reaperVersion},
    {"REAPER_PROJECT_FILE",c.projectFile},
    {"REAPER_PROJECT_DIR",c.workingDirectory},
    {"REAPER_RESOURCE_DIR",c.resourceDirectory},
    {"REAPER_API_DOCS","https://www.reaper.fm/sdk/reascript/reascripthelp.html"},
    {"REAPER_EXTENSION_SDK","https://www.reaper.fm/sdk/plugin/plugin.php"},
    {"REAPER_SDK_SOURCE","https://github.com/justinfrankel/reaper-sdk"},
    {"TERM_PROGRAM","REAPER Terminal"},
    {"TERM_PROGRAM_VERSION",c.extensionVersion}
  };
}
