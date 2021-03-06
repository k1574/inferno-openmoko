#define Rectangle   WRectangle
#include <windows.h>
#undef Rectangle

#include <dat.h>
#include <fns.h>
#include <error.h>
#include <draw.h>
#include <keyboard.h>
#include <cursor.h>

extern ulong displaychan;

extern  char*   runestoutf(char*, const Rune*, int);
extern  int bytesperline(Rectangle, int);
extern  NORETURN main(int argc, char **argv);
static  void    dprint(__in_z __format_string const char*, ...);
static  DWORD WINAPI    winproc(LPVOID);

static  HINSTANCE   inst = 0;
static  HINSTANCE   previnst = 0;
static  int     cmdshow = SW_SHOW;
static  HWND        window = 0;
static  HDC     screen = 0;
static  HPALETTE    palette = 0;
static  int     maxxsize = 0;
static  int     maxysize = 0;
static  int     isunicode = 1;
static  HCURSOR     hcursor = 0;

char    *argv0 = "inferno";

static  int     winscreen_attached = 0;


extern  DWORD   PlatformId;
char*   gkscanid = "emu_win32vk";
#ifndef __TINYC__
int WINAPI
WinMain(HINSTANCE winst, HINSTANCE wprevinst, LPSTR cmdline, int wcmdshow)
{
    inst = winst;
    previnst = wprevinst;
    cmdshow = wcmdshow;

    /* cmdline passed into WinMain does not contain name of executable.
     * The globals __argc and __argv to include this info - like UNIX
     */
    main(__argc, __argv);
    return 0;
}
#endif
static void
dprint(__in_z __format_string const char *fmt, ...)
{
    va_list arg;
    char buf[128];

    va_start(arg, fmt);
    vseprint(buf, buf+sizeof(buf), fmt, (LPSTR)arg);
    va_end(arg);
    OutputDebugStringA("inferno: ");
    OutputDebugStringA(buf);
}

static void
graphicscmap(PALETTEENTRY *pal)
{
    int r, g, b, cr, cg, cb, v, p;
    int num, den;
    int i, j;
    for(r=0,i=0;r!=4;r++) for(v=0;v!=4;v++,i+=16){
        for(g=0,j=v-r;g!=4;g++) for(b=0;b!=4;b++,j++){
            den=r;
            if(g>den) den=g;
            if(b>den) den=b;
            if(den==0)  /* divide check -- pick grey shades */
                cr=cg=cb=v*17;
            else{
                num=17*(4*den+v);
                cr=r*num/den;
                cg=g*num/den;
                cb=b*num/den;
            }
            p = i+(j&15);
            pal[p].peRed = cr*0x01010101;
            pal[p].peGreen = cg*0x01010101;
            pal[p].peBlue = cb*0x01010101;
            pal[p].peFlags = 0;
        }
    }
}

static void
graphicsgmap(PALETTEENTRY *pal, int d)
{
    int i, j, s, m, p;

    s = 8-d;
    m = 1;
    while(--d >= 0)
        m *= 2;
    m = 255/(m-1);
    for(i=0; i < 256; i++){
        j = (i>>s)*m;
        p = 255-i;
        pal[p].peRed = pal[p].peGreen = pal[p].peBlue = (255-j)*0x01010101;
        pal[p].peFlags = 0;
    }
}

static ulong
autochan(void)
{
    HDC dc;
    int bpp;

    dc = GetDC(NULL);
    if (dc == NULL)
        return CMAP8;

    bpp = GetDeviceCaps(dc, BITSPIXEL);
    if (bpp < 15)
        return CMAP8;
    if (bpp < 24)
        return RGB15;
    if (bpp < 32)
        return RGB24;
    return XRGB32;
}

