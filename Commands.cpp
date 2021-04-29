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
#define MAX_PATH_LENGTH 255
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

//enum of the 5 types of commands, 4 specials and 1 regular.
typedef enum e_special_cmd {PIPERR_CMD,PIPE_CMD,REDIRECT_CMD,REDIRECT_APPEND_CMD,REGULAR_CMD} SpecialCmd;

/**
 * isSpecialCmd (string cmd , int* pos):
 * gets a command string and returns the enum fitting the type of the command.
 * also puts the position of the "delimiter" in *pos.
 * 
 * */
SpecialCmd isSpecialCmd(string cmd,int* pos){
  if (cmd.find("|&")!=string::npos){
    *pos = cmd.find("|&");
    return PIPERR_CMD;
  }
  if (cmd.find("|")!=string::npos){
    *pos = cmd.find("|"); 
    return PIPE_CMD;
  }
  if (cmd.find(">>")!=string::npos){
    *pos = cmd.find(">>"); 
    return REDIRECT_APPEND_CMD;
  }
  if (cmd.find(">")!=string::npos){
    *pos = cmd.find(">");
    return REDIRECT_CMD;
  }
  return REGULAR_CMD;
}

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell()
{
  // TODO: add your implementation
  _prompt_name = "smash";
  _jobsList = JobsList();
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
  //TODO:
  /*
1) builtin commands should ignore &

*/
  string cmd_s = _trim(string(cmd_line));


  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  char *cmd = &cmd_s[0];
  _removeBackgroundSign(cmd);
  if (firstWord.compare("chprompt") == 0)
  {
    return new ChangePromptCommand(cmd);
  }
  if (firstWord.compare("showpid") == 0)
  {
    return new ShowPIDCommand(cmd);
  }
  if (firstWord.compare("pwd") == 0)
  {
    return new PwdCommand(cmd);
  }
  if (firstWord.compare("cd") == 0)
  {
    return new CdCommand(cmd);
  }
  if (firstWord.compare("jobs") == 0)
  {
    return new JobsCommand(cmd);
  }
  if (firstWord.compare("kill") == 0)
  {
    return new KillCommand(cmd);
  }
  if (firstWord.compare("fg") == 0)
  {
    return new ForegroundCommand(cmd);
  }
  if (firstWord.compare("bg") == 0)
  {
    return new BackgroundCommand(cmd);
  }
  if (firstWord.compare("quit") == 0)
  {
    return new QuitCommand(cmd);
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

  string cmd_s = _trim(string(cmd_line));
// recognize type of command
  int pos;
  SpecialCmd cmd_type = isSpecialCmd(cmd_s,&pos);
  //not necessarily using switch..  
  //Just thinking outloud. 
  switch(cmd_type){
    //Pipes- should execute both sides.
    //redirect output of left side to the input of right side in some way (fd handeling or change of std::cout/cin) 
    case PIPERR_CMD: // " |& "
      break;
    case PIPE_CMD: // " | "
      break;
    //Redirect- 
    case REDIRECT_CMD: // " > "
      break;
    case REDIRECT_APPEND_CMD: // " >> "
      break;
    case REGULAR_CMD: 
      break;
  }

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

JobsList &SmallShell::getJobsList()
{
  return _jobsList;
}

void SmallShell::setLastWorkingDir(string new_dir)
{
  _last_working_dir = new_dir;
}
//jobsList
std::map<int, JobsList::JobEntry> &JobsList::getJobs()
{
  return _jobs;
}

std::map<int, JobsList::JobEntry *> &JobsList::getStoppedJobs()
{
  return _stopped_jobs;
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *lastJobId)
{
  if (!_stopped_jobs.empty())
  {
    auto max_it = _stopped_jobs.rbegin();
    *lastJobId = max_it->first;
    return max_it->second;
  }
  else
  {
    return nullptr;
  }
}

void JobsList::removeJobById(int id)
{
  _jobs.erase(id);
}

void JobsList::killAllJobs()
{
  std::cout << "smash: sending SIGKILL signal to " << _jobs.size() << " jobs:" << std::endl;
  for (auto itt = _jobs.begin(); itt != _jobs.end(); itt++)
  {
    itt->second.die();
  }
}


//jobEntry
JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  auto found = _jobs.find(jobId);
  return (found == _jobs.end()) ? nullptr : &(found->second);
}

/**
 *  getLastJob: 
 * Returns the last jobEntry in the JobsList - if the joblist is empty , returns nullptr.
 * Also puts the ID of the last job in *lastJobId.
 * */
JobsList::JobEntry *JobsList::getLastJob(int *lastJobId)
{

  if (!_jobs.empty())
  {
    auto max_it = _jobs.rbegin();
    *lastJobId = max_it->first;
    return &max_it->second;
  }
  else
  {
    return nullptr;
  }
}

JobsList::JobEntry::JobEntry(int id,pid_t pid, std::string command)
{
  _timeMade = time(nullptr);
  _jobId = id;
  _pid = pid;
  _command = command;
}

int JobsList::JobEntry::getId()
{
  return _jobId;
}

void JobsList::JobEntry::setId(int id)
{
  _jobId = id;
}
pid_t JobsList::JobEntry::get_pid() const
{
  return _pid;
}
void JobsList::JobEntry::printJob()
{
  std::string stopped_string = "";
  if (_stopped)
  {
    stopped_string = "(stopped)";
  }
  std::cout << "[" << _jobId << "] " << *this << " " << difftime(time(nullptr), _timeMade) << " secs " << stopped_string << std::endl;
}
std::ostream &operator<<(std::ostream &os, const JobsList::JobEntry &job)
{
  os << job._command << " : " << job.get_pid();
  return os;
}
void JobsList::JobEntry::die()
{
  std::cout << this->get_pid() << ": " << this->_command << std::endl;
  kill(this->get_pid(), SIGKILL);
}

//General Command Zone
Command::Command(const char *cmd_line)
{
  _args = vector<string>(COMMAND_MAX_ARGS);
  _args_num = _parseCommandLine(cmd_line, _args);
}
//chprompt

void ChangePromptCommand::execute()
{
  SmallShell &smash = SmallShell::getInstance();
  if (_args_num == 1)
  {
    smash.setPromptName("smash");
  }
  else
  {
    smash.setPromptName(_args[1]);
  }
}

// showpid
void ShowPIDCommand::execute()
{
  std::cout << "smash pid is " << getpid() << std::endl;
}

//pwd
void PwdCommand::execute()
{

  char *buff = (char *)malloc(pathconf(".", _PC_PATH_MAX));
  getcwd(buff, pathconf(".", _PC_PATH_MAX));
  std::cout << buff << std::endl;
  free(buff);
}

//cd

void CdCommand::execute()
{
  if (_args_num > 2)
  {
    std::cerr << "smash error: cd: too many arguments" << std::endl;
    return;
  }
  std::string new_path;
  if (_args[1].compare("-") == 0)
  {
    //last pwd
    if (SmallShell::getInstance().getLastWorkingDir().empty())
    {
      std::cerr << "smash error: cd: OLDPWD not set" << std::endl;
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

  if (chdir(new_path.c_str()) == -1)
  {
    // error handling
    perror("smash error: chdir failed");
    free(buff);
    return;
  }

  SmallShell::getInstance().setLastWorkingDir(buff);
  free(buff);
}

// jobs
void JobsCommand::execute()
{
  std::map<int, JobsList::JobEntry> _jobs = SmallShell::getInstance().getJobsList().getJobs();
  for (auto it = _jobs.begin(); it != _jobs.end(); it++)
  {
    it->second.printJob();
  }
}

//kill
void KillCommand::execute()
{
  try
  {
    if (_args_num != 3)
    {
      throw std::invalid_argument("");
    }

    int jobId = std::stoi(_args[2]);
    JobsList::JobEntry *target = SmallShell::getInstance().getJobsList().getJobById(jobId);
    if (!target)
    {
      std::cerr << "smash error: kill: job-id " << _args[2] << " does not exist" << std::endl;
      return;
    }
    int signum = std::stoi(_args[1]);
    if (signum > 0)
    {
      throw std::invalid_argument("");
    }
    signum *= -1;

    if (kill(target->get_pid(), signum) != 0)
    {
      perror("smash error: kill failed");
    }

    std::cout << "signal number " << signum << " was sent to pid " << target->get_pid() << std::endl;
  }
  catch (const std::invalid_argument &)
  {
    std::cerr << "smash error: kill: invalid arguments" << std::endl;
    return;
  }
}

// fg

void ForegroundCommand::execute()
{
  JobsList jobsList = SmallShell::getInstance().getJobsList();
  std::map<int, JobsList::JobEntry> jobs = jobsList.getJobs();
  if (_args_num == 1)
  {
    if (jobs.empty())
    {
      std::cerr << "smash error: fg: jobs list is empty" << std::endl;
    }
    else
    {
      auto last = jobs.rbegin();
      std::cout << last->second << std::endl;
      kill(last->second.get_pid(), SIGCONT);
      waitpid(last->second.get_pid(), nullptr, 0);
      jobsList.removeJobById(last->second.getId());
    }
    return;
  }
  if (_args_num == 2)
  {
    auto found_job = jobs.find(stoi(_args[1]));
    if (found_job == jobs.end())
    {
      std::cerr << "smash error: fg: job-id " << _args[1] << " does not exist" << std::endl;
    }
    else
    {
      std::cout << found_job->second << std::endl;
      kill(found_job->second.get_pid(), SIGCONT);
      waitpid(found_job->second.get_pid(), nullptr, 0);
      jobsList.removeJobById(found_job->second.getId());
    }
  }
  else
  {
    std::cerr << "smash error: fg: invalid arguments" << std::endl;
  }
}

//bg
void BackgroundCommand::execute()
{
  try
  {
    if (_args_num != 2 && _args_num != 1)
    {
      throw std::invalid_argument("");
    }

    JobsList JobsList = SmallShell::getInstance().getJobsList();
    JobsList::JobEntry *target;

    if (_args_num == 2)
    {
      int jobId = std::stoi(_args[1]);

      target = JobsList.getJobById(jobId);
      if (!target)
      {
        std::cerr << "smash error: bg: job-id " << jobId << " does not exist" << std::endl;
        return;
      }

      if (JobsList.getStoppedJobs().find(jobId) == JobsList.getStoppedJobs().end())
      {
        std::cerr << "smash error: bg: job-id " << jobId << " is already running in the background" << std::endl;
        return;
      }
    }
    else
    {
      target = JobsList.getLastStoppedJob(nullptr);

      if (target == nullptr)
      {
        std::cerr << "smash error: bg: there is no stopped jobs to resume" << std::endl;
        return;
      }
    }

    std::cout << target;
    kill(target->get_pid(), SIGCONT);
    JobsList.removeJobById(target->getId());
  }
  catch (const std::invalid_argument &)
  {
    std::cerr << "smash error: bg: invalid arguments" << std::endl;
    return;
  }
}

//quit
void QuitCommand::execute()
{
  if (_args[1] == "kill")
  {
    JobsList jobsList = SmallShell::getInstance().getJobsList();
    jobsList.killAllJobs();
  }
  exit(1);
}

//ExternalCommand
ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line)
{
  pid_t id = fork();
  string cmd_s = _trim(string(cmd_line));

  char *cmd = &cmd_s[0];
  char *arg[] = {"-c", cmd, nullptr};
  if (id == 0)
  {
    setpgrp();
    execv("/bin/bash", arg);
  }
  else
  {
    int *jobId;
    SmallShell::getInstance().getJobsList().getLastJob(jobId);
    JobsList::JobEntry job = JobsList::JobEntry(*jobId+1,id,cmd_s);

    SmallShell::getInstance().getJobsList().getJobs().insert(std::pair<int,JobsList::JobEntry>(*jobId+1,job));
    //if last character is not &
    if (strcmp(cmd_s.substr(cmd_s.length() - 1, 1).c_str(), "&") != 0)
    {
      //run in foreground
      waitpid(id, nullptr, 0);
    }
  }
}
void ExternalCommand::execute(){

}

//pipe
void PipeCommand::execute(){
  int fd[2];
  pipe(fd);
  if(pid_t id = fork()==0){
    close(fd[0]); // close read 
    int oldOut = dup(1);
    dup2(fd[1],1); // redirects output to stdout
    _cmd1->execute();
    dup2(oldOut,1); // restore output
  }
  else{
    close(fd[1]);
    waitpid(id,nullptr,0);
    int oldIn = dup(0);
    dup2(fd[0],0);
    _cmd2->execute();
    dup2(oldIn,0);
  }
}

//redirection
void RedirectionCommand::execute(){
  // fd will have to be created somewhere in this class, and will be set to write or append.
  int oldOut = dup(1);
  dup2(_fd,1);
  // will only
  _cmd->execute();
  dup2(oldOut,1);
}
