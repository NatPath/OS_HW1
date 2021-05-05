#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#include <string.h>
#include <unistd.h>
#include <queue>
#include <functional>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)


class Command {
// TODO: Add your data members
protected:
std::string _original_cmd;
std::vector<std::string> _args;
int _args_num;

 public:
 // command(const char* cmd_line,std::streambuf* output = std::cout.rdbuf() );
  Command(){};
  Command(std::string& cmd_line);
  virtual ~Command(){
  }
  virtual void execute() = 0;
  std::string getOriginalCommand();
  void executeWrapper();
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(std::string& cmd_line):Command(cmd_line){};
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(std::string& cmd_line):Command(cmd_line){};
  virtual ~ExternalCommand() {}
  void execute() override;
};

class TimeOutCommand : public Command {
  std::string _to_execute;
  int _time;

  public:
  TimeOutCommand(std::string& cmd, std::string& _to_execute, int time);
  virtual ~TimeOutCommand() {}
  void execute() override;
};


class PipeCommand : public Command {
  // TODO: Add your data members
 private:
 Command* _cmd1;
 Command* _cmd2;
 int _fdt_entry;
 public:
 //maybe other arguments
  PipeCommand(Command* command1, Command* command2, bool err);
  virtual ~PipeCommand();
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 int _fd;
 Command* _cmd;
 public:
  explicit RedirectionCommand(Command* cmd,const char * file_name,bool append);
  virtual ~RedirectionCommand();
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangePromptCommand : public BuiltInCommand {  
  public:
  ChangePromptCommand(std::string& cmd_line):BuiltInCommand(cmd_line){}
  virtual ~ChangePromptCommand() {}
  void execute() override;
};

class ShowPIDCommand : public BuiltInCommand {  
  public:
  ShowPIDCommand(std::string& cmd_line):BuiltInCommand(cmd_line){}
  virtual ~ShowPIDCommand() {}
  void execute() override;
};

class PwdCommand : public BuiltInCommand {  
  public:
  PwdCommand(std::string& cmd_line):BuiltInCommand(cmd_line){}
  virtual ~PwdCommand() {}
  void execute() override;
};

class CdCommand : public BuiltInCommand {  
  public:
  CdCommand(std::string& cmd_line):BuiltInCommand(cmd_line){}
  virtual ~CdCommand() {}
  void execute() override;
};

class JobsCommand : public BuiltInCommand {  
  public:
  JobsCommand(std::string& cmd_line):BuiltInCommand(cmd_line){}
  virtual ~JobsCommand() {}
  void execute() override;
};

/*
class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
  ChangeDirCommand(const char* cmd_line, char** plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};
*/



class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
  public:
  QuitCommand(std::string& cmd_line) : BuiltInCommand(cmd_line){};
  virtual ~QuitCommand() {}
  void execute() override;
};



class JobsList;
class JobsList {
 public:
  class JobEntry {
   // TODO: Add your data members
   protected:
   int _jobId;
   pid_t _pid;
   time_t _timeMade;
   std::string _command;
   bool _stopped;

   public:
   JobEntry(){}
   JobEntry(int id,pid_t pid,std::string& command);
   int getId();
   void setId(int id);
   pid_t get_pid() const;
   void set_stopped(bool stopped);
   void printJob();
   void stopJob();
   void contJob();
   void die();
   void proceedJob();
   time_t getTimeMade();
   time_t getTimeMade() const;
   std::string getCommand();
   friend std::ostream& operator<<(std::ostream& os,const JobEntry& job);
  };

  class TimedJob: public JobEntry{
    int _alarm_time;
    public:
    TimedJob(int id,pid_t pid,std::string& command, int alarm_time):JobEntry(id, pid,command){_alarm_time = alarm_time;};
    TimedJob(JobEntry* base, int alarm_time);
    int getAlarmTime();
    int getAlarmTime() const;
  };
 // TODO: Add your data members
 private:
 JobEntry* _fg_job;
 std::map<int,JobEntry> _jobs;
 std::map<int,JobEntry*> _stopped_jobs;
 public:
  JobsList() = default;
  ~JobsList(){}
  //void addJob(Command* cmd, bool isStopped = false);
  void addJob(std::string& cmd_line,pid_t pid, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *lastJobId);
  JobEntry* getFgJob();
  std::map<int,JobEntry>& getJobs();
  std::map<int,JobEntry*>& getStoppedJobs();
  void setFgJob(JobsList::JobEntry* fg_job);
  // TODO: Add extra methods or modify exisitng ones as needed
};




class KillCommand : public BuiltInCommand {  
  public:
  KillCommand(std::string& cmd_line):BuiltInCommand(cmd_line){}
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  //ForegroundCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line);
  ForegroundCommand(std::string& cmd_line) : BuiltInCommand(cmd_line){};
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(std::string& cmd_line):BuiltInCommand(cmd_line){};
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class CatCommand : public BuiltInCommand {
 public:
  CatCommand(std::string& cmd_line):BuiltInCommand(cmd_line){};
  virtual ~CatCommand() {}
  void execute() override;
};


class TimeCompare{
  public:
  bool operator() (JobsList::TimedJob&, JobsList::TimedJob&);
};






class SmallShell {
 private:
  // TODO: Add your data members
  pid_t _pid;
  std::string _prompt_name ;
  std::string _last_working_dir;
  JobsList _jobsList;
  JobsList::JobEntry* _fg_job;
  std::priority_queue<JobsList::TimedJob,std::vector<JobsList::TimedJob>,TimeCompare> _timedJobs;
  SmallShell();
 public:
  Command *CreateCommand(std::string& cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(std::string& cmd_line);
  // TODO: add extra methods as needed
  std::string getPromptName();
  void setPromptName(std::string new_name);
  std::string getLastWorkingDir();
  void setLastWorkingDir(std::string& new_dir);
  JobsList& getJobsList();
  JobsList::JobEntry* getFgJob();
  std::priority_queue<JobsList::TimedJob,std::vector<JobsList::TimedJob>,TimeCompare>& getTimedJobs();
  pid_t getPid();
  void killFg();
  void stopFg();
  
};
void deleteFinishedJobs();

#endif //SMASH_COMMAND_H_