char*
attachscreen(__out Rectangle *r,
             __out ulong *chan,
             __out int *d,
             __out int *width,
             __out int *softscreen)
{
    int i;
    ulong c;
    DWORD h;
    RECT bs;
    RGBQUAD *rgb;
    HBITMAP bits;
    BITMAPINFO *bmi;
    LOGPALETTE *logpal;
    PALETTEENTRY *pal;
    int bsh, bsw, sx, sy;

    static  int     winscreen_k = 0;
    static  char*   winscreen_data = 0;

    if(winscreen_attached)
    {
        c = displaychan;
        goto Return;
    }

    /* Compute bodersizes */
    memset(&bs, 0, sizeof(bs));
    AdjustWindowRect(&bs, WS_OVERLAPPEDWINDOW, 0);
    bsw = bs.right - bs.left;
    bsh = bs.bottom - bs.top;
    sx = GetSystemMetrics(SM_CXFULLSCREEN) - bsw;
    Xsize -= Xsize % 4; /* Round down */
    if(Xsize > sx)
        Xsize = sx;
    sy = GetSystemMetrics(SM_CYFULLSCREEN) - bsh + 20;
    if(Ysize > sy)
        Ysize = sy;

    logpal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + 256*sizeof(PALETTEENTRY));
    if(logpal == nil)
        return nil;
    logpal->palVersion = 0x300;
    logpal->palNumEntries = 256;
    pal = logpal->palPalEntry;

    c = displaychan;
    if(c == 0)
        c = autochan();
    winscreen_k = 8;
    if(TYPE(c) == CGrey){
        graphicsgmap(pal, NBITS(c));
        c = GREY8;
    }else{
        if(c == RGB15)
            winscreen_k = 16;
        else if(c == RGB24)
            winscreen_k = 24;
        else if(c == XRGB32)
            winscreen_k = 32;
        else
            c = CMAP8;
        graphicscmap(pal);
    }

    palette = CreatePalette(logpal);

    if(winscreen_k == 8)
        bmi = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD));
    else
        bmi = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER));
    if(bmi == nil)
        return nil;
    bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi->bmiHeader.biWidth = Xsize;
    bmi->bmiHeader.biHeight = -Ysize;   /* - => origin upper left */
    bmi->bmiHeader.biPlanes = 1;    /* always 1 */
    bmi->bmiHeader.biBitCount = winscreen_k;
    bmi->bmiHeader.biCompression = BI_RGB;
    bmi->bmiHeader.biSizeImage = 0; /* Xsize*Ysize*(k/8) */
    bmi->bmiHeader.biXPelsPerMeter = 0;
    bmi->bmiHeader.biYPelsPerMeter = 0;
    bmi->bmiHeader.biClrUsed = 0;
    bmi->bmiHeader.biClrImportant = 0;  /* number of important colors: 0 means all */

    if(winscreen_k == 8){
        rgb = bmi->bmiColors;
        for(i = 0; i < 256; i++){
            rgb[i].rgbRed = pal[i].peRed;
            rgb[i].rgbGreen = pal[i].peGreen;
            rgb[i].rgbBlue = pal[i].peBlue;
        }
    }

    screen = CreateCompatibleDC(NULL);
    if(screen == nil){
        fprint(2, "screen dc nil\n");
        return nil;
    }

    if(SelectPalette(screen, palette, 1) == nil){
        fprint(2, "select pallete failed\n");
    }
    i = RealizePalette(screen);
    GdiFlush();
    bits = CreateDIBSection(screen, bmi, DIB_RGB_COLORS, (void**)&winscreen_data, nil, 0);
    if(bits == nil){
        fprint(2, "CreateDIBSection failed\n");
        return nil;
    }

    SelectObject(screen, bits);
    GdiFlush();
    CreateThread(0, 16384, winproc, nil, 0, &h);
    winscreen_attached = 1;

Return:
    r->min.x = 0;
    r->min.y = 0;
    r->max.x = Xsize;
    r->max.y = Ysize;
    displaychan = c;
    *chan = c;
    *d = winscreen_k;
    *width = (Xsize/4)*(winscreen_k/8);
    *softscreen = 1;
    return winscreen_data;
}

