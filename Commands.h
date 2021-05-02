#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#include <string.h>
#include <unistd.h>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)


class Command {
// TODO: Add your data members
protected:
const char* _original_cmd;
std::vector<std::string> _args;
int _args_num;

 public:
 // command(const char* cmd_line,std::streambuf* output = std::cout.rdbuf() );
  Command(){};
  Command(const char* cmd_line);
  virtual ~Command(){};
  virtual void execute() = 0;
  void executeWrapper();
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line):Command(cmd_line){};
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(const char* cmd_line):Command(cmd_line){};
  virtual ~ExternalCommand() {}
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
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 int _fd;
 Command* _cmd;
 public:
  explicit RedirectionCommand(Command* cmd,const char * file_name,bool append);
  virtual ~RedirectionCommand() {close(_fd);}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangePromptCommand : public BuiltInCommand {  
  public:
  ChangePromptCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
  virtual ~ChangePromptCommand() {}
  void execute() override;
};

class ShowPIDCommand : public BuiltInCommand {  
  public:
  ShowPIDCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
  virtual ~ShowPIDCommand() {}
  void execute() override;
};

class PwdCommand : public BuiltInCommand {  
  public:
  PwdCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
  virtual ~PwdCommand() {}
  void execute() override;
};

class CdCommand : public BuiltInCommand {  
  public:
  CdCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
  virtual ~CdCommand() {}
  void execute() override;
};

class JobsCommand : public BuiltInCommand {  
  public:
  JobsCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
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
  QuitCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
  virtual ~QuitCommand() {}
  void execute() override;
};



class JobsList;
class JobsList {
 public:
  class JobEntry {
   // TODO: Add your data members
   int _jobId;
   pid_t _pid;
   time_t _timeMade;
   std::string _command;
   bool _stopped;

   public:
   JobEntry(int id,pid_t pid,std::string command);
   int getId();
   void setId(int id);
   pid_t get_pid() const;
   void set_stopped(bool stopped);
   void printJob();
   void stopJob();
   void die();
   void proceedJob();
   friend std::ostream& operator<<(std::ostream& os,const JobEntry& job);
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
  void addJob(const char * cmd_line,pid_t pid, bool isStopped = false);
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
  KillCommand(const char* cmd_line):BuiltInCommand(cmd_line){}
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  //ForegroundCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line);
  ForegroundCommand(const char* cmd_line) : BuiltInCommand(cmd_line){};
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class CatCommand : public BuiltInCommand {
 public:
  CatCommand(const char* cmd_line):BuiltInCommand(cmd_line){};
  virtual ~CatCommand() {}
  void execute() override;
};


class SmallShell {
 private:
  // TODO: Add your data members
  std::string _prompt_name ;
  std::string _last_working_dir;
  JobsList _jobsList;
  JobsList::JobEntry* _fg_job;
  SmallShell();
 public:
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
  std::string getPromptName();
  void setPromptName(std::string new_name);
  std::string getLastWorkingDir();
  void setLastWorkingDir(std::string new_dir);
  JobsList& getJobsList();
  JobsList::JobEntry* getFgJob();
  void killFg();
  void stopFg();
  
};

#endif //SMASH_COMMAND_H_
