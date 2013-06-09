#include "Process.h"

#include "Logger.h"

#include <stdexcept>

Process::Process() :
    m_empty(true),
    m_low_priority(false)
{}

Process::Process(const std::string& cmd, const std::vector<std::string>& argv) :
    m_impl(new ProcessImpl(cmd, argv)), 
    m_empty(false),
    m_low_priority(false)
{}

bool Process::SetLowPriority(bool low) {
    if (m_empty)
        return false;
    if (m_low_priority != low) {
        if (m_impl->SetLowPriority(low)) {
            m_low_priority = low;
            return true;
        } else
            return false;
    } else
        return true;
}

void Process::Kill()
{ if (m_impl) m_impl->Kill(); }

void Process::RequestTermination()
{ m_impl.reset(); }

void Process::Free() {
    if (m_impl)
        m_impl->Free();
}


#if defined(FREEORION_WIN32)

Process::ProcessImpl::ProcessImpl(const std::string& cmd, const std::vector<std::string>& argv) :
    m_free(false)
{
    std::string args;
    for (unsigned int i = 0; i < argv.size(); ++i) {
        args += argv[i];
        if (i + 1 < argv.size())
            args += ' ';
    }

    ZeroMemory(&m_startup_info, sizeof(STARTUPINFO));
    m_startup_info.cb = sizeof(STARTUPINFO);
    ZeroMemory(&m_process_info, sizeof(PROCESS_INFORMATION));

    if (!CreateProcess(const_cast<char*>(cmd.c_str()), const_cast<char*>(args.c_str()), 0, 0, false, 
        CREATE_NO_WINDOW, 0, 0, &m_startup_info, &m_process_info)) {
            std::string err_str;
            DWORD err = GetLastError();
            DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;
            LPSTR buf;
            if (FormatMessageA(flags, 0, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, 0)) {
                err_str += buf;
                LocalFree(buf);
            }
            throw std::runtime_error("Process::Process : Failed to create child process.  Windows error was: \"" + err_str + "\"");
        }
        WaitForInputIdle(m_process_info.hProcess, 1000); // wait for process to finish setting up, or for 1 sec, which ever comes first
}

Process::ProcessImpl::~ProcessImpl()
{ if (!m_free) Kill(); }

bool Process::ProcessImpl::SetLowPriority(bool low) {
    if (low)
        return (SetPriorityClass(m_process_info.hProcess, BELOW_NORMAL_PRIORITY_CLASS) != 0);
    else
        return (SetPriorityClass(m_process_info.hProcess, NORMAL_PRIORITY_CLASS) != 0);
}

void Process::ProcessImpl::Kill() {
    if (!TerminateProcess(m_process_info.hProcess, 0)) {
        std::string err_str;
        DWORD err = GetLastError();
        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;
        LPSTR buf;
        if (FormatMessageA(flags, 0, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, 0)) {
            err_str += buf;
            LocalFree(buf);
        }
        Logger().errorStream() << "Process::ProcessImpl::Kill : Error terminating process: " << err_str;
    }

    if (!CloseHandle(m_process_info.hProcess)) {
        std::string err_str;
        DWORD err = GetLastError();
        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;
        LPSTR buf;
        if (FormatMessageA(flags, 0, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, 0)) {
            err_str += buf;
            LocalFree(buf);
        }
        Logger().errorStream() << "Process::ProcessImpl::Kill : Error closing process handle: " << err_str;
    }

    if (!CloseHandle(m_process_info.hThread)) {
        std::string err_str;
        DWORD err = GetLastError();
        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;
        LPSTR buf;
        if (FormatMessageA(flags, 0, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, 0)) {
            err_str += buf;
            LocalFree(buf);
        }
        Logger().errorStream() << "Process::ProcessImpl::Kill : Error closing thread handle: " << err_str;
    }
}

#elif defined(FREEORION_LINUX) || defined(FREEORION_MACOSX)

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#include <cstdio>
#include <sys/wait.h>


Process::ProcessImpl::ProcessImpl(const std::string& cmd, const std::vector<std::string>& argv) :
    m_free(false)
{
    std::vector<char*> args;
    for (unsigned int i = 0; i < argv.size(); ++i) {
        args.push_back(const_cast<char*>(&(const_cast<std::string&>(argv[i])[0])));
    }
    args.push_back(0);

    switch (m_process_id = fork()) {
    case -1: { // error
        throw std::runtime_error("Process::Process : Failed to fork a new process.");
        break;
    }

    case 0: { // child process side of fork
        execv(cmd.c_str(), &args[0]);
        perror("execv failed");
        break;
    }

    default: // original process side of fork (execution continues after a set-up wait)
        sleep(1); // wait a second to let the child process set up
        break;
    }
}

Process::ProcessImpl::~ProcessImpl()
{ if (!m_free) Kill(); }

bool Process::ProcessImpl::SetLowPriority(bool low) {
    if (low)
        return (setpriority(PRIO_PROCESS, m_process_id, 10) == 0);
    else
        return (setpriority(PRIO_PROCESS, m_process_id, 0) == 0);
}

void Process::ProcessImpl::Kill() {
    int status;
    kill(m_process_id, SIGHUP); 
    waitpid(m_process_id, &status, 0);
}

#endif

void Process::ProcessImpl::Free()
{ m_free = true; }
