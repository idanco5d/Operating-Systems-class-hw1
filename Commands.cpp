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

using namespace std;

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
  unsigned int idx = str.find_last_not_of(WHITESPACE);
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
//  // replace the & (background sign) with space and then remove all tailing spaces.
//  str[idx] = ' ';
//  // truncate the command line string up to the last non-space character
//  str[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

//////////////////////////////////// END OF COURSE STAFF FUNCTIONS ////////////////////////////////////

//////////////////////////////////// HELPER FUNCTIONS SECTION ////////////////////////////////////

//int findAmountOfSpaces (string str) {
//    int amountOfSpaces = 0;
//    bool isPrevSpace = false;
//    for (int i = 0; (unsigned int)i < str.length(); i++) {
//        if (WHITESPACE.find(str[i]) != std::string::npos) {
//            if (!isPrevSpace) {
//                amountOfSpaces++;
//            }
//            isPrevSpace = true;
//        }
//        else {
//            isPrevSpace = false;
//        }
//    }
//    return amountOfSpaces;
//}
//
//string getCmd_s(string cmd_line) {
//    return _trim(cmd_line);
//}
//
//string getFirstWord(string cmd_line) {
//    string cmd_s = getCmd_s(cmd_line);
//    return cmd_s.substr(0, cmd_s.find_first_of(" \n"));
//}
//
//string getSecondArg(string cmd_line) {
//    string cmd_s = getCmd_s(cmd_line);
//    string firstWord = getFirstWord(cmd_line);
//    string cmdName = cmd_s.substr(0, cmd_s.find_first_of(" "));
//    string fromSecondWordOfCmd_s;
//    string secondArg;
//    if (cmd_s.length() > firstWord.length()) {
//        fromSecondWordOfCmd_s = _trim(cmd_s.substr(cmdName.length()+1,cmd_s.length()-cmdName.length()-1));
//        secondArg = _trim(cmd_s.substr(cmd_s.find_first_of(" ")+1, fromSecondWordOfCmd_s.find_first_of(" ")));
//    }
//    return secondArg;
//}

bool isBuiltInCommand (const string& firstWord) {
    if (firstWord== "chprompt") {
        return true;
    }
    else if (firstWord == "pwd") {
        return true;
    }
    else if (firstWord == "showpid") {
        return true;
    }
    else if (firstWord == "cd") {
        return true;
    }
    else if (firstWord == "jobs") {
        return true;
    }
    else if (firstWord == "fg") {
        return true;
    }
    else if (firstWord == "bg") {
        return true;
    }
    else if (firstWord == "quit") {
        return true;
    }
    else if (firstWord == "kill") {
        return true;
    }
    return false;
}

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

SmallShell::SmallShell() : shellName("smash"), cmdForegroundPid(0), jobs_list(), prevDir(""){
// TODO: add your implementation
}

//SmallShell::~SmallShell() {
//// TODO: add your implementation
//}

void SmallShell::setShellName(string newName) {
    this->shellName = newName;
}

void SmallShell::addJobToShell(pid_t pid, string cmd_line, bool isStopped) {
    jobs_list.addJob(pid, cmd_line, isStopped);
}

