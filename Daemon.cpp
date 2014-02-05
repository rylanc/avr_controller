#include "Daemon.h"

#include <iostream>
#include <syslog.h>
#include <unistd.h>
#include <stdio.h>

Daemon::Daemon(boost::asio::io_service &io_service) : m_signals(io_service), m_lock_fd(-1) {}

Daemon::~Daemon()
{
  syslog(LOG_INFO | LOG_USER, "Daemon stopped");
  if (m_lock_fd > 0) {
    lockf(m_lock_fd, F_ULOCK, 0);
    close(m_lock_fd);
    //unlink(config.pidfile);
    m_lock_fd = -1;
  }
}

int Daemon::init(const std::string &pidfile)
{
  boost::asio::io_service &io_service = m_signals.get_io_service();

  // Register signal handlers so that the daemon may be shut down. You may
  // also want to register for other signals, such as SIGHUP to trigger a
  // re-read of a configuration file.
  m_signals.add(SIGINT);
  m_signals.add(SIGTERM);
  m_signals.async_wait([&](const boost::system::error_code&, int)
		       { io_service.stop(); });
                     
  // Inform the io_service that we are about to become a daemon. The
  // io_service cleans up any internal resources, such as threads, that may
  // interfere with forking.
  io_service.notify_fork(boost::asio::io_service::fork_prepare);
  
  // Fork the process and have the parent exit. If the process was started
  // from a shell, this returns control to the user. Forking a new process is
  // also a prerequisite for the subsequent call to setsid().
  if (pid_t pid = fork()) {
    if (pid > 0) {
        // We're in the parent process and need to exit.
        //
        // When the exit() function is used, the program terminates without
        // invoking local variables' destructors. Only global variables are
        // destroyed. As the io_service object is a local variable, this means
        // we do not have to call:
        //
        //   io_service.notify_fork(boost::asio::io_service::fork_parent);
        //
        // However, this line should be added before each call to exit() if
        // using a global io_service object. An additional call:
        //
        //   io_service.notify_fork(boost::asio::io_service::fork_prepare);
        //
        // should also precede the second fork().
        exit(0);
      }
      else {
        syslog(LOG_ERR | LOG_USER, "First fork failed: %m");
        return 1;
      }
    }
    
    // Make the process a new session leader. This detaches it from the
    // terminal.
    setsid();

    // A process inherits its working directory from its parent. This could be
    // on a mounted filesystem, which means that the running daemon would
    // prevent this filesystem from being unmounted. Changing to the root
    // directory avoids this problem.
    chdir("/");

    // The file mode creation mask is also inherited from the parent process.
    // We don't want to restrict the permissions on files created by the
    // daemon, so the mask is cleared.
    umask(0);

    // A second fork ensures the process cannot acquire a controlling terminal.
    if (pid_t pid = fork()) {
      if (pid > 0)
        exit(0);
      else {
        syslog(LOG_ERR | LOG_USER, "Second fork failed: %m");
        return 1;
      }
    }
    
    // Close the standard streams. This decouples the daemon from the terminal
    // that started it.
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // We don't want the daemon to have any standard input.
    if (open("/dev/null", O_RDONLY) < 0)
    {
      syslog(LOG_ERR | LOG_USER, "Unable to open /dev/null: %m");
      return 1;
    }

    // Send standard output to a log file.
    const char* output = "/tmp/asio.daemon.out";
    const int flags = O_WRONLY | O_CREAT | O_APPEND;
    const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    if (open(output, flags, mode) < 0)
    {
      syslog(LOG_ERR | LOG_USER, "Unable to open output file %s: %m", output);
      return 1;
    }

    // Also send standard error to the same log file.
    if (dup(1) < 0)
    {
      syslog(LOG_ERR | LOG_USER, "Unable to dup output descriptor: %m");
      return 1;
    }
    
    m_lock_fd = open(pidfile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (m_lock_fd < 0) {
      // error
    }
    
    if (lockf(m_lock_fd, F_TLOCK, 0)) {
      // error
    }
    
    dprintf(m_lock_fd, "%d\n", getpid());

    // Inform the io_service that we have finished becoming a daemon. The
    // io_service uses this opportunity to create any internal file descriptors
    // that need to be private to the new process.
    io_service.notify_fork(boost::asio::io_service::fork_child);
    
    // The io_service can now be used normally.
    syslog(LOG_INFO | LOG_USER, "Daemon started");

  return 0;
}


