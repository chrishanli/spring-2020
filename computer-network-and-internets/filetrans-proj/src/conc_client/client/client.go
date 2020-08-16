/* 客户端命令行工具 */

// 主作者：李涵（多线程）
// 次作者：何强（普通信息）

package client

import (
	"conc_client/inter"
	"conc_client/pref"
	"conc_client/utils"
	"fmt"
	"github.com/gookit/color"	// 颜色文字
	"github.com/schollz/progressbar/v3" //进度条
	"io"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"time"
)

const localPort = "5656"
const localErrPort = "9898"
var HostConn net.Conn = nil
var ErrInfoConn net.Conn = nil
var CmdConc *exec.Cmd = nil
var isConcRunSucceeded = false

// 此函数用来询问用户同意 HQ
func askUser(question string) (answer bool) {
	fmt.Println()
	pref.HdrWrn()
	fmt.Print(question + "(y/n)")
	var buf string
	fmt.Scan(&buf)
	if buf[0] == 'y' || buf[0] == 'Y' {
		// yes
		return true
	} else {
		return false
	}
}

// 更新传输状态 LH
func updateTransferStatus(updatePipe chan int) {
	// 获得当前任务
	totalSize, _ := inter.GetCurrentTotalSize(HostConn)
	// 新建一个bar
	bar := progressbar.NewOptions64(totalSize, progressbar.OptionSetRenderBlankState(true), progressbar.OptionSetWidth(20))
	// 开始计时
	startTime := time.Now()
	for {
		select {
		case <- updatePipe:
			// 停止传输讯号
			_ = bar.Set64(totalSize)
			// 结束计时
			timeCosts := time.Since(startTime)
			// 拿到传输文件总大小
			fmt.Println()
			pref.HdrInf()
			fmt.Printf("Speed: %.2fMB/s, used time: %s\n", (float64(totalSize) / 1048576) / timeCosts.Seconds(), timeCosts)
			pref.HdrInf()
			fmt.Println("Checking file consistency...")
			updatePipe <- 1
			return
		default:
			// 并无停止讯号，更新界面
			currSize, _ := inter.GetCurrentSeek(HostConn)
			_ = bar.Set64(currSize)
			time.Sleep(30 * time.Millisecond)
		}
	}
}

// 处理收集到的"Note" LH
func dealWithNotice(pipe chan int, updatePipe chan int, noteStr string) {
	noteNum := noteStr[3:6]
	if noteNum == "001" {
		pipe <- 2 // 等待主函数处理的，一律望pipe送
	} else if noteNum == "002" {
		// 新建一个协程用来更新界面
		go updateTransferStatus(updatePipe)
	} else if noteNum == "003" {
		// 传送完毕
		updatePipe <- 1
	} else if noteNum == "005" {
		// 文件损坏，正在等待传输。可以不做任何操作
	}
}

// 此函数用来处理收到的"Error" HQ
func dealWithError(errStr string) (isExitNeeded bool) {
	noteNum := errStr[3:6]
	if noteNum == "001" {
		// 连接错误Panic
		pref.HdrErr()
		fmt.Println("Target server is not connectable.")
		return true
	} else if noteNum == "002" {
		// 并无文件Panic
		pref.HdrErr()
		fmt.Println("No such file or directory for the specified path.")
		return true
	} else if noteNum == "003" {
		// 连接错误Panic
		pref.HdrErr()
		fmt.Println("Connection was broken while transferring.")
		return true
	} else if noteNum == "005" {
		// 连接错误Panic
		pref.HdrErr()
		fmt.Println("Client/Server is not fit.")
		return true
	} else if noteNum == "006" {
		// 连接错误Panic
		pref.HdrErr()
		fmt.Println("Send instructions failed: ", errStr[7:25])
		return true
	}else if noteNum == "007" {
		// 连接错误Panic
		pref.HdrErr()
		fmt.Println("Failed on opening list.send/fetching file info.")
		return true
	}
	//...
	fmt.Println()
	pref.HdrInf()
	fmt.Println("Unknown replied error number: ", noteNum)
	return false
}

