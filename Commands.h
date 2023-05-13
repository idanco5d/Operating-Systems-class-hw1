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
    pid_t pid;
 public:
  Command(string cmd_line, pid_t pid = 0);
  virtual ~Command();
  virtual void execute() = 0;
  string getCmdLine() const;
  void setPid(pid_t pid);
  pid_t getPid() const;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(string cmd_line, pid_t pid = 0);
  virtual ~BuiltInCommand()=default;
};

class ExternalCommand : public Command {
 public:
  ExternalCommand(string cmd_line);
  virtual ~ExternalCommand() =default;
  void execute() override;
  //to delete!!!
//  void printExtCmd() const;
  //not to delete
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(string cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(string cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
  ChangeDirCommand(string cmd_line);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(string cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(string cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members
private:
    JobsList* jobs;
public:
  QuitCommand(string cmd_line, JobsList* jobs);
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
   //shared_ptr<Command> cmd;
  public:
     JobEntry(int jobId, pid_t jobPid, time_t timeCreated, string cmd_line);
     ~JobEntry();
     int getJobId() const;
     string getCmdLine() const;
     pid_t getJobPid() const;
     time_t getTimeCreated() const;
     JobStatus getJobStatus() const;
     void printJobCmd() const;
     void setJobStatus(JobStatus status);
     //shared_ptr<Command> getCmd() const;
      //to delete!!!!!
//      void printJob() const;
      //not to delete
  };
 // TODO: Add your data members
private:
    std::vector<shared_ptr<JobEntry>> job_list;
    int maximalJobId;
    std::vector<shared_ptr<JobEntry>> stoppedJobs;
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
  shared_ptr<JobEntry> getLastStoppedJob(int *jobId);
  bool isListEmpty() const;
  void resumeJob(shared_ptr<JobEntry> job, bool toCont = true);
  bool isJobInTheList(int job_id);
  std::vector<shared_ptr<JobEntry>>& get_Job_List();
  void print_jobs_for_quit_command();
  void initializeMaximalJobId();
  bool isCmdInList(shared_ptr<Command> cmd) const;
  shared_ptr<JobEntry> getJobByCmd(shared_ptr<Command> cmd);
  void stopJob(shared_ptr<JobEntry> jobToStop);
  void removeFinishedStoppedJobs();
  // TODO: Add extra methods or modify exisitng ones as needed
    shared_ptr<JobsList::JobEntry> getJobByPid(pid_t pid);
};

class JobsCommand : public BuiltInCommand {
 // TODO: Add your data members
 string cmd_line;
 JobsList* job_list;
 public:
  JobsCommand(string cmd_line, JobsList* jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* job_list;
 public:
  ForegroundCommand(string cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* job_list;
 public:
  BackgroundCommand(string cmd_line, JobsList* jobs);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
/* Bonus */
// TODO: Add your data members
 public:
  explicit TimeoutCommand(string cmd_line);
  virtual ~TimeoutCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  ChmodCommand(string cmd_line);
  virtual ~ChmodCommand() {}
  void execute() override;
};

class GetFileTypeCommand : public BuiltInCommand {
  // TODO: Add your data members
 public:
  GetFileTypeCommand(string cmd_line);
  virtual ~GetFileTypeCommand() {}
  void execute() override;
};

class SetcoreCommand : public BuiltInCommand {
// TODO: Add your data members
    JobsList* jobs;
public:
    SetcoreCommand(string cmd_line,JobsList* jobs );
    virtual ~SetcoreCommand() {}
    void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 JobsList* jobs;
 public:
  KillCommand(string cmd_line, JobsList* jobs);
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
  void setShellName(string newName);
  string getShellName();
  pid_t getForegroundProcessPid() const;
  void addJobToShell(pid_t pid, string cmd_line, bool isStopped=false);
  void setCmdForeground(pid_t pid, string cmd_line);
  void handleOneCommand(shared_ptr<Command> cmd, string cmd_line);
  void handleBGCommand(shared_ptr<Command> cmd, string cmd_line);
  void handleExternalCommand(shared_ptr<Command> cmd, string cmd_line);
  void handlePipeIoCommand(string cmd_line, int pipeIOIndex, string pipeIOString, bool isTwoCharsPipeIO);
  void handleIoCommand(string first_cmd, string second_cmd, bool isTwoCharsPipeIO);
  void handlePipeCommand(string first_cmd, string second_cmd, bool isTwoCharsPipeIO);
  void stopJobInShell(shared_ptr<JobsList::JobEntry> job);
  void resumeJobInShell(shared_ptr<JobsList::JobEntry> job, bool toCont = true);
  const string& getPrevDir() const;
  void setPrevDir(string newDir);
  string getForegroundProcessCmdLine() const;
};

class QuitException : public std::exception {};

#endif //SMASH_COMMAND_H_