void SmallShell::setCmdForeground(pid_t pid, string cmd_line) {
    cmdForegroundPid = pid;
    cmdForegroundCmdLine = cmd_line;
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
    return std::make_shared<GetCurrDirCommand>(cmd_line_for_builtin);
  }
  else if (cmd_words[0] == "showpid") {
    return std::make_shared<ShowPidCommand>(cmd_line_for_builtin);
  }
  else if (cmd_words[0] == "cd") {
      if (cmd_words.size() > 2) {
          //perror("smash error: cd: too many arguments\n");
          std::cerr << "smash error: cd: too many arguments" << std::endl;
          return nullptr;
      }
      return std::make_shared<ChangeDirCommand>(cmd_line_for_builtin);
  }
  else if (cmd_words[0] == "jobs") {
      return std::make_shared<JobsCommand>(cmd_line_for_builtin);
  }
  else if (cmd_words[0] == "fg") {
      return std::make_shared<ForegroundCommand>(cmd_line_for_builtin);
  }
  else if (cmd_words[0] == "bg") {
      return std::make_shared<BackgroundCommand>(cmd_line_for_builtin);
  }
  else if (cmd_words[0] == "quit") {
      return std::make_shared<QuitCommand>(cmd_line_for_builtin);
  }
  else if (cmd_words[0] == "kill") {
      return std::make_shared<KillCommand>(cmd_line_for_builtin);
  }
  else if (cmd_words[0] == "setcore") {
      return std::make_shared<SetcoreCommand>(cmd_line);
  }
  else if (cmd_words[0] == "getfileinfo") {
      return std::make_shared<GetFileTypeCommand>(cmd_line);
  }
  else if (cmd_words[0] == "chmod") {
      return std::make_shared<ChmodCommand>(cmd_line);
  }
  else {
      return std::make_shared<ExternalCommand>(cmd_line);
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

void SmallShell::handleBGCommand(shared_ptr<Command> cmd, string cmd_line) {
    pid_t pid = CHECK_SYSCALL_AND_GET_VALUE_RVOID(fork(),fork,pid);
    if (pid > 0) {
        jobs_list.addJob(pid, cmd_line);
    }
    else {
        CHECK_SYSCALL(setpgrp(), setpgrp);
        cmd->execute();
        throw QuitException();
    }
}

void SmallShell::handleExternalCommand(shared_ptr<Command> cmd, string cmd_line) {
    pid_t pid = CHECK_SYSCALL_AND_GET_VALUE_RVOID(fork(),fork,pid);
    if (pid > 0) {
        setCmdForeground(pid,cmd_line);
        int status;
        CHECK_SYSCALL(waitpid(pid, &status, WUNTRACED),waitpid);
        setCmdForeground(0,"");
        return;
    }
    else {
        CHECK_SYSCALL(setpgrp(),setpgrp);
        cmd->execute();
        throw QuitException();
    }
}

void SmallShell::handleOneCommand(shared_ptr<Command> cmd, string cmd_line) {
    if (!cmd) {
        return;
    }
    string cmd_line_copy = cmd_line;
    _removeBackgroundSign(cmd_line_copy);
    std::vector<string> cmd_split = splitStringIntoWords(cmd_line_copy);
    bool checkIfBuiltIn = isBuiltInCommand(cmd_split[0]);
    try {
        if (!checkIfBuiltIn && _isBackgroundComamnd(cmd_line.c_str())) {
            handleBGCommand(cmd, cmd_line);
            return;
        }
        if (!checkIfBuiltIn) {
            handleExternalCommand(cmd, cmd_line);
            return;
        }
        //handle built in command in the parent shell
        cmd->execute();
        //delete cmd;
    }
    catch(QuitException&) {
        //delete cmd;
        throw QuitException();
    }
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

/////////////////////////// END OF SMALLSHELL CLASS SECTION ///////////////////////////

/////////////////////////// GENERAL COMMANDS SECTION ///////////////////////////

Command::Command(string cmd_line, pid_t pid) : cmd_line(cmd_line), pid(pid) {

}

string Command::getCmdLine() const {
    return cmd_line;
}

void Command::setPid(pid_t pid) {
    this->pid = pid;
}

pid_t Command::getPid() const {
    return pid;
}

BuiltInCommand::BuiltInCommand(string cmd_line, pid_t pid) : Command(cmd_line,pid) {
}

Command::~Command()  {
}

//BuiltInCommand::~BuiltInCommand()  {
//    //delete cmd_line;
//}


/////////////////////////// END OF GENERAL COMMANDS SECTION ///////////////////////////

/////////////////////////// PWD COMMAND SECTION ///////////////////////////

GetCurrDirCommand::GetCurrDirCommand(string cmd_line) : BuiltInCommand(cmd_line){}

void GetCurrDirCommand::execute() {
    char* path = CHECK_SYSCALL_AND_GET_VALUE_RVOID_PTRS(getcwd(nullptr,0),getcwd,path);
    std::cout << path << std::endl;
    free(path);
    //printf("%s\n", getcwd(nullptr,0));
}

/////////////////////////// END OF PWD COMMAND SECTION ///////////////////////////

/////////////////////////// SHOWPID COMMAND SECTION ///////////////////////////

ShowPidCommand::ShowPidCommand(string cmd_line) : BuiltInCommand(cmd_line){}

void ShowPidCommand::execute() {
    pid_t pid = CHECK_SYSCALL_AND_GET_VALUE_RVOID(getpid(),getpid,pid);
    std::cout << "smash pid is " << to_string(pid) << std::endl;
}

/////////////////////////// END OF SHOWPID COMMAND SECTION ///////////////////////////

/////////////////////////// CD COMMAND SECTION ///////////////////////////

ChangeDirCommand::ChangeDirCommand(string cmd_line) : BuiltInCommand(cmd_line) {}

void changeToPrevDirCaseMakaf () {
    SmallShell& shell = SmallShell::getInstance();
    string prevDir = shell.getPrevDir();
    if (prevDir.empty()) {
        //perror("smash error: cd: OLDPWD not set\n");
        std::cerr << "smash error: cd: OLDPWD not set" << std::endl;
        return;
    }
    char* currDir = CHECK_SYSCALL_AND_GET_VALUE_RVOID_PTRS(getcwd(nullptr,0),getcwd,currDir);
    string currDirStr(currDir);
    CHECK_SYSCALL(chdir(prevDir.c_str()),chdir);
    shell.setPrevDir(currDirStr);
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
    SmallShell& shell = SmallShell::getInstance();
    shell.setPrevDir(currDir);
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
    //CHECK_SYSCALL(chdir(newDir.c_str()),chdir);
    SmallShell& shell = SmallShell::getInstance();
    shell.setPrevDir(currDir);
    free(currDir_char);
}


void ChangeDirCommand::execute() {
    std::vector<string> cmd_words = splitStringIntoWords(cmd_line);
    if (cmd_words[1] == "-") {
        changeToPrevDirCaseMakaf();
        return;
    }
    char* currDir = CHECK_SYSCALL_AND_GET_VALUE_RVOID_PTRS(getcwd(nullptr,0),getcwd,currDir);
    if (cmd_words[1] == "..") {
        changeToParentDirCaseDotDot();
        free(currDir);
        return;
    }
    //try absolute address
    if (chdir(cmd_words[1].c_str()) != 0) {
        tryRelativeAddress(cmd_words[1]);
    }
    else {
        SmallShell& shell = SmallShell::getInstance();
        shell.setPrevDir(currDir);
    }
    free(currDir);
}

/////////////////////////// END OF CD COMMAND SECTION ///////////////////////////

/////////////////////////// FG COMMAND SECTION ///////////////////////////

ForegroundCommand::ForegroundCommand(string cmd_line) : BuiltInCommand(cmd_line){}

void ForegroundCommand::execute() {
    SmallShell& shell = SmallShell::getInstance();
    JobsList& job_list = shell.getJobsList();
    job_list.removeFinishedJobs();
    shared_ptr<JobsList::JobEntry> jobToForeground;
    std::vector<string> cmd_split = splitStringIntoWords(cmd_line);
    if (cmd_split.size() > 2 || (cmd_split.size() == 2 && !isNumber(cmd_split[1]))) {
        std::cerr << "smash error: fg: invalid arguments" << std::endl;
        return;
    }
    if (job_list.isListEmpty()) {
        std::cerr << "smash error: fg: jobs list is empty" << std::endl;
        return;
    }
    if (cmd_split.size() == 1) {
        jobToForeground = job_list.getLastJob(nullptr);
    }
    else {
        jobToForeground = job_list.getJobById(std::stoi(cmd_split[1]));
    }
    if (!jobToForeground) {
        string errPrint = "smash error: fg: job-id " + cmd_split[1] + " does not exist" + '\n';
        std::cerr << errPrint << std::endl;
        return;
    }
    jobToForeground->printJobCmd();

    if (jobToForeground->getJobStatus() == STOPPED) {
        shell.resumeJobInShell(jobToForeground);
    }
    int status;
    shell.setCmdForeground(jobToForeground->getJobPid(),jobToForeground->getCmdLine());
    CHECK_SYSCALL(waitpid(jobToForeground->getJobPid(),&status,WUNTRACED),waitpid);
    shell.setCmdForeground(0,"");
    if (!WIFSTOPPED(status)) {
        job_list.removeJobById(jobToForeground->getJobId());
    }
}

/////////////////////////// END OF FG COMMAND SECTION ///////////////////////////

//////////////////////////////////// BG COMMAND SECTION ////////////////////////////////////

BackgroundCommand::BackgroundCommand(string cmd_line) : BuiltInCommand(cmd_line){}

bool isBgValidArguments(const string& cmd_word_2, unsigned long argumentCount) {
    if (argumentCount > 2 || (argumentCount > 1 && !isNumber(cmd_word_2))) {
        std::cerr << "smash error: bg: invalid arguments\n";
        return false;
    }
    return true;
}

shared_ptr<JobsList::JobEntry> getJobForBgAndPrintErrors (unsigned long argumentCount, const string& secondArg, JobsList& job_list) {
    if (argumentCount > 1) {
        shared_ptr<JobsList::JobEntry> job = job_list.getJobById(stoi(secondArg));
        if (!job) {
            std::cerr << "smash error: bg: job-id " << secondArg << " does not exist" << std::endl;
        }
        return job;
    }
    else {
        shared_ptr<JobsList::JobEntry> job = job_list.getLastStoppedJob();
        if (!job) {
            std::cerr << "smash error: bg: there is no stopped jobs to resume" << std::endl;
        }
        return job;
    }
    return nullptr;
}

bool isJobStoppedAndPrintError (const shared_ptr<JobsList::JobEntry>& job) {
    if (job->getJobStatus() == RUNNING) {
        std::cerr << "smash error: bg: job-id " << to_string(job->getJobId()) << " is already running in the background"
                  << std::endl;
        return false;
    }
    return true;
}

void BackgroundCommand::execute() {
    std::vector<string> cmd_split = splitStringIntoWords(cmd_line);
    string secondArg = cmd_split.size() > 1 ? cmd_split[1] : "";
    if (!isBgValidArguments(secondArg, cmd_split.size())) {
        return;
    }
    SmallShell& shell = SmallShell::getInstance();
    JobsList& job_list = shell.getJobsList();
    shared_ptr<JobsList::JobEntry> job = getJobForBgAndPrintErrors(cmd_split.size(),secondArg,job_list);
    if (!job) {
        return;
    }
    if (!isJobStoppedAndPrintError(job)) {
        return;
    }
    time_t current_time = time(nullptr);
    if (current_time == -1) {
        return;
    }
    double job_run_time = difftime(current_time, job->getTimeCreated());
    if (job_run_time == -1) {
        return;
    }
    std::cout << job->getCmdLine() << " : " << std::setprecision(0) << job_run_time << std::endl;
    job_list.resumeJob(job);
}

//////////////////////////////////// END OF BG COMMAND SECTION ////////////////////////////////////

/////////////////////////// JOBS COMMAND SECTION ///////////////////////////

JobsCommand::JobsCommand(string cmd_line) : BuiltInCommand(cmd_line){
}

void JobsCommand::execute() {
    SmallShell& shell = SmallShell::getInstance();
    JobsList& job_list = shell.getJobsList();
    job_list.removeFinishedJobs();
    job_list.printJobsList();
}

/////////////////////////// END OF JOBS COMMAND SECTION ///////////////////////////

/////////////////////////// QUIT COMMAND SECTION ///////////////////////////

QuitCommand::QuitCommand(string cmd_line): BuiltInCommand(cmd_line){}

void QuitCommand::execute() {
    if (cmd_line == "quit") {
        throw QuitException();
    }
    SmallShell& shell = SmallShell::getInstance();
    JobsList& jobs = shell.getJobsList();
    std::vector<string> cmd_split = splitStringIntoWords(cmd_line);
    if (cmd_split.size() > 1 && cmd_split[1] == "kill"){
        jobs.removeFinishedJobs();
        std::cout << "smash: sending SIGKILL signal to " << to_string(jobs.get_Job_List().size()) << " jobs:" << std::endl;
        jobs.print_jobs_for_quit_command();
        jobs.killAllJobs();
        throw QuitException();
    }
}

/////////////////////////// END OF QUIT COMMAND SECTION ///////////////////////////

/////////////////////////// KILL COMMAND SECTION ///////////////////////////

KillCommand::KillCommand(string cmd_line): BuiltInCommand(cmd_line){}

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
    SmallShell& shell = SmallShell::getInstance();
    JobsList& jobs = shell.getJobsList();
    std::vector<string> cmd_split = splitStringIntoWords(cmd_line);
    bool second_word_ok,third_word_ok;
    second_word_ok = cmd_split.size() > 1 && is_sigment(cmd_split[1]);
    third_word_ok = cmd_split.size() > 2 && isNumber(cmd_split[2]);
    jobs.removeFinishedJobs();
    if (cmd_split.size() != 3 || !second_word_ok || !third_word_ok ) {
        std::cerr << "smash error: kill: invalid arguments" << std::endl;
        return;
    }
    shared_ptr<JobsList::JobEntry> job_to_kill = jobs.getJobById(stoi(cmd_split[2]));
    if(jobs.get_Job_List().empty() || !job_to_kill)
    {
        string printErr = "smash error: kill: job-id " + cmd_split[2] + " does not exist" + '\n';
        std::cerr << printErr;
        return;
    }
    int signal_int=stoi(cmd_split[1].substr(1,cmd_split[1].length()-1));
    CHECK_SYSCALL(kill(job_to_kill->getJobPid(),signal_int),kill);
    if (signal_int == SIGSTOP) {
        shell.stopJobInShell(job_to_kill);
    }
    if (signal_int == SIGCONT) {
        shell.resumeJobInShell(job_to_kill,false);
    }
    std::cout << "signal number " + to_string(signal_int) + " was sent to pid " + to_string(job_to_kill->getJobPid()) << std::endl;
}


/////////////////////////// END OF KILL COMMAND SECTION ///////////////////////////

/////////////////////////// SETCORE COMMAND SECTION ///////////////////////////

SetcoreCommand::SetcoreCommand(string cmd_line) : BuiltInCommand(cmd_line){}


void SetcoreCommand::execute() {
    std::vector<string> cmd_split = splitStringIntoWords(cmd_line);
    if (cmd_split.size()!=3 || !isNumber(cmd_split[1]) || !isNumber(cmd_split[2])) {
        std::cerr << "smash error: setcore: invalid arguments" << std::endl;
        return;
    }
    int num_cores = CHECK_SYSCALL_AND_GET_VALUE_RVOID(sysconf(_SC_NPROCESSORS_ONLN),sysconf,num_cores);
    if (num_cores< std::stoi(cmd_split[2]))
    {
        std::cerr << "smash error: setcore: invalid core number" << std::endl;
        return;
    }
    SmallShell& shell = SmallShell::getInstance();
    JobsList& jobs = shell.getJobsList();
    if (!jobs.getJobById(std::stoi(cmd_split[1])))
    {
        string printErr = "smash error: setcore: job-id " + cmd_split[1] + " does not exist" + '\n';
        std::cerr << printErr;
        return;
    }
    pid_t jobPid= jobs.getJobById(std::stoi(cmd_split[1]))->getJobPid();
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(std::stoi(cmd_split[2]), &mask);
    CHECK_SYSCALL(sched_setaffinity(jobPid, sizeof(mask), &mask),sched_setaffinity);
}


/////////////////////////// END OF SETCORE COMMAND SECTION ///////////////////////////

/////////////////////////// GETFILETYPE COMMAND SECTION ///////////////////////////

GetFileTypeCommand::GetFileTypeCommand(string cmd_line)  : BuiltInCommand(cmd_line){}

const char* get_file_type(const char *path) {
    struct stat st;
    CHECK_SYSCALL_PTRS(stat(path, &st),stat);
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
    std::vector<string> cmd_split = splitStringIntoWords(cmd_line);
    if (cmd_split.size() != 2) {
        std::cerr << "smash error: gettype: invalid arguments" <<std::endl;
        return;
    }
    const char* theType= get_file_type(cmd_split[1].c_str());
    if (!theType) {
        return;
    }
    if( strcmp(theType,"unknown")==0){
        string printErr = "smash error: setcore: job-id " + cmd_split[1] + " does not exist" + '\n';
        std::cerr << printErr;
        return;
    }
    else
    {
        long num_of_bytes = get_file_size(cmd_split[1].c_str());
        if (num_of_bytes == -1) {
            return;
        }
        std::cout << cmd_split[1] + "'s type is " + theType+ " and takes up " + to_string(num_of_bytes) + " bytes" << std::endl;
        return;
    }
}


/////////////////////////// END OF GETFILETYPE COMMAND SECTION ///////////////////////////

/////////////////////////// CHMOD COMMAND SECTION ///////////////////////////

ChmodCommand::ChmodCommand(string cmd_line): BuiltInCommand(cmd_line){}

int validation_ch(int mode) {
    if ((mode >= 0) && (mode <= 0777)) {
        return mode;
    } else {
        return -1;
    }
}

void ChmodCommand::execute() {
    std::vector<string> cmd_split = splitStringIntoWords(cmd_line);
    int mode= cmd_split.size() > 1 ? validation_ch(std::stoi(cmd_split[1],0,8)) : -1;
    if ( cmd_split.size() != 3 || ! isNumber(cmd_split[1]) || mode==-1) {
        std::cerr << "smash error: chmod: invalid arguments" << std::endl;
        return;
    }
    CHECK_SYSCALL(chmod(cmd_split[2].c_str(),static_cast<mode_t>(mode)),chmod);
}


/////////////////////////// END OF GETFILETYPE COMMAND SECTION ///////////////////////////

/////////////////////////// EXTERNAL COMMAND SECTION ///////////////////////////

ExternalCommand::ExternalCommand(string cmd_line) : Command(cmd_line){}

bool isComplexExternalCommand (string cmd_line) {
    for (unsigned int i = 0; i < cmd_line.length(); i++) {
        if (cmd_line[i] == '?' || cmd_line[i] == '*') {
            return true;
        }
    }
    return false;
}

void runExternalCommand(bool isComplex, char * const * args_for_complex, char * const * args_for_simple) {
    if (isComplex) {
        CHECK_SYSCALL(execvp("/bin/bash", args_for_complex),execvp);
    }
    else {
        CHECK_SYSCALL(execvp(args_for_simple[0], args_for_simple),execvp);
    }
}



void copyWordsIntoArgs(const std::vector<string>& words, char** args_for_simple) {
    for (unsigned int i = 0; i < words.size(); i++) {
        args_for_simple[i] = new char[words[i].length()+1];
        strcpy(args_for_simple[i],words[i].c_str());
    }
}

void ExternalCommand::execute() {
    bool isComplex = isComplexExternalCommand(cmd_line);
    string cmd_line_cpy_str(cmd_line);
    _removeBackgroundSign(cmd_line_cpy_str);
    std::vector<string> cmd_split = splitStringIntoWords(cmd_line_cpy_str);
    char bashStr[] = "bash";
    char minusCStr[] = "-c";
    char* args_for_complex[4] = {bashStr, minusCStr, const_cast<char*>(cmd_line_cpy_str.c_str()), nullptr};
    char** args_for_simple= new char*[cmd_split.size()+1];
    args_for_simple[cmd_split.size()] = nullptr;
    copyWordsIntoArgs(cmd_split,args_for_simple);
    runExternalCommand(isComplex,args_for_complex,args_for_simple);
    for (unsigned int i = 0; i < cmd_split.size(); i++) {
        delete[] args_for_simple[i];
    }
    delete[] args_for_simple;
    throw QuitException();
//    if (_isBackgroundComamnd(cmd_line.c_str())) {
//        runExternalCommand(isComplex,args_for_complex,args_for_simple);
//    }
//    else {
//        pid_t pid = CHECK_SYSCALL_AND_GET_VALUE_RVOID(fork(),fork,pid);
//        bool isParent = (pid > 0);
//        if (isParent) {
//            CHECK_SYSCALL(waitpid(pid, nullptr, WUNTRACED),waitpid);
//        }
//        else {
//            CHECK_SYSCALL(setpgrp(),setpgrp);
//            runExternalCommand(isComplex,args_for_complex,args_for_simple);
//            for (unsigned int i = 0; i < cmd_split.size(); i++) {
//                delete[] args_for_simple[i];
//            }
//            delete[] args_for_simple;
//            throw QuitException();
//        }
//    }
//    for (unsigned int i = 0; i < cmd_split.size(); i++) {
//        delete[] args_for_simple[i];
//    }
//    delete[] args_for_simple;
}


/////////////////////////// END OF EXTERNAL COMMAND SECTION ///////////////////////////

/////////////////////////// JOBS CLASSES SECTION ///////////////////////////

JobsList::JobsList() : job_list(), maximalJobId(0), stoppedJobs() {}

bool JobsList::isCmdInList(shared_ptr<Command> cmd) const {
    if (!cmd) {
        return false;
    }
    for (const auto & it : job_list) {
        if (it->getJobPid() == cmd->getPid()) {
            return true;
        }
    }
    return false;
}

shared_ptr<JobsList::JobEntry> JobsList::getJobByCmd(shared_ptr<Command> cmd) {
    if (!cmd) {
        return nullptr;
    }
    for (auto & it : job_list) {
        if (it->getJobPid() == cmd->getPid()) {
            return it;
        }
    }
    return nullptr;
}

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
        stopJob(newJob);
    }
    maximalJobId++;
    job_list.push_back(newJob);
}