// 等待"发送成功"或"不成功" LH
func waitSuccess(pipe chan int, updatePipe chan int, pendingErr *bool) {
	// 不停尝试连接CONC的错误收集端口
	var conn net.Conn
	for {
		var err error
		conn, err = net.Dial("tcp", "localhost:"+localErrPort)
		if err != nil {
			time.Sleep(1*time.Second)
			continue
		}
		break
	}

	// 不停接收CONC发来的信息，直到发来结束标示
	buf := make([]byte, 500)
	for {
		n, err := conn.Read(buf)
		if n < 3 || err != nil {
			if io.EOF == err {
				// 主程序退出，结束携程
				break
			} else {
				*pendingErr = true
				pipe <- 0
				// 等待主线程重置或结束该协程
				ret := <- pipe
				if ret == 1 {
					// 结束协程
					break
				}
			}
		}
		if string(buf[0:2]) == "ER" {
			// 发生错误，打印
			_, _ = conn.Write([]byte("OK"))

			dealWithError(string(buf))

			*pendingErr = true
			pipe <- 0
			// 等待重置或结束该协程
			ret := <- pipe
			if ret == 1 {
				// 结束协程
				break
			}
		} else if string(buf[0:2]) == "CP" {
			// 结束标记，结束协程
			_, _ = conn.Write([]byte("OK"))
			//fmt.Println("\n"+pref.HdrInf, "This transfer session is over.")
			break
		} else if string(buf[0:2]) == "IF" {
			// 讯息标记，等待主线程拿到信息
			_, _ = conn.Write([]byte("OK"))
			dealWithNotice(pipe, updatePipe, string(buf))
		} else if string(buf[0:2]) == "QU" {
			// 问询标记，等待外壳询问用户
			quesLen := utils.BytesToInt32(buf[3:7])
			_, _ = conn.Write([]byte("OK"))
			_, _ = conn.Read(buf)
			quesString := string(buf[0:quesLen])
			if askUser(quesString) == true {
				// 用户回答"好的"
				_, _ = conn.Write([]byte("OK"))
			} else {
				_, _ = conn.Write([]byte("NO"))
			}
		}
	}
	pipe <- 1
}

// 运行conc LH
func RunConcClient(isZipNeeded bool, isEncryptNeeded bool) (isOK bool) {
	// 按照系统不同操作
	clientRootPath, _ := filepath.Split(os.Args[0])
	if clientRootPath == "" { clientRootPath = ".\\" }
	concRootPath := clientRootPath+"res"+string(os.PathSeparator)
	switch runtime.GOOS {
	case "windows":
		concRootPath = concRootPath+"conc.exe"
		break
	case "darwin":
		concRootPath = concRootPath+"conc"
		break
	case "linux":
		concRootPath = concRootPath+"conc"
		break
	case "freebsd":
		concRootPath = concRootPath+"conc"
		break
	default:
		// 其他系统不支持
		return false
	}
	// 执行操作
	if isZipNeeded && isEncryptNeeded {
		CmdConc = exec.Command(concRootPath, "cl", "-z", "--en")
	} else if isZipNeeded {
		CmdConc = exec.Command(concRootPath, "cl", "-z")
	} else if isEncryptNeeded {
		CmdConc = exec.Command(concRootPath, "cl", "--en")
	} else {
		CmdConc = exec.Command(concRootPath, "cl")
	}
	//CmdConc.Stdout = os.Stdout // 供测试用
	if err := CmdConc.Start(); err != nil {
		// 执行程序出错
		pref.HdrErr()
		fmt.Println("The CONC core (assumed in", concRootPath, ") did not run successfully.")
		isOK = false
		// 设置为nil，以防空指针错误
		CmdConc = nil
		return
	}
	isOK = true
	return
}

