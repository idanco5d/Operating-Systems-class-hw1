#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <unistd.h>
#include <sys/wait.h>


#define CHECK_SYSCALL_AND_GET_VALUE_RVOID( syscall, syscall_name, var_name) \
syscall;                                                               \
if (var_name == -1) {                                                  \
    perror( "smash error: " #syscall_name " failed" );                         \
    return;\
 }

#define CHECK_SYSCALL( syscall, syscall_name ) do { \
/* safely invoke a system call */ \
if( (syscall) == -1 ) { \
perror( "smash error: " #syscall_name " failed" ); \
} \
} while( 0 )


using namespace std;

void ctrlZHandler(int sig_num) {
	// TODO: Add your implementation
    std::cout << "smash: got ctrl-Z" << std::endl;
    pid_t cmdPid = SmallShell::getInstance().getForegroundProcessPid();
    if (cmdPid != 0) {
        pid_t waitReturnValue = CHECK_SYSCALL_AND_GET_VALUE_RVOID(waitpid(cmdPid,nullptr,WNOHANG),waitpid,waitReturnValue);
        if (waitReturnValue == 0) {
            CHECK_SYSCALL(kill(cmdPid,SIGSTOP),kill);
            SmallShell::getInstance().addJobToShell(cmdPid,SmallShell::getInstance().getForegroundProcessCmdLine(),true);
            SmallShell::getInstance().setCmdForeground(0,"");
            std::cout << "smash: process " << cmdPid << " was stopped" << std::endl;
        }
    }
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    std::cout << "smash: got ctrl-C" << std::endl;
    pid_t cmdPid = SmallShell::getInstance().getForegroundProcessPid();
    if (cmdPid != 0) {
        pid_t waitReturnValue = CHECK_SYSCALL_AND_GET_VALUE_RVOID(waitpid(cmdPid,nullptr,WNOHANG),waitpid,waitReturnValue);
        if (waitReturnValue == 0) {
            CHECK_SYSCALL(kill(cmdPid,SIGKILL),kill);
            std::cout << "smash: process "+ to_string(cmdPid) + " was killed" << std::endl;
            //delete cmd;
            SmallShell::getInstance().setCmdForeground(0,"");
        }
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