void
flushmemscreen(Rectangle r)
{
    RECT wr;

    if(r.max.x<=r.min.x || r.max.y<=r.min.y)
        return;
    wr.left = r.min.x;
    wr.top = r.min.y;
    wr.right = r.max.x;
    wr.bottom = r.max.y;
    InvalidateRect(window, &wr, 0);
}

static void
scancode(WPARAM wparam, LPARAM lparam, int keyup)
{
    uchar buf[2];

    if(!(lparam & (1<<30))) {       /* don't auto-repeat chars */
        buf[0] = wparam;
        buf[1] = wparam >> 8;
        if (keyup)
            buf[1] |= 0x80;
        qproduce(gkscanq, buf, sizeof buf);
    }
}

#ifndef VK_BROWSER_BACK
#define VK_BROWSER_BACK 0xA6
#endif
#ifndef VK_BROWSER_FORWARD
#define VK_BROWSER_FORWARD 0xA7
#endif

LRESULT CALLBACK
WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    PAINTSTRUCT paint;
    HDC hdc;
    LPMINMAXINFO mmi;
    LONG x, y, w, h, b;
    HCURSOR dcurs;
    static WPARAM lastkeydown = 0;

    switch(msg) {
    case WM_SETCURSOR:
        /* User set */
        if(hcursor != NULL) {
            SetCursor(hcursor);
            break;
        }
        /* Pick the default */
        dcurs = LoadCursor(NULL, IDC_ARROW);
        SetCursor(dcurs);
        break;
    case WM_LBUTTONDBLCLK:
        b = (1<<8) | 1;
        goto process;
    case WM_MBUTTONDBLCLK:
        b = (1<<8) | 2;
        goto process;
    case WM_RBUTTONDBLCLK:
        b = (1<<8) | 4;
        goto process;
    case WM_MOUSEMOVE:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        b = 0;
    process:
        x = LOWORD(lparam);
        y = HIWORD(lparam);
        if(wparam & MK_LBUTTON)
            b |= 1;
        if(wparam & MK_MBUTTON)
            b |= 2;
        if(wparam & MK_RBUTTON) {
            if(wparam & MK_CONTROL)
                b |= 2;  //simulate middle button
            else
                b |= 4;  //right button
        }
        mousetrack(b, x, y, 0);
        break;
    // TODO: Alt+char, Ctrl+0, Ctrl+=, Ctrl+F1, Shift+F1
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        //print("%s %ux %ux\n", msg==WM_SYSKEYDOWN?"WM_SYSKEYDOWN":"WM_KEYDOWN", wparam, lparam);
        if(gkscanq)
            scancode(wparam, lparam, 0);
        else {
            Rune r = 0;
            switch(wparam) {
            case VK_F1:     r = KF+1;   break;
            case VK_F2:     r = KF+2;   break;
            case VK_F3:     r = KF+3;   break;
            case VK_F4:     r = KF+4;   break;
            case VK_F5:     r = KF+5;   break;
            case VK_F6:     r = KF+6;   break;
            case VK_F7:     r = KF+7;   break;
            case VK_F8:     r = KF+8;   break;
            case VK_F9:     r = KF+9;   break;
            case VK_F10:    r = KF+10;  break;
            case VK_F11:    r = KF+11;  break;
            case VK_F12:    r = KF+12;  break;
            case VK_HOME:   r = Home;   break;
            case VK_END:    r = End;    break;
            case VK_UP:     r = Up;     break;
            case VK_DOWN:   r = Down;   break;
            case VK_LEFT:   r = Left;   break;
            case VK_RIGHT:  r = Right;  break;
            case VK_PRIOR:  r = Pgup;   break; /* VK_PAGE_UP */
            case VK_NEXT:   r = Pgdown; break; /* VK_PAGE_DOWN */
            case VK_PRINT:  r = Print;  break; /* ? */
            case VK_SCROLL: r = Scroll; break;
            case VK_PAUSE:  r = Pause;  break;
            case VK_INSERT: r = Ins;    break;
            case VK_DELETE: r = Del;    break;
            case VK_BROWSER_BACK:       r = BrowserBack;   break; /* thinkpad */
            case VK_BROWSER_FORWARD:    r = BrowserForward;break; /* thinkpad */
            }
            if(r)
                gkbdputc(gkbdq, r);
        }
        lastkeydown = wparam | (lparam & 0x01FF0000); // to distinguish LShift & RShift
        break;
    case WM_SYSKEYUP:
    case WM_KEYUP:
        //print("%s %ux %ux\n", msg==WM_SYSKEYUP?"WM_SYSKEYUP":"WM_KEYUP", wparam, lparam);
        if(gkscanq)
            scancode(wparam, lparam, 1);
        else {
            Rune r = 0;
            if(wparam==VK_SNAPSHOT)
                r = Print;
            else if(lastkeydown == (wparam | (lparam & 0x01FF0000))) {
                if(lastkeydown == (VK_SHIFT | 0x002A0000))
                    r = LShift;
                else if(lastkeydown == (VK_SHIFT | 0x00360000))
                    r = RShift;
                else if(wparam == VK_MENU)
                    r = (lastkeydown&0x01000000) ? RAlt : LAlt;
                else if(wparam == VK_CONTROL)
                    r = (lastkeydown&0x01000000) ? RCtrl : LCtrl;
            }
            if(r)
                gkbdputc(gkbdq, r);
        }
        lastkeydown = 0;
        break;
    case WM_SYSCHAR:
    case WM_CHAR:
        //print("%s %ux %ux\n", msg==WM_SYSCHAR?"WM_SYSCHAR":"WM_CHAR", wparam, lparam);
        lastkeydown = 0;
        if(gkscanq)
            break;
        switch(wparam) {
        case '\n':
            wparam = '\r';
            break;
        case '\r':
            wparam = '\n';
            break;
        case '\t':
            if(GetKeyState(VK_SHIFT)<0)
                wparam = BackTab;
            else
                wparam = '\t';
            break;
        }
        if(HIWORD(lparam) & KF_ALTDOWN) {
            //wparam = APP | (HIWORD(lparam) & 0xFF); // make it independent on Rus/Lat layout
        } else
            gkbdputc(gkbdq, wparam);
        break;
    case WM_CLOSE:
        // no longer used?
        //m.b = 128;
        //m.modify = 1;
        //mousetrack(128, 0, 0, 1);
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        cleanexit(0);
        break;
    case WM_PALETTECHANGED:
        if((HWND)wparam == hwnd)
            break;
    /* fall through */
    case WM_QUERYNEWPALETTE:
        hdc = GetDC(hwnd);
        SelectPalette(hdc, palette, 0);
        if(RealizePalette(hdc) != 0)
            InvalidateRect(hwnd, nil, 0);
        ReleaseDC(hwnd, hdc);
        break;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &paint);
        SelectPalette(hdc, palette, 0);
        RealizePalette(hdc);
        x = paint.rcPaint.left;
        y = paint.rcPaint.top;
        w = paint.rcPaint.right - x;
        h = paint.rcPaint.bottom - y;
        BitBlt(hdc, x, y, w, h, screen, x, y, SRCCOPY);
        EndPaint(hwnd, &paint);
        break;
    case WM_GETMINMAXINFO:
        mmi = (LPMINMAXINFO)lparam;
        mmi->ptMaxSize.x = maxxsize;
        mmi->ptMaxSize.y = maxysize;
        mmi->ptMaxTrackSize.x = maxxsize;
        mmi->ptMaxTrackSize.y = maxysize;
        break;
    case WM_COMMAND:
    case WM_CREATE:
    case WM_SETFOCUS:
    case WM_DEVMODECHANGE:
    case WM_WININICHANGE:
    case WM_INITMENU:
    default:
        if(isunicode)
            return DefWindowProcW(hwnd, msg, wparam, lparam);
        return DefWindowProcA(hwnd, msg, wparam, lparam);
    }
    return 0;
}

