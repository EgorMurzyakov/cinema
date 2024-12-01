#ifndef HEADER_H
#define HEADER_H

#include <windows.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>
#include <codecvt>      // ��� �������������� UTF-16 � wide string
#include <io.h>         // ��� ������ � _setmode
#include <locale>       // ��� ������ � �����������
#include <fcntl.h>      // ��� ������ _O_U16TEXT

using namespace std;

// ������������ ���������
namespace Config {
    constexpr int BOX_WIDTH = 6;              // ������ �����
    constexpr int BOX_HEIGHT = 3;             // ������ �����
    constexpr int CHANSE_NOT_FREE_PLACES = 9; // ����������� �������� �����
}
using namespace Config;

// ��������� ��� ������������� ������ �����
struct Seat {
    wstring status; // "x" � ������, "0" � ����� ����, ����� � ����� �����
};

// ��������� ��� ����
struct Row {
    vector<Seat> seats;
};

// ��������� ����
struct Session {
    vector<Row> rows;
    wstring film_name;
    wstring time_film;
};

//��������� ������
struct Day {
    vector<Session> Session_one;
    vector<Session> Session_two;
    vector<Session> Session_three;
};

    // ��������� �������

void setMode16() {
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
}

// ������� ������� � ������������� �����
void fullScreen() {
    COORD coord;
    SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, &coord);
    keybd_event(VK_MENU, 0, 0, 0); // ������� Alt
    keybd_event(VK_RETURN, 0, 0, 0); // ������� Enter
    keybd_event(VK_RETURN, 0, KEYEVENTF_KEYUP, 0); // ������ Enter
    keybd_event(VK_MENU, 0, KEYEVENTF_KEYUP, 0); // ������ Alt
    this_thread::sleep_for(chrono::milliseconds(100));
}


// ��������� ������� �������
void setCursorPosition(int x, int y) {
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// ��������� ����� ������ � ����
void SetColor(int text, int background) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (background << 4) | text);
}


// �������� � �������������� �����
// ��������, �������� �� ������ ������
bool isNumber(const wstring& str) {
    for (wchar_t c : str) {
        if (!iswdigit(c)) {
            return false;
        }
    }
    return true;
}
// �������������� ������ � �����
int stringToInt(const wstring& str) {
    return stoi(str);
}


// ������� ���������
// ������� ��������� �������� �����
void drawOccupiedBox(int& x, int y) {
    SetColor(7, 7);
    setCursorPosition(x, y + 1);
    wcout << L"|    |";
    setCursorPosition(x, y + 2);
    wcout << L"| " << L" " << L" " << L" |";
    setCursorPosition(x, y + 3);
    wcout << L"|____|";
    SetColor(15, 0);
    x += BOX_WIDTH + 1;
}

// ������� ��������� ������ ����
void drawRowNumberBox(int& x, int y, int rowNumber) {
    x += 1;
    setCursorPosition(x, y + 2);
    wcout << L"Row " << rowNumber;
    x += BOX_WIDTH + 1;
}

// ������� ��������� ���������� �����
void drawAvailableBox(int& x, int y, const wstring& number) {
    setCursorPosition(x, y);
    wcout << L" ____ ";
    setCursorPosition(x, y + 1);
    wcout << L"|    |";
    setCursorPosition(x, y + 2);
    wcout << L"| " << (stringToInt(number) < 10 ? L" " : L"") << number << L" |";
    setCursorPosition(x, y + 3);
    wcout << L"|____|";
    x += BOX_WIDTH + 1;
}

// ������ ����� ��� ����������� �����
void drawBox(int& x, int y, const Seat& seat, int rowNumber) {
    if (isNumber(seat.status) && seat.status != L"0") {
        drawAvailableBox(x, y, seat.status);
    }
    else if (seat.status == L"x") {
        drawOccupiedBox(x, y);
    }
    else if (seat.status == L"0") {
        drawRowNumberBox(x, y, rowNumber);
    }
}

//// ��������� ����
void drawRow(int y, const Row& row, int rowNumber) {
    int x = 0;
    for (const auto& seat : row.seats) {
        drawBox(x, y, seat, rowNumber);
    }
}

// ��������� ����
void GenerationRoom(Session& session, const int rowCount, const int placeCount, wstring name, wstring time_f) {
    session.rows.resize(rowCount);
    session.film_name = name;
    session.time_film = time_f;
    for (int i = 0; i < rowCount; ++i) {
        session.rows[i].seats.resize(placeCount);
        for (int j = 0; j < placeCount; ++j) {
            if (j == 0 || j == placeCount - 1) {
                session.rows[i].seats[j].status = L"0";
            }
            else {
                int rand_not_free = rand() % CHANSE_NOT_FREE_PLACES;
                session.rows[i].seats[j].status = (rand_not_free == 0) ? L"x" : to_wstring(j);
            }
        }
    }
}

