#include <iostream>
#include <signal.h>
#include "signals.h"
#include <sys/wait.h>
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
  //std::cin.ignore();
  SmallShell::getInstance().stopFg();
  //std::cin.ignore();
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
  //std::cin.ignore();
  SmallShell::getInstance().killFg();
  //std::cin.ignore();
  
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
  std::cout<<"smash: got an alarm"<<std::endl;
  JobsList::TimedJob job = SmallShell::getInstance().getTimedJobs().top();
  if (!waitpid(job.get_pid(), nullptr, WNOHANG))
  {
    SmallShell::getInstance().getTimedJobs().pop();
    kill(job.get_pid(), SIGKILL);
    std::cout << "smash: " << job.getCommand() << " timed out!"<<std::endl;
    if (!SmallShell::getInstance().getTimedJobs().empty()){
      int top_original_alarm_time = SmallShell::getInstance().getTimedJobs().top().getAlarmTime();
      int top_already_passed_time = time(nullptr)-SmallShell::getInstance().getTimedJobs().top().getTimeMade();
      alarm(top_original_alarm_time-top_already_passed_time);
    }
    
  }
}

