#include <QDir>
#include <QDirIterator>
#include <QString>
#include <libssh/sftp.h>
#include <memory>
#include <unordered_map>

#include "ssh.h"

template<typename Func>
class ScopeGuard
{
public:
  explicit ScopeGuard(const Func& func) : func_(func) { }
  explicit ScopeGuard(Func&& func) : func_(std::move(func)) { }

  ScopeGuard(ScopeGuard&& other)
  noexcept(std::is_nothrow_move_constructible<Func>::value)
  : func_(std::move_if_noexcept(other.func_)), active_(other.active_)
  {
    other.active_ = false;
  }

  /// execute final action
  ~ScopeGuard() { if (active_) func_(); }

  // non-copyable
  ScopeGuard(const ScopeGuard&) = delete;
  ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
  Func func_;
  bool active_ = true;
};

template<typename Func>
ScopeGuard<Func> finally(Func&& func)
{
  return ScopeGuard<Func>(std::forward<Func>(func));
}

class SftpServerSession {
public:
    SftpServerSession(sftp_session sftp);

    int init();
    bool readMessage();

private:
    QString getFilename(sftp_client_message msg);
    QString getDataFilename(sftp_client_message msg);
    QString getFilename(const char* raw);

    sftp_attributes_struct getFileAttrs(const QFileInfo& fileInfo);

    bool replyReadLink(sftp_client_message msg);
    bool replyRealPath(sftp_client_message msg);
    bool replyStat(sftp_client_message msg, bool link);

    bool replyOpenDir(sftp_client_message msg);
    bool replyReadDir(sftp_client_message msg);
    bool replyClose(sftp_client_message msg);

    bool replyOpenFile(sftp_client_message msg);
    bool replyReadFile(sftp_client_message msg);
    bool replyWriteFile(sftp_client_message msg);

    bool replyRemove(sftp_client_message msg);
    bool replyMakeDir(sftp_client_message msg);
    bool replyRemoveDir(sftp_client_message msg);
    bool replyRename(sftp_client_message msg);
    bool replySymlink(sftp_client_message msg);

    SftpSessionPtr m_sftpSession;
    QDir m_cwd;

    std::unordered_map<void*, std::unique_ptr<QDirIterator>> m_openDirHandles;
    std::unordered_map<void*, std::unique_ptr<QFile>> m_openFileHandles;
};