// 在conc进程运行后，进行setup LH
func Setup(addr string, port string, sendPath string) (isSetupOK bool) {
	isSetupOK = false
	// 建立和错误服务器的通讯
	pipe := make(chan int)
	updatePipe := make(chan int)
	pendingErr := false
	go waitSuccess(pipe, updatePipe, &pendingErr)

	// 建立和程序的通讯，每1秒尝试1次，共尝试5次
	var err error
	pref.HdrInf()
	fmt.Print("Connecting to the CONC core..")
	for i := 0; i < 5; i++ {
		fmt.Print(".")
		HostConn, err = net.Dial("tcp", "localhost:" + localPort)
		if err != nil {
			HostConn = nil
		} else {
			break
		}
		time.Sleep(1 * time.Second)
	}
	if HostConn != nil {
		defer HostConn.Close()
	} else {
		color.Red.Println(" Error")
		pref.HdrErr()
		fmt.Println("Too many failed attempt to connect the CONC core.")
		return
	}

	// 讲话
	fmt.Println(" CONC core: 1.0.2")
	isConcRunSucceeded = true
	isOK := inter.SayHello(HostConn)
	if !isOK {
		pref.HdrErr()
		fmt.Println("Error occurred when saying Hello to conc.")
		return
	}

	// 设置身份为用户
	isOK = inter.SetTypeClient(HostConn)
	if !isOK {
		pref.HdrErr()
		fmt.Println("Error occurred when switching type to client.")
		return
	}

	// 设置host和端口
	isOK = inter.SetRemoteAddr(HostConn, addr)
	if !isOK {
		pref.HdrErr()
		fmt.Println("Error occurred when setting target address.")
		return
	}

	isOK = inter.SetRemotePort(HostConn, port)
	if !isOK {
		pref.HdrErr()
		fmt.Println("Error occurred when setting target port.")
		return
	}

	// 连线
	pref.HdrInf()
	fmt.Println("Trying to dial", addr, "...")
	isOK = inter.CallConnect(HostConn)
	if !isOK {
		pref.HdrErr()
		fmt.Println("Target server is not connectable.")
		return
	}

	ret := <- pipe
	// 检查有没有错，如果有错（即没有连接成功）就结束goroutine及本程序
	if ret == 0 {
		// 联系goroutine，让它关闭
		pref.HdrErr()
		fmt.Println("Error occurred while sending file(s).")
		pipe <- 1
		// 退出
		isOK = inter.CallEndProgress(HostConn)
		if !isOK {
			pref.HdrErr()
			fmt.Println("Error occurred implementing ENDP instructions.")
		}
		return
	}

	// 设置发送资料地址及发送
	isOK = inter.SetSendPref(HostConn, sendPath)
	if !isOK {
		pref.HdrErr()
		fmt.Println("Error occurred while sending preferences of the connection")
		return
	}

	// 等待文件检测
	pref.HdrInf()
	fmt.Println("Checking file(s)...")

	// 检查是否完成或出错
	if <- pipe == 0 {
		pref.HdrErr()
		fmt.Println("Error occurred in transmission.")
		return
	}

	// 无条件退出
	_ = inter.CallEndProgress(HostConn)

	// 已经完成传输
	<- updatePipe
	pref.HdrInf(); fmt.Println("File transmission is over. Have a nice day :)")
	return true
}

// 清除函数，以防用户希望终止或程序意外崩溃 LH
func Cleanser() {
	fmt.Println()
	pref.HdrInf()
	fmt.Println("Cleaning caches...")
	if HostConn != nil {
		// 发送一条EX指令
		_, _ = HostConn.Write([]byte("EXit User attempts to exit."))
		// 关闭conn
		HostConn.Close()
		HostConn = nil
	}
	if ErrInfoConn != nil {
		// 发送一条EX指令
		_, _ = ErrInfoConn.Write([]byte("EXit User attempts to exit."))
		// 关闭conn
		ErrInfoConn.Close()
		ErrInfoConn = nil
	}
	// 关闭关联进程（如有）
	if isConcRunSucceeded == true {
		_ = CmdConc.Process.Kill()
	}
	os.Exit(1)
}
