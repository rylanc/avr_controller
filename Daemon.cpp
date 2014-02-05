#include "pre.h"
#include "Daemon.h"

#include <system_error>
#include <cstdio>
#include <syslog.h>

inline void throw_sys_err(const char* msg)
{ throw std::system_error(errno, std::system_category(), msg); }

Daemon::Daemon(boost::asio::io_service &io_service, const std::string &pidfile,
               const std::string &logfile) :
  m_signals(io_service, SIGINT, SIGTERM), m_pid_file(pidfile), m_lock_fd(-1)
{
  // Register signal handlers so that the daemon may be shut down. You may
  // also want to register for other signals, such as SIGHUP to trigger a
  // re-read of a configuration file
  m_signals.async_wait([&](const boost::system::error_code&, int)
		                 { io_service.stop(); });
                     
  // Inform the io_service that we are about to become a daemon. The
  // io_service cleans up any internal resources, such as threads, that may
  // interfere with forking
  io_service.notify_fork(boost::asio::io_service::fork_prepare);
  
  // Fork the process and have the parent exit. If the process was started
  // from a shell, this returns control to the user. Forking a new process is
  // also a prerequisite for the subsequent call to setsid()
  if (pid_t pid = fork()) {
    if (pid > 0) {
        // We're in the parent process and need to exit
        exit(0);
      }
      else
        throw_sys_err("First fork failed");
    }
    
    // Make the process a new session leader. This detaches it from the
    // terminal
    setsid();

    // A process inherits its working directory from its parent. This could be
    // on a mounted filesystem, which means that the running daemon would
    // prevent this filesystem from being unmounted. Changing to the root
    // directory avoids this problem
    chdir("/");

    // The file mode creation mask is also inherited from the parent process.
    // We don't want to restrict the permissions on files created by the
    // daemon, so the mask is cleared
    umask(0);

    // A second fork ensures the process cannot acquire a controlling terminal
    if (pid_t pid = fork()) {
      if (pid > 0)
        exit(0);
      else
        throw_sys_err("Second fork failed");
    }
    
    // Close the standard streams. This decouples the daemon from the terminal
    // that started it
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // We don't want the daemon to have any standard input
    if (open("/dev/null", O_RDONLY) < 0)
      throw_sys_err("Unable to open /dev/null");

    // Send standard output/error to a log file
    int log_fd = open(logfile.c_str(), O_WRONLY | O_CREAT | O_APPEND,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (log_fd < 0)
      throw_sys_err("Unable to open log file");
  
    dup2(log_fd, STDOUT_FILENO);
    setvbuf(stdout, NULL, _IOLBF, BUFSIZ);
  
    dup2(log_fd, STDERR_FILENO);
    setvbuf(stderr, NULL, _IOLBF, BUFSIZ);
    
    m_lock_fd = open(m_pid_file.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (m_lock_fd < 0) {
      throw_sys_err("Unable to open pidfile");
    }
    
    if (lockf(m_lock_fd, F_TLOCK, 0)) {
      throw_sys_err("Unable to lock pidfile");
    }
    
    dprintf(m_lock_fd, "%d\n", getpid());

    // Inform the io_service that we have finished becoming a daemon. The
    // io_service uses this opportunity to create any internal file descriptors
    // that need to be private to the new process
    io_service.notify_fork(boost::asio::io_service::fork_child);
    
    // The io_service can now be used normally
    syslog(LOG_INFO | LOG_USER, "Daemon started");
}

Daemon::~Daemon()
{
  syslog(LOG_INFO | LOG_USER, "Daemon stopped");
  if (m_lock_fd > 0) {
    lockf(m_lock_fd, F_ULOCK, 0);
    close(m_lock_fd);
    unlink(m_pid_file.c_str());
    m_lock_fd = -1;
  }
}