void JobsList::resumeJob(shared_ptr<JobsList::JobEntry> job, bool toCont) {
    if (toCont) {
        CHECK_SYSCALL(kill(job->getJobPid(),SIGCONT),kill);
    }
    job->setJobStatus(RUNNING);
    for (auto it = stoppedJobs.begin(); it != stoppedJobs.end(); ++it) {
        if ((*it)->getJobPid() == job->getJobPid()) {
            stoppedJobs.erase(it);
            break;
        }
    }
}

void JobsList::printJobsList() {
    for (const auto& it : job_list) {
        string str_cmd_line = it->getCmdLine();
        time_t current_time = CHECK_SYSCALL_AND_GET_VALUE_RVOID(time(nullptr),time,current_time);
        double job_run_time = CHECK_SYSCALL_AND_GET_VALUE_RVOID(difftime(current_time,it->getTimeCreated()),difftime,job_run_time);

        std::cout << "[" << to_string(it->getJobId()) << "] " + str_cmd_line + " : " + to_string(it->getJobPid()) + " "  <<
                                                         job_run_time << " secs";
        if (it->getJobStatus() == STOPPED) {
            std::cout << " (stopped)";
        }
        std::cout << endl;
    }
}

void JobsList::killAllJobs(){

    for (auto & it : job_list)
    {
        CHECK_SYSCALL(kill(it->getJobPid(),15),kill);
        //CHECK_SYSCALL(waitpid(it->getJobPid(), nullptr,WUNTRACED),waitpid);
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

void JobsList::removeFinishedStoppedJobs() {
    for (auto it = stoppedJobs.begin(); it != stoppedJobs.end();) {
        pid_t waitReturnValue = CHECK_SYSCALL_AND_GET_VALUE_RVOID(waitpid((*it)->getJobPid(), nullptr,WNOHANG),waitpid,waitReturnValue);
        if(waitReturnValue > 0)
        {
            it = stoppedJobs.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void JobsList::removeFinishedJobs(){
    for (auto  it = job_list.begin(); it != job_list.end();)
    {
        pid_t waitReturnValue = CHECK_SYSCALL_AND_GET_VALUE_RVOID(waitpid((*it)->getJobPid(), nullptr,WNOHANG),waitpid,waitReturnValue);
        if(waitReturnValue > 0)
        {
            it = job_list.erase(it);
        }
        else
        {
            ++it;
        }
    }
    initializeMaximalJobId();
    removeFinishedStoppedJobs();
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
    return stoppedJobs.empty() ? nullptr : stoppedJobs.back();
}

bool JobsList::isListEmpty() const {
    return job_list.empty();
}

bool JobsList::isJobInTheList(int job_id) {
    for (auto & it : job_list)
    {
        if(it->getJobId()==job_id)
        {
            return true;
        }
    }
    return false;
}


std::vector<shared_ptr<JobsList::JobEntry>>& JobsList::get_Job_List() {
    return job_list;
}

void JobsList::print_jobs_for_quit_command() {
    for (auto & it : job_list) {
        std::cout << to_string(it->getJobPid()) << ": " << it->getCmdLine() << std::endl;
    }
}

void JobsList::stopJob(shared_ptr<JobEntry> jobToStop) {
    jobToStop->setJobStatus(STOPPED);
    stoppedJobs.push_back(jobToStop);
}

shared_ptr<JobsList::JobEntry> JobsList::getJobByPid(pid_t pid) {
    for (auto & job : job_list) {
        if (job->getJobPid() == pid) {
            return job;
        }
    }
    return nullptr;
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
    std::cout << cmd_line << " : " << to_string(jobPid) << std::endl;
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