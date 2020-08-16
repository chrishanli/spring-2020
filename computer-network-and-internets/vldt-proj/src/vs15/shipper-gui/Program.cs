using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace shipper_gui
{
    static class Program
    {
        /// <summary>
        /// 应用程序的主入口点。
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            // FrmSel
            FormSel frmSelect = new FormSel();
            frmSelect.ShowDialog();

            switch (frmSelect.UserSelected)
            {
                case FormSel.Selected.ENCODER:
                    Application.Run(new FormEncoderMain());
                    break;
                case FormSel.Selected.DECODER:
                    Application.Run(new FormDecoderMain());
                    break;
                default:
                    break;
            }

            // 退出程序时
            killProcess();
        }

        static void killProcess()
        {
            Process[] pro = Process.GetProcesses();//获取已开启的所有进程
            //遍历所有查找到的进程
            for (int i = 0; i < pro.Length; i++)
            {
                //判断此进程是否是要查找的进程
                if (pro[i].ProcessName.ToString().ToLower() == "encode")
                {
                    pro[i].Kill(); //结束进程
                }
            }
        }
    }
}
