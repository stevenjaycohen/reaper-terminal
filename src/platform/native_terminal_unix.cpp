#include "native_terminal.h"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
bool executable(const char* name){
  if(std::strchr(name,'/'))return access(name,X_OK)==0;
  const char* path=std::getenv("PATH");if(!path)return false;std::string p(path);
  for(std::size_t at=0;at<=p.size();){auto end=p.find(':',at);auto dir=p.substr(at,end-at);if(access((dir+"/"+name).c_str(),X_OK)==0)return true;if(end==std::string::npos)break;at=end+1;}
  return false;
}
std::string quote(const std::string& value){std::string out="'";for(char c:value){if(c=='\'')out+="'\\''";else out+=c;}return out+="'";}
std::string bootstrap(const ReaperTerminalContext& c){
  std::string script;
  for(const auto& [key,value]:terminalEnvironment(c))script+="export "+key+"="+quote(value)+";";
  script+="printf '\\nREAPER Terminal %s — REAPER %s\\nDirectory: %s\\nProject: %s\\nAPI: %s\\nExtension SDK: %s\\nSDK source: %s\\n\\n' "+quote(c.extensionVersion)+" "+quote(c.reaperVersion)+" "+quote(c.workingDirectory)+" "+quote(c.projectFile.empty()?"(unsaved)":c.projectFile)+" "+quote("https://www.reaper.fm/sdk/reascript/reascripthelp.html")+" "+quote("https://www.reaper.fm/sdk/plugin/plugin.php")+" "+quote("https://github.com/justinfrankel/reaper-sdk")+";";
  script+="cd "+quote(c.workingDirectory)+";exec \"${SHELL:-/bin/sh}\" -l";return script;
}
[[noreturn]] void run(const ReaperTerminalContext& c){
  setsid();if(!c.workingDirectory.empty())chdir(c.workingDirectory.c_str());
  for(const auto& [key,value]:terminalEnvironment(c))setenv(key.c_str(),value.c_str(),1);
#if defined(__APPLE__)
  std::string script=bootstrap(c),escaped;for(char ch:script){if(ch=='\\'||ch=='\"')escaped+='\\';escaped+=ch;}
  std::string apple="tell application \"Terminal\" to do script \""+escaped+"\"";
  execlp("osascript","osascript","-e",apple.c_str(),(char*)nullptr);
#else
  const std::string script=bootstrap(c);
  const char* overrideCommand=std::getenv("REAPER_TERMINAL_COMMAND");
  if(overrideCommand&&*overrideCommand&&executable(overrideCommand))execlp(overrideCommand,overrideCommand,(char*)nullptr);
  if(executable("kgx"))execlp("kgx","kgx","--working-directory",c.workingDirectory.c_str(),"--title","REAPER Terminal","--","/bin/sh","-lc",script.c_str(),(char*)nullptr);
  if(executable("gnome-terminal"))execlp("gnome-terminal","gnome-terminal","--working-directory",c.workingDirectory.c_str(),"--title","REAPER Terminal","--","/bin/sh","-lc",script.c_str(),(char*)nullptr);
  if(executable("konsole"))execlp("konsole","konsole","--workdir",c.workingDirectory.c_str(),"-p","tabtitle=REAPER Terminal","-e","/bin/sh","-lc",script.c_str(),(char*)nullptr);
  if(executable("xfce4-terminal"))execlp("xfce4-terminal","xfce4-terminal","--working-directory",c.workingDirectory.c_str(),"--title","REAPER Terminal","--command",("/bin/sh -lc "+quote(script)).c_str(),(char*)nullptr);
  if(executable("x-terminal-emulator"))execlp("x-terminal-emulator","x-terminal-emulator","-T","REAPER Terminal","-e","/bin/sh","-lc",script.c_str(),(char*)nullptr);
  if(executable("xterm"))execlp("xterm","xterm","-T","REAPER Terminal","-e","/bin/sh","-lc",script.c_str(),(char*)nullptr);
#endif
  _exit(127);
}
}

bool launchNativeTerminal(const ReaperTerminalContext& c,std::string& error){
  pid_t first=fork();if(first<0){error=std::strerror(errno);return false;}
  if(first==0){pid_t terminal=fork();if(terminal<0)_exit(126);if(terminal==0)run(c);_exit(0);}
  int status=0;while(waitpid(first,&status,0)<0&&errno==EINTR){}
  if(!WIFEXITED(status)||WEXITSTATUS(status)!=0){error="could not create terminal launcher process";return false;}
  return true;
}
