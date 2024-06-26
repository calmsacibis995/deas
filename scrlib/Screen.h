//
// General declarations for screen management package.
//
// Copyright (C) 1994 Cronyx Ltd.
// Author: Serge Vakulenko, <vak@zebub.msk.su>
//
// This software is distributed with NO WARRANTIES, not even the implied
// warranties for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// Authors grant any other persons or organisations permission to use
// or modify this software as long as this message is kept with the software,
// all derivative works or modified versions.
//
#include <memory>

#include <stdarg.h>

#define CYRILLIC // enable internal cyrillics (CY, Cs, Ce, Ct)

#define cntrl(c) ((c) & 0x1f)
#define meta(c) ((c) | 0x100)
#define alt(c) ((c) | 0x200)

class Screen;

class Box {
    friend class Screen;

private:
    short y, x;
    short ny, nx;
    short *mem;

public:
    Box(Screen &scr, int by, int bx, int bny, int bnx);
    ~Box()
    {
        if (mem)
            delete mem;
    }
};

class Cursor {
    friend class Screen;

private:
    short y, x;

public:
    Cursor(Screen &scr);
};

//
// Defines for Unix.
//

struct Keytab {
    const char *tcap;
    const char *str;
    short val;
};

struct TtyPrivate;
struct KeyPrivate;

class Screen {
    friend class Cursor;
    friend class Box;

private:
    int cury, curx;
    unsigned flgs;
    int clear;
    short **scr;
    short *firstch;
    short *lastch;
    short *lnum;

    int oldy, oldx;
    unsigned oldflgs;
    short **oldscr;

    int Visuals;    // Маска видео возможностей дисплея
    int VisualMask; // Маска разрешенных атрибутов

    int NormalAttr;
    int ClearAttr;
    static const int GraphAttr = 0x8000;

    static const int VisualBold = 1;    // Есть повышенная яркость (md)
    static const int VisualDim = 2;     // Есть пониженная яркость (mh)
    static const int VisualInverse = 4; // Есть инверсия (so или mr)
    static const int VisualColors = 8;  // Есть цвета (NF, NB, CF, CB, C2, MF, MB)
    static const int VisualGraph = 16;  // Есть псевдографика (G1, G2, GT)

    int UpperCaseMode;

    int scrool, rscrool;
    int beepflag;
    int hidden;

    short ctab[16], btab[16];
    char outbuf[256], *outptr;

    unsigned char linedraw[11];

#ifdef CYRILLIC
    unsigned char CyrInputTable['~' - ' ' + 1];
    char CyrOutputTable[64];
    int cyrinput;
    int cyroutput;
    void cyr(int cflag);
#endif

    std::unique_ptr<TtyPrivate> ttydata;
    std::unique_ptr<KeyPrivate> keydata;

    friend void _Screen_tstp();

    int Init();
    void Open();
    void Flush();

    void InitKey(Keytab *keymap = 0);

    int InitCap(char *buf);
    void GetCap(struct Captab *ct);

    void SetTty();
    void ResetTty();
    void FlushTtyInput();
    int InputChar();

    void setAttr(int attr);
    void setColor(int fg, int bg);
    void moveTo(int y, int x);
    void pokeChar(int y, int x, int c);
    void putChar(unsigned char c);
    int putRawChar(unsigned char c);
    void clearScreen();
    const char *skipDelay(const char *str);
    void scroolScreen();
    void initChoice(int row, int col, const char *c1, const char *c2, const char *c3);
    int menuChoice(int c, int i);
    char *editString(int r, int c, int w, char *str, int cp, int color);
    int inputKey(struct Keytab *kp);

public:
    int Lines;
    int Columns;

    static const int ULC = '\20';
    static const int UT = '\21';
    static const int URC = '\22';
    static const int CLT = '\23';
    static const int CX = '\24';
    static const int CRT = '\25';
    static const int LLC = '\26';
    static const int LT = '\27';
    static const int LRC = '\30';
    static const int VERT = '\31';
    static const int HOR = '\32';

