//
//  Multi Media Card User Interface  - prototypes
//  

#define CMDLINE_BAD_CMD         (-1)  // return value for cmd not found
#define CMDLINE_TOO_MANY_ARGS   (-2)  // return value for too many args
#define CMD_BUF_SIZE    512
#define PATH_BUF_SIZE   512  // must hold full path name, file name & null term
#define DATA_BUF_SIZE   64 * (2 * 512)  // buf size to hold temp data

extern char CmdBuf[CMD_BUF_SIZE];
extern unsigned char CmdBufIdx;

typedef int (*pfnCmdLine)(int argc, char *argv[]);  // Cmd line function callback type

typedef struct {  // struct for entries in cmd list table
    const char *pcCmd;  // ptr to string of cmd name
    pfnCmdLine pfnCmd;  // func ptr to actual cmd func
    const char *pcHelp;  // ptr to string of help info of cmd
} tCmdLineEntry;

extern tCmdLineEntry CmdTable[];  // cmd table provided by uif application
extern void char_collect(unsigned char KeyStroke);
extern int CmdLineParse(char *pcCmdLine);
extern void CmdLineProcess(char *CmdBuf);
extern int Cmd_udptx(int argc, char *argv[]);
extern int Cmd_udplog(int argc, char *argv[]);
extern int Cmd_udplog0(int argc, char *argv[]);
