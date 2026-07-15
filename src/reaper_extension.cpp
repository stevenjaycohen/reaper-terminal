#include "native_terminal.h"
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <reaper_plugin.h>
#define REAPERAPI_IMPLEMENT
#include <reaper_plugin_functions.h>

namespace {
int commandId{},settingsCommandId{};
constexpr const char* settingsSection="REAPER Terminal";
constexpr const char* startupCommandKey="startup_command";
std::string home(){
#ifdef _WIN32
  const char* p=std::getenv("USERPROFILE");
#else
  const char* p=std::getenv("HOME");
#endif
  return p&&*p?p:".";
}
ReaperTerminalContext context(){
  ReaperTerminalContext c;char project[4096]{};EnumProjects(-1,project,sizeof(project));
  if(project[0]&&std::filesystem::is_regular_file(project)){c.projectFile=project;c.workingDirectory=std::filesystem::path(project).parent_path().string();}
  else {
    char mediaPath[4096]{};GetProjectPathEx(nullptr,mediaPath,sizeof(mediaPath));
    if(mediaPath[0]&&std::filesystem::is_directory(mediaPath))c.workingDirectory=mediaPath;
    else c.workingDirectory=(std::filesystem::path(home())/"Documents"/"REAPER Media").string();
  }
  if(const char* resource=GetResourcePath())c.resourceDirectory=resource;
  if(const char* version=GetAppVersion())c.reaperVersion=version;
  if(const char* startup=GetExtState(settingsSection,startupCommandKey))c.startupCommand=startup;
  return c;
}
void showError(const std::string& error){ShowMessageBox(("Could not open a native terminal:\n\n"+error+"\n\nSet REAPER_TERMINAL_COMMAND to an executable to override terminal detection.").c_str(),"REAPER Terminal",0);}
void configureStartupCommand(){
  char value[4096]{};if(const char* current=GetExtState(settingsSection,startupCommandKey)){std::strncpy(value,current,sizeof(value)-1);}
  if(GetUserInputs("REAPER Terminal settings",1,"Startup command (blank for shell):,extrawidth=240",value,sizeof(value)))SetExtState(settingsSection,startupCommandKey,value,true);
}
bool command(int id,int){if(id==settingsCommandId){configureStartupCommand();return true;}if(id!=commandId)return false;std::string error;if(!launchNativeTerminal(context(),error))showError(error);return true;}
void menu(const char* id,void* raw,int flag){if(std::string(id)=="Main extensions"&&flag==0){InsertMenu((HMENU)raw,-1,MF_BYPOSITION|MF_STRING,commandId,"Open REAPER Terminal");InsertMenu((HMENU)raw,-1,MF_BYPOSITION|MF_STRING,settingsCommandId,"REAPER Terminal settings...");}}
}

extern "C" REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE,reaper_plugin_info_t* rec){
  if(!rec)return 0;if(rec->caller_version!=REAPER_PLUGIN_VERSION||!rec->GetFunc)return 0;if(REAPERAPI_LoadAPI(rec->GetFunc)!=0)return 0;
  commandId=rec->Register("command_id",(void*)"REAPER_TERMINAL_OPEN");if(!commandId)return 0;
  settingsCommandId=rec->Register("command_id",(void*)"REAPER_TERMINAL_SETTINGS");if(!settingsCommandId)return 0;
  static gaccel_register_t action{};action.accel.cmd=commandId;action.desc="REAPER Terminal: Open native terminal";
  static gaccel_register_t settingsAction{};settingsAction.accel.cmd=settingsCommandId;settingsAction.desc="REAPER Terminal: Configure startup command";
  rec->Register("gaccel",&action);rec->Register("gaccel",&settingsAction);rec->Register("hookcommand",(void*)command);rec->Register("hookcustommenu",(void*)menu);AddExtensionsMainMenu();
  if(std::getenv("REAPER_TERMINAL_TEST_AUTO_OPEN")){std::string error;if(!launchNativeTerminal(context(),error))showError(error);}
  return 1;
}
