#include "native_terminal.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <algorithm>
#include <vector>

namespace {
std::wstring wide(const std::string& s){if(s.empty())return {};int n=MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,nullptr,0);std::wstring w(n,L'\0');MultiByteToWideChar(CP_UTF8,0,s.c_str(),-1,w.data(),n);w.pop_back();return w;}
std::vector<wchar_t> environment(const ReaperTerminalContext& c){
  std::map<std::wstring,std::wstring> values;LPWCH base=GetEnvironmentStringsW();
  if(base){for(const wchar_t* p=base;*p;){std::wstring item(p);auto eq=item.find(L'=');if(eq!=std::wstring::npos&&eq>0)values[item.substr(0,eq)]=item.substr(eq+1);p+=item.size()+1;}FreeEnvironmentStringsW(base);}
  for(const auto& [k,v]:terminalEnvironment(c))values[wide(k)]=wide(v);
  std::vector<wchar_t> block;for(const auto& [k,v]:values){block.insert(block.end(),k.begin(),k.end());block.push_back(L'=');block.insert(block.end(),v.begin(),v.end());block.push_back(0);}block.push_back(0);return block;
}
bool exists(const wchar_t* program){return SearchPathW(nullptr,program,L".exe",0,nullptr,nullptr)>0;}
std::wstring psQuote(const std::string& value){std::wstring w=wide(value),out=L"'";for(wchar_t c:w){out+=c;if(c==L'\'')out+=L'\'';}return out+L"'";}
std::wstring bootstrap(const ReaperTerminalContext& c){std::wstring out;for(const auto& [key,value]:terminalEnvironment(c))out+=L"$env:"+wide(key)+L"="+psQuote(value)+L";";out+=L"Write-Host '';Write-Host ('REAPER Terminal "+wide(c.extensionVersion)+L" — REAPER "+wide(c.reaperVersion)+L"');Write-Host ('Directory: "+wide(c.workingDirectory)+L"');Write-Host ('Project: "+wide(c.projectFile.empty()?"(unsaved)":c.projectFile)+L"');Write-Host 'API: https://www.reaper.fm/sdk/reascript/reascripthelp.html';Write-Host 'Extension SDK: https://www.reaper.fm/sdk/plugin/plugin.php';Write-Host 'SDK source: https://github.com/justinfrankel/reaper-sdk';Write-Host '';";if(!c.startupCommand.empty())out+=L"Invoke-Expression "+psQuote(c.startupCommand)+L";";return out;}
}
bool launchNativeTerminal(const ReaperTerminalContext& c,std::string& error){
  const std::wstring setup=bootstrap(c);
  std::wstring command=exists(L"wt")?L"wt.exe -w new -d \""+wide(c.workingDirectory)+L"\" powershell.exe -NoExit -Command \""+setup+L"\"":L"powershell.exe -NoExit -Command \""+setup+L"\"";
  std::vector<wchar_t> mutableCommand(command.begin(),command.end());mutableCommand.push_back(0);auto env=environment(c);
  STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};
  DWORD flags=CREATE_UNICODE_ENVIRONMENT|(exists(L"wt")?0:CREATE_NEW_CONSOLE);
  if(!CreateProcessW(nullptr,mutableCommand.data(),nullptr,nullptr,FALSE,flags,env.data(),wide(c.workingDirectory).c_str(),&startup,&process)){error="CreateProcess failed: "+std::to_string(GetLastError());return false;}
  CloseHandle(process.hThread);CloseHandle(process.hProcess);return true;
}
