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

    SmallShell& shell = SmallShell::getInstance();
    std::cout << "smash: got ctrl-Z" << std::endl;
    shared_ptr<Command> cmd = shell.getCmdForeground();
    if (cmd) {
        pid_t waitReturnValue = CHECK_SYSCALL_AND_GET_VALUE_RVOID(waitpid(cmd->getPid(),nullptr,WNOHANG),waitpid,waitReturnValue);
        if (waitReturnValue == 0) {
            CHECK_SYSCALL(kill(cmd->getPid(),SIGSTOP),kill);
            std::cout << "smash: process "+ to_string(cmd->getPid()) + " was stopped" << std::endl;
            cmd->setPid(shell.getForegroundProcessPid());
            shell.addJobToShell(cmd,true);
            shell.setCmdForeground(nullptr);
        }
    }
}

void ctrlCHandler(int sig_num) {
  // TODO: Add your implementation
    SmallShell& shell = SmallShell::getInstance();
    std::cout << "smash: got ctrl-C" << std::endl;
    shared_ptr<Command> cmd = shell.getCmdForeground();
    if (cmd) {
        pid_t waitReturnValue = CHECK_SYSCALL_AND_GET_VALUE_RVOID(waitpid(cmd->getPid(),nullptr,WNOHANG),waitpid,waitReturnValue);
        if (waitReturnValue == 0) {
            CHECK_SYSCALL(kill(shell.getForegroundProcessPid(),SIGKILL),kill);
            std::cout << "smash: process "+ to_string(cmd->getPid()) + " was killed" << std::endl;
            //delete cmd;
            shell.setCmdForeground(nullptr);
        }
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

