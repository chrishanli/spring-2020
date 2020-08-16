package main

import (
    "github.com/chrishanli/libkij"
    "github.com/chrishanli/tui-go"
    "log"
    "os"
    "os/exec"
    "path/filepath"
    "strings"
)

const JPGCompLogo = "     _ ____   ____  ____                      \n    | |  _ \\ / ___|/ ___|___  _ __ ___  _ __  \n _  | | |_) | |  _| |   / _ \\| '_ ` _ \\| '_ \\ \n| |_| |  __/| |_| | |__| (_) | | | | | | |_) |\n \\___/|_|    \\____|\\____\\___/|_| |_| |_| .__/ \n                                       |_|   "

func main() {
    // display a init loader
    initWin := libkij.InitWin {
        Logo:        JPGCompLogo,
        Prompt:      "Please wait while JPGComp is initiating...",
        ShowPeriod:  1000,
        NeedProgBar: false,
    }
    libkij.NewInitWindow(&initWin)

    // menu
    menuWin := libkij.SelectWin{
        MainTitle:         "JPEG Compressor",
        ChoicePadTitle:    "Menu",
        ChoicePadDesc:     "Please choose an action",
        Choices:           []string {
            "Start compress",
        },
        ChoicePadFootnote: "Press Enter to submit",
        StatusBarContent:  "libjpgcomp 1.0",
    }
    sel := libkij.NewSelectWin(&menuWin)
    if sel != 0 {
        return
    }

    // display file chooser
    fileWin := libkij.FileSelWin{
        Extensions:    []string{ "data" },
        HiddenDisplay: false,
    }
    filePath := libkij.NewFileSelWin(&fileWin)
    outputFileName := strings.Split(filepath.Base(filePath), ".")[0] + "_out.txt"
    outputPath := filepath.Dir(filePath) + string(os.PathSeparator) + outputFileName

    // confirm
    confirmWin := libkij.InfoWin {
        MainTitle:    "JPEG Compressor",
        InfoWinTitle: " Confirm ",
        Info:         "The compressed file will be output as \n\"" + outputFileName + "\" in the same directory.",
        Button:       []string { "OK", "Cancel" },
    }
    sel = libkij.NewInfoWin(&confirmWin)
    if sel == 1 {
        return
    }

    // start
    jpgExecPath := filepath.Dir(os.Args[0]) + string(os.PathSeparator)
    jpgTool := exec.Command(jpgExecPath + "jpg", filePath, outputPath)
    waitProgress("Please wait...")
    err := jpgTool.Run()
    ui.Quit()
    if err != nil {
        if e := err.(*exec.ExitError); e != nil {
            // is an exit error
            // check exit code
            if exitCode := e.ExitCode(); exitCode > 0 {
                reportErr(exceptions[exitCode])
            } else {
                reportErr(" fatal: unexpected exception happened ")
            }
        } else {
            // not an exit error
            reportErr(" fatal: unexpected exception happened ")
        }
    } else {
        confirmWin.InfoWinTitle = " Success "
        confirmWin.Info = " Process was successfully done. "
        libkij.NewInfoWin(&confirmWin)
    }
}

// show a waiting prompt
var ui tui.UI = nil

func waitProgress(promptText string) {
    // 新建主题
    t := tui.NewTheme()
    // 新建logo
    logo := tui.NewHBox(tui.NewSpacer(), tui.NewLabel(JPGCompLogo), tui.NewSpacer())

    // 新建一个提示信息
    prompt := tui.NewHBox(tui.NewSpacer(), tui.NewLabel(promptText), tui.NewSpacer())

    // 新建窗口容纳上述内容，以及有一个蓝色背景
    window := &libkij.StyledBox{
        Style: "initwin",
        Box:   tui.NewVBox(
            tui.NewPadder(10, 1, logo),
            tui.NewPadder(8, 1, prompt),
        ),
    }
    t.SetStyle("initwin", tui.Style{Bg: tui.ColorBlue, Fg: tui.ColorWhite})
    // 新建纵向框架
    wrapper := tui.NewVBox(
        tui.NewSpacer(),
        window,
        tui.NewSpacer(),
    )
    // 新建横向框架
    content := tui.NewHBox(tui.NewSpacer(), wrapper, tui.NewSpacer())

    // 新建屏幕
    var root *tui.Box = tui.NewVBox(
        content,
    )

    // 新建UI及设置它的主题
    var err error
    ui, err = tui.New(root)
    if err != nil {
        log.Fatal(err)
    }

    ui.SetTheme(t)
    go ui.Run()
}

var errWin = libkij.ErrorWin{
    MainTitle:     "",
    ErrorWinTitle: " Error ",
    Error:         "",
    Button:        []string {"OK"},
}

var exceptions = []string {
    "Success",
    "Unexpected runtime failure",
    "Exception-2: No such file or directory",
    "Exception-3: Out of memory",
    "Exception-4: Null pointer",
    "Exception-5: File access denied when output",
}

func reportErr(info string) {
    errWin.Error = info
    libkij.NewErrorWin(&errWin)
}