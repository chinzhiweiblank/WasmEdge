// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2019-2022 Second State INC

#include "common/defines.h"
#if !WASMEDGE_OS_MACOS
#error
#endif

#define _DARWIN_C_SOURCE
#include "common/errcode.h"
#include "wasi/api.hpp"
#include <Availability.h>
#include <AvailabilityMacros.h>
#include <TargetConditionals.h>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <dirent.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/event.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace WasmEdge {
namespace Host {
namespace WASI {
inline namespace detail {

inline constexpr __wasi_errno_t fromErrNo(int ErrNo) noexcept {
  switch (ErrNo) {
  case 0:
    return __WASI_ERRNO_SUCCESS;
  case E2BIG:
    return __WASI_ERRNO_2BIG;
  case EACCES:
    return __WASI_ERRNO_ACCES;
  case EADDRINUSE:
    return __WASI_ERRNO_ADDRINUSE;
  case EADDRNOTAVAIL:
    return __WASI_ERRNO_ADDRNOTAVAIL;
  case EAFNOSUPPORT:
    return __WASI_ERRNO_AFNOSUPPORT;
  case EAGAIN:
    return __WASI_ERRNO_AGAIN;
  case EALREADY:
    return __WASI_ERRNO_ALREADY;
  case EBADF:
    return __WASI_ERRNO_BADF;
  case EBADMSG:
    return __WASI_ERRNO_BADMSG;
  case EBUSY:
    return __WASI_ERRNO_BUSY;
  case ECANCELED:
    return __WASI_ERRNO_CANCELED;
  case ECHILD:
    return __WASI_ERRNO_CHILD;
  case ECONNABORTED:
    return __WASI_ERRNO_CONNABORTED;
  case ECONNREFUSED:
    return __WASI_ERRNO_CONNREFUSED;
  case ECONNRESET:
    return __WASI_ERRNO_CONNRESET;
  case EDEADLK:
    return __WASI_ERRNO_DEADLK;
  case EDESTADDRREQ:
    return __WASI_ERRNO_DESTADDRREQ;
  case EDOM:
    return __WASI_ERRNO_DOM;
  case EDQUOT:
    return __WASI_ERRNO_DQUOT;
  case EEXIST:
    return __WASI_ERRNO_EXIST;
  case EFAULT:
    return __WASI_ERRNO_FAULT;
  case EFBIG:
    return __WASI_ERRNO_FBIG;
  case EHOSTUNREACH:
    return __WASI_ERRNO_HOSTUNREACH;
  case EIDRM:
    return __WASI_ERRNO_IDRM;
  case EILSEQ:
    return __WASI_ERRNO_ILSEQ;
  case EINPROGRESS:
    return __WASI_ERRNO_INPROGRESS;
  case EINTR:
    return __WASI_ERRNO_INTR;
  case EINVAL:
    return __WASI_ERRNO_INVAL;
  case EIO:
    return __WASI_ERRNO_IO;
  case EISCONN:
    return __WASI_ERRNO_ISCONN;
  case EISDIR:
    return __WASI_ERRNO_ISDIR;
  case ELOOP:
    return __WASI_ERRNO_LOOP;
  case EMFILE:
    return __WASI_ERRNO_MFILE;
  case EMLINK:
    return __WASI_ERRNO_MLINK;
  case EMSGSIZE:
    return __WASI_ERRNO_MSGSIZE;
  case EMULTIHOP:
    return __WASI_ERRNO_MULTIHOP;
  case ENAMETOOLONG:
    return __WASI_ERRNO_NAMETOOLONG;
  case ENETDOWN:
    return __WASI_ERRNO_NETDOWN;
  case ENETRESET:
    return __WASI_ERRNO_NETRESET;
  case ENETUNREACH:
    return __WASI_ERRNO_NETUNREACH;
  case ENFILE:
    return __WASI_ERRNO_NFILE;
  case ENOBUFS:
    return __WASI_ERRNO_NOBUFS;
  case ENODEV:
    return __WASI_ERRNO_NODEV;
  case ENOENT:
    return __WASI_ERRNO_NOENT;
  case ENOEXEC:
    return __WASI_ERRNO_NOEXEC;
  case ENOLCK:
    return __WASI_ERRNO_NOLCK;
  case ENOLINK:
    return __WASI_ERRNO_NOLINK;
  case ENOMEM:
    return __WASI_ERRNO_NOMEM;
  case ENOMSG:
    return __WASI_ERRNO_NOMSG;
  case ENOPROTOOPT:
    return __WASI_ERRNO_NOPROTOOPT;
  case ENOSPC:
    return __WASI_ERRNO_NOSPC;
  case ENOSYS:
    return __WASI_ERRNO_NOSYS;
  case ENOTCONN:
    return __WASI_ERRNO_NOTCONN;
  case ENOTDIR:
    return __WASI_ERRNO_NOTDIR;
  case ENOTEMPTY:
    return __WASI_ERRNO_NOTEMPTY;
  case ENOTRECOVERABLE:
    return __WASI_ERRNO_NOTRECOVERABLE;
  case ENOTSOCK:
    return __WASI_ERRNO_NOTSOCK;
  case ENOTSUP:
    return __WASI_ERRNO_NOTSUP;
  case ENOTTY:
    return __WASI_ERRNO_NOTTY;
  case ENXIO:
    return __WASI_ERRNO_NXIO;
  case EOVERFLOW:
    return __WASI_ERRNO_OVERFLOW;
  case EOWNERDEAD:
    return __WASI_ERRNO_OWNERDEAD;
  case EPERM:
    return __WASI_ERRNO_PERM;
  case EPIPE:
    return __WASI_ERRNO_PIPE;
  case EPROTO:
    return __WASI_ERRNO_PROTO;
  case EPROTONOSUPPORT:
    return __WASI_ERRNO_PROTONOSUPPORT;
  case EPROTOTYPE:
    return __WASI_ERRNO_PROTOTYPE;
  case ERANGE:
    return __WASI_ERRNO_RANGE;
  case EROFS:
    return __WASI_ERRNO_ROFS;
  case ESPIPE:
    return __WASI_ERRNO_SPIPE;
  case ESRCH:
    return __WASI_ERRNO_SRCH;
  case ESTALE:
    return __WASI_ERRNO_STALE;
  case ETIMEDOUT:
    return __WASI_ERRNO_TIMEDOUT;
  case ETXTBSY:
    return __WASI_ERRNO_TXTBSY;
  case EXDEV:
    return __WASI_ERRNO_XDEV;
  default:
    assumingUnreachable();
  }
}

inline constexpr __wasi_errno_t fromEAIErrNo(int ErrNo) noexcept {
  switch (ErrNo) {
  case EAI_ADDRFAMILY:
    return __WASI_ERRNO_AIADDRFAMILY;
  case EAI_AGAIN:
    return __WASI_ERRNO_AIAGAIN;
  case EAI_BADFLAGS:
    return __WASI_ERRNO_AIBADFLAG;
  case EAI_FAIL:
    return __WASI_ERRNO_AIFAIL;
  case EAI_FAMILY:
    return __WASI_ERRNO_AIFAMILY;
  case EAI_MEMORY:
    return __WASI_ERRNO_AIMEMORY;
  case EAI_NODATA:
    return __WASI_ERRNO_AINODATA;
  case EAI_NONAME:
    return __WASI_ERRNO_AINONAME;
  case EAI_SERVICE:
    return __WASI_ERRNO_AISERVICE;
  case EAI_SOCKTYPE:
    return __WASI_ERRNO_AISOCKTYPE;
  case EAI_SYSTEM:
    return __WASI_ERRNO_AISYSTEM;
  default:
    assumingUnreachable();
  }
}

inline constexpr clockid_t toClockId(__wasi_clockid_t Clock) noexcept {
  switch (Clock) {
  case __WASI_CLOCKID_REALTIME:
    return CLOCK_REALTIME;
  case __WASI_CLOCKID_MONOTONIC:
    return CLOCK_MONOTONIC;
  case __WASI_CLOCKID_PROCESS_CPUTIME_ID:
    return CLOCK_PROCESS_CPUTIME_ID;
  case __WASI_CLOCKID_THREAD_CPUTIME_ID:
    return CLOCK_THREAD_CPUTIME_ID;
  default:
    assumingUnreachable();
  }
}

inline constexpr timespec toTimespec(__wasi_timestamp_t Timestamp) noexcept {
  using namespace std::chrono;
  const auto Total = nanoseconds(Timestamp);
  const auto Second = duration_cast<seconds>(Total);
  const auto Nano = Total - Second;
  timespec Result{};
  Result.tv_sec = Second.count();
  Result.tv_nsec = Nano.count();
  return Result;
}

inline constexpr __wasi_timestamp_t
fromTimespec(const timespec &Time) noexcept {
  using namespace std::chrono;
  const auto Result = seconds(Time.tv_sec) + nanoseconds(Time.tv_nsec);
  return Result.count();
}

inline constexpr timeval toTimeval(__wasi_timestamp_t Timestamp) noexcept {
  using namespace std::chrono;
  const auto Total = duration_cast<microseconds>(nanoseconds(Timestamp));
  const auto Second = duration_cast<seconds>(Total);
  const auto Micro = Total - Second;
  timeval Result{};
  Result.tv_sec = Second.count();
  Result.tv_usec = Micro.count();
  return Result;
}
inline constexpr timeval toTimeval(timespec Timespec) noexcept {
  using namespace std::chrono;
  timeval Result{};
  Result.tv_sec = Timespec.tv_sec;
  Result.tv_usec =
      duration_cast<microseconds>(nanoseconds(Timespec.tv_nsec)).count();
  return Result;
}

inline constexpr __wasi_filetype_t fromFileType(mode_t Mode) noexcept {
  switch (Mode & S_IFMT) {
  case S_IFBLK:
    return __WASI_FILETYPE_BLOCK_DEVICE;
  case S_IFCHR:
    return __WASI_FILETYPE_CHARACTER_DEVICE;
  case S_IFDIR:
    return __WASI_FILETYPE_DIRECTORY;
  case S_IFREG:
    return __WASI_FILETYPE_REGULAR_FILE;
  case S_IFSOCK:
    return __WASI_FILETYPE_SOCKET_STREAM;
  case S_IFLNK:
    return __WASI_FILETYPE_SYMBOLIC_LINK;
  case S_IFIFO:
  default:
    return __WASI_FILETYPE_UNKNOWN;
  }
}

inline constexpr __wasi_filetype_t fromFileType(uint8_t Type) noexcept {
  switch (Type) {
  case DT_BLK:
    return __WASI_FILETYPE_BLOCK_DEVICE;
  case DT_CHR:
    return __WASI_FILETYPE_CHARACTER_DEVICE;
  case DT_DIR:
    return __WASI_FILETYPE_DIRECTORY;
  case DT_LNK:
    return __WASI_FILETYPE_SYMBOLIC_LINK;
  case DT_REG:
    return __WASI_FILETYPE_REGULAR_FILE;
  case DT_SOCK:
    return __WASI_FILETYPE_SOCKET_STREAM;
  case DT_FIFO:
  case DT_UNKNOWN:
  default:
    return __WASI_FILETYPE_UNKNOWN;
  }
}

inline constexpr int toWhence(__wasi_whence_t Whence) noexcept {
  switch (Whence) {
  case __WASI_WHENCE_CUR:
    return SEEK_CUR;
  case __WASI_WHENCE_END:
    return SEEK_END;
  case __WASI_WHENCE_SET:
    return SEEK_SET;
  default:
    assumingUnreachable();
  }
}

#if __has_builtin(__builtin_available)
#define available(macos_major, macos_minor, macos_patch, ios_major, ios_minor, \
                  ios_patch, tvos_major, tvos_minor, tvos_patch,               \
                  watchos_major, watchos_minor, watchos_patch)                 \
  __builtin_available(                                                         \
      macOS macos_major##.##macos_minor##.##macos_patch,                       \
      iOS ios_major##.##ios_minor##.##ios_patch,                               \
      tvOS tvos_major##.##tvos_minor##.##tvos_patch,                           \
      watchOS watchos_major##.##watchos_minor##.##watchos_patch, *)
#else

bool darwinAvailable(unsigned int Major, unsigned int Minor,
                     unsigned int Patch) noexcept;
#if defined(TARGET_OS_WATCH) && TARGET_OS_WATCH
#define available(macos_major, macos_minor, macos_patch, ios_major, ios_minor, \
                  ios_patch, tvos_major, tvos_minor, tvos_patch,               \
                  watchos_major, watchos_minor, watchos_patch)                 \
  darwinAvailable(watchos_major, watchos_minor, watchos_patch)
#elif defined(TARGET_OS_TV) && TARGET_OS_TV
#define available(macos_major, macos_minor, macos_patch, ios_major, ios_minor, \
                  ios_patch, tvos_major, tvos_minor, tvos_patch,               \
                  watchos_major, watchos_minor, watchos_patch)                 \
  darwinAvailable(tvos_major, tvos_minor, tvos_patch)
#elif defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#define available(macos_major, macos_minor, macos_patch, ios_major, ios_minor, \
                  ios_patch, tvos_major, tvos_minor, tvos_patch,               \
                  watchos_major, watchos_minor, watchos_patch)                 \
  darwinAvailable(ios_major, ios_minor, ios_patch)
#elif defined(TARGET_OS_MAC) && TARGET_OS_MAC
#define available(macos_major, macos_minor, macos_patch, ios_major, ios_minor, \
                  ios_patch, tvos_major, tvos_minor, tvos_patch,               \
                  watchos_major, watchos_minor, watchos_patch)                 \
  darwinAvailable(macos_major, macos_minor, macos_patch)
#else
#error Unknown platform!
#endif

#endif

} // namespace detail
} // namespace WASI
} // namespace Host
} // namespace WasmEdge