    Screen(int colormode = 2, int graphmode = 1);
    ~Screen();
    void Close();
    void Restore();
    void Reopen();

    int Col() { return (curx); }
    int Row() { return (cury); }

    int HasDim() { return (Visuals & VisualMask & (VisualColors | VisualDim)); }
    int HasBold() { return (Visuals & VisualMask & (VisualColors | VisualBold)); }
    int HasInverse() { return (Visuals & VisualMask & (VisualColors | VisualInverse)); }
    int HasColors() { return (Visuals & VisualMask & VisualColors); }
    int HasGraph() { return (Visuals & VisualMask & VisualGraph); }

    void Clear(int attr = 0);
    void Clear(int y, int x, int ny, int nx, int attr = 0, int sym = ' ');
    void ClearLine(int y, int x, int attr = 0);
    void ClearLine(int attr) { ClearLine(cury, curx, attr); }

    void ClearOk(int ok) { clear = ok; }

    // It is possible to set current column
    // to the left from the left margin
    // or to the right of the right margin.
    // Printed text will be clipped properly.
    void Move(int y, int x)
    {
        curx = x;
        cury = y;
        hidden = 0;
    }

    void Put(int c, int attr = 0);
    void Put(Box &box, int attr = 0);
    void Put(const char *str, int attr = 0)
    {
        while (*str)
            Put(*str++, attr);
    }

    void Put(int y, int x, int ch, int attr = 0)
    {
        Move(y, x);
        Put(ch, attr);
    }
    void Put(int y, int x, const char *str, int attr = 0)
    {
        Move(y, x);
        Put(str, attr);
    }
    void Put(int y, int x, Box &box, int attr = 0)
    {
        Move(y, x);
        Put(box, attr);
    }
    void PutLimited(const char *str, int lim, int attr = 0)
    {
        while (--lim >= 0 && *str)
            Put(*str++, attr);
    }
    void PutLimited(int y, int x, const char *str, int lim, int attr = 0)
    {
        Move(y, x);
        while (--lim >= 0 && *str)
            Put(*str++, attr);
    }

    void Print(int attr, const char *fmt, ...);
    void Print(int y, int x, int attr, const char *fmt, ...);
    void PrintVect(int attr, const char *fmt, va_list vect);
    void PrintVect(int y, int x, int attr, const char *fmt, va_list vect)
    {
        Move(y, x);
        PrintVect(attr, fmt, vect);
    }

    int GetChar(int y, int x) { return scr[y][x]; }
    int GetChar() { return GetChar(cury, curx); }

    int GetKey();
    void UngetKey(int key);
    void AddHotKey(int key, void (*f)(int));
    void DelHotKey(int key);

    void SetCursor(Cursor c) { Move(c.y, c.x); }
    void HideCursor() { hidden = 1; }

    void Sync(int y);
    void Sync();
    void Redraw()
    {
        clearScreen();
        Sync();
    }
    void Beep() { beepflag = 1; }

    void DelLine(int line, int attr = 0);
    void InsLine(int line, int attr = 0);

    void Error(int c, int i, const char *head, const char *reply, const char *s, ...);
    int Popup(const char *head, const char *mesg, const char *mesg2, const char *c1, const char *c2,
              const char *c3, int c, int i);
    char *GetString(int w, const char *str, const char *head, const char *mesg, int c, int i);
    void PopupString(const char *title, const char *str, const char *reply, int color, int inverse);
    int SelectFromList(int color, int inverse, char *head, char *mesg, int cnt, ...);
    int SelectFromTable(int color, int inverse, char *head, char *mesg, int cnt, char *table[]);

    void HorLine(int y, int x, int nx, int attr = 0);
    void VertLine(int x, int y, int ny, int attr = 0);
    void DrawFrame(int y, int x, int ny, int nx, int attr = 0);

    void AttrSet(int y, int x, int ny, int nx, int attr = 0);
    void AttrLow(int y, int x);

    void putStr(const char *s);
    const char *Goto(const char *movestr, int x, int y);
};

inline Cursor::Cursor(Screen &scr)
{
    y = scr.cury;
    x = scr.curx;
}
