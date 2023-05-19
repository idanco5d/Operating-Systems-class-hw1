#include <unistd.h>
#include <string.h>
#include <iostream>
#include <memory>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include "Commands.h"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iomanip>
#include <unistd.h>
#include <cmath>

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define CHECK_SYSCALL( syscall, syscall_name ) do { \
/* safely invoke a system call */ \
if( (syscall) == -1 ) { \
perror( "smash error: " #syscall_name " failed" );  \
return;\
} \
} while( 0 )

#define CHECK_SYSCALL_PTRS( syscall, syscall_name ) do { \
/* safely invoke a system call */ \
if( (syscall) == -1 ) { \
perror( "smash error: " #syscall_name " failed" );          \
return nullptr;                                                         \
} \
} while( 0 ) \


#define CHECK_SYSCALL_AND_GET_VALUE_RVOID( syscall, syscall_name, var_name) \
syscall;                                                               \
if (var_name == -1) {                                                  \
    perror( "smash error: " #syscall_name " failed" );                         \
    return;\
 }

#define CHECK_SYSCALL_AND_GET_VALUE_RVOID_PTRS( syscall, syscall_name, var_name) \
syscall;                                                               \
if (!var_name) {                                                  \
    perror( "smash error: " #syscall_name " failed" );                         \
    return;\
 }



//////////////////////////////////// COURSE STAFF FUNCTIONS ////////////////////////////////////

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(string& str) {
  // find last character other than spaces
  size_t idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (str[idx] != '&') {
    return;
  }
  str = str.substr(0,idx);
  str = _trim(str);
}

//////////////////////////////////// END OF COURSE STAFF FUNCTIONS ////////////////////////////////////

//////////////////////////////////// HELPER FUNCTIONS SECTION ////////////////////////////////////

bool isDigit(char c) {
    if (c>='0' && c<='9') {
        return true;
    }
    return false;
}

bool isNumber(string str) {
    for (unsigned int i = 0; i < str.length(); i++) {
        if ((!isdigit(str[i]) && i > 0) || (str[i] != '-' && !isDigit(str[i]))) {
            return false;
        }
    }
    return true;
}

std::vector<string> splitStringIntoWords(const string& str)
{
    std::vector<string> words;
    std::istringstream iss(str);
    string word;
    while (iss >> word)
    {
        words.push_back(word);
    }
    return words;
}

//////////////////////////// END OF HELPER FUNCTIONS SECTION ////////////////////////////

// TODO: Add your implementation for classes in Commands.h

/////////////////////////// SMALLSHELL CLASS SECTION ///////////////////////////

SmallShell::SmallShell() : shellName("smash"), cmdForegroundPid(0), jobs_list(){
// TODO: add your implementation
}

void SmallShell::setShellName(const string& newName) {
    this->shellName = newName;
}

void SmallShell::addJobToShell(pid_t pid, string cmd_line, bool isStopped) {
    jobs_list.addJob(pid, cmd_line, isStopped);
}

void SmallShell::setCmdForeground(pid_t pid, const string& cmd_line) {
    cmdForegroundPid = pid;
    cmdForegroundCmdLine = std::move(cmd_line);
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
shared_ptr<Command> SmallShell::CreateCommand(const string& cmd_line) {
  string cmd_line_for_builtin = cmd_line;
  _removeBackgroundSign(cmd_line_for_builtin);
  std::vector<string> cmd_words = splitStringIntoWords(cmd_line_for_builtin);
  if (cmd_words[0] == "chprompt") {
      if (cmd_words.size() == 1) {
          setShellName("smash");
      }
      else {
          setShellName(cmd_words[1]);
      }
      return nullptr;
  }
  else if (cmd_words[0] == "pwd") {
    return std::make_shared<GetCurrDirCommand>(cmd_line_for_builtin, cmd_words);
  }
  else if (cmd_words[0] == "showpid") {
    return std::make_shared<ShowPidCommand>(cmd_line_for_builtin, cmd_words);
  }
  else if (cmd_words[0] == "cd") {
      if (cmd_words.size() > 2) {
          std::cerr << "smash error: cd: too many arguments" << std::endl;
          return nullptr;
      }
      return std::make_shared<ChangeDirCommand>(cmd_line_for_builtin, cmd_words);
  }
  else if (cmd_words[0] == "jobs") {
      return std::make_shared<JobsCommand>(cmd_line_for_builtin, cmd_words);
  }
  else if (cmd_words[0] == "fg") {
      return std::make_shared<ForegroundCommand>(cmd_line_for_builtin, cmd_words);
  }
  else if (cmd_words[0] == "bg") {
      return std::make_shared<BackgroundCommand>(cmd_line_for_builtin, cmd_words);
  }
  else if (cmd_words[0] == "quit") {
      return std::make_shared<QuitCommand>(cmd_line_for_builtin, cmd_words);
  }
  else if (cmd_words[0] == "kill") {
      return std::make_shared<KillCommand>(cmd_line_for_builtin, cmd_words);
  }
  else if (cmd_words[0] == "setcore") {
      return std::make_shared<SetcoreCommand>(cmd_line, cmd_words);
  }
  else if (cmd_words[0] == "getfileinfo") {
      return std::make_shared<GetFileTypeCommand>(cmd_line, cmd_words);
  }
  else if (cmd_words[0] == "chmod") {
      return std::make_shared<ChmodCommand>(cmd_line, cmd_words);
  }
  else if (cmd_words[0] == "timeout") {
      std::vector<string> cmd_words_for_timeout = splitStringIntoWords(cmd_line);
      return std::make_shared<TimeoutCommand>(cmd_line, cmd_words_for_timeout);
  }
  else {
      return std::make_shared<ExternalCommand>(cmd_line, cmd_words);
  }
  return nullptr;
}

string pipeIOStringInCommand(string cmd_line, bool* isTwoCharsCommand, int* pipeIOIndex) {
    string pipeIOString;
    if (!isTwoCharsCommand || !pipeIOIndex) {
        return pipeIOString;
    }
    for (unsigned int i = 0; i < cmd_line.length(); i++) {
        if (i == cmd_line.length()-1) {
            break;
        }
        string currentTwoChars;
        currentTwoChars = string(1,cmd_line[i]) + cmd_line[i+1];
        if (currentTwoChars == ">>" || currentTwoChars == "|&") {
            *pipeIOIndex = i;
            *isTwoCharsCommand = true;
            pipeIOString = currentTwoChars;
            break;
        }
        if (cmd_line[i] == '>' || cmd_line[i] == '|') {
            *pipeIOIndex = i;
            *isTwoCharsCommand = false;
            pipeIOString = cmd_line[i];
            break;
        }
    }
    return pipeIOString;
}

void SmallShell::handleOneCommand(shared_ptr<Command> cmd, string cmd_line) {
    if (!cmd) {
        return;
    }
    cmd->execute();
}

void SmallShell::handleIoCommand(string first_cmd, string second_cmd, bool isTwoCharsPipeIO) {
    int stdout_dup = CHECK_SYSCALL_AND_GET_VALUE_RVOID(dup(STDOUT_FILENO),dup,stdout_dup);
    CHECK_SYSCALL(close(STDOUT_FILENO),close);
    int flags;
    //append file case
    if (isTwoCharsPipeIO) {
        flags = O_APPEND | O_CREAT | O_WRONLY;
    }
    else {
        flags = O_CREAT | O_TRUNC | O_WRONLY;
    }
    int fd = open(_trim(second_cmd).c_str(),flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
    if (fd == -1) {
        perror("smash error: open failed");
        CHECK_SYSCALL(dup2(stdout_dup,STDOUT_FILENO),dup2);
        CHECK_SYSCALL(close(stdout_dup),close);
        return;
    }
    //handle the command and re-open stdout
    shared_ptr<Command> cmd = CreateCommand(first_cmd.c_str());
    handleOneCommand(cmd,first_cmd.c_str());
    CHECK_SYSCALL(close(fd),close);
    CHECK_SYSCALL(dup2(stdout_dup,STDOUT_FILENO),dup2);
    CHECK_SYSCALL(close(stdout_dup),close);
}

void SmallShell::handlePipeCommand(string first_cmd, string second_cmd, bool isTwoCharsPipeIO) {
    int fds[2];
    CHECK_SYSCALL(pipe(fds),pipe);
    int stdin_dup = CHECK_SYSCALL_AND_GET_VALUE_RVOID(dup(STDIN_FILENO),dup,stdin_dup);
    int fd_to_write = isTwoCharsPipeIO ? STDERR_FILENO : STDOUT_FILENO;
    int out_backup = CHECK_SYSCALL_AND_GET_VALUE_RVOID(dup(fd_to_write),dup,out_backup);
    CHECK_SYSCALL(dup2(fds[1],fd_to_write),dup2);
    shared_ptr<Command> cmd1 = CreateCommand(first_cmd.c_str());
    handleOneCommand(cmd1,first_cmd.c_str());
    CHECK_SYSCALL(dup2(out_backup, fd_to_write),dup2);
    CHECK_SYSCALL(close(fds[1]),close);
    CHECK_SYSCALL(dup2(fds[0],STDIN_FILENO),dup2);
    shared_ptr<Command> cmd2 = CreateCommand(second_cmd.c_str());
    handleOneCommand(cmd2,second_cmd.c_str());
    CHECK_SYSCALL(close(fds[0]),close);
    CHECK_SYSCALL(dup2(stdin_dup, STDIN_FILENO),dup2);
}

void SmallShell::handlePipeIoCommand(string cmd_line, int pipeIOIndex, string pipeIOString, bool isTwoCharsPipeIO) {
    if (_isBackgroundComamnd(cmd_line.c_str())) {
        _removeBackgroundSign(cmd_line);
    }
    int pipeIOLength = isTwoCharsPipeIO ? 2 : 1;
    string first_cmd = cmd_line.substr(0,pipeIOIndex),
            second_cmd = cmd_line.substr(pipeIOIndex+pipeIOLength,cmd_line.length()-pipeIOIndex-pipeIOLength);
    //pipe case
    if (pipeIOString[0] == '|') {
        handlePipeCommand(first_cmd,second_cmd,isTwoCharsPipeIO);
    }
    else {
        handleIoCommand(first_cmd,second_cmd,isTwoCharsPipeIO);
    }
}

void SmallShell::executeCommand(string cmd_line) {
    if (_trim(cmd_line).empty()) {
        return;
    }
    int pipeIOIndex = -1;
    bool isTwoCharsPipeIO = false;
    string pipeIOString = pipeIOStringInCommand(cmd_line,&isTwoCharsPipeIO,&pipeIOIndex);
    if (pipeIOIndex == -1 || pipeIOString.empty()) {
        shared_ptr<Command> cmd = CreateCommand(cmd_line);
        handleOneCommand(cmd,cmd_line);
    }
    else {
        handlePipeIoCommand(cmd_line,pipeIOIndex,pipeIOString,isTwoCharsPipeIO);
    }
}

string SmallShell::getShellName()  {
    return shellName;
}

pid_t SmallShell::getForegroundProcessPid() const {
    return cmdForegroundPid;
}

void SmallShell::stopJobInShell(shared_ptr<JobsList::JobEntry> job) {
    jobs_list.stopJob(job);
}

void SmallShell::resumeJobInShell(shared_ptr<JobsList::JobEntry> job, bool toCont) {
    jobs_list.resumeJob(job, toCont);
}

const string& SmallShell::getPrevDir() const {
    return prevDir;
}

void SmallShell::setPrevDir(string newDir) {
    prevDir = newDir;
}

string SmallShell::getForegroundProcessCmdLine() const {
    return cmdForegroundCmdLine;
}

JobsList &SmallShell::getJobsList() {
    return jobs_list;
}

bool SmallShell::areThereTimedJobsInShell() const {
    return jobs_list.areThereTimedJobs();
}

void SmallShell::setClosestAlarmInShell() const {
    jobs_list.setClosestAlarm();
}

void SmallShell::removeTimedJobByPidFromShell(pid_t pid) {
    jobs_list.removeTimedJobByPid(pid);
}

void SmallShell::removeJobFromShellByPid(pid_t pid) {
    jobs_list.removeJobByPid(pid);
}

double SmallShell::getMinTimeoutLeftFromShell() const {
    return jobs_list.getMinTimeoutLeft();
}

/////////////////////////// END OF SMALLSHELL CLASS SECTION ///////////////////////////

/////////////////////////// GENERAL COMMANDS SECTION ///////////////////////////

Command::Command(string cmd_line, std::vector<string> cmd_split, pid_t pid, bool isTimedOut, int timer, std::string timeoutCmdLine, time_t alarmCreatedAt) : cmd_line(cmd_line), cmd_split(cmd_split), pid(pid), isTimedOut(isTimedOut), timer(timer), timeoutCmdLine(timeoutCmdLine), alarmCreatedAt(alarmCreatedAt) {

}

pid_t Command::getPid() const {
    return pid;
}

BuiltInCommand::BuiltInCommand(string cmd_line,std::vector<string> cmd_split, pid_t pid) : Command(cmd_line,cmd_split,pid) {
}

Command::~Command()  {
}

/////////////////////////// END OF GENERAL COMMANDS SECTION ///////////////////////////

/////////////////////////// PWD COMMAND SECTION ///////////////////////////

GetCurrDirCommand::GetCurrDirCommand(string cmd_line,std::vector<string> cmd_split) : BuiltInCommand(cmd_line, cmd_split){}

void GetCurrDirCommand::execute() {
    char* path = CHECK_SYSCALL_AND_GET_VALUE_RVOID_PTRS(getcwd(nullptr,0),getcwd,path);
    std::cout << path << std::endl;
    free(path);
}

/////////////////////////// END OF PWD COMMAND SECTION ///////////////////////////

/////////////////////////// SHOWPID COMMAND SECTION ///////////////////////////

ShowPidCommand::ShowPidCommand(string cmd_line, std::vector<string> cmd_split) : BuiltInCommand(cmd_line,cmd_split){}

void ShowPidCommand::execute() {
    pid_t pid = CHECK_SYSCALL_AND_GET_VALUE_RVOID(getpid(),getpid,pid);
    std::cout << "smash pid is " << std::to_string(pid) << std::endl;
}

/////////////////////////// END OF SHOWPID COMMAND SECTION ///////////////////////////

/////////////////////////// CD COMMAND SECTION ///////////////////////////

ChangeDirCommand::ChangeDirCommand(string cmd_line, std::vector<string> cmd_split) : BuiltInCommand(cmd_line, cmd_split) {}

void changeToPrevDirCaseMakaf () {
    string prevDir = SmallShell::getInstance().getPrevDir();
    if (prevDir.empty()) {
        std::cerr << "smash error: cd: OLDPWD not set" << std::endl;
        return;
    }
    char* currDir = CHECK_SYSCALL_AND_GET_VALUE_RVOID_PTRS(getcwd(nullptr,0),getcwd,currDir);
    string currDirStr(currDir);
    CHECK_SYSCALL(chdir(prevDir.c_str()),chdir);
    SmallShell::getInstance().setPrevDir(currDirStr);
    free(currDir);
}

void changeToParentDirCaseDotDot () {
    char* currDir = CHECK_SYSCALL_AND_GET_VALUE_RVOID_PTRS(getcwd(nullptr,0),getcwd,currDir);
    string newDir = currDir;
    if (chdir("..") == -1) {
        perror( "smash error: chdir failed" );
        free(currDir);
        return;
    }
    SmallShell::getInstance().setPrevDir(currDir);
    free(currDir);
}

void tryRelativeAddress(const string& secondArg) {
    char* currDir_char = CHECK_SYSCALL_AND_GET_VALUE_RVOID_PTRS(getcwd(nullptr,0),getcwd,currDir_char);
    string currDir(currDir_char);
    string newDir = currDir + "/" + secondArg;
    if (chdir(newDir.c_str()) == -1) {
        perror( "smash error: chdir failed" );
        free(currDir_char);
        return;
    }
    SmallShell::getInstance().setPrevDir(currDir);
    free(currDir_char);
}


void ChangeDirCommand::execute() {
    if (cmd_split[1] == "-") {
        changeToPrevDirCaseMakaf();
        return;
    }
    char* currDir = CHECK_SYSCALL_AND_GET_VALUE_RVOID_PTRS(getcwd(nullptr,0),getcwd,currDir);
    if (cmd_split[1] == "..") {
        changeToParentDirCaseDotDot();
        free(currDir);
        return;
    }
    //try absolute address
    if (chdir(cmd_split[1].c_str()) != 0) {
        tryRelativeAddress(cmd_split[1]);
    }
    else {
        SmallShell::getInstance().setPrevDir(currDir);
    }
    free(currDir);
}

/////////////////////////// END OF CD COMMAND SECTION ///////////////////////////

/////////////////////////// FG COMMAND SECTION ///////////////////////////

ForegroundCommand::ForegroundCommand(string cmd_line, std::vector<string> cmd_split) : BuiltInCommand(cmd_line, cmd_split){}

void ForegroundCommand::execute() {
    SmallShell::getInstance().getJobsList().removeFinishedJobs();
    shared_ptr<JobsList::JobEntry> jobToForeground;
    if (cmd_split.size() > 2 || (cmd_split.size() == 2 && !isNumber(cmd_split[1]))) {
        std::cerr << "smash error: fg: invalid arguments" << std::endl;
        return;
    }
    if (SmallShell::getInstance().getJobsList().isListEmpty()) {
        std::cerr << "smash error: fg: jobs list is empty" << std::endl;
        return;
    }
    if (cmd_split.size() == 1) {
        jobToForeground = SmallShell::getInstance().getJobsList().getLastJob(nullptr);
    }
    else {
        jobToForeground = SmallShell::getInstance().getJobsList().getJobById(std::stoi(cmd_split[1]));
    }
    if (!jobToForeground) {
        string errPrint = "smash error: fg: job-id " + cmd_split[1] + " does not exist";
        std::cerr << errPrint << std::endl;
        return;
    }
    jobToForeground->printJobCmd();

    if (jobToForeground->getJobStatus() == STOPPED) {
        SmallShell::getInstance().resumeJobInShell(jobToForeground);
    }
    int status;
    SmallShell::getInstance().setCmdForeground(jobToForeground->getJobPid(),jobToForeground->getCmdLine());
    CHECK_SYSCALL(waitpid(jobToForeground->getJobPid(),&status,WUNTRACED),waitpid);
    SmallShell::getInstance().setCmdForeground(0,"");
    if (!WIFSTOPPED(status)) {
        SmallShell::getInstance().getJobsList().removeJobById(jobToForeground->getJobId());
    }
}

/////////////////////////// END OF FG COMMAND SECTION ///////////////////////////

//////////////////////////////////// BG COMMAND SECTION ////////////////////////////////////

BackgroundCommand::BackgroundCommand(string cmd_line, std::vector<string> cmd_split) : BuiltInCommand(cmd_line, cmd_split){}

bool isBgValidArguments(const string& cmd_word_2, unsigned long argumentCount) {
    if (argumentCount > 2 || (argumentCount > 1 && !isNumber(cmd_word_2))) {
        std::cerr << "smash error: bg: invalid arguments" << std::endl;
        return false;
    }
    return true;
}

shared_ptr<JobsList::JobEntry> getJobForBgAndPrintErrors (unsigned long argumentCount, const string& secondArg) {
    if (argumentCount > 1) {
        shared_ptr<JobsList::JobEntry> job = SmallShell::getInstance().getJobsList().getJobById(stoi(secondArg));
        if (!job) {
            std::cerr << "smash error: bg: job-id " << secondArg << " does not exist" << std::endl;
        }
        return job;
    }
    else {
        shared_ptr<JobsList::JobEntry> job = SmallShell::getInstance().getJobsList().getLastStoppedJob();
        if (!job) {
            std::cerr << "smash error: bg: there is no stopped jobs to resume" << std::endl;
        }
        return job;
    }
    return nullptr;
}

bool isJobStoppedAndPrintError (const shared_ptr<JobsList::JobEntry>& job) {
    if (job->getJobStatus() == RUNNING) {
        std::cerr << "smash error: bg: job-id " << std::to_string(job->getJobId()) << " is already running in the background"
                  << std::endl;
        return false;
    }
    return true;
}

void BackgroundCommand::execute() {
    SmallShell::getInstance().getJobsList().removeFinishedJobs();
    string secondArg = cmd_split.size() > 1 ? cmd_split[1] : "";
    if (!isBgValidArguments(secondArg, cmd_split.size())) {
        return;
    }
    shared_ptr<JobsList::JobEntry> job = getJobForBgAndPrintErrors(cmd_split.size(),secondArg);
    if (!job) {
        return;
    }
    if (!isJobStoppedAndPrintError(job)) {
        return;
    }
    std::cout << job->getCmdLine() << " : " << job->getJobPid() << std::endl;
    CHECK_SYSCALL(kill(job->getJobPid(),SIGCONT),kill);
    job->setJobStatus(RUNNING);
}

//////////////////////////////////// END OF BG COMMAND SECTION ////////////////////////////////////

/////////////////////////// JOBS COMMAND SECTION ///////////////////////////

JobsCommand::JobsCommand(string cmd_line, std::vector<string> cmd_split) : BuiltInCommand(cmd_line, cmd_split){
}

void JobsCommand::execute() {
    SmallShell::getInstance().getJobsList().removeFinishedJobs();
    SmallShell::getInstance().getJobsList().printJobsList();
}

/////////////////////////// END OF JOBS COMMAND SECTION ///////////////////////////

/////////////////////////// QUIT COMMAND SECTION ///////////////////////////

QuitCommand::QuitCommand(string cmd_line, std::vector<string> cmd_split): BuiltInCommand(cmd_line, cmd_split){}

void QuitCommand::execute() {
    if (cmd_line == "quit") {
        throw QuitException();
    }
    if (cmd_split.size() > 1 && cmd_split[1] == "kill"){
        SmallShell::getInstance().getJobsList().removeFinishedJobs();
        std::cout << "smash: sending SIGKILL signal to " << std::to_string(SmallShell::getInstance().getJobsList().get_Job_List().size()) << " jobs:" << std::endl;
        SmallShell::getInstance().getJobsList().print_jobs_for_quit_command();
        SmallShell::getInstance().getJobsList().killAllJobs();
        throw QuitException();
    }
}

/////////////////////////// END OF QUIT COMMAND SECTION ///////////////////////////

/////////////////////////// KILL COMMAND SECTION ///////////////////////////

KillCommand::KillCommand(string cmd_line, std::vector<string> cmd_split): BuiltInCommand(cmd_line, cmd_split){}

bool is_sigment(const string& s1){
    if (s1.length()>3){
        return false;
    }
    string s1_from_second_char = s1.substr(1,s1.length());
    if (s1[0] != '-' || !isNumber(s1_from_second_char)
    || !(0<=stoi(s1_from_second_char) && stoi(s1_from_second_char)<=31)) {
        return false;
    }
    return true;
}

void KillCommand::execute() {
    bool second_word_ok,third_word_ok;
    second_word_ok = cmd_split.size() > 1 && is_sigment(cmd_split[1]);
    third_word_ok = cmd_split.size() > 2 && isNumber(cmd_split[2]);
    SmallShell::getInstance().getJobsList().removeFinishedJobs();
    if (cmd_split.size() != 3 || !second_word_ok || !third_word_ok ) {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }
    shared_ptr<JobsList::JobEntry> job_to_kill = SmallShell::getInstance().getJobsList().getJobById(stoi(cmd_split[2]));
    if(SmallShell::getInstance().getJobsList().get_Job_List().empty() || !job_to_kill)
    {
        string printErr = "smash error: kill: job-id " + cmd_split[2] + " does not exist";
        std::cerr << printErr << std::endl;
        return;
    }
    int signal_int=stoi(cmd_split[1].substr(1,cmd_split[1].length()-1));
    CHECK_SYSCALL(kill(job_to_kill->getJobPid(),signal_int),kill);
    if (signal_int == SIGSTOP) {
        SmallShell::getInstance().stopJobInShell(job_to_kill);
    }
    if (signal_int == SIGCONT) {
        SmallShell::getInstance().resumeJobInShell(job_to_kill,false);
    }
    std::cout << "signal number " + std::to_string(signal_int) + " was sent to pid " + std::to_string(job_to_kill->getJobPid()) << std::endl;
}


/////////////////////////// END OF KILL COMMAND SECTION ///////////////////////////

/////////////////////////// SETCORE COMMAND SECTION ///////////////////////////

SetcoreCommand::SetcoreCommand(string cmd_line, std::vector<string> cmd_split) : BuiltInCommand(cmd_line, cmd_split){}


void SetcoreCommand::execute() {
    if (cmd_split.size()!=3 || !isNumber(cmd_split[1]) || !isNumber(cmd_split[2])) {
        std::cerr << "smash error: setcore: invalid arguments" << std::endl;
        return;
    }
    int num_cores = CHECK_SYSCALL_AND_GET_VALUE_RVOID(sysconf(_SC_NPROCESSORS_ONLN),sysconf,num_cores);
    if (num_cores - 1 < std::stoi(cmd_split[2]) || std::stoi(cmd_split[2]) < 0)
    {
        std::cerr << "smash error: setcore: invalid core number" << std::endl;
        return;
    }
    //JobsList& jobs = SmallShell::getInstance().getJobsList();
    if (!SmallShell::getInstance().getJobsList().getJobById(std::stoi(cmd_split[1])))
    {
        string printErr = "smash error: setcore: job-id " + cmd_split[1] + " does not exist";
        std::cerr << printErr << std::endl;
        return;
    }
    pid_t jobPid= SmallShell::getInstance().getJobsList().getJobById(std::stoi(cmd_split[1]))->getJobPid();
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(std::stoi(cmd_split[2]), &mask);
    CHECK_SYSCALL(sched_setaffinity(jobPid, sizeof(mask), &mask),sched_setaffinity);
}


/////////////////////////// END OF SETCORE COMMAND SECTION ///////////////////////////

/////////////////////////// GETFILETYPE COMMAND SECTION ///////////////////////////

GetFileTypeCommand::GetFileTypeCommand(string cmd_line, std::vector<string> cmd_split)  : BuiltInCommand(cmd_line, cmd_split){}

const char* get_file_type(const char *path) {
    struct stat st;
    CHECK_SYSCALL_PTRS(lstat(path, &st),lstat);
    switch (st.st_mode & S_IFMT) {
        case S_IFBLK:  return "block device";
        case S_IFCHR:  return "character device";
        case S_IFDIR:  return "directory";
        case S_IFIFO:  return "FIFO";
        case S_IFLNK:  return "symbolic link";
        case S_IFREG:  return "regular file";
        case S_IFSOCK: return "socket";
        default:       return "unknown";
    }
}

long get_file_size(const char *path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        perror("smash error: stat failed");
        return -1;
    }
    return st.st_size;
}

void GetFileTypeCommand::execute() {
    if (cmd_split.size() != 2) {
        std::cerr << "smash error: gettype: invalid arguments" <<std::endl;
        return;
    }
    const char* theType= get_file_type(cmd_split[1].c_str());
    if (!theType) {
        return;
    }
    if( strcmp(theType,"unknown")==0){
        string printErr = "smash error: setcore: job-id " + cmd_split[1] + " does not exist";
        std::cerr << printErr << std::endl;
        return;
    }
    else
    {
        long num_of_bytes = get_file_size(cmd_split[1].c_str());
        if (num_of_bytes == -1) {
            return;
        }
        std::cout << cmd_split[1] + "'s type is " + theType+ " and takes up " + std::to_string(num_of_bytes) + " bytes" << std::endl;
        return;
    }
}


/////////////////////////// END OF GETFILETYPE COMMAND SECTION ///////////////////////////

/////////////////////////// CHMOD COMMAND SECTION ///////////////////////////

ChmodCommand::ChmodCommand(string cmd_line, std::vector<string> cmd_split): BuiltInCommand(cmd_line, cmd_split){}

int validation_ch(int mode) {
    if ((mode >= 0) && (mode <= 0777)) {
        return mode;
    } else {
        return -1;
    }
}

void ChmodCommand::execute() {
    int mode= cmd_split.size() > 1 ? validation_ch(std::stoi(cmd_split[1],0,8)) : -1;
    if ( cmd_split.size() != 3 || ! isNumber(cmd_split[1]) || (std::stoi(cmd_split[1]) < 0 || std::stoi(cmd_split[1]) > 777)) {
        std::cerr << "smash error: chmod: invalid arguments" << std::endl;
        return;
    }
    CHECK_SYSCALL(chmod(cmd_split[2].c_str(),static_cast<mode_t>(mode)),chmod);
}

/////////////////////////// END OF GETFILETYPE COMMAND SECTION ///////////////////////////

/////////////////////////// EXTERNAL COMMAND SECTION ///////////////////////////

ExternalCommand::ExternalCommand(string cmd_line, std::vector<string> cmd_split) : Command(cmd_line, cmd_split){}

bool isComplexExternalCommand (string cmd_line) {
    for (unsigned int i = 0; i < cmd_line.length(); i++) {
        if (cmd_line[i] == '?' || cmd_line[i] == '*') {
            return true;
        }
    }
    return false;
}

void runExternalCommand(const char* prog, char * const * args, string& cmd_line, bool isTimedOut, int timer, string& timeoutCmdLine, time_t alarmCreatedAt) {
    bool isBGCommand = _isBackgroundComamnd(cmd_line.c_str());
    pid_t pid = CHECK_SYSCALL_AND_GET_VALUE_RVOID(fork(), fork, pid);
    if (pid > 0) {
        if (isTimedOut) {
            SmallShell::getInstance().getJobsList().removeFinishedJobs();
            SmallShell::getInstance().getJobsList().addTimeoutJob(pid, alarmCreatedAt, timer, timeoutCmdLine);
        }
        if (isBGCommand) {
            if (isTimedOut) {
                SmallShell::getInstance().addJobToShell(pid,timeoutCmdLine);
            }
            else {
                SmallShell::getInstance().addJobToShell(pid,cmd_line);
            }
        }
        else {
            SmallShell::getInstance().setCmdForeground(pid,cmd_line);
            CHECK_SYSCALL(waitpid(pid, nullptr, WUNTRACED),waitpid);
            SmallShell::getInstance().setCmdForeground(0,"");
            if (isTimedOut) {
                SmallShell::getInstance().removeTimedJobByPidFromShell(pid);
            }
        }
    }
    else {
        CHECK_SYSCALL(setpgrp(), setpgrp);
        CHECK_SYSCALL(execvp(prog,args),execvp);
        delete[] args;
        throw QuitException();
    }
}

void copyWordsIntoArgs(const std::vector<string>& words, char** args_for_simple) {
    for (unsigned int i = 0; i < words.size(); i++) {
        args_for_simple[i] = new char[words[i].length()+1];
        strcpy(args_for_simple[i],words[i].c_str());
    }
}

void deleteWordsArgs(char** args_for_simple, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        delete[] args_for_simple[i];
    }
    delete[] args_for_simple;
}

void ExternalCommand::execute() {
    bool isComplex = isComplexExternalCommand(cmd_line);
    string cmd_line_cpy_str(cmd_line);
    _removeBackgroundSign(cmd_line_cpy_str);
    if (isComplex) {
        char bashStr[] = "bash";
        char minusCStr[] = "-c";
        char* args_for_complex[4] = {bashStr, minusCStr, const_cast<char*>(cmd_line_cpy_str.c_str()), nullptr};
        runExternalCommand("/bin/bash",args_for_complex,cmd_line, isTimedOut, timer, timeoutCmdLine,alarmCreatedAt);
    }
    else {
        char** args_for_simple= new char*[cmd_split.size()+1];
        args_for_simple[cmd_split.size()] = nullptr;
        copyWordsIntoArgs(cmd_split,args_for_simple);
        runExternalCommand(args_for_simple[0],args_for_simple,cmd_line, isTimedOut, timer, timeoutCmdLine,alarmCreatedAt);
        deleteWordsArgs(args_for_simple,cmd_split.size());
    }
}

void Command::toBeTimedOut() {
    isTimedOut = true;
}

void Command::setTimer(int timer) {
    this->timer = timer;
}


/////////////////////////// END OF EXTERNAL COMMAND SECTION ///////////////////////////

/////////////////////////// JOBS CLASSES SECTION ///////////////////////////

JobsList::JobsList() : job_list(), maximalJobId(0), timed_jobs() {}

void JobsList::addJob(pid_t pid, string cmd_line, bool isStopped) {
    removeFinishedJobs();
    shared_ptr<JobsList::JobEntry> job = getJobByPid(pid);
    if (job) {
        if (isStopped) {
            stopJob(job);
        }
        else {
            job->setJobStatus(RUNNING);
        }
        return;
    }
    shared_ptr<JobEntry> newJob = std::make_shared<JobEntry>(maximalJobId+1, pid, cmd_line);
    if (isStopped) {
        newJob->setJobStatus(STOPPED);
    }
    maximalJobId++;
    job_list.push_back(newJob);
}

void JobsList::resumeJob(shared_ptr<JobsList::JobEntry> job, bool toCont) {
    if (toCont) {
        CHECK_SYSCALL(kill(job->getJobPid(),SIGCONT),kill);
    }
    job->setJobStatus(RUNNING);
}

void JobsList::printJobsList() {
    for (const auto& it : job_list) {
        string str_cmd_line = it->getCmdLine();
        time_t current_time = CHECK_SYSCALL_AND_GET_VALUE_RVOID(time(nullptr),time,current_time);
        double job_run_time = CHECK_SYSCALL_AND_GET_VALUE_RVOID(difftime(current_time,it->getTimeCreated()),difftime,job_run_time);

        std::cout << "[" << std::to_string(it->getJobId()) << "] " + str_cmd_line + " : " + std::to_string(it->getJobPid()) + " "  <<
                                                         job_run_time << " secs";
        if (it->getJobStatus() == STOPPED) {
            std::cout << " (stopped)";
        }
        std::cout << std::endl;
    }
}

void JobsList::killAllJobs(){

    for (auto & it : job_list)
    {
        CHECK_SYSCALL(kill(it->getJobPid(),15),kill);
    }
    job_list.clear();
}

void JobsList::initializeMaximalJobId() {
    int maxJobId = 0;
    for (auto & it : job_list)
    {
        if (it->getJobId() > maxJobId) {
            maxJobId = it->getJobId();
        }
    }
    maximalJobId = maxJobId;
}

void JobsList::removeFinishedJobs(){
    for (auto  it = job_list.begin(); it != job_list.end();)
    {
        pid_t waitReturnValue = CHECK_SYSCALL_AND_GET_VALUE_RVOID(waitpid((*it)->getJobPid(), nullptr,WNOHANG),waitpid,waitReturnValue);
        if(waitReturnValue > 0)
        {
            removeTimedJobByPid((*it)->getJobPid());
            it = job_list.erase(it);
        }
        else
        {
            ++it;
        }
    }
    initializeMaximalJobId();
}


shared_ptr<JobsList::JobEntry> JobsList::getJobById(int jobId) {
    for (auto & it : job_list)
    {
        if(it->getJobId()==jobId){
            return it;
        }
    }
    return nullptr;
}

void JobsList::removeJobById(int jobId) {
    for (auto  it = job_list.begin(); it != job_list.end(); ++it)
    {
        if((*it)->getJobId()==jobId){
            job_list.erase(it);
            return;
        }
    }
}

shared_ptr<JobsList::JobEntry> JobsList::getLastJob(int *lastJobId) {
    if (lastJobId) {
        *lastJobId=(*job_list.rbegin())->getJobId();
    }
    return *job_list.rbegin();
}

shared_ptr<JobsList::JobEntry> JobsList::getLastStoppedJob() {
    int maxId = 0;
    shared_ptr<JobsList::JobEntry> lastStopped = nullptr;
    for (auto & job : job_list) {
        if (job->getJobStatus() == STOPPED && (!lastStopped || job->getJobId() > maxId)) {
            maxId = job->getJobId();
            lastStopped = job;
        }
    }
    return lastStopped;
}

bool JobsList::isListEmpty() const {
    return job_list.empty();
}

std::vector<shared_ptr<JobsList::JobEntry>>& JobsList::get_Job_List() {
    return job_list;
}

void JobsList::print_jobs_for_quit_command() {
    for (auto & it : job_list) {
        std::cout << std::to_string(it->getJobPid()) << ": " << it->getCmdLine() << std::endl;
    }
}

void JobsList::stopJob(shared_ptr<JobEntry> jobToStop) {
    jobToStop->setJobStatus(STOPPED);
}

shared_ptr<JobsList::JobEntry> JobsList::getJobByPid(pid_t pid) {
    for (auto & job : job_list) {
        if (job->getJobPid() == pid) {
            return job;
        }
    }
    return nullptr;
}

void JobsList::addTimeoutJob(pid_t pid, time_t timeCreated, int timer, string cmd_line) {
    TimeoutEntry entry(pid,timeCreated,timer, cmd_line);
    timed_jobs.push_back(entry);
}

std::vector<JobsList::TimeoutEntry> &JobsList::getTimeoutEntries() {
    return timed_jobs;
}

void JobsList::removeTimedJobByPid(pid_t pid) {
    for (auto it = timed_jobs.begin(); it != timed_jobs.end(); it++) {
        if (it->pid == pid) {
            timed_jobs.erase(it);
            return;
        }
    }
}

bool JobsList::areThereTimedJobs() const {
    return !timed_jobs.empty();
}

void JobsList::setClosestAlarm() const {
    if (timed_jobs.empty()) {
        return;
    }
    int minTimer = -1;
    TimeoutEntry entry(0,0,0,"");
    for (auto & process : timed_jobs) {
        if (minTimer == -1 || process.timer < minTimer) {
            minTimer = process.timer;
            entry = process;
        }
    }
    if (entry.pid == 0) {
        return;
    }
    time_t current_time = CHECK_SYSCALL_AND_GET_VALUE_RVOID(time(nullptr),time,current_time);
    int time_passed_from_entry = CHECK_SYSCALL_AND_GET_VALUE_RVOID(ceil(difftime(current_time,entry.timeCreated)),difftime,time_passed_from_entry);
    int when_to_alert = entry.timer - time_passed_from_entry;
    if (when_to_alert <= 0) {
        when_to_alert = 1;
    }
    alarm(when_to_alert);
}

void JobsList::removeJobByPid(pid_t pid) {
    for(auto it = job_list.begin(); it != job_list.end();++it) {
        if ((*it)->getJobPid() == pid) {
            job_list.erase(it);
            return;
        }
    }
}

double JobsList::getMinTimeoutLeft() const {
    double min = -1;
    time_t now = time(nullptr);
    if (now == -1) {
        perror("smash error: time failed");
    }
    for (auto & process : timed_jobs) {
        double currCreatedToNow = difftime(process.timeCreated,now);
        double currTimeoutLeft = process.timer - currCreatedToNow;
        if (min == -1 || currTimeoutLeft < min) {
            min = currTimeoutLeft;
        }
    }
    return min;
}

/////////////////////////// JOB ENTRY SECTION ///////////////////////////

JobsList::JobEntry::JobEntry(int jobId, pid_t jobPid, string cmd_line) : status(RUNNING), jobId(jobId), jobPid(jobPid), cmd_line(cmd_line) {
    this->timeCreated = CHECK_SYSCALL_AND_GET_VALUE_RVOID(time(nullptr),time,this->timeCreated);
}

JobsList::JobEntry::~JobEntry() {
}

int JobsList::JobEntry::getJobId() const {
    return jobId;
}

void JobsList::JobEntry::printJobCmd() const {
    std::cout << cmd_line << " : " << std::to_string(jobPid) << std::endl;
}

string JobsList::JobEntry::getCmdLine() const {
    return cmd_line;
}

time_t JobsList::JobEntry::getTimeCreated() const {
    return timeCreated;
}

pid_t JobsList::JobEntry::getJobPid() const {
    return jobPid;
}

void JobsList::JobEntry::setJobStatus(JobStatus status) {
    this->status = status;
}

JobStatus JobsList::JobEntry::getJobStatus() const {
    return status;
}

/////////////////////////// END OF JOB ENTRY SECTION ///////////////////////////

/////////////////////////// END OF JOBS SECTION ///////////////////////////
TimeoutCommand::TimeoutCommand(string cmd_line, std::vector<string> cmd_split) : BuiltInCommand(cmd_line,cmd_split) {

}

JobsList::TimeoutEntry::TimeoutEntry(pid_t pid, time_t timeCreated, int timer, string cmd_line) : pid(pid), timeCreated(timeCreated), timer(timer), cmd_line(cmd_line) {
}

void TimeoutCommand::execute() {
    if (cmd_split.size() < 3 || !isNumber(cmd_split[1]) || std::stoi(cmd_split[1]) < 0) {
        std::cerr << "smash error: timeout: invalid arguments" << std::endl;
        return;
    }
    string cmd_line_without_timeout;
    for (unsigned int i = 2; i < cmd_split.size(); i++) {
        string addSpaceIfRequired;
        if (i != cmd_split.size() - 1) {
            addSpaceIfRequired += " ";
        }
        cmd_line_without_timeout += cmd_split[i] + addSpaceIfRequired;
    }
    std::shared_ptr<Command> cmd = SmallShell::getInstance().CreateCommand(cmd_line_without_timeout);
    cmd->toBeTimedOut();
    cmd->setTimer(std::stoi(cmd_split[1]));
    cmd->timeoutCmdLine = cmd_line;
    time_t current_time = CHECK_SYSCALL_AND_GET_VALUE_RVOID(time(nullptr),time,current_time);
    cmd->alarmCreatedAt = current_time;
    if (!SmallShell::getInstance().areThereTimedJobsInShell() || SmallShell::getInstance().getMinTimeoutLeftFromShell() > std::stoi(cmd_split[1])) {
        alarm(std::stoi(cmd_split[1]));
    }
    cmd->execute();
}