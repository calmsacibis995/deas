extern "C" {
#if defined(__linux__)      // Temporary workaround
#define TERMIOS
#ifdef TERMIOS
#include <sys/termios.h>
#include <sys/ioctl.h>
#define TERMIO
#if !defined(__linux__)
#define termio termios
#define TCGETA TIOCGETA
#define TCSETA TIOCSETA
#define TCSETAW TIOCSETAW
#endif
#ifndef OLCUC
#define OLCUC 0
#endif
#ifndef IUCLC
#define IUCLC 0
#endif
#ifndef OCRNL
#define OCRNL 0
#endif
#ifndef XCASE
#define XCASE 0
#endif
#else
#ifdef TERMIO
#include <termio.h>
#else
#include <sgtty.h>
#endif
#endif
#endif
};

struct TtyPrivate {
#if defined(TERMIO) || defined(TERMIOS)
    struct termio oldtio, newtio;
#else
    struct sgttyb tty;
    int ttyflags;
#ifdef TIOCSETC
    struct tchars oldtchars, newtchars;
#endif
#endif

#ifdef NEEDLITOUT
    int oldlocal, newlocal;
#endif

#ifdef TIOCSLTC
    struct ltchars oldchars, newchars;
#endif
};
