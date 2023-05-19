#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <string.h>
#include <list>
#include <vector>
#include <memory>

typedef enum e_JobStatus {STOPPED, RUNNING} JobStatus;

using std::string;
using std::list;
using std::shared_ptr;

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class Command {
// TODO: Add your data members
protected:
    string cmd_line;
    std::vector<string> cmd_split;
    pid_t pid;
    bool isTimedOut;
    int timer;
 public:
    std::string timeoutCmdLine;
    time_t alarmCreatedAt;
  Command(string cmd_line, std::vector<string> cmd_split, pid_t pid = 0, bool isTimedOut = false, int timer = 0, std::string timeoutCmdLine = "", time_t alarmCreatedAt = 0);
  virtual ~Command();
  virtual void execute() = 0;
  pid_t getPid() const;
  void toBeTimedOut();
  void setTimer(int timer);
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(string cmd_line, std::vector<string> cmd_split, pid_t pid = 0);
  virtual ~BuiltInCommand()=default;
};

class ExternalCommand : public Command {

 public:
  ExternalCommand(string cmd_line, std::vector<string> cmd_split);
  virtual ~ExternalCommand() =default;
  void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  ChangeDirCommand(string cmd_line, std::vector<string> cmd_split);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(string cmd_line, std::vector<string> cmd_split);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(string cmd_line, std::vector<string> cmd_split);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
private:
public:
  QuitCommand(string cmd_line, std::vector<string> cmd_split);
  virtual ~QuitCommand() {}
  void execute() override;
};


class JobsList {
private:

 public:
  class JobEntry {
   // TODO: Add your data members
   JobStatus status;
   int jobId;
   pid_t jobPid;
   time_t timeCreated;
   string cmd_line;
  public:
     JobEntry(int jobId, pid_t jobPid, string cmd_line);
     ~JobEntry();
     int getJobId() const;
     string getCmdLine() const;
     pid_t getJobPid() const;
     time_t getTimeCreated() const;
     JobStatus getJobStatus() const;
     void printJobCmd() const;
     void setJobStatus(JobStatus status);
  };
 class TimeoutEntry {
 public:
     pid_t pid;
     time_t timeCreated;
     int timer;
     string cmd_line;
     TimeoutEntry(pid_t pid, time_t timeCreated, int timer, string cmd_line);
     ~TimeoutEntry()=default;
 };
 // TODO: Add your data members
private:
    std::vector<shared_ptr<JobEntry>> job_list;
    int maximalJobId;
    std::vector<TimeoutEntry> timed_jobs;
public:
  JobsList();
  ~JobsList()=default;
  void addJob(pid_t pid, string cmd_line, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  shared_ptr<JobEntry>  getJobById(int jobId);
  void removeJobById(int jobId);
  shared_ptr<JobEntry>  getLastJob(int* lastJobId);
  shared_ptr<JobEntry> getLastStoppedJob();
  bool isListEmpty() const;
  void resumeJob(shared_ptr<JobEntry> job, bool toCont = true);
  std::vector<shared_ptr<JobEntry>>& get_Job_List();
  void print_jobs_for_quit_command();
  void initializeMaximalJobId();
  void stopJob(shared_ptr<JobEntry> jobToStop);
  void addTimeoutJob(pid_t pid, time_t timeCreated, int timer, string cmd_line);
  std::vector<TimeoutEntry>& getTimeoutEntries();
  void removeTimedJobByPid(pid_t pid);
  // TODO: Add extra methods or modify exisitng ones as needed
    shared_ptr<JobsList::JobEntry> getJobByPid(pid_t pid);
    bool areThereTimedJobs() const;
    void setClosestAlarm() const;
    void removeJobByPid(pid_t pid);
    double getMinTimeoutLeft() const;
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  JobsCommand(string cmd_line, std::vector<string> cmd_split);
  virtual ~JobsCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(string cmd_line, std::vector<string> cmd_split);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  BackgroundCommand(string cmd_line, std::vector<string> cmd_split);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Bonus */
// TODO: Add your data members
 public:
  explicit TimeoutCommand(string cmd_line, std::vector<string> cmd_split);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  ChmodCommand(string cmd_line, std::vector<string> cmd_split);
  virtual ~ChmodCommand() {}
  void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  GetFileTypeCommand(string cmd_line, std::vector<string> cmd_split);
  virtual ~GetFileTypeCommand() {}
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
// TODO: Add your data members
public:
    SetcoreCommand(string cmd_line, std::vector<string> cmd_split);
    virtual ~SetcoreCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(string cmd_line, std::vector<string> cmd_split);
  virtual ~KillCommand() {}
  void execute() override;
};

class SmallShell {
 private:
  // TODO: Add your data members
  string shellName;
  pid_t cmdForegroundPid;
  string cmdForegroundCmdLine;
  SmallShell();
  JobsList jobs_list;
  string prevDir;
 public:
  shared_ptr<Command> CreateCommand(const string& cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell()=default;
  void executeCommand(string cmd_line);
  // TODO: add extra methods as needed
  void setShellName(const string& newName);
  string getShellName();
  pid_t getForegroundProcessPid() const;
  void addJobToShell(pid_t pid, string cmd_line, bool isStopped=false);
  void setCmdForeground(pid_t pid, const string& cmd_line);
  void handleOneCommand(shared_ptr<Command> cmd, string cmd_line);
  void handlePipeIoCommand(string cmd_line, int pipeIOIndex, string pipeIOString, bool isTwoCharsPipeIO);
  void handleIoCommand(string first_cmd, string second_cmd, bool isTwoCharsPipeIO);
  void handlePipeCommand(string first_cmd, string second_cmd, bool isTwoCharsPipeIO);
  void stopJobInShell(shared_ptr<JobsList::JobEntry> job);
  void resumeJobInShell(shared_ptr<JobsList::JobEntry> job, bool toCont = true);
  const string& getPrevDir() const;
  void setPrevDir(string newDir);
  string getForegroundProcessCmdLine() const;
  JobsList& getJobsList();
  bool areThereTimedJobsInShell() const;
  void setClosestAlarmInShell() const;
  void removeTimedJobByPidFromShell(pid_t pid);
  void removeJobFromShellByPid(pid_t pid);
  double getMinTimeoutLeftFromShell() const;
};

class QuitException : public std::exception {};

#endif //SMASH_COMMAND_H_
