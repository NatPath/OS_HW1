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
#define DO_SYS_RET( SYSCALL , RET_VALUE ) do { \
  RET_VALUE = SYSCALL ;\
  if ( RET_VALUE ==-1){\
    perror_wrap(#SYSCALL);\
    return;\
  }\
} while (0)\

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
  string func_string = func_errored;
  string error_string= "smash error: " + func_string.substr(0,func_string.find('(')) + " failed";
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

/*
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
*/
int _parseCommandLine(string& cmd_line, vector<string> &args)
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(cmd_line).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = s;
    args[++i] = "\0";
  }
  return i;

  FUNC_EXIT()
}
bool _isBackgroundCommand(std::string& cmd_line)
{
  
  return cmd_line[cmd_line.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(std::string& cmd_line)
{
  if (cmd_line==""){
    return;
  }
  // find last character other than spaces
  unsigned int idx = cmd_line.find_last_not_of(WHITESPACE);
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
//  cmd_line[cmd_line.find_last_not_of(WHITESPACE, idx) + 1] = 0;
  cmd_line = cmd_line.substr(0,cmd_line.length()-1);
}

//enum of the 5 types of commands, 4 specials and 1 regular.
typedef enum e_special_cmd {PIPERR_CMD,PIPE_CMD,REDIRECT_CMD,REDIRECT_APPEND_CMD,REGULAR_CMD} SpecialCmd;

/**
 * isSpecialCmd (string cmd , int* pos):
 * gets a command string and returns the enum fitting the type of the command.
 * also puts the position of the "delimiter" in *pos.
 * 
 * */
SpecialCmd isSpecialCmd(string& cmd,int* pos){
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
  _pid=getpid();
}


SmallShell::~SmallShell()
{
  // TODO: add your implementation
}

void SmallShell::killFg(){
  std::cout<<"smash: got ctrl-C"<<std::endl;
  int junk;
  JobsList::JobEntry* fg = _jobsList.getFgJob();
  if(fg){
    DO_SYS_RET(kill(fg->get_pid(),SIGKILL),junk);
    std::cout<<"smash: process "<<fg->get_pid()<<" was killed"<<std::endl;
  }
}

void SmallShell::stopFg(){
  std::cout<<"smash: got ctrl-Z"<<std::endl;
  int junk;
  JobsList::JobEntry* fg = _jobsList.getFgJob();
   if(fg){
    SmallShell::getInstance().getJobsList().getStoppedJobs().insert(std::pair<int,JobsList::JobEntry*>(fg->getId(),fg));
    DO_SYS_RET(kill(fg->get_pid(),SIGSTOP),junk);
    fg->stopJob();
    std::cout<<"smash: process "<<fg->get_pid()<<" was stopped"<<std::endl;
  }
}
pid_t SmallShell::getPid(){
  return _pid;
} 

// Command Zone

string Command::getOriginalCommand(){
  return _original_cmd;
}

TimeOutCommand* handleTimeOut(string& cmd){
   string cmd_t = _trim(cmd);
   if (cmd_t == "timeout"){
     return nullptr;
   }
   int firstSpace = cmd_t.find(" ");
   int secondSpace = cmd_t.find(" ",firstSpace+1); 
   int time = stoi(cmd_t.substr(firstSpace+1,secondSpace-firstSpace-1)); // guaranteed to work by TAs
   string to_execute = cmd.substr(cmd.find(cmd_t.substr(secondSpace+1)));
   if (to_execute.length()==0){
     return nullptr;
   }
   return new TimeOutCommand(cmd,to_execute,time);
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(std::string& cmd_line)
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
  string cmd_s = _trim(cmd_line);
  if (cmd_s.length() == 0 ){
    return nullptr;
  }
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  _removeBackgroundSign(firstWord);
  _removeBackgroundSign(cmd_s);
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
  if (firstWord.compare("quit") == 0)
  {
    return new QuitCommand(cmd_s);
  }
  if (firstWord.compare("cat") == 0)
  {
    return new CatCommand(cmd_s);
  }
  if(firstWord.compare("timeout") == 0){

    return handleTimeOut(cmd_line);
  }

  return new ExternalCommand(cmd_line);

  /*
  if (string(args[0]).compare("chprompt")==0){
        
  }
  */
  //delete[] args;
}


void handlePipe(string& cmd, int delimeter){
  string first_string = cmd.substr(0,delimeter);
  first_string= _trim(first_string);
  //const char* first = first_string.c_str();
 // char * first =(char*) malloc(first_string.length()+1);
 // memcpy(first,first_string.c_str(),first_string.length());
  //first[first_string.length()] = '\0';
  string second_string = cmd.substr(delimeter+1, cmd.string::npos);
  second_string = _trim(second_string);
//  const char* second = second_string.c_str();
  
  Command *cmd1 = SmallShell::getInstance().CreateCommand(first_string);

  Command *cmd2 = SmallShell::getInstance().CreateCommand(second_string);

  PipeCommand new_pipe = PipeCommand(cmd1,cmd2,false);
  new_pipe.execute();
}

void handlePiperr(string& cmd, int delimeter){
  string first_string = cmd.substr(0,delimeter);
  first_string= _trim(first_string);
  string second_string = cmd.substr(delimeter+2, cmd.string::npos);
  second_string = _trim(second_string);

 // const char * first = cmd.substr(0,delimeter).c_str();
  Command *cmd1 = SmallShell::getInstance().CreateCommand(first_string);
  //const char * second = cmd.substr(delimeter+2,cmd.length()-1).c_str();
  Command *cmd2 = SmallShell::getInstance().CreateCommand(second_string);

  PipeCommand new_pipe = PipeCommand(cmd1,cmd2,true);
  new_pipe.execute();
}


void handleRegular(string& cmd_line){
   Command *new_command = SmallShell::getInstance().CreateCommand(cmd_line);

  if (new_command == nullptr)
  {
    std::cout << "invalid command" << std::endl;
    return;
  }
  new_command->execute();
  delete(new_command);
}

void handleRedirection(string& cmd, int delimeter){
  /*
  const char* first = (const char*)malloc(cmd.size()*sizeof(char));
  const char* second = (const char*)malloc(cmd.size()*sizeof(char));
  */
  string first_string = cmd.substr(0,delimeter);
  first_string= _trim(first_string);
  string second_string = cmd.substr(delimeter+1, cmd.string::npos);
  second_string = _trim(second_string);
  //Command *cmd1 = SmallShell::getInstance().CreateCommand(_trim(cmd.substr(0,delimeter)).c_str());
  Command *cmd1 = SmallShell::getInstance().CreateCommand(first_string);
  //RedirectionCommand new_re = RedirectionCommand(cmd1,_trim(cmd.substr(delimeter+1,cmd.string::npos)).c_str(),false);
  RedirectionCommand new_re = RedirectionCommand(cmd1,second_string.c_str(),false);
  new_re.execute();
}

void handleRedirectionAppend(string& cmd, int delimeter){
  string first_string = cmd.substr(0,delimeter);
  first_string=_trim(first_string);
  //first_string= _trim(first_string);
  string second_string = cmd.substr(delimeter+2, cmd.string::npos);
  second_string=_trim(second_string);
 // second_string = _trim(second_string);
  //const char * first = cmd.substr(0,delimeter).c_str();
  Command *cmd1 = SmallShell::getInstance().CreateCommand(first_string);
  //const char * second = _trim(cmd.substr(delimeter+2,cmd.string::npos)).c_str();
  RedirectionCommand new_re = RedirectionCommand(cmd1,second_string.c_str(),true);
  new_re.execute();
}



bool TimeCompare::operator()(JobsList::TimedJob& big, JobsList::TimedJob& small){
  return (big.getAlarmTime()-difftime(time(nullptr), big.getTimeMade())) > (small.getAlarmTime()-difftime(time(nullptr), small.getTimeMade()));
}



void SmallShell::executeCommand(string& cmd_line)
{
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)

  string cmd_s = _trim(cmd_line);
  if (cmd_s==""){
    return;
  }
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


std::priority_queue<JobsList::TimedJob,std::vector<JobsList::TimedJob>,TimeCompare>& SmallShell::getTimedJobs(){
  return _timedJobs;
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

void SmallShell::setLastWorkingDir(string& new_dir)
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
      *lastJobId = 0;
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


//TimeOut
TimeOutCommand::TimeOutCommand(std::string & cmd, std::string& to_execute,int time):Command(cmd){
  _to_execute = to_execute;
  _time = time;
}

/*
void print_heap(priority_queue<JobsList::TimedJob,std::vector<JobsList::TimedJob>,TimeCompare>& queue){
  auto queq
}
*/
void TimeOutCommand::execute(){

  int junk;
  int status;
  string cmd_s = _trim(_to_execute);
  //char *cmd = &cmd_s[0];
  pid_t id;
  DO_SYS_RET(fork(),id);
  if (id == 0)
  {
    setpgrp();
    _removeBackgroundSign(cmd_s);
    execl("/bin/bash", "bash","-c",cmd_s.c_str(), nullptr);

  }
  else
  {
    // add timeout jobEntry
    int jobid = 0;
    SmallShell::getInstance().getJobsList().getLastJob(&jobid);
    string orig_cmd_s = _trim(_original_cmd); // why trim this?
    JobsList::JobEntry job = JobsList::JobEntry(jobid+1,id,orig_cmd_s);
    auto entered_job = SmallShell::getInstance().getJobsList().getJobs().insert(std::pair<int,JobsList::JobEntry>(jobid+1,job));
    // set alarm
    int ret;
    /*
    if (!SmallShell::getInstance().getTimedJobs().empty()){
      if (_time< SmallShell::getInstance().getTimedJobs().top().getTimeLeft()){
        DO_SYS(alarm(_time),ret);
      }
    }
    else{
      DO_SYS(alarm(_time),ret);
    }
    */

    //add timed job
    JobsList::TimedJob timed_job = JobsList::TimedJob(jobid+1,id,_to_execute,_time);
    SmallShell::getInstance().getTimedJobs().push(timed_job);
    DO_SYS_RET(alarm(SmallShell::getInstance().getTimedJobs().top().getTimeLeftForTimer()),ret);
    //std::cout << "top timer " << SmallShell::getInstance().getTimedJobs().top().getTimeLeftForTimer() <<std::endl;
    //std::cout << "checking getTimedJobs heap: " << SmallShell::getInstance().getTimedJobs() << std::endl();


   
    //if last character is not &
    if (strcmp(cmd_s.substr(cmd_s.length() - 1, 1).c_str(), "&") != 0)
    {
      //run in foreground
      SmallShell::getInstance().getJobsList().setFgJob(&entered_job.first->second);
      DO_SYS_RET(waitpid(id, &status, WUNTRACED),junk);
      
      if (!WIFSTOPPED(status)){
        SmallShell::getInstance().getJobsList().getJobs().erase(jobid+1);
      }
      SmallShell::getInstance().getJobsList().setFgJob(nullptr);

    }
  }

}

// TimedJob 
JobsList::TimedJob::TimedJob(JobsList::JobEntry* base, int alarmTime){
  _alarm_time = alarmTime;
  _jobId = base->getId();
  _pid = base->get_pid();
  _command = base->getCommand();
  _timeMade = base->getTimeMade();
  _stopped = false;
}

  time_t JobsList::TimedJob::getTimeLeftForTimer(){
    return this->getAlarmTime()-difftime(time(nullptr),this->getTimeMade());        
  }
  time_t JobsList::TimedJob::getTimeLeftForTimer() const{
    return this->getAlarmTime()-difftime(time(nullptr),this->getTimeMade());        
  }


//jobEntry
JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  auto found = _jobs.find(jobId);
  return (found == _jobs.end()) ? nullptr : &(found->second);
}

string JobsList::JobEntry::getCommand(){
  return _command;
}

void JobsList::JobEntry::set_stopped(bool stopped){
  _stopped = stopped;
}
void JobsList::JobEntry::stopJob(){
  this->set_stopped(true);
  SmallShell::getInstance().getJobsList().getStoppedJobs().insert(make_pair(_jobId,this));


}
void JobsList::JobEntry::contJob(){
  this->set_stopped(false);
  SmallShell::getInstance().getJobsList().getStoppedJobs().erase(_jobId);

}

time_t JobsList::JobEntry::getTimeMade(){
  return _timeMade;
}
time_t JobsList::JobEntry::getTimeMade() const{
  return _timeMade;
}

//timedJob
int JobsList::TimedJob::getAlarmTime(){
  return _alarm_time;
}
int JobsList::TimedJob::getAlarmTime() const{
  return _alarm_time;
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
    *lastJobId = 0;
    return nullptr;
  }
}

JobsList::JobEntry::JobEntry(int id,pid_t pid, std::string& command)
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
Command::Command(string& cmd_line)
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
  SmallShell& smash = SmallShell::getInstance();
  std::cout << "smash pid is " << smash.getPid() << std::endl;
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
  if(_args_num == 1){
    return;
  }
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
  string string_buff = string(buff);

  SmallShell::getInstance().setLastWorkingDir(string_buff);
  free(buff);
}

