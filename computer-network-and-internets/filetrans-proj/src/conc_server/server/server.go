// 服务器端命令行
// 主作者：李涵

package server

import (
	"conc_server/pref"
	"context"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
)

var CmdConc *exec.Cmd = nil
var cancel context.CancelFunc

// 运行内核
func RunConc(workingDir string, monAddr string, monPort string) (isOK bool) {
	// 按照系统不同操作
	serverRootPath, _ := filepath.Split(os.Args[0])
	if serverRootPath == "" { serverRootPath = ".\\" }
	concRootPath := serverRootPath+"res"+string(os.PathSeparator)
	var concPath string
	switch runtime.GOOS {
	case "windows":
		concPath = concRootPath+"conc.exe"
		break
	case "darwin":
		concPath = concRootPath+"conc"
		break
	case "linux":
		concPath = concRootPath+"conc"
		break
	case "freebsd":
		concPath = concRootPath+"conc"
		break
	default:
		// 其他系统不支持
		return false
	}
	// 执行操作
	argWorkingDir := "-w="+workingDir
	argMonAddr := "-a="+monAddr
	argMonPort := "-p="+monPort

	var ctx context.Context
	ctx, cancel = context.WithCancel(context.Background())
	CmdConc = exec.CommandContext(ctx, concPath, "sv", argWorkingDir, argMonAddr, argMonPort)
	CmdConc.Stdout = os.Stdout
	CmdConc.Stderr = os.Stderr
	if err := CmdConc.Start(); err != nil {
		// 执行程序出错
		pref.HdrErr()
		fmt.Println("The CONC core (in", concRootPath, ") did not run successfully.")
		isOK = false
		return
	}
	isOK = true
	// 重定向输出
	_ = CmdConc.Wait()
	pref.HdrWrn()
	fmt.Println("You are now disconnected to CONC.")
	return
}

// 清除函数，以防用户希望终止或程序意外崩溃
func Cleanser() {
	fmt.Println()
	pref.HdrInf()
	fmt.Println("Cleaning caches...")
	cancel()

	os.Exit(0)
}