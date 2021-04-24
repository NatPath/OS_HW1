#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <algorithm>
#include <iomanip>
#include "Commands.h"

using namespace std;

#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#define WHITESPACE " "
# define MAX_PATH_LENGTH 255
#endif

string _ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
  return _rtrim(_ltrim(s));
}

/*
int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}
*/
int _parseCommandLine(const char *cmd_line, vector<string> &args)
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = s;
    args[++i] = "\0";
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundCommand(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell()
{
  // TODO: add your implementation
  _prompt_name = "smash";
}

SmallShell::~SmallShell()
{
  // TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line)
{
  // For example:
  /*
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
  /*
  char ** args=new char*[COMMAND_MAX_ARGS];
  int num_args = _parseCommandLine(cmd_line,args);
  num_args--;
  */

  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("chprompt") == 0)
  {
    return new ChangePromptCommand(cmd_line);
  }
  if (firstWord.compare("showpid") == 0)
  {
    return new ShowPIDCommand(cmd_line);
  }
  if (firstWord.compare("pwd") == 0)
  {
    return new PwdCommand(cmd_line);
  }
  if (firstWord.compare("cd") == 0)
  {
    return new CwdCommand(cmd_line);
  }

  /*
  if (string(args[0]).compare("chprompt")==0){
        
  }
  */
  //delete[] args;

  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
  Command *new_command = CreateCommand(cmd_line);

  if (new_command == nullptr)
  {
    std::cout << "invalid command" << std::endl;
    return;
  }
  new_command->execute();
}

string SmallShell::getPromptName()
{
  return _prompt_name;
}
void SmallShell::setPromptName(string new_name)
{
  _prompt_name = new_name;
}

string SmallShell::getLastWorkingDir()
{
  return _last_working_dir;
}

void SmallShell::setLastWorkingDir(string new_dir)
{
  _last_working_dir = new_dir;
}
//jobsList
JobsList::JobEntry* JobsList::getJobById(int jobId){
  return &jobs.find(jobId)->second;
}

JobsList::JobEntry * JobsList::getLastJob(int* lastJobId){
  auto max_it = std::max_element(std::begin(jobs),std::end(jobs));
  *lastJobId = max_it->first;
  return &max_it->second;
}

//jobEntry
int JobsList::JobEntry::getId(){
  return jobId;
}

void JobsList::JobEntry::setId(int id){
  jobId = id;
}


//General Command Zone
Command::Command(const char *cmd_line)
{
  _args = vector<string>(COMMAND_MAX_ARGS);
  _args_num = _parseCommandLine(cmd_line, _args);
}
//chprompt ZONE

void ChangePromptCommand::execute()
{
  SmallShell &smash = SmallShell::getInstance();
  smash.setPromptName(_args[1]);
}

// showpid
void ShowPIDCommand::execute()
{
  std::cout << "smash pid is ";
  std::cout << getpid();
}

//pwd
void PwdCommand::execute()
{

  char *buff = (char *)malloc(pathconf(".", _PC_PATH_MAX));
  getcwd(buff, pathconf(".", _PC_PATH_MAX));
  std::cout << buff;
  free(buff);
}

//cwd

void CwdCommand::execute()
{
  if (_args_num > 2)
  {
    std::cerr << "smash error: cd: too many arguments";
    return;
  }
  std::string new_path;
  if (_args[1].compare("-") == 0)
  {
    //last pwd
    if (SmallShell::getInstance().getLastWorkingDir().empty())
    {
      std::cerr << "smash error: cd: OLDPWD not set";
      return;
    }
    new_path = SmallShell::getInstance().getLastWorkingDir();
  }
  else
  {
    new_path = _args[1];
  }
  char *buff = (char *)malloc(pathconf(".", _PC_PATH_MAX));
  getcwd(buff, pathconf(".", _PC_PATH_MAX));
  
  
  if(chdir(new_path.c_str()) == -1){
    // error handling
    perror("smash error: chdir failed");
    free(buff);
    return;
  }

  SmallShell::getInstance().setLastWorkingDir(buff);
  free(buff);
}