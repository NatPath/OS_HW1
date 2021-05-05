#include <iostream>
#include <signal.h>
#include "signals.h"
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
  JobsList::TimedJob job = SmallShell::getInstance().getTimedJobs().top();
  SmallShell::getInstance().getTimedJobs().pop();
  kill(job.get_pid(),SIGKILL);
  std::cout<<"smash: "<<job.getCommand()<<" timed out!";
}

