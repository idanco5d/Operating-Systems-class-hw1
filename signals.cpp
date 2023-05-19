#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <unistd.h>
#include <sys/wait.h>
#include <cmath>


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
            SmallShell::getInstance().setCmdForeground(0,"");
        }
    }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
  SmallShell::getInstance().getJobsList().removeFinishedJobs();
  std::vector<JobsList::TimeoutEntry>& timedJobs = SmallShell::getInstance().getJobsList().getTimeoutEntries();
  time_t now = CHECK_SYSCALL_AND_GET_VALUE_RVOID(time(nullptr), time, now);
  std::cout << "smash: got an alarm" << std::endl;
  for (auto & process : timedJobs) {
      double time_passed = CHECK_SYSCALL_AND_GET_VALUE_RVOID(difftime(now, process.timeCreated),difftime, time_passed);
      if (time_passed >= process.timer) {
          pid_t waitReturnValue = CHECK_SYSCALL_AND_GET_VALUE_RVOID(waitpid(process.pid,nullptr,WNOHANG),waitpid,waitReturnValue);
          if (waitReturnValue == 0) {
              std::cout << "smash: " + process.cmd_line + " timed out!" << std::endl;
              CHECK_SYSCALL(kill(process.pid,SIGKILL),kill);
              SmallShell::getInstance().removeJobFromShellByPid(process.pid);
              SmallShell::getInstance().removeTimedJobByPidFromShell(process.pid);
              break;
          }
      }
  }
  if (SmallShell::getInstance().areThereTimedJobsInShell()) {
      SmallShell::getInstance().setClosestAlarmInShell();
  }
}

