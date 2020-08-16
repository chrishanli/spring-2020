/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **
* Copyright © 2020 Han Li Studios. All rights reserved.                         *
*                                                                               *
*    libkij.h - the KIJ TUI extension (header)                                  *
*    NOTICE: This software follows the MIT license:                             *
*                                                                               *
* Permission is hereby granted, free of charge, to any person obtaining a       *
* copy of this software and associated documentation files(the "Software"),     *
* to deal in the Software without restriction, including without limitation     *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,      *
* and / or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions :         *
*                                                                               *
* The above copyright notice and this permission notice shall be included       *
* in all copies or substantial portions of the Software.                        *
*                                                                               *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS       *
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL        *
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER    *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING       *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER           *
* DEALINGS IN THE SOFTWARE.                                                     *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef libkij_h
#define libkij_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

// a_1) properties of a sign in dialogue
typedef struct {
    const char* logo;
    const char* prompt;
    const char* statusBarText;
} Kij_AuthWin;

// a_2) return data of a sign in dialogue
typedef struct {
    const char* username;
    const char* password;
} Kij_AuthWin_Results;

// b) properties of an error dialogue
typedef struct {
    const char*   mainTitle;
    const char*   winTitle;
    const char*   errInfo;
    const char**  buttons;
    int     buttonsCount;
} Kij_ErrorWin;

// c) properties of an information dialogue
typedef struct {
    const char*   mainTitle;
    const char*   winTitle;
    const char*   info;
    const char**  buttons;
    int     buttonsCount;
} Kij_InfoWin;

// d) properties of an initialise indicator window
typedef struct {
    const char*   logo;
    const char*   prompt;
    int     showPeriod;
    char    needProgBar;
} Kij_InitWin;

// e_1) properties of an entry dialogue
typedef struct {
    const char*   mainTitle;
    const char*   winTitle;
    const char**  inputBoxLabels;
    int     inputBoxLabelsCount;
    const char**  buttons;
    int     buttonsCount;
} Kij_InputWin;

// e_2) return data of an entry dialogue
typedef struct {
    int     selectedButton;
    const char**  texts;
    int     textsCount;
} Kij_InputWin_Results;

// f) properties of a menu selector dialogue
typedef struct {
    const char*   mainTitle;
    const char*   winTitle;
    const char*   choiceDesc;
    const char**  choices;
    int     choicesCount;
    const char*   winFootNote;
    const char*   statusBarText;
} Kij_SelectWin;

// g) properties of a table window
typedef struct {
    const char*   mainTitle;
    const char*   winTitle;
    const char**  columnNames;
    int*    columnWidths;
    int     columnCount;
    char**  rows;
    int     rowsCount;
    const char**  buttons;
    int     buttonsCount;
} Kij_TableWin;

// logo
#define KIJ_DEFLOGO "` _ _ _     _    _ _\n| (_) |   | |  (_|_)\n| |_| |__ | | ___ _\n| | | '_ \\| |/ / | |\n| | | |_) |   <| | |\n|_|_|_.__/|_|\\_\\_| |\n                _/ |\n               |__/`"


// 1.1 1 create and display a new sign-in dialogue
extern Kij_AuthWin_Results* Kij_NewAuthWin(Kij_AuthWin* win);

// 1.3 2 destroy return data of a sign-in dialogue
extern void Kij_Destroy_AuthWin_Results(Kij_AuthWin_Results* win);

// 2.1 4 create and display a new error dialogue
extern int Kij_NewErrorWin(Kij_ErrorWin* win);

// 3.1 6 create and display a new information dialogue
extern int Kij_NewInfoWin(Kij_InfoWin* win);

// 4.1 8 create and display a new entry dialogue
extern Kij_InputWin_Results* Kij_NewInputWin(Kij_InputWin* win);

// 4.3 10 destroy return data of an entry dialogue
extern void Kij_Destroy_InputWin_Results(Kij_InputWin_Results* win);

// 5.1 11 create and display a new menu selector dialogue
extern int Kij_NewSelectWin(Kij_SelectWin* win);

// 6.1 13 create and display a new table window
extern int Kij_NewTableWin(Kij_TableWin* win);

// 7.1 15 create and display a new initialise indicator
extern void Kij_NewInitWin(Kij_InitWin* win);

#ifdef __cplusplus
}
#endif

#endif /* libkij_h */
