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

  _removeBackgroundSign(cmd_s.c_str());
  if (firstWord.compare("chprompt") == 0)
  {
    return new ChangePromptCommand(cmd_s);
  }
  if (firstWord.compare("showpid") == 0)
  {
    return new ShowPIDCommand(cmd_s);
  }
  if (firstWord.compare("pwd") == 0)
  {
    return new PwdCommand(cmd_s);
  }
  if (firstWord.compare("cd") == 0)
  {
    return new CdCommand(cmd_s);
  }
  if (firstWord.compare("jobs") == 0)
  {
    return new JobsCommand(cmd_s);
  }
  if (firstWord.compare("kill") == 0)
  {
    return new KillCommand(cmd_s);
  }
  if (firstWord.compare("fg") == 0)
  {
    return new ForegroundCommand(cmd_s);
  }
  if (firstWord.compare("bg") == 0)
  {
    return new BackgroundCommand(cmd_s);
  }
  if (firstWord.compare("quit") == 0){
    return new QuitCommand(cmd_s);
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

std::map<int,JobsList::JobEntry*>& JobsList::getStoppedJobs(){
  return _stopped_jobs;
}


JobsList::JobEntry * JobsList::getLastStoppedJob(int *lastJobId){
  if(!_stopped_jobs.empty()){
    auto max_it = _stopped_jobs.rbegin();
    *lastJobId = max_it->first;
    return max_it->second;
  }
  else{
    return nullptr;
  } 
}

void JobsList::removeJobById(int id)
{
  _jobs.erase(id);
}

void JobsList::killAllJobs(){
  std::cout << "smash: sending SIGKILL signal to " <<_jobs.size()<<" jobs:" <<std::endl;
  for (auto itt = _jobs.begin() ; itt!=_jobs.end(); itt++){
    itt->second.die();
  }
}
//jobEntry
JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  auto found = _jobs.find(jobId);
  return (found ==_jobs.end())? nullptr:&(found->second);
}

JobsList::JobEntry *JobsList::getLastJob(int *lastJobId)
{
  
  if(!_jobs.empty())
  {
    auto max_it = _jobs.rbegin();
    *lastJobId = max_it->first;
    return &max_it->second;
  }
  else{
    return nullptr;
  }
}

JobsList::JobEntry::JobEntry(int id, std::string command)
{
  _timeMade = time(nullptr);
  _jobId = id;
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
void JobsList::JobEntry::die(){
  std::cout << this->get_pid() << ": " << this->_command << std::endl;  
  kill(this->get_pid(),SIGKILL);
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
    if (_args_num != 2 && _args_num !=1 )
    {
      throw std::invalid_argument("");
    }

    JobsList JobsList = SmallShell::getInstance().getJobsList();
    JobsList::JobEntry* target; 

    if(_args_num==2){    
      int jobId =std::stoi(_args[1]);
      
      target = JobsList.getJobById(jobId);
      if(!target){
        std::cerr<<"smash error: bg: job-id "<<jobId<<" does not exist" << std::endl ;
        return;
      }

      if(JobsList.getStoppedJobs().find(jobId) ==JobsList.getStoppedJobs().end()){
        std::cerr<<"smash error: bg: job-id "<<jobId<<" is already running in the background" << std::endl;
        return;
      } 

    }
    else{
      target = JobsList.getLastStoppedJob(nullptr); 
      
      if(target == nullptr){
        std::cerr<<"smash error: bg: there is no stopped jobs to resume" << std::endl;
        return;
      }

    }

    std::cout<< target;
    kill(target->get_pid(),SIGCONT);
    JobsList.removeJobById(target->getId());
  }
  catch (const std::invalid_argument &)
  {
    std::cerr << "smash error: bg: invalid arguments" << std::endl;
    return;
  }
}

//quit
void QuitCommand::execute(){
  if (_args[1] == "kill"){
    JobsList jobsList = SmallShell::getInstance().getJobsList();       
    jobsList.killAllJobs();
  }
  exit(1);
}


//ExternalCommand
ExternalCommand::ExternalCommand(const char* cmd_line){
  if (execv("bash",)!=-1)
  
}