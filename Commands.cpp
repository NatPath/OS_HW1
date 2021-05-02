#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <algorithm>
#include <iomanip>
#include "Commands.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

// a macro in the spirit of the one showed in the lecture to make syscalls safe.
// first argument is the syscall command.
// second arguments it the variable (which should be intialized beforehand) which will hold the return value
#define DO_SYS( SYSCALL , RET_VALUE ) do { \
  RET_VALUE = SYSCALL ;\
  if ( RET_VALUE ==-1){\
    perror_wrap(#SYSCALL);\
  }\
} while (0)\


/**
 * perror_wrap -
 * Gets a function call . for example gets "wait(1)"
 * Does the perror with the required formating. in the example  "smash error: wait failed"
 * */
void perror_wrap(const char* func_errored){
  std::string func_string = func_errored;  
  std::string error_string = "smash error: " + func_string.substr(0,func_string.find('(')) + " failed";
  perror(error_string.c_str());
  
}

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

void SmallShell::killFg(){
  std::cout<<"smash: got ctrl-C";
  int junk;
  JobsList::JobEntry* fg = _jobsList.getFgJob();
  if(fg){
    DO_SYS(kill(fg->get_pid(),SIGKILL),junk);
    std::cout<<"smash: process "<<fg->get_pid()<<" was killed";
  }
}

void SmallShell::stopFg(){
  std::cout<<"smash: got ctrl-Z";
  int junk;
  JobsList::JobEntry* fg = _jobsList.getFgJob();
   if(fg){
    SmallShell::getInstance().getJobsList().getStoppedJobs().insert(std::pair<int,JobsList::JobEntry*>(fg->getId(),fg));
    DO_SYS(kill(fg->get_pid(),SIGSTOP),junk);
    fg->set_stopped(true);

    std::cout<<"smash: process "<<fg->get_pid()<<" was stopped";
  }
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
  if (cmd_s==""){
    return nullptr;
  }
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
  if (firstWord.compare("cat") == 0)
  {
    return new CatCommand(cmd);
  }

  return new ExternalCommand(cmd_line);

  /*
  if (string(args[0]).compare("chprompt")==0){
        
  }
  */
  //delete[] args;
}


void handlePipe(string cmd, int delimeter){
  const char * first = cmd.substr(0,delimeter).c_str();
  Command *cmd1 = SmallShell::getInstance().CreateCommand(first);
  const char * second = cmd.substr(delimeter+1,cmd.length()-1).c_str();
  Command *cmd2 = SmallShell::getInstance().CreateCommand(second);

  PipeCommand new_pipe = PipeCommand(cmd1,cmd2,false);
  new_pipe.execute();

}

void handlePiperr(string cmd, int delimeter){
  const char * first = cmd.substr(0,delimeter).c_str();
  Command *cmd1 = SmallShell::getInstance().CreateCommand(first);
  const char * second = cmd.substr(delimeter+2,cmd.length()-1).c_str();
  Command *cmd2 = SmallShell::getInstance().CreateCommand(second);

  PipeCommand new_pipe = PipeCommand(cmd1,cmd2,true);
  new_pipe.execute();

}


void handleRegular(const char *cmd_line){
   Command *new_command = SmallShell::getInstance().CreateCommand(cmd_line);

  if (new_command == nullptr)
  {
    std::cout << "invalid command" << std::endl;
    return;
  }
  new_command->execute();
  delete(new_command);
}

void handleRedirection(string cmd, int delimeter){
  const char * first = cmd.substr(0,delimeter).c_str();
  Command *cmd1 = SmallShell::getInstance().CreateCommand(first);
  const char * second = _trim(cmd.substr(delimeter+1,cmd.length()-delimeter-1)).c_str();
 // int fd = open(second,O_CREAT| O_TRUNC);
  RedirectionCommand new_re = RedirectionCommand(cmd1,second,false);
  new_re.execute();
  delete(cmd1);
}

void handleRedirectionAppend(string cmd, int delimeter){
  const char * first = cmd.substr(0,delimeter).c_str();
  Command *cmd1 = SmallShell::getInstance().CreateCommand(first);
  const char * second = cmd.substr(delimeter+2,cmd.length()-1).c_str();
 // int fd = open(second,O_CREAT| O_TRUNC);
  RedirectionCommand new_re = RedirectionCommand(cmd1,second,true);
  new_re.execute();
  delete(cmd1);
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
      handlePiperr(cmd_s,pos);
      break;
    case PIPE_CMD: // " | "
      handlePipe(cmd_s,pos);
      break;
    //Redirect- 
    case REDIRECT_CMD: // " > "
      handleRedirection(cmd_s,pos);
      break;
    case REDIRECT_APPEND_CMD: // " >> "
      handleRedirectionAppend(cmd_s,pos);
      break;
    case REGULAR_CMD: 
      handleRegular(cmd_line);
      break;
  }

 
}

