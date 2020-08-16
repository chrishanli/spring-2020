// 命令行程序
// 作者：李涵

package cli

import (
	"conc_client/client"
	"conc_client/pref"
	"fmt"
	"os"

	"github.com/urfave/cli"
)

// 运行命令行程序
func RunCli() {

	// 定义变量用于接收控制台输入的值
	var dirToSend string
	var targetAddr string
	var targetPort string

	var isZipNeeded = false
	var isCipherNeeded = false

	// 新建命令行程式
	app := cli.NewApp()
	app.Name = "client"
	app.Usage = "The client terminal programme for conc"
	app.UsageText = `client [file-or-dir] [server-addr] [server-port]`
	app.Copyright = "Copyright (c) 2020 Han Li Studios."
	app.Version = "1.0.0"
	app.Flags = []cli.Flag {
		// 是否需要进行压缩
		cli.BoolFlag {
			Name:        "z",
			Usage:       "compress before sending folder(s) (beta)",
			Destination: &isZipNeeded,
		},
		// 是否需要进行加密
		cli.BoolFlag {
			Name:        "e",
			Usage:       "encrypt data (beta)",
			Destination: &isCipherNeeded,
		},
	}

	// 定义命令行程序主要的工作
	app.Action = func(c *cli.Context) error {
		if c.NArg() < 3 {
			// 打印错误
			pref.HdrErr()
			fmt.Println("No sufficient arguments.")
			pref.HdrInf()
			fmt.Println("Try", os.Args[0], "--help to get arguments hint.")
		} else {
			dirToSend = c.Args().Get(0)
			targetAddr = c.Args().Get(1)
			targetPort = c.Args().Get(2)
			forthArg := c.Args().Get(3)
			fifthArg := c.Args().Get(4)
			isZipNeeded = forthArg == "-z" || fifthArg == "-z"
			isCipherNeeded = forthArg == "-e" || fifthArg == "-e"

				// 调用程序
			isOK := client.RunConcClient(isZipNeeded, isCipherNeeded)
			if isOK == false {
				return nil
			}
			// 如果调用成功，就启动传输
			isOK = client.Setup(targetAddr, targetPort, dirToSend)
			// 调用清洁程序
			client.Cleanser()
		}
		return nil
	}

	//执行程序
	app.Run(os.Args)
}
