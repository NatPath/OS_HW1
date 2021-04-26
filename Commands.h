#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>
#include <string.h>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define _GLIBCXX_USE_CXX11_ABI 0

class Command {
// TODO: Add your data members
protected:
std::vector<std::string> _args;
int _args_num;

 public:
  Command(const char* cmd_line);
  virtual ~Command(){};
  virtual void execute() = 0;
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
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};


class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
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


class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
  QuitCommand(const char* cmd_line, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};




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
   JobEntry(int id,std::string command);
   int getId();
   void setId(int id);
   pid_t get_pid() const;
   void printJob();
   void stopJob();
   void proceedJob();
   friend std::ostream& operator<<(std::ostream& os,const JobEntry& job);
  };
 // TODO: Add your data members
 std::map<int,JobEntry> _jobs;
 public:
  JobsList() = default;
  ~JobsList(){}
  void addJob(Command* cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry * getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry * getLastJob(int* lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);
  std::map<int,JobEntry>& getJobs();
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
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
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
  CatCommand(const char* cmd_line);
  virtual ~CatCommand() {}
  void execute() override;
};


class SmallShell {
 private:
  // TODO: Add your data members
  std::string _prompt_name ;
  std::string _last_working_dir;
  JobsList _jobsList;
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
  
};

#endif //SMASH_COMMAND_H_