string SmallShell::getPromptName()
{
  return _prompt_name;
}

JobsList::JobEntry* SmallShell::getFgJob(){
  return _fg_job;
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

JobsList::JobEntry * JobsList::getFgJob(){
  return _fg_job;
}

JobsList::JobEntry *JobsList::getLastStoppedJob(int *lastJobId)
{
  if (!_stopped_jobs.empty())
  {
    auto max_it = _stopped_jobs.rbegin();
    if (lastJobId!=nullptr){
      *lastJobId = max_it->first;
    }
    return max_it->second;
  }
  else
  {
    if (lastJobId!=nullptr){
      *lastJobId = -1;
    }
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
void JobsList::setFgJob(JobsList::JobEntry* fg_job){
  _fg_job=fg_job;
}


//jobEntry
JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  auto found = _jobs.find(jobId);
  return (found == _jobs.end()) ? nullptr : &(found->second);
}

void JobsList::JobEntry::set_stopped(bool stopped){
  _stopped = stopped;
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
    *lastJobId = -1;
    return nullptr;
  }
}

JobsList::JobEntry::JobEntry(int id,pid_t pid, std::string command)
{
  _timeMade = time(nullptr);
  _jobId = id;
  _pid = pid;
  _command = command;
  _stopped = false;
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
  _original_cmd=cmd_line;
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
  std::map<int, JobsList::JobEntry>* jobs = &SmallShell::getInstance().getJobsList().getJobs();
  int status;
  int ret;
  int pid;
  int jobs_size=jobs->size();
  for (auto it = jobs->begin(),next_it=it; it != jobs->end(); it= next_it)
  {
    next_it++;
    pid=it->second.get_pid();
    DO_SYS(waitpid(it->second.get_pid(),&status,WNOHANG),ret);
  
    if(ret){
      SmallShell::getInstance().getJobsList().removeJobById(it->first);
    }
    else{
      it->second.printJob();
    }
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

    int ret;
    DO_SYS(kill(target->get_pid(),signum),ret);

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
  int junk;
  JobsList *jobsList = &SmallShell::getInstance().getJobsList();
  std::map<int, JobsList::JobEntry> *jobs = &jobsList->getJobs();
  if (_args_num == 1)
  {
    if (jobs->empty())
    {
      std::cerr << "smash error: fg: jobs list is empty" << std::endl;
    }
    else
    {
      auto last = jobs->rbegin();
      std::cout << last->second << std::endl;
      DO_SYS(kill(last->second.get_pid(), SIGCONT),junk);
      SmallShell::getInstance().getJobsList().setFgJob(&last->second);
      waitpid(last->second.get_pid(), nullptr, 0);
      jobsList->removeJobById(last->second.getId());
      SmallShell::getInstance().getJobsList().setFgJob(nullptr);
    }
    return;
  }
  if (_args_num == 2)
  {
    auto found_job = jobs->find(stoi(_args[1]));
    if (found_job == jobs->end())
    {
      std::cerr << "smash error: fg: job-id " << _args[1] << " does not exist" << std::endl;
    }
    else
    {
      std::cout << found_job->second << std::endl;
      DO_SYS(kill(found_job->second.get_pid(), SIGCONT),junk);
      waitpid(found_job->second.get_pid(), nullptr, 0);
      jobsList->removeJobById(found_job->second.getId());
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
  int junk;
  try
  {
    if (_args_num != 2 && _args_num != 1)
    {
      throw std::invalid_argument("");
    }

    JobsList* JobsList = &SmallShell::getInstance().getJobsList();
    JobsList::JobEntry *target;

    if (_args_num == 2)
    {
      int jobId = std::stoi(_args[1]);

      target = JobsList->getJobById(jobId);
      if (!target)
      {
        std::cerr << "smash error: bg: job-id " << jobId << " does not exist" << std::endl;
        return;
      }

      if (JobsList->getStoppedJobs().find(jobId) == JobsList->getStoppedJobs().end())
      {
        std::cerr << "smash error: bg: job-id " << jobId << " is already running in the background" << std::endl;
        return;
      }
    }
    else
    {
      target = JobsList->getLastStoppedJob(nullptr);

      if (target == nullptr)
      {
        std::cerr << "smash error: bg: there is no stopped jobs to resume" << std::endl;
        return;
      }
    }

    std::cout << target;
    DO_SYS(kill(target->get_pid(), SIGCONT),junk);
    JobsList->removeJobById(target->getId());
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
void ExternalCommand::execute(){
  int junk;
  string cmd_s = _trim(string(_original_cmd));
  char *cmd = &cmd_s[0];
  pid_t id;
  DO_SYS(fork(),id);
  if (id == 0)
  {
    setpgrp();
    _removeBackgroundSign(cmd);
    execl("/bin/bash", "bash","-c",cmd, nullptr);
  }
  else
  {
    int jobid = 0;
    SmallShell::getInstance().getJobsList().getLastJob(&jobid);
    JobsList::JobEntry job = JobsList::JobEntry(jobid+1,id,cmd_s);

    SmallShell::getInstance().getJobsList().getJobs().insert(std::pair<int,JobsList::JobEntry>(jobid+1,job));
    //if last character is not &
    if (strcmp(cmd_s.substr(cmd_s.length() - 1, 1).c_str(), "&") != 0)
    {
      //run in foreground
      SmallShell::getInstance().getJobsList().setFgJob(&job);
      DO_SYS(waitpid(id, nullptr, 0),junk);
      SmallShell::getInstance().getJobsList().getJobs().erase(jobid+1);
      SmallShell::getInstance().getJobsList().setFgJob(nullptr);

    }

  }
  

}

//pipe
PipeCommand::PipeCommand(Command* command1, Command* command2, bool err){
  _cmd1 = command1;
  _cmd2 = command2;
  _fdt_entry = err? 2:1;
}
void PipeCommand::execute(){
  int fd[2];
  int pipe_ret_value;
  DO_SYS ( pipe(fd) , pipe_ret_value );
  //the above line is supposed to be a safer way doing the below commented block
  /*
  if (pipe(fd)==-1){
    perror("smash error: pipe failed");
    return;
  }
  */
  pid_t id;
  DO_SYS(fork(),id);
  int junk_ret;// dummy argument for DO_SYS
  //son
  if(id ==0){
    setpgrp();
    DO_SYS(close(fd[0]),junk_ret); // close read
    int oldOut;
    DO_SYS(dup(_fdt_entry),oldOut);
    DO_SYS(dup2(fd[1],_fdt_entry),junk_ret);// redirects output to stdout
    _cmd1->execute();
    DO_SYS(dup2(oldOut,_fdt_entry),junk_ret); //restores output back
  }
  //parent
  else{
    DO_SYS(close(fd[1]),junk_ret);
    int oldIn;
    DO_SYS(dup(0),oldIn);
    DO_SYS(dup2(fd[0],0),junk_ret);
    DO_SYS(waitpid(id,nullptr,0),junk_ret);
    _cmd2->execute();
    DO_SYS(dup2(oldIn,0),junk_ret);
  }
  close(fd[0]);
  close(fd[1]);
}

//Redirection
RedirectionCommand::RedirectionCommand(Command* cmd,const char* file_name,bool append):Command(){
  _cmd = cmd;
  if(append){
    DO_SYS(open(file_name,O_RDWR | O_CREAT | O_APPEND,S_IRWXU),_fd);
  }
  else{
    DO_SYS(open(file_name,O_RDWR | O_CREAT | O_TRUNC,S_IRWXU),_fd);
  }
}
void RedirectionCommand::execute(){
  // fd will have to be created somewhere in this class, and will be set to write or append.
  int oldOut;
  int junk;
  DO_SYS(dup(1),oldOut);
  DO_SYS(dup2(_fd,1),junk);
  _cmd->execute();
  DO_SYS(dup2(oldOut,1),junk);
}

//cat 

//writes the content of a file with the given fd to standard output
void print_file(int fd){
  size_t size_of_file= lseek(fd,0,SEEK_END);
  lseek(fd,0,SEEK_SET);
  char *buff=(char*)malloc(size_of_file*sizeof(char));
  if (read(fd,buff,size_of_file)==-1){
    std::cerr<<"smash error: read failed";
  }
  else{
    int junk=0;
    DO_SYS(write(1,buff,size_of_file),junk);
  }
}
void CatCommand::execute(){
  if (_args_num == 1){
    std::cerr<<"smash error: cat: not enough arguments";
  }
  auto itt= _args.begin();
  itt++;
  int current_file;

  for (;itt!=_args.end() && *itt!="";itt++){
    current_file = open((*itt).c_str(),O_RDONLY);
    if (current_file==-1){
      perror("smash error: open failed");
    }
    else{
      print_file(current_file);
      close(current_file);
    }
  }
}