static DWORD WINAPI
winproc(LPVOID x)
{
    MSG msg;
    RECT size;
    WNDCLASSW wc;
    WNDCLASSA wca;
    DWORD ws;

    if(!previnst){
        wc.style = CS_DBLCLKS;
        wc.lpfnWndProc = WindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = inst;
        wc.hIcon = LoadIcon(inst, MAKEINTRESOURCE(100));
        wc.hCursor = NULL;
        wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

        wc.lpszMenuName = 0;
        wc.lpszClassName = L"inferno";

        if(RegisterClassW(&wc) == 0){
            wca.style = wc.style;
            wca.lpfnWndProc = wc.lpfnWndProc;
            wca.cbClsExtra = wc.cbClsExtra;
            wca.cbWndExtra = wc.cbWndExtra;
            wca.hInstance = wc.hInstance;
            wca.hIcon = wc.hIcon;
            wca.hCursor = wc.hCursor;
            wca.hbrBackground = wc.hbrBackground;

            wca.lpszMenuName = 0;
            wca.lpszClassName = "inferno";
            isunicode = 0;

            if(RegisterClassA(&wca) == 0)
            {
                fprint(2, "can't register class\n"); /* TODO: oserror */
                ExitThread(0);
            }
        }
    }

    size.left = 0;
    size.top = 0;
    size.right = Xsize;
    size.bottom = Ysize;

    ws = WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX;

    if(AdjustWindowRect(&size, ws, 0)) {
        maxxsize = size.right - size.left;
        maxysize = size.bottom - size.top;
    }else{
        maxxsize = Xsize + 40;
        maxysize = Ysize + 40;
    }

    if(isunicode) {
        window = CreateWindowExW(
            0,          /* extended style */
            L"inferno",     /* class */
            L"Inferno",     /* caption */
            ws, /* style */
            CW_USEDEFAULT,      /* init. x pos */
            CW_USEDEFAULT,      /* init. y pos */
            maxxsize,       /* init. x size */
            maxysize,       /* init. y size */
            NULL,           /* parent window (actually owner window for overlapped) */
            NULL,           /* menu handle */
            inst,           /* program handle */
            NULL            /* create parms */
            );
    }else{
        window = CreateWindowExA(
            0,          /* extended style */
            "inferno",      /* class */
            "Inferno",      /* caption */
            ws, /* style */
            CW_USEDEFAULT,      /* init. x pos */
            CW_USEDEFAULT,      /* init. y pos */
            maxxsize,       /* init. x size */
            maxysize,       /* init. y size */
            NULL,           /* parent window (actually owner window for overlapped) */
            NULL,           /* menu handle */
            inst,           /* program handle */
            NULL            /* create parms */
            );
    }

    if(window == nil){
        fprint(2, "can't make window\n"); /* TODO: oserror */
        ExitThread(0);
    }

    SetForegroundWindow(window);
    ShowWindow(window, cmdshow);
    UpdateWindow(window);
    // CloseWindow(window);

    if(isunicode) {
        while(GetMessageW(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }else{
        while(GetMessageA(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }
    winscreen_attached = 0;
    ExitThread(msg.wParam);
    return 0;
}

void
setpointer(int x, int y)
{
    POINT pt;

    pt.x = x; pt.y = y;
    ClientToScreen(window, &pt);
    SetCursorPos(pt.x, pt.y);
}

void
drawcursor(Drawcursor* c)
{
    HCURSOR nh, oh;
    Rectangle ir;
    int i, h, j, bpl, ch, cw;
    uchar *bs, *bc, *and, *xor, *cand, *cxor;

    /* Set the default system cursor */
    if(c->data == nil) {
        oh = hcursor;
        hcursor = NULL;
        if(oh != NULL) {
            SendMessage(window, WM_SETCURSOR, (int)window, 0);
            DestroyCursor(oh);
        }
        return;
    }

    ir.min.x = c->minx;
    ir.min.y = c->miny;
    ir.max.x = c->maxx;
    ir.max.y = c->maxy;
    bpl = bytesperline(ir, 1);

    h = (c->maxy-c->miny)/2;

    ch = GetSystemMetrics(SM_CYCURSOR);
    cw = (GetSystemMetrics(SM_CXCURSOR)+7)/8;

    i = ch*cw;
    and = (uchar*)malloc(2*i);
    if(and == nil)
        return;
    xor = and + i;
    memset(and, 0xff, i);
    memset(xor, 0, i);

    cand = and;
    cxor = xor;
    bc = c->data;
    bs = c->data + h*bpl;

    for(i = 0; i < ch && i < h; i++) {
        for(j = 0; j < cw && j < bpl; j++) {
            cand[j] = ~(bs[j] | bc[j]);
            cxor[j] = ~bs[j] & bc[j];
        }
        cand += cw;
        cxor += cw;
        bs += bpl;
        bc += bpl;
    }
    nh = CreateCursor(inst, -c->hotx, -c->hoty, 8*cw, ch, and, xor);
    if(nh != NULL) {
        oh = hcursor;
        hcursor = nh;
        SendMessage(window, WM_SETCURSOR, (int)window, 0);
        if(oh != NULL)
            DestroyCursor(oh);
    }else{
        print("CreateCursor error %d\n", GetLastError());
        print("CXCURSOR=%d\n", GetSystemMetrics(SM_CXCURSOR));
        print("CYCURSOR=%d\n", GetSystemMetrics(SM_CYCURSOR));
    }
    free(and);
}

/*
 * thanks to drawterm for these
 */

static char*
clipreadunicode(HANDLE h)
{
    Rune *p;
    size_t n;
    char *q;

    p = (Rune*)GlobalLock(h);
    if(p == nil)
        error(Enovmem);
    n = runenlen(p, runestrlen(p)+1);
    q = (char*)malloc(n);
    if(q == nil) {
        GlobalUnlock(h);
        error(Enovmem);
    }
    runestoutf(q, p, n);
    GlobalUnlock(h);
    return q;
}

static char *
clipreadutf(HANDLE h)
{
    char *p;

    p = (char*)GlobalLock(h);
    if(p == nil)
        error(Enovmem);
    p = strdup(p);
    GlobalUnlock(h);
    if(p == nil)
        error(Enovmem);
    return p;
}

char*
clipread(void)
{
    HANDLE h;
    char *p;

    if(!OpenClipboard(window))
        return strdup("");

    if((h = GetClipboardData(CF_UNICODETEXT)))
        p = clipreadunicode(h);
    else if((h = GetClipboardData(CF_TEXT)))
        p = clipreadutf(h);
    else
        p = strdup("");

    CloseClipboard();
    return p;
}

int
clipwrite(char *buf)
{
    HANDLE h;
    char *p, *e;
    Rune *rp;
    int n;

    n = 0;
    if(buf != nil)
        n = strlen(buf);
    if(!OpenClipboard(window))
        return -1;

    if(!EmptyClipboard()){
        CloseClipboard();
        return -1;
    }

    h = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, (n+1)*sizeof(Rune));
    if(h == NULL)
        error(Enovmem);
    rp = (Rune *)GlobalLock(h);
    if(rp == NULL)
        error(Enovmem);
    p = buf;
    e = p+n;
    while(p<e)
        p += chartorune(rp++, p);
    *rp = 0;
    GlobalUnlock(h);

    SetClipboardData(CF_UNICODETEXT, h);

    h = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, n+1);
    if(h == NULL)
        error(Enovmem);
    p = (char*)GlobalLock(h);
    if(p == NULL)
        error(Enovmem);
    memmove(p, buf, n);
    p[n] = 0;
    GlobalUnlock(h);
    SetClipboardData(CF_TEXT, h);

    CloseClipboard();
    return n;
}
