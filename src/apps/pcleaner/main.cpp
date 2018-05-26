#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)
# define PLATFORM_UNIX
#endif

#ifdef PLATFORM_UNIX
#include <unistd.h>

int run(int ppid, int cpid, int signum)
{
    int rc = 0;
    while (true)
    {
        rc = kill(ppid, 0);  // ask if can send signal
        // printf("rc = %d\n", rc);
        if (rc == -1) {  // cannot send signal
            if (errno == ESRCH) {  // process exited
                kill(cpid, signum);
                return 0;
            }
            else {
                fprintf(stderr, "unexpected errno on parent: %d\n", errno);
                return 1;
            }
        }
        rc = kill(cpid, 0);
        if (rc == -1) {
            if (errno == ESRCH) {  // process exited
                return 0;
            }
            else {
                fprintf(stderr, "unexpected errno on child: %d\n", errno);
                return 1;
            }
        }
        usleep(500000);  // microseconds
    }
}
#else

#include <windows.h>
#include <TlHelp32.h>
#include <tchar.h>


char* t2a(const TCHAR* tStr) {
#ifdef _UNICODE
    size_t len = _tcslen(tStr);
    char *res = (char*)malloc(len + 1);
    wcstombs_s(NULL, res, len + 1, tStr, _TRUNCATE);
    if (!res) {
        // TODO: handle error ...
    }
    return res;
#else
    return tStr;
#endif
}


char* getErrorMsg(DWORD eNum) {

    // https://stackoverflow.com/questions/455434/how-should-i-use-formatmessage-properly-in-c

    TCHAR *sysMsg = NULL; //(TCHAR*)malloc(sizeof(TCHAR) * 256);
    // TCHAR *p;

    DWORD flags = 0;
    // use system message tables to retrieve error text
    flags |= FORMAT_MESSAGE_FROM_SYSTEM;
    // allocate buffer on heap for error text
    flags |= FORMAT_MESSAGE_ALLOCATE_BUFFER;
    // we do not pass insertion variables
    flags |= FORMAT_MESSAGE_IGNORE_INSERTS;

    // TODO: make sure this always returns ascii ... use non-localized ...
    FormatMessage(
            flags,
            NULL,  // unused with FORMAT_MESSAGE_FROM_SYSTEM
            eNum,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
            (TCHAR*)&sysMsg,
            0,  // minimum size for output buffer
            NULL);

    char *res = t2a(sysMsg);
    LocalFree(sysMsg);

    return res;
}


#define PERROR(msg) do { \
    DWORD eNum = GetLastError(); \
    char *sysMsg = getErrorMsg(eNum); \
    const char *fmt = "[ERROR] %s (%d) in %s:%d (%s)\n"; \
    fprintf(stderr, fmt, sysMsg, eNum, __FILE__, __LINE__, msg); \
    free(sysMsg); \
} while(0)


#define PFATAL(msg) do { \
    DWORD eNum = GetLastError(); \
    char *sysMsg = getErrorMsg(eNum); \
    const char *fmt = "[FATAL] %s (%d) in %s:%d (%s)\n"; \
    fprintf(stderr, fmt, sysMsg, eNum, __FILE__, __LINE__, msg); \
    free(sysMsg); \
    exit(1); \
} while(0)


int terminateChildren(DWORD ppid, UINT exitCode) {

    HANDLE snapshot;
    HANDLE cProcess;
    PROCESSENTRY32 entry;

    printf("killing child processes of pid %d ...\n", ppid);

    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
        PFATAL("CreateToolhelp32Snapshot");

    entry.dwSize = sizeof(entry);

    if (!Process32First(snapshot, &entry))
        PFATAL("Process32First");

    int rv = 0;

    do {

        DWORD cpid = entry.th32ProcessID;
        char *exeName = t2a(entry.szExeFile);

        if (entry.th32ParentProcessID == ppid) {

            printf("terminating '%s' (%d) and children ...\n", exeName, cpid);
            HANDLE pProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, cpid);
            if (pProcess == INVALID_HANDLE_VALUE) {
                fprintf(stderr,
                        "[ERROR] error opening process '%s' (%d):\n",
                        exeName,
                        cpid);
                PERROR("OpenProcess");
                rv = 1;
                continue;
            }

            if (!TerminateProcess(pProcess, 15)) {
                fprintf(stderr,
                        "[ERROR] error terminating process '%s' (%d):\n",
                        exeName,
                        cpid);
                PERROR("TerminateProcess");
                rv = 1;
                continue;
            }

            terminateChildren(cpid, exitCode);
        }

        free(exeName);

    } while (Process32Next(snapshot, &entry));

    CloseHandle(snapshot);
    return rv;
}

int run(int ppid, int cpid, int signum)
{
    DWORD state;
    int rc;

    setbuf(stdout, NULL);

    HANDLE pProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, ppid);
    if (pProcess == INVALID_HANDLE_VALUE)
        PFATAL("OpenProcess");

    HANDLE cProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, cpid);
    if (cProcess == INVALID_HANDLE_VALUE)
        PFATAL("OpenProcess");

    int rv = 0;

    while (true) {

        if (!GetExitCodeProcess(pProcess, &state))
            PFATAL("GetExitCodeProcess");
        // printf("state: %d\n", state);
        if (state != STILL_ACTIVE) {
            printf("parent process is dead, terminating "
                   "child process tree ...\n");
            // exit code arbitrarily set to 15 to signal termination
            if (!TerminateProcess(cProcess, 15)) {
                PERROR("TerminateProcess");
                rv = 1;
            }
            rv |= terminateChildren(cpid, 15);
            break;
        }

        if (!GetExitCodeProcess(cProcess, &state))
            PFATAL("GetExitCodeProcess");
        if (state != STILL_ACTIVE) {
            break;
        }

        Sleep(500);  // milliseconds
    }

    if (!CloseHandle(pProcess))
        PFATAL("CloseHandle(ppid)");

    if (!CloseHandle(cProcess))
        PFATAL("CloseHandle(cpid)");

    return rv;
}
#endif

void showUsage()
{
    fprintf(stderr, "%s", "usage: pcleaner ppid cpid [signum]\n");
    exit(1);
}

int main(int argc, char *argv[]) {

    if (argc < 3 || argc > 4)
        showUsage();

    int ppid = atoi(argv[1]);
    int cpid = atoi(argv[2]);
    int signum = 15;
    if (argc == 4) {
        signum = atoi(argv[3]);
    }

    return run(ppid, cpid, signum);
}
