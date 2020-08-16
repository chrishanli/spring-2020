using System;
using System.Windows.Forms;

namespace CONC_GUI
{
    static class Programme
    {
        /// <summary>
        /// 应用程序的主入口点。
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            // 检查是否用管理员运行
            System.Security.Principal.WindowsIdentity identity = System.Security.Principal.WindowsIdentity.GetCurrent();
            Application.EnableVisualStyles();
            System.Security.Principal.WindowsPrincipal principal = new System.Security.Principal.WindowsPrincipal(identity);
            // 判断当前登录用户是否为管理员。如果不是管理员

            if (!principal.IsInRole(System.Security.Principal.WindowsBuiltInRole.Administrator))
            {
                MessageBox.Show("Administrator privilege is required to run the CONC GUI. Please reopen this application as administrator.", "Info");
                System.Environment.Exit(0);
            }


            FormInitialize frmInit = new FormInitialize();
            frmInit.ShowDialog();

            // 选择的值
            if (frmInit.selectedClient)
            {
                FormMain clientConc = new FormMain();
                Application.Run(clientConc);

                if (clientConc.processCONC != null)
                {
                    try
                    {
                        if (!clientConc.processCONC.HasExited)
                        {
                            clientConc.processCONC.Kill();
                        }
                    }
                    catch (Exception e) {
                        MessageBox.Show(e.ToString(), "System Exceptions");
                    };
                    clientConc.processCONC = null;
                }

            } else if (frmInit.selectedServer)
            {
                Application.Run(new FormServerConfig());
            }
            
        }

        public static string RunCmd(string command)
        {
            //實例一個Process類，啟動一個獨立進程    
            System.Diagnostics.Process p = new System.Diagnostics.Process();
            //Process類有一個StartInfo屬性，這個是ProcessStartInfo類，包括了一些屬性和方法，下面我們用到了他的幾個屬性：    
            p.StartInfo.FileName = "cmd.exe";           //設定程序名    
            p.StartInfo.Arguments = "/c " + command;    //設定程式執行參數    
            p.StartInfo.UseShellExecute = false;        //關閉Shell的使用   
            p.StartInfo.RedirectStandardInput = true;   //重定向標準輸入   
            p.StartInfo.RedirectStandardOutput = true;  //重定向標準輸出   
            p.StartInfo.RedirectStandardError = true;   //重定向錯誤輸出   
            p.StartInfo.CreateNoWindow = true;          //設置不顯示窗口  
            p.Start();   //啟動    
            //p.StandardInput.WriteLine(command);       //也可以用這種方式輸入要執行的命令    
            //p.StandardInput.WriteLine("exit");        //不過要記得加上Exit要不然下一行程式執行的時候會當機    
            return p.StandardOutput.ReadToEnd();        //從輸出流取得命令執行結果    

        }
    }
}
