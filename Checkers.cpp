#include <iostream>
#include <windows.h>
#include <string>
#include <conio.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <fstream>
#include <locale>

#define KEY_RIGHT 77
#define KEY_LEFT  75
#define KEY_ENTER 13
#define EMPTY      0
#define WHITE_P    1
#define BLACK_P    2
#define WHITE_KING 3
#define BLACK_KING 4

using namespace std;

const WORD C_LIGHT = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
const WORD C_DARK = BACKGROUND_RED | BACKGROUND_GREEN;
const WORD C_GREY = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
const WORD C_BLACK = 0;
const WORD C_TXT = C_DARK | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
const WORD C_WARN = C_DARK | FOREGROUND_RED | FOREGROUND_INTENSITY;
const WORD C_OK = C_DARK | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
const WORD C_HINT = C_DARK | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

static void fillAttr(HANDLE h, WORD attr, int x, int y, int len) {
    COORD p = { (SHORT)x, (SHORT)y };
    DWORD n;
    FillConsoleOutputAttribute(h, attr, (DWORD)len, p, &n);
}
static void fillChar(HANDLE h, char c, int x, int y, int len) {
    COORD p = { (SHORT)x, (SHORT)y };
    DWORD n;
    FillConsoleOutputCharacterA(h, c, (DWORD)len, p, &n);
}
static void moveTo(HANDLE h, int x, int y) {
    COORD p = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(h, p);
}
static void showCursor(HANDLE h, bool show) {
    CONSOLE_CURSOR_INFO ci;
    ci.dwSize = show ? 25 : 100;
    ci.bVisible = show ? TRUE : FALSE;
    SetConsoleCursorInfo(h, &ci);
}
static void hidecurs() {
    showCursor(GetStdHandle(STD_OUTPUT_HANDLE), false);
}
void DrawSQ(HANDLE h, WORD o, int w, int heg) {
    for (int l = 0; l < 3; l++) {
        fillAttr(h, o, w, heg + l, 6);
    }
    fillChar(h, ' ', w, heg + 1, 6);
}
void DrawCH(HANDLE h, WORD o, int w, int heg) {
    fillAttr(h, o, w, heg, 2);
}
void DrawBoard(HANDLE h, int a[8][8]) {
    int sx = 6, sy = 3;
    for (int i = 7; i >= 0; i--) {
        for (int u = 7; u >= 0; u--) {
            WORD color = ((i + u) % 2 == 1) ? C_DARK : C_LIGHT;
            DrawSQ(h, color, sx, sy);
            sx += 6;
        }
        sx = 6;
        sy += 3;
    }
}
void Drawcheckers(HANDLE h, int a[8][8]) {
    int px = 8, py = 4;
    for (int i = 7; i >= 0; i--) {
        for (int u = 7; u >= 0; u--) {
            int piece = a[i][u];
            if (piece == WHITE_P || piece == WHITE_KING)
                DrawCH(h, C_LIGHT, px, py);
            else if (piece == BLACK_P || piece == BLACK_KING)
                DrawCH(h, C_BLACK, px, py);
            if (piece == WHITE_KING || piece == BLACK_KING) {
                WORD kAttr = (piece == WHITE_KING)
                    ? (C_LIGHT | FOREGROUND_RED | FOREGROUND_INTENSITY)
                    : (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
                fillAttr(h, kAttr, px, py, 1);
                fillChar(h, 'D', px, py, 1);
            }
            px += 6;
        }
        py += 3;
        px = 8;
    }
}
void Draw123(HANDLE h) {
    int y = 0;
    for (int i = 0; i < 9; i++) { DrawSQ(h, C_GREY, 0, y); y += 3; }
    COORD p = { 3, 4 };
    DWORD ip;
    for (int e = '8'; e >= '1'; e--) {
        FillConsoleOutputCharacterA(h, (char)e, 1, p, &ip);
        p.Y += 3;
    }
}
void DrawAB(HANDLE h) {
    int x = 0;
    for (int i = 0; i < 9; i++) { DrawSQ(h, C_GREY, x, 0); x += 6; }
    COORD p = { 8, 1 };
    DWORD ip;
    for (int e = 'A'; e <= 'H'; e++) {
        FillConsoleOutputCharacterA(h, (char)e, 1, p, &ip);
        p.X += 6;
    }
}
void clearPanel(HANDLE h) {
    for (int row = 5; row <= 24; row++) {
        fillAttr(h, C_DARK, 56, row, 58);
        fillChar(h, ' ', 56, row, 58);
    }
}
void panelPrint(HANDLE h, int row, const wstring& txt, WORD attr = C_TXT) {
    fillAttr(h, C_DARK, 56, row, 58);
    fillChar(h, ' ', 56, row, 58);
    wstring s = (txt.length() > 55) ? txt.substr(0, 55) : txt;
    SetConsoleTextAttribute(h, attr);
    moveTo(h, 57, row);
    wcout << s;
    SetConsoleTextAttribute(h, 7);
}
void Drawmenu(HANDLE h, int k) {
    DWORD cnt;
    COORD p;
    p = { 54, 0 };  FillConsoleOutputAttribute(h, C_GREY, 62, p, &cnt);
    p = { 54, 26 }; FillConsoleOutputAttribute(h, C_GREY, 62, p, &cnt);
    p = { 54, 4 };  FillConsoleOutputAttribute(h, C_GREY, 62, p, &cnt);
    for (int d = 0; d < 27; d++) {
        fillAttr(h, C_GREY, 54, d, 2);
        fillAttr(h, C_GREY, 114, d, 2);
    }
    p = { 56, 1 };
    for (int d = 0; d < 25; d++) {
        FillConsoleOutputAttribute(h, C_DARK, 58, p, &cnt);
        p.Y++;
    }
    struct { int bx; int tx; int w; const wchar_t* name; } items[4] = {
        { 62, 63,  7, L"Грати"  },
        { 73, 74,  9, L"Правила"},
        { 86, 87,  7, L"Автор"  },
        { 98, 99,  7, L"Вихід"  }
    };
    for (int i = 0; i < 4; i++) {
        WORD bg = (k == i + 1) ? C_LIGHT : C_DARK;
        WORD attr = bg | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
            | ((k == i + 1) ? 0 : FOREGROUND_INTENSITY);
        fillAttr(h, bg, items[i].bx, 2, items[i].w);
        SetConsoleTextAttribute(h, attr);
        moveTo(h, items[i].tx, 2);
        wcout << items[i].name;
    }
    SetConsoleTextAttribute(h, 7);
}
static void waitEsc() {
    int k = 0;
    while (k != 27) { if (_kbhit()) k = _getch(); Sleep(10); }
}
void Author(HANDLE h) {
    clearPanel(h);
    panelPrint(h, 8, L"", C_TXT);
    panelPrint(h, 9, L"        Цю роботу створив студент", C_TXT);
    panelPrint(h, 10, L"            Гнатюк Анатолій", C_TXT);
    panelPrint(h, 12, L"        Натисніть Esc, щоб вийти в меню", C_HINT);
    waitEsc();
    clearPanel(h);
}
void Rules(HANDLE h) {
    clearPanel(h);
    const wstring rul[] = {
        L"Шашки — гра для двох гравців на дошці 8x8.",
        L"12 білих і 12 чорних шашок на темних клітинах.",
        L"Першими ходять БІЛІ.",
        L"",
        L"Звичайна шашка ходить вперед по діагоналі.",
        L"При взятті — перестрибує через шашку суперника",
        L"в будь-якому напрямку, якщо за нею є вільне поле.",
        L"Биття ОБОВ'ЯЗКОВЕ. Після взяття — якщо можна",
        L"бити ще — хід продовжується тією ж шашкою.",
        L"",
        L"Шашка в останньому ряді - ДАМКА (D).",
        L"Дамка рухається по діагоналі на будь-яку відстань",
        L"вперед і назад, б'є так само через суперника.",
        L"",
        L"Формат ходу:  a3-b4  (колонка A-H, ряд 1-8).",
        L"'q' — вийти з поточної партії.",
        L"",
        L"   Натисніть Esc, щоб вийти в меню"
    };
    for (int i = 0; i < 18; i++) panelPrint(h, 5 + i, rul[i], C_TXT);
    waitEsc();
    clearPanel(h);
}
wstring getpl(HANDLE h, int player) {
    clearPanel(h);
    if (player == 1)
        panelPrint(h, 10, L"Введіть ім'я гравця (БІЛІ):", C_TXT);
    else
        panelPrint(h, 10, L"Введіть ім'я гравця (ЧОРНІ):", C_TXT);
    panelPrint(h, 11, L"", C_TXT);
    fillAttr(h, C_TXT, 57, 11, 30);
    SetConsoleTextAttribute(h, C_TXT);
    showCursor(h, true);
    moveTo(h, 57, 11);
    wstring name;
    wcin >> name;
    showCursor(h, false);
    SetConsoleTextAttribute(h, 7);
    return name;
}
class point {
    COORD prev, next;
public:
    point() { prev = { 0,0 }; next = { 0,0 }; }
    point& Setprev(COORD d) { prev = d; return *this; }
    point& Setnext(COORD t) { next = t; return *this; }
    COORD Getprev() { return prev; }
    COORD Getnext() { return next; }
};
bool parseMove(const wstring& s, point& p) {
    if (s.length() < 5 || s[2] != L'-') return false;
    wchar_t c1 = towlower(s[0]), c2 = towlower(s[3]);
    if (c1 < L'a' || c1 > L'h' || c2 < L'a' || c2 > L'h') return false;
    if (s[1] < L'1' || s[1] > L'8' || s[4] < L'1' || s[4] > L'8') return false;
    short x1 = (short)(7 - (c1 - L'a')), y1 = (short)(s[1] - L'1');
    short x2 = (short)(7 - (c2 - L'a')), y2 = (short)(s[4] - L'1');
    p.Setprev({ x1, y1 });
    p.Setnext({ x2, y2 });
    return true;
}
bool isOwn(int player, int piece) {
    if (!piece) return false;
    return (player == 1) ? (piece == WHITE_P || piece == WHITE_KING)
        : (piece == BLACK_P || piece == BLACK_KING);
}
bool isOpp(int player, int piece) {
    return piece && !isOwn(player, piece);
}
bool checkMove(int b[8][8], int pl, int x1, int y1, int x2, int y2, bool& cap, short& capX, short& capY) {
    cap = false; capX = capY = -1;
    if (!isOwn(pl, b[y1][x1])) return false;
    if (b[y2][x2] != EMPTY)    return false;
    int dx = x2 - x1, dy = y2 - y1;
    if (!dx || abs(dx) != abs(dy)) return false;
    int sx = (dx > 0) ? 1 : -1;
    int sy = (dy > 0) ? 1 : -1;
    int piece = b[y1][x1];
    bool king = (piece == WHITE_KING || piece == BLACK_KING);
    if (!king) {
        if (abs(dx) == 1) {
            if (pl == 1 && dy != 1)  return false;
            if (pl == 2 && dy != -1) return false;
            return true;
        }
        if (abs(dx) == 2) {
            int mx = x1 + sx, my = y1 + sy;
            if (isOpp(pl, b[my][mx])) {
                cap = true; capX = (short)mx; capY = (short)my;
                return true;
            }
        }
        return false;
    }
    int cx = x1 + sx, cy = y1 + sy;
    int eCnt = 0;
    while (cx != x2 || cy != y2) {
        if (cx < 0 || cx > 7 || cy < 0 || cy > 7) return false;
        int sq = b[cy][cx];
        if (sq) {
            if (isOwn(pl, sq)) return false;
            if (++eCnt > 1)    return false;
            capX = (short)cx; capY = (short)cy;
        }
        cx += sx; cy += sy;
    }
    if (eCnt == 1) cap = true;
    return true;
}
bool pieceCanCapture(int b[8][8], int pl, int x, int y) {
    int piece = b[y][x];
    bool king = (piece == WHITE_KING || piece == BLACK_KING);
    int dirs[4][2] = { {1,1},{1,-1},{-1,1},{-1,-1} };
    if (!king) {
        for (int d = 0; d < 4; d++) {
            int mx = x + dirs[d][0], my = y + dirs[d][1];
            int lx = x + dirs[d][0] * 2, ly = y + dirs[d][1] * 2;
            if (lx < 0 || lx > 7 || ly < 0 || ly > 7) continue;
            if (isOpp(pl, b[my][mx]) && b[ly][lx] == EMPTY)
                return true;
        }
    }
    else {
        for (int d = 0; d < 4; d++) {
            int cx = x + dirs[d][0], cy = y + dirs[d][1];
            bool found = false;
            while (cx >= 0 && cx < 8 && cy >= 0 && cy < 8) {
                int sq = b[cy][cx];
                if (sq == EMPTY) {
                    if (found) return true;
                }
                else if (isOwn(pl, sq)) {
                    break;
                }
                else {
                    if (found) break;
                    found = true;
                }
                cx += dirs[d][0]; cy += dirs[d][1];
            }
        }
    }
    return false;
}
bool anyCapture(int b[8][8], int pl) {
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++)
            if (isOwn(pl, b[y][x]) && pieceCanCapture(b, pl, x, y))
                return true;
    return false;
}
bool anyMove(int b[8][8], int pl) {
    int dirs[4][2] = { {1,1},{1,-1},{-1,1},{-1,-1} };
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (!isOwn(pl, b[y][x])) continue;
            bool king = (b[y][x] == WHITE_KING || b[y][x] == BLACK_KING);
            if (!king) {
                if (pieceCanCapture(b, pl, x, y)) return true;
                for (int d = 0; d < 4; d++) {
                    if (pl == 1 && dirs[d][1] != 1)  continue;
                    if (pl == 2 && dirs[d][1] != -1) continue;
                    int nx = x + dirs[d][0], ny = y + dirs[d][1];
                    if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8 && b[ny][nx] == EMPTY)
                        return true;
                }
            }
            else {
                for (int d = 0; d < 4; d++) {
                    int cx = x + dirs[d][0], cy = y + dirs[d][1];
                    while (cx >= 0 && cx < 8 && cy >= 0 && cy < 8) {
                        int sq = b[cy][cx];
                        if (sq == EMPTY) { return true; }
                        if (isOwn(pl, sq)) { break; }
                        int nx = cx + dirs[d][0], ny = cy + dirs[d][1];
                        if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8 && b[ny][nx] == EMPTY)
                            return true;
                        break;
                    }
                }
            }
        }
    }
    return false;
}
bool isDraw(int b[8][8]) {
    int wk = 0, bk = 0, other = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (b[y][x] == WHITE_KING) wk++;
            else if (b[y][x] == BLACK_KING) bk++;
            else if (b[y][x] != EMPTY) other++;
        }
    }
    return (wk == 1 && bk == 1 && other == 0);
}
void play(HANDLE h, wstring pl1, wstring pl2, int board[8][8]) {
    DWORD cnt;
    COORD p;
    int  cur = 1;
    bool multiCap = false;
    short actX = -1, actY = -1;
    wstring gameResult = L"Партія перервана";
    wstring hist[500];
    int hCnt = 0;
    bool running = true;
    DrawBoard(h, board);
    Drawcheckers(h, board);
    fillAttr(h, C_GREY, 54, 4, 62);
    while (running) {
        clearPanel(h);
        wstring info = (cur == 1)
            ? (L"Хід БІЛИХ:  " + pl1)
            : (L"Хід ЧОРНИХ: " + pl2);
        panelPrint(h, 5, info, C_TXT);
        if (multiCap) {
            panelPrint(h, 6, L"Продовжуйте взяття тією ж шашкою!", C_WARN);
        }
        else if (anyCapture(board, cur)) {
            panelPrint(h, 6, L"Є взяття — воно обов'язкове!", C_WARN);
        }
        panelPrint(h, 8, L"Останні ходи:", C_HINT);
        int st = (hCnt > 6) ? hCnt - 6 : 0;
        for (int i = st; i < hCnt; i++) {
            int moveNum = i / 2 + 1;
            wstring line;
            if (i % 2 == 0) {
                wchar_t buf[32];
                swprintf(buf, 32, L"%d. %ls", moveNum, hist[i].c_str());
                line = buf;
            }
            else {
                wchar_t buf[32];
                swprintf(buf, 32, L"   %ls", hist[i].c_str());
                line = buf;
            }
            panelPrint(h, 9 + i - st, line, C_TXT);
        }
        panelPrint(h, 17, L"Формат ходу: a3-b4", C_HINT);
        panelPrint(h, 18, L"або 'q' — вийти з партії", C_TXT);
        panelPrint(h, 20, L"Ваш хід: ", C_TXT);
        fillAttr(h, C_TXT, 66, 20, 10);
        showCursor(h, true);
        moveTo(h, 66, 20);
        SetConsoleTextAttribute(h, C_TXT);
        wstring inp;
        wcin >> inp;
        SetConsoleTextAttribute(h, 7);
        showCursor(h, false);
        moveTo(h, 66, 20);
        for (int m = 0; m < 500; m++) {
            wcout << " ";
        }
        p = { 54, 0 };  FillConsoleOutputAttribute(h, C_GREY, 62, p, &cnt);
        p = { 54, 26 }; FillConsoleOutputAttribute(h, C_GREY, 62, p, &cnt);
        p = { 54, 4 };  FillConsoleOutputAttribute(h, C_GREY, 62, p, &cnt);
        for (int d = 0; d < 27; d++) {
            fillAttr(h, C_GREY, 54, d, 2);
            fillAttr(h, C_GREY, 114, d, 2);
        }
        Draw123(h);
        DrawAB(h);
        DrawBoard(h, board);
        Drawcheckers(h, board);
        fillAttr(h, C_GREY, 54, 4, 62);
        if (wcin.fail()) {
            wcin.clear();
            wcin.ignore(1000, L'\n');
            panelPrint(h, 22, L"Помилка читання! Спробуйте ще раз.", C_WARN);
            Sleep(1200);
            continue;
        }
        if (inp == L"q" || inp == L"Q") {
            running = false;
            break;
        }
        fillAttr(h, C_DARK, 65, 20, 49);
        for (int u = 21; u < 26; u++) {
            fillAttr(h, C_DARK, 56, u, 58);
        }
        point mv;
        if (!parseMove(inp, mv)) {
            fillAttr(h, C_GREY, 54, 4, 62);
            panelPrint(h, 22, L"Помилка формату! Приклад: a3-b4", C_WARN);
            Sleep(1400);
            continue;
        }
        short x1 = mv.Getprev().X, y1 = mv.Getprev().Y;
        short x2 = mv.Getnext().X, y2 = mv.Getnext().Y;
        if (!isOwn(cur, board[y1][x1])) {
            panelPrint(h, 22, L"Це не ваша шашка!", C_WARN);
            Sleep(1400);
            continue;
        }
        if (multiCap && (x1 != actX || y1 != actY)) {
            panelPrint(h, 22, L"Ви повинні бити активною шашкою!", C_WARN);
            Sleep(1400);
            continue;
        }
        bool mustCap = multiCap
            ? pieceCanCapture(board, cur, actX, actY)
            : anyCapture(board, cur);
        bool cap = false;
        short cx = -1, cy = -1;
        if (!checkMove(board, cur, x1, y1, x2, y2, cap, cx, cy)) {
            panelPrint(h, 22, L"Хід не за правилами!", C_WARN);
            Sleep(1400);
            continue;
        }
        if (mustCap && !cap) {
            panelPrint(h, 22, L"Биття обов'язкове — оберіть хід із взяттям!", C_WARN);
            Sleep(1600);
            continue;
        }
        board[y2][x2] = board[y1][x1];
        board[y1][x1] = EMPTY;
        if (cap) board[cy][cx] = EMPTY;
        if (hCnt < 500) hist[hCnt++] = inp;
        bool promoted = false;
        if (cur == 1 && y2 == 7 && board[y2][x2] == WHITE_P) {
            board[y2][x2] = WHITE_KING;
            promoted = true;
            panelPrint(h, 22, L"Шашка стала ДАМКОЮ!", C_OK);
        }
        if (cur == 2 && y2 == 0 && board[y2][x2] == BLACK_P) {
            board[y2][x2] = BLACK_KING;
            promoted = true;
            panelPrint(h, 22, L"Шашка стала ДАМКОЮ!", C_OK);
        }
        DrawBoard(h, board);
        Drawcheckers(h, board);
        if (promoted) Sleep(800);
        if (cap && !promoted && pieceCanCapture(board, cur, x2, y2)) {
            multiCap = true;
            actX = x2; actY = y2;
        }
        else {
            multiCap = false;
            actX = actY = -1;
            cur = (cur == 1) ? 2 : 1;
            if (isDraw(board)) {
                clearPanel(h);
                panelPrint(h, 10, L"           ==================================", C_HINT);
                panelPrint(h, 11, L"                     НІЧИЯ! Дві дамки. ", C_HINT);
                panelPrint(h, 12, L"           ==================================", C_HINT);
                panelPrint(h, 14, L"                Натисніть будь-яку клавішу...", C_HINT);
                gameResult = L"Нічия";
                while (!_kbhit())Sleep(50);
                _getch();
                running = false;
            }
            else if (!anyMove(board, cur)) {
                clearPanel(h);
                int winner = (cur == 1) ? 2 : 1;
                wstring winName = (winner == 1) ? pl1 : pl2;
                wstring winLine = winName + L" — ПЕРЕМОЖЕЦЬ!";
                panelPrint(h, 10, L"================================", C_OK);
                panelPrint(h, 11, winLine, C_OK);
                panelPrint(h, 12, L"================================", C_OK);
                panelPrint(h, 14, L"Натисніть будь-яку клавішу...", C_TXT);
                while (!_kbhit()) Sleep(50);
                _getch();
                gameResult = (winner == 1)
                    ? (pl1 + L" (білі)-переможець")
                    : (pl2 + L" (чорні)-переможець");
                running = false;
            }
        }
    }
    if (hCnt > 0) {
        wofstream file("game_history.txt", ios::app);
        file.imbue(locale(""));
        file << L"=== " << pl1 << L" (білі) vs " << pl2 << L" (чорні) ===\n";
        for (int i = 0; i < hCnt; i++) {
            if (i % 2 == 0) file << (i / 2 + 1) << L". ";
            file << hist[i] << L"  ";
            if (i % 2 == 1) file << L"\n";
        }
        if (hCnt % 2 != 0) file << L"\n";
        file << L"Результат: " << gameResult << L"\n";
        file << L"================================\n\n";
        file.close();
    }
}
void initBoard(int a[8][8]) {
    for (int i = 0; i < 8; i++)
        for (int u = 0; u < 8; u++)
            a[i][u] = EMPTY;
    for (int i = 0; i < 8; i++) {
        for (int u = 0; u < 8; u++) {
            if ((i + u) % 2 == 1) {
                if (i < 3) a[i][u] = WHITE_P;
                if (i > 4) a[i][u] = BLACK_P;
            }
        }
    }
}
int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT sr = { 0, 0, 119, 29 };
    SetConsoleWindowInfo(h, TRUE, &sr);
    COORD csz = { 120, 30 };
    SetConsoleScreenBufferSize(h, csz);
    SetConsoleTitleA("Checkers");
    hidecurs();
    int arr[8][8];
    initBoard(arr);
    Draw123(h);
    DrawAB(h);
    DrawBoard(h, arr);
    Drawcheckers(h, arr);
    Drawmenu(h, 1);
    fillAttr(h, C_GREY, 54, 4, 62);
    int t = 1, fl = 1;
    while (fl) {
        SetConsoleTitleA("Checkers");
        if (_kbhit()) {
            int k = _getch();
            switch (k) {
            case KEY_RIGHT:
                t = (t < 4) ? t + 1 : 1;
                Drawmenu(h, t);
                fillAttr(h, C_GREY, 54, 4, 62);
                break;
            case KEY_LEFT:
                t = (t > 1) ? t - 1 : 4;
                Drawmenu(h, t);
                fillAttr(h, C_GREY, 54, 4, 62);
                break;
            case KEY_ENTER:
                switch (t) {
                case 1: {
                    wstring p1 = getpl(h, 1);
                    wstring p2 = getpl(h, 2);
                    initBoard(arr);
                    Draw123(h);
                    DrawAB(h);
                    DrawBoard(h, arr);
                    Drawcheckers(h, arr);
                    Drawmenu(h, t);
                    fillAttr(h, C_GREY, 54, 4, 62);
                    play(h, p1, p2, arr);
                    initBoard(arr);
                    Draw123(h);
                    DrawAB(h);
                    DrawBoard(h, arr);
                    Drawcheckers(h, arr);
                    Drawmenu(h, t);
                    fillAttr(h, C_GREY, 54, 4, 62);
                    break;
                }
                case 2:
                    Rules(h);
                    Drawmenu(h, t);
                    DrawBoard(h, arr);
                    Drawcheckers(h, arr);
                    fillAttr(h, C_GREY, 54, 4, 62);
                    break;
                case 3:
                    Author(h);
                    Drawmenu(h, t);
                    DrawBoard(h, arr);
                    Drawcheckers(h, arr);
                    fillAttr(h, C_GREY, 54, 4, 62);
                    break;
                case 4:
                    fl = 0;
                    break;
                }
                break;
            }
        }
        Sleep(10);
    }
    CloseHandle(h);
    return 0;
}