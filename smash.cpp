#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"
#include <sys/stat.h>
 #include <fcntl.h>
#define _GLIBCXX_USE_CXX11_ABI 0

int main(int argc, char* argv[]) {
    if(signal(SIGTSTP , ctrlZHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if(signal(SIGINT , ctrlCHandler)==SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    //TODO: setup sig alarm handler
    struct sigaction act;
    act.sa_flags = SA_RESTART;
    act.sa_handler = alarmHandler;
    if (sigaction(SIGALRM, &act, NULL) < 0) {
		perror("smash error: failed to set alarm handler");
	}

    SmallShell& smash = SmallShell::getInstance();
   

    while(true) {
        //std::cout << "smash> ";
        std::cout << smash.getPromptName()<<"> ";
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        deleteFinishedJobs();
        smash.executeCommand(cmd_line);
    }
    return 0;
}