void deleteFinishedJobs(){
  std::map<int, JobsList::JobEntry>* jobs = &SmallShell::getInstance().getJobsList().getJobs();
  int ret;
  int pid;
  int status;
  for (auto it = jobs->begin(),next_it=it; it != jobs->end(); it= next_it)
  {
    next_it++;
    pid=it->second.get_pid();
    DO_SYS_RET(waitpid(pid,&status,WNOHANG),ret);
  
    if(ret){
      SmallShell::getInstance().getJobsList().removeJobById(it->first);
    }
  }
}

// jobs
void JobsCommand::execute()
{
  std::map<int, JobsList::JobEntry>* jobs = &SmallShell::getInstance().getJobsList().getJobs();
  int status;
  int ret;
  int pid;
  for (auto it = jobs->begin(),next_it=it; it != jobs->end(); it= next_it)
  {
    next_it++;
    pid=it->second.get_pid();
    DO_SYS_RET(waitpid(pid,&status,WNOHANG),ret);
  
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
    DO_SYS_RET(kill(target->get_pid(),signum),ret);
    if (signum == SIGSTOP || signum == SIGTSTP ){
      target->stopJob();
    }
    if (signum == SIGCONT){
      target->contJob();
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
  int junk;
  int status;
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
      DO_SYS_RET(kill(last->second.get_pid(), SIGCONT),junk);
      SmallShell::getInstance().getJobsList().setFgJob(&last->second);
      waitpid(last->second.get_pid(), &status, WUNTRACED);
      if (!WIFSTOPPED(status)){
        jobsList->removeJobById(last->second.getId());
      }
      SmallShell::getInstance().getJobsList().setFgJob(nullptr);
    }
    return;
  }
  if (_args_num == 2)
  {
    try{
    auto found_job = jobs->find(stoi(_args[1]));
    

    if (found_job == jobs->end())
    {
      std::cerr << "smash error: fg: job-id " << _args[1] << " does not exist" << std::endl;
    }
    else
    {
      std::cout << found_job->second << std::endl;
      DO_SYS_RET(kill(found_job->second.get_pid(), SIGCONT),junk);
      SmallShell::getInstance().getJobsList().setFgJob(&found_job->second);
      waitpid(found_job->second.get_pid(), &status, WUNTRACED);
      if (!WIFSTOPPED(status)){
        jobsList->removeJobById(found_job->second.getId());
      }
      SmallShell::getInstance().getJobsList().setFgJob(nullptr);
    }
    }
    catch(const std::invalid_argument &){
      std::cerr << "smash error: fg: invalid arguments" << std::endl;
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

    std::cout << *target << std::endl;
    DO_SYS_RET(kill(target->get_pid(), SIGCONT),junk);
    target->contJob();
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
  exit(0);
}

//ExternalCommand
void ExternalCommand::execute(){
  int junk;
  int status;
  string cmd_s = _trim(_original_cmd);
  //char *cmd = &cmd_s[0];
  pid_t id;
  DO_SYS_RET(fork(),id);
  if (id == 0)
  {
    setpgrp();
    _removeBackgroundSign(cmd_s);
    execl("/bin/bash", "bash","-c",cmd_s.c_str(), nullptr);

  }
  else
  {
    int jobid = 0;
    SmallShell::getInstance().getJobsList().getLastJob(&jobid);
    JobsList::JobEntry job = JobsList::JobEntry(jobid+1,id,cmd_s);

    auto entered_job = SmallShell::getInstance().getJobsList().getJobs().insert(std::pair<int,JobsList::JobEntry>(jobid+1,job));
    //if last character is not &
    if (strcmp(cmd_s.substr(cmd_s.length() - 1, 1).c_str(), "&") != 0)
    {
      //run in foreground
      SmallShell::getInstance().getJobsList().setFgJob(&entered_job.first->second);
      DO_SYS_RET(waitpid(id, &status, WUNTRACED),junk);
      /*
      std::cout<<"WIFEXITED of proccess which finished waiting: " << WIFEXITED(status)<<std::endl;
      std::cout<<"WIFSIGNALED of proccess which finished waiting: " << WIFSIGNALED(status)<<std::endl;
      std::cout<<"WIFSTOPPED of proccess which finished waiting: " << WIFSTOPPED(status)<<std::endl;
      std::cout<<"WSTOPSIG of proccess which finished waiting: " << WSTOPSIG(status)<<std::endl;
      */

      if (!WIFSTOPPED(status)){
        SmallShell::getInstance().getJobsList().getJobs().erase(jobid+1);
      }
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
PipeCommand::~PipeCommand(){
  delete(_cmd1);
  delete(_cmd2);
}
void PipeCommand::execute(){
  int fd[2];
  int pipe_ret_value;
  DO_SYS_RET ( pipe(fd) , pipe_ret_value );
  //the above line is supposed to be a safer way doing the below commented block
  /*
  if (pipe(fd)==-1){
    perror("smash error: pipe failed");
    return;
  }
  */
  pid_t id;
  DO_SYS_RET(fork(),id);
  int junk_ret;// dummy argument for DO_SYS_RET
  //son
  if(id ==0){
    DO_SYS_RET(setpgrp(),junk_ret);
    DO_SYS_RET(close(fd[0]),junk_ret); // close read
    int oldOut;
    DO_SYS_RET(dup(_fdt_entry),oldOut);
    DO_SYS_RET(dup2(fd[1],_fdt_entry),junk_ret);// redirects output to stdout
    _cmd1->execute();
    DO_SYS_RET(dup2(oldOut,_fdt_entry),junk_ret); //restores output back
    exit(0);
  }
  //parent
  else{
    DO_SYS_RET(close(fd[1]),junk_ret);
    int oldIn;
    DO_SYS_RET(dup(0),oldIn);
    DO_SYS_RET(dup2(fd[0],0),junk_ret);
    DO_SYS_RET(waitpid(id,nullptr,0),junk_ret);
    _cmd2->execute();
    DO_SYS_RET(dup2(oldIn,0),junk_ret);
  }
  close(fd[0]);
  close(fd[1]);
}

//Redirection
//Redirection ctor
RedirectionCommand::RedirectionCommand(Command* cmd,const char* file_name,bool append):Command(){
  _cmd = cmd;
  if(append){
    DO_SYS_RET(open(file_name,O_RDWR | O_CREAT | O_APPEND,S_IRWXU),_fd);
  }
  else{
    DO_SYS_RET(open(file_name,O_RDWR | O_CREAT | O_TRUNC,S_IRWXU),_fd);
  }
}
//Redirection Dtor
RedirectionCommand::~RedirectionCommand(){
  //close(_fd);
  delete(_cmd);
}

void RedirectionCommand::execute(){
  // fd will have to be created somewhere in this class, and will be set to write or append.
  //this means the sys_open failed.
  if (_fd==-1){
    return;
  }
  int oldOut;
  int junk;
  DO_SYS_RET(dup(1),oldOut);
  DO_SYS_RET(dup2(_fd,1),junk);
  _cmd->execute();
  DO_SYS_RET(dup2(oldOut,1),junk);
}

//cat 

//writes the content of a file with the given fd to standard output
void print_file(int fd){
  size_t size_of_file= lseek(fd,0,SEEK_END);
  lseek(fd,0,SEEK_SET);
  char *buff=(char*)malloc(size_of_file*sizeof(char));
  if (read(fd,buff,size_of_file)==-1){
    perror("smash error: read failed");
  }
  else{
    if (write(1,buff,size_of_file)==-1){
      perror("smash error: write failed");
    }
  }
  free(buff);
}
void CatCommand::execute(){
  if (_args_num == 1){
    std::cerr<<"smash error: cat: not enough arguments"<<std::endl;
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