wstring replaceDash(wstring str) {
    replace(str.begin(), str.end(), L'�', L'-');
    return str;
}

// �������� ������� \r �� ������
void removeCarriageReturn(std::wstring& line) {
    line.erase(std::remove(line.begin(), line.end(), L'\r'), line.end());
}


// ��������� ��� � ������� �� �����
void GenerationDay(Day& day, wstring filename, int rowCount, int placeCount) {


    // ��������� ����������� �����
    wstringstream stream(filename);
    wstring line;
    vector<Session>* currentSessionGroup = nullptr;

    while (getline(stream, line)) {
        removeCarriageReturn(line); // ������� \r �� ������

        line = replaceDash(line);  // ������ ������� �������
        if (line.empty()) {
            continue; // ���������� ������ ������
        }


        if (line == L"Cinema room 1") {
            currentSessionGroup = &day.Session_one;
        }
        else if (line == L"Cinema room 2") {
            currentSessionGroup = &day.Session_two;
        }
        else if (line == L"Cinema room 3 ") {
            currentSessionGroup = &day.Session_three;
        }
        else if (currentSessionGroup && !line.empty() && iswdigit(line[0])) {
            int sessionCount = 0;
            try {
                sessionCount = stoi(line); // ����������� ������ � �����
                currentSessionGroup->resize(sessionCount);
            }
            catch (const exception&) {
                wcerr << L"������: ������������ ���������� �������." << endl;
                return;
            }

            for (int i = 0; i < sessionCount; ++i) {
                wstring timeRange, filmName;

                if (!getline(stream, timeRange) || timeRange.empty()) {
                    wcerr << L"������: ������������ ��� ����������� ����� ������." << endl;
                    return;
                }

                if (!getline(stream, filmName) || filmName.empty()) {
                    wcerr << L"������: ������������ ��� ����������� �������� ������." << endl;
                    return;
                }

                (*currentSessionGroup)[i].film_name = filmName;
                (*currentSessionGroup)[i].time_film = timeRange;

                GenerationRoom((*currentSessionGroup)[i], rowCount, placeCount, filmName, timeRange);
            }
        }
        else {
            wcerr << L"������: ����������� ������ � �����." << endl;
            return;
        }
    }
}


//������� ������ :
void ClearScreen() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count, cellCount;
    COORD homeCoords = { 0, 0 };

    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        cellCount = csbi.dwSize.X * csbi.dwSize.Y; // ����������
        FillConsoleOutputCharacter(hConsole, ' ', cellCount, homeCoords, &count);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);
        SetConsoleCursorPosition(hConsole, homeCoords);
    }
}

void closeWindow() {
    // �������� Alt
    keybd_event(VK_MENU, 0x38, 0, 0);
    // �������� F4
    keybd_event(VK_F4, 0x3E, 0, 0);
    // ��������� F4
    keybd_event(VK_F4, 0x3E, KEYEVENTF_KEYUP, 0);
    // ��������� Alt
    keybd_event(VK_MENU, 0x38, KEYEVENTF_KEYUP, 0);
}


void waitForInput() {
    system("pause");
}

void DrawSession(Session& session, int rowCount, int placeCount) {
    int y = 0;
    wcout << setw(65) << session.time_film << endl;
    ++y;
    wcout << setw(67) << session.film_name << endl;
    ++y;
    for (size_t i = 0; i < session.rows.size(); ++i) {
        drawRow(y, session.rows[i], i + 1);
        y += BOX_HEIGHT + 2;
    }
    setCursorPosition(0, y);
}

// ������� ��� �������� ����� �����
bool correctInput(int& number) {
    string input;
    getline(cin, input); // ������ ������
    if (input.empty()) {
        return false; // �������� �� ������ ����
    }
    try {
        size_t pos;
        number = stoi(input, &pos); // ������� ������������� ������ � �����
        if (pos != input.size()) {
            return false; // ���� ���� ������ �������
        }
    }
    catch (const invalid_argument&) {
        return false; // ���� �������������� �� �������
    }
    catch (const out_of_range&) {
        return false; // ���� ����� ������� �� �������
    }
    return true; // �����
}


//�������� ������ void choosingPlace()
void changePlaces(Session& session, int row, int place) {
    --row; // ���������� � 0-����������
    --place; // ���������� � 0-����������
    if (row < 0 || row >= session.rows.size() || place < 0 || place >= session.rows[row].seats.size()) {
        wcout << L"������������ ����� �����.\n";
        return;
    }

    if (session.rows[row].seats[place].status == L"x" || session.rows[row].seats[place].status == L"0") {
        wcout << L"����� ������, �������� ������.\n";
    }
    else {
        ClearScreen();
        wcout << L"����� ������� �������������.\n";
        session.rows[row].seats[place].status = L"x";
    }
}

#endif // HEADER_H