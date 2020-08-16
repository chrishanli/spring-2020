using System;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Drawing;
using System.IO;
using System.Windows.Forms;
using System.Media;
using System.Diagnostics;
using System.Net.NetworkInformation;
using System.Resources;

namespace CONC_GUI
{
    public partial class FormMain : Form
    {

        private string serverAddr;// 默认服务器IP（暂时未有）
        private string port = "8957";// 默认目标主机端口

        private int concMonitor = 5656;
        private int concErrInfo = 9898;

        public Process processCONC = null;

        private bool txtPathHasText = false;
        private bool txtAddrHasText = false;
        private bool txtPortHasText = false;

        private string filePath;//定义文件路径

        Stopwatch stpwth = new Stopwatch();

        public FormMain()
        {
            InitializeComponent();
            txtPort.Text = port;
            txtPort.ForeColor = Color.Black;
            txtPortHasText = true;

        }

        /*——有关进度条定义部分：——*/
        private delegate void SetPos(long currentvalue);//要更新进度条
        Thread thprogress;
        ProgressSpecifier ps;
        long current_progress;
        long total_progress;

        // 开始进度条
        private void updateProgressBar(long currentvalue)
        {
            if (this.InvokeRequired)
            {
                SetPos setpos = new SetPos(updateProgressBar);
                this.Invoke(setpos, new object[] { currentvalue });
            }
            else
            {
                this.progressBar.Value = Convert.ToInt32(currentvalue);
            }

        }

        /*——有关5656端口全局变量定义部分：——*/
        byte[] buffer5656 = new byte[1024];
        private Socket socket5656;

        
        
        //conc sv -a=127.0.0.1 -p=3306 -w=D:\recv

        /*——有关9898端口全局变量定义部分：——*/
        Thread th9898;//负责与9898端口进行通信的线程
        byte[] buffer9898 = new byte[1024];
        public Socket socket9898;

        // 开启conc
        bool StartConc(string command)
        {
            // 新建进程
            processCONC = new Process();
            string exeName = Application.StartupPath + @"\res\conc";

            processCONC.StartInfo.FileName = exeName;
            processCONC.StartInfo.Arguments = command;

            processCONC.StartInfo.RedirectStandardError = false;//重定向错误流
            processCONC.StartInfo.RedirectStandardInput = true;//重定向输入流
            processCONC.StartInfo.RedirectStandardOutput = false;//重定向输出流
            
            processCONC.StartInfo.UseShellExecute = false;
            processCONC.StartInfo.CreateNoWindow = true;  // 隐藏窗口运行
            try
            {
                processCONC.Start();
            }
            catch (Exception)
            {
                return false;
            }
            return true;
        }


        void TaskCompleted()
        {
            // 进度条置100
            if (thprogress != null)
            {
                updateProgressBar(100);
                thprogress.Abort();
            }
            thprogress = null;


            // 发回一则“ENDP”
            if (socket5656 != null)
            {
                try
                {
                    socket5656.Send(Encoding.UTF8.GetBytes("ENDP"));
                    socket5656.Receive(buffer5656);
                    socket5656.Dispose();
                }
                catch (Exception) { };
            }
            socket5656 = null;

            // 程序结束
            btnStart.Enabled = true;
            btnAdv.Enabled = true;
            btnBrwFile.Enabled = true;
            btnBrwFolder.Enabled = true;

            // 断开连接
            if (socket9898 != null)
                socket9898.Dispose();
            socket9898 = null;

            this.txtStatus.Text = "Prepared";
            this.sizeStatus.Text = "";

            // 阻塞所有thread
            if (th9898 != null)
                th9898.Abort();
            th9898 = null;

            if (processCONC != null)
            {
                try
                {
                    processCONC.Kill();
                }
                catch (Exception) { };
            }
            processCONC = null;
        }

        /*——有关9898端口全局函数定义部分：——*/
        void Communicate9898()
        {
            while (true) {//一直重复
                /* socket9898.执行
            {Shutdown(SocketShutdown.Both); 
                 socket9898.Close();*/
                int r = 0;
                while ((r = socket9898.Receive(buffer9898)) == 0) ;
                socket9898.Send(Encoding.UTF8.GetBytes("OK"));
                string s = Encoding.UTF8.GetString(buffer9898, 0, r);
                string info = s.Substring(0, 2);
                if (info == "ER")
                {
                    string error_info = s.Substring(3, 3);
                    if (error_info == "001")
                    {
                        SystemSounds.Exclamation.Play();
                        MessageBox.Show(this, "Could not start sending process, because the remote server address at the given port is not connectable. Errorcode: " + error_info, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    else if (error_info == "002")
                    {
                        SystemSounds.Exclamation.Play();
                        MessageBox.Show(this, "File or directory could not be found: ", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    else if (error_info == "003")
                    {
                        SystemSounds.Exclamation.Play();
                        MessageBox.Show(this, "Connection failed while transferring. ", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    else if (error_info == "005")
                    {
                        SystemSounds.Exclamation.Play();
                        MessageBox.Show(this, "Client/Server status fault. ", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    else if (error_info == "006")
                    {
                        SystemSounds.Exclamation.Play();
                        MessageBox.Show(this, "Instruction was unable to send.", "ERROR 006", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }
                    else if (error_info == "007")
                    {
                        SystemSounds.Exclamation.Play();
                        MessageBox.Show(this, "Bad list.send.", "ERROR 007", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }

                    try
                    {
                        TaskCompleted();
                    }
                    catch (Exception) { };
                }
                else if (info == "CP")
                {
                    // 清洁函数
                    try
                    {
                        TaskCompleted();
                    }
                    catch (Exception) { };
                }
                else if (info == "IF")
                {
                    string tip_info = s.Substring(3, 3);
                    //extBox_Dialog.AppendText("收到了IF" + tip_info);
                    
                    if (tip_info == "001")
                    {
                        txtDialogue.AppendText("Connection to server was established.\r\n");
                    }
                    else if (tip_info == "002")
                    {
                        /*---  TSZE命令  ---*/
                        txtDialogue.AppendText("File transfer has just started.\r\n");
                        socket5656.Send(Encoding.UTF8.GetBytes("TSZE"));
                        socket5656.Receive(buffer5656);

                        //获取总共任务量
                        total_progress = ConcUtilities.GetLong(buffer5656);
                        txtDialogue.AppendText("Task size: " + total_progress + "\r\n");

                        // 设置进度条
                        EstablishProgress();
                        // test
                        // txtDialogue.AppendText("Progress bar set done\r\n");
                    }
                    else if (tip_info == "003")
                    {

                        txtDialogue.AppendText("Transfer is over.\r\n");

                        stpwth.Stop();
                        TimeSpan ts = stpwth.Elapsed;
                        this.txtDialogue.AppendText(String.Format("Speed: {0:F} MB/s", ((double)total_progress / 1048576) / ts.TotalSeconds));
                        stpwth.Reset();

                        // 设置completed图像
                        System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormMain));
                        this.imgConc.Image = (System.Drawing.Image)(resources.GetObject("imgCompleted.Image"));
                        try
                        {
                            TaskCompleted();
                        }
                        catch (Exception) { };

                        return;
                    }
                    else { }
                }
                else if (info == "QU")
                {
                    // socket9898.Send(Encoding.UTF8.GetBytes("OK"));
                    int r1 = socket9898.Receive(buffer9898);
                    string s1 = Encoding.UTF8.GetString(buffer9898, 0, r1);
                    socket9898.Send(Encoding.UTF8.GetBytes("OK"));
                    int r2 = socket9898.Receive(buffer9898);
                    string s2 = Encoding.UTF8.GetString(buffer9898, 0, r2);
                    DialogResult result = MessageBox.Show(s2, "Tips", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                    if (result == DialogResult.Yes)
                    {
                        socket9898.Send(Encoding.UTF8.GetBytes("OK"));
                        return;
                    }
                    else
                    {
                        socket9898.Send(Encoding.UTF8.GetBytes("NO"));
                        return;
                    }
                }
                //txtDialogue.AppendText(socketClient.RemoteEndPoint + ":" + s + "\r\r\n");
            }

        }

        //先连9898 等9898连上以后把套接字存到全局变量里 然后再连5656 等5656连上以后也把套接字存到全局变量
        //然后就用一个线程去监听9898套接字发来的信息

        /*接收服务器发来的消息*/
        private string receive_Message(Socket socket, byte[] receive_buffer)
        {
            int r = socket.Receive(receive_buffer);
            string s = Encoding.UTF8.GetString(receive_buffer, 0, r);
            return s;
        }
        /*向服务器端发送消息*/
        private void send_Message(Socket socket, string message, byte[] send_buffer)
        {
            string sendMessage = message;//发送到服务端的命令
            send_buffer = Encoding.UTF8.GetBytes(sendMessage);//Encoding.UTF8.GetBytes()将要发送的字符串转换成UTF8字节数组
            socket.Send(send_buffer);  //将接受成功的消息返回给SocketServer服务器 
        }


        /*——与端口5656进行通信——*/
        private bool EstablishConnect5656()
        {
            socket5656 = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);//创建负责与5656端口进行通信的socket对象
            IPAddress ip = IPAddress.Parse("127.0.0.1");//获取服务器IP地址
            IPEndPoint point5656 = new IPEndPoint(ip, concMonitor);
            try
            {
                socket5656.Connect(point5656);//连接服务器IP与端口
                txtDialogue.AppendText("Conc was opened.\r\n");
            }
            catch (Exception)
            {
                txtDialogue.AppendText("Monitor port on conc was not connectable\r\n");
                return false;
            }

            /*---  CLNT命令  ---*/
            socket5656.Send(Encoding.UTF8.GetBytes("CLNT"));//Encoding.UTF8.GetBytes()将要发送的字符串转换成UTF8字节数组
            if (receive_Message(socket5656, buffer5656) != "OK")
            {
                txtDialogue.AppendText("Monitor port on conc was not connectable\r\n");
                return false;
            }

            /*---  RMAD命令  ---*/
            socket5656.Send(Encoding.UTF8.GetBytes("RMAD"));
            if (receive_Message(socket5656, buffer5656) != "OK")
            {
                txtDialogue.AppendText("Monitor port on conc was not connectable\r\n");
                return false;
            }

            socket5656.Send(ConcUtilities.ConvertHex(serverAddr.Length));//目标主机名长度
            if (receive_Message(socket5656, buffer5656) != "OK")
            {
                txtDialogue.AppendText("Monitor port on conc was not connectable\r\n");
                return false;
            }

            socket5656.Send(Encoding.UTF8.GetBytes(serverAddr));//目标主机名
            if (receive_Message(socket5656, buffer5656) != "OK")
            {
                txtDialogue.AppendText("Monitor port on conc was not connectable\r\n");
                return false;
            }

            /*---  RMPO命令  ---*/
            socket5656.Send(Encoding.UTF8.GetBytes("RMPO"));
            if (receive_Message(socket5656, buffer5656) != "OK")
            {
                txtDialogue.AppendText("Monitor port on conc was not connectable\r\n");
                return false;
            }

            //目标主机端口长度
            socket5656.Send(ConcUtilities.ConvertHex(port.Length));
            if (receive_Message(socket5656, buffer5656) != "OK")
            {
                txtDialogue.AppendText("Monitor port on conc was not connectable\r\n");
                return false;
            }

            socket5656.Send(Encoding.UTF8.GetBytes(port));//目标主机端口
            if (receive_Message(socket5656, buffer5656) != "OK")
            {
                txtDialogue.AppendText("Monitor port on conc was not connectable\r\n");
                return false;
            }

            /*---  CONN命令  ---*/
            socket5656.Send(Encoding.UTF8.GetBytes("CONN"));
            if (receive_Message(socket5656, buffer5656) != "OK")
            {
                txtDialogue.AppendText("Monitor port on conc was not connectable\r\n");
                return false;
            }

            /*---  SEND命令  ---*/
            socket5656.Send(Encoding.UTF8.GetBytes("SEND"));
            if (receive_Message(socket5656, buffer5656) != "OK")
            {
                txtDialogue.AppendText("Monitor port on conc was not connectable\r\n");
                return false;
            }

            socket5656.Send(ConcUtilities.ConvertHex(Encoding.UTF8.GetByteCount(filePath)));
            if (receive_Message(socket5656, buffer5656) != "OK")
            {
                txtDialogue.AppendText("Monitor port on conc was not connectable\r\n");
                return false;
            }

            socket5656.Send(Encoding.UTF8.GetBytes(filePath));
            if (receive_Message(socket5656, buffer5656) != "OK")
            {
                txtDialogue.AppendText("Monitor port on conc was not connectable\r\n");
                return false;
            }
            return true;
        }

        /*——绘制进度条——*/
        private void Progress()
        {
            /*---  SEEK命令  ---*/
            // 计时开始
            stpwth.Start();
            while (current_progress != total_progress)
            {
                socket5656.Send(Encoding.UTF8.GetBytes("SEEK"));
                socket5656.Receive(buffer5656);

                current_progress = ConcUtilities.GetLong(buffer5656);
                double prog_in_100 = (double)current_progress / total_progress * 100;
                updateProgressBar((long)prog_in_100);
                sizeStatus.Text = String.Format("{0:F}%", prog_in_100);

                Thread.Sleep(100);//当前任务休眠0.1秒
            }
        }

        /*——执行进度条——*/
        private void EstablishProgress()
        {
            thprogress = new Thread(new ThreadStart(Progress));
            thprogress.IsBackground = true;//将thprogress设置为后台线程
            thprogress.Start();//标记这个线程准备就绪了，可以随时被执行  

        }

        /*——与端口9898进行通信——*/
        private void EstablishConnect9898()
        {
            socket9898 = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            IPAddress ip = IPAddress.Parse("127.0.0.1");// SBBBBBB
            IPEndPoint point9898 = new IPEndPoint(ip, concErrInfo);
            try
            {
                socket9898.Connect(point9898);//连接服务器IP与端口
            }
            catch (Exception)
            {
                txtDialogue.AppendText("ErrorInfo port on conc was unable to connect.\r\n");
                return;
            }
            th9898 = new Thread(Communicate9898);
            th9898.IsBackground = true;//将th9898设置为后台线程
            th9898.Start();//标记这个线程准备就绪了，可以随时被执行  
        }



        /*——事件处理函数：——*/

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            DialogResult res = MessageBox.Show(this, "Are you sure to exit the CONC GUI?", "Confirm", MessageBoxButtons.YesNo);
            if (res == DialogResult.No)
            {
                e.Cancel = true;
                return;
            }

            // 阻塞所有thread
            if (th9898 != null)
            {
                th9898.Abort();
                th9898 = null;
                if (socket9898 != null)
                    socket9898.Dispose();
            }
            if (thprogress != null)
            {
                thprogress.Abort();
                thprogress = null;
                if (socket5656 != null)
                    socket5656.Dispose();
            }

        }


        private void Form1_Load(object sender, EventArgs e)
        {
            Control.CheckForIllegalCrossThreadCalls = false;
            txtDialogue.Visible = false;
        }

        private void ShowProgressIndicator()
        {
            ps = new ProgressSpecifier();
            ps.ShowDialog();
        }

        // 点击了开始按钮
        private void btnStart_Click(object sender, EventArgs e)
        {
            filePath = txtPath.Text;
            port = txtPort.Text;

            // 设置conc图像
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormMain));
            this.imgConc.Image = (System.Drawing.Image)(resources.GetObject("imgConc.Image"));

            // 显示加载中窗体
            progressBar.Style = ProgressBarStyle.Marquee;
            Thread thProgInd = new Thread(new ThreadStart(ShowProgressIndicator));
            thProgInd.Start();

            if (!txtPathHasText || !txtAddrHasText || !txtPortHasText)
            {
                thProgInd.Abort();
                ps.Close();
                progressBar.Style = ProgressBarStyle.Blocks;
                SystemSounds.Exclamation.Play();
                MessageBox.Show(this, "File/directory path, target address or target port was not specified.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            if (!File.Exists(txtPath.Text) && !Directory.Exists(txtPath.Text))
            {
                thProgInd.Abort();
                ps.Close();
                progressBar.Style = ProgressBarStyle.Blocks;
                SystemSounds.Exclamation.Play();
                MessageBox.Show(this, "Bad file or directory: " + txtPath.Text, "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            else
            {
                // 可以开始
                // 命令行执行
                bool isStartOK = StartConc("cl --mon-port=" + concMonitor + " --errinf-port=" + concErrInfo);
                if (isStartOK != true)
                {
                    SystemSounds.Exclamation.Play();
                    MessageBox.Show(this, "the conc core refuses to run.", "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                // 设置控件不可用
                btnStart.Enabled = false;
                btnAdv.Enabled = false;
                btnBrwFile.Enabled = false;
                btnBrwFolder.Enabled = false;
                this.txtDialogue.Clear();

                // 联系conc，及连接它的各种通讯端口
                serverAddr = txtAddr.Text;
                EstablishConnect9898();
                EstablishConnect5656();
                this.txtDialogue.Visible = true;
                this.progressBar.Visible = true;

                thProgInd.Abort();
                ps.Close();
                progressBar.Style = ProgressBarStyle.Blocks;

                this.txtStatus.Text = "Sending";
            }
        }

        private void btnBrowseFolder_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            if (fbd.ShowDialog() == DialogResult.OK)
            {
                txtPath.Text = fbd.SelectedPath;
                txtPathHasText = true;
                txtPath.ForeColor = Color.Black;
            }
        }

        private void btnBrowFile_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Multiselect = false;
            if (ofd.ShowDialog() == DialogResult.OK)
            {
                txtPath.Text = ofd.FileName;
                txtPathHasText = true;
                txtPath.ForeColor = Color.Black;
            }
        }

        // 测试连通性
        private void btnPing_Click(object sender, EventArgs e)
        {
            if (!txtAddrHasText)
            {
                SystemSounds.Exclamation.Play();
                MessageBox.Show(this, "Target address was not specified.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            // 显示加载中窗体
            ProgressSpecifier ps = new ProgressSpecifier();
            ps.Show();

            // 尝试ping
            Ping ping = new Ping();
            byte[] sendBuf = Encoding.UTF8.GetBytes("Ping test");
            int timeout = 500;

            PingReply rep = ping.Send(txtAddr.Text, timeout, sendBuf);
            ps.Close();
            if (rep.Status == IPStatus.Success)
            {
                MessageBox.Show(this, "Target is connectable.\nRemote Server: " + rep.Address+"\nRoundtrip Time: "+rep.RoundtripTime+" ms", "Ping Info", MessageBoxButtons.OK, MessageBoxIcon.Information);
            } else
            {
                MessageBox.Show(this, "Target is not connectable.\nRemote Server: " + rep.Address + "\nTimeout Occurred.", "Ping Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
        
        // 点击了“Advanced”
        private void btnAdv_Click(object sender, EventArgs e)
        {
            FormAdvanced frmAdv = new FormAdvanced(concMonitor.ToString(), concErrInfo.ToString());
            frmAdv.ShowDialog();
            // 如果改过了port
            if (frmAdv.portHasChanged == true)
            {
                this.port = frmAdv.portMon;
            }
        }

        // 对方主机及文件选择框
        private void txtPath_Enter(object sender, EventArgs e)
        {
            if (txtPathHasText == false)
                txtPath.Text = "";
            txtPath.ForeColor = Color.Black;
        }

        private void txtPath_Leave(object sender, EventArgs e)
        {
            if (txtPath.Text == "")
            {
                txtPath.Text = "Select Path or Browse...";
                txtPath.ForeColor = Color.Gray;
                txtPathHasText = false;
            }
            else
                txtPathHasText = true;
        }

        // 一个具有提示语的txtField
        private void txtAddr_Enter(object sender, EventArgs e)
        {
            if (txtAddrHasText == false)
                txtAddr.Text = "";
            txtAddr.ForeColor = Color.Black;
        }

        private void txtAddr_Leave(object sender, EventArgs e)
        {
            if (txtAddr.Text == "")
            {
                txtAddr.Text = "Target Address or Domain";
                txtAddr.ForeColor = Color.Gray;
                txtAddrHasText = false;
            }
            else
                txtAddrHasText = true;
        }

        private void txtPort_Enter(object sender, EventArgs e)
        {
            if (txtPortHasText == false)
                txtPort.Text = "";
            txtPort.ForeColor = Color.Black;
        }

        private void txtPort_Leave(object sender, EventArgs e)
        {
            if (txtPort.Text == "")
            {
                txtPort.Text = "Target Port";
                txtPort.ForeColor = Color.Gray;
                txtPortHasText = false;
            }
            else
                txtPortHasText = true;
        }

        private void btnInfo_Click(object sender, EventArgs e)
        {
            FormInfo fi = new FormInfo();
            fi.Show();
        }

        // 容许txtbox自动滚动到最低端
        private void txtDialogue_TextChanged(object sender, EventArgs e)
        {
            txtDialogue.SelectionStart = txtDialogue.Text.Length;
            txtDialogue.ScrollToCaret();
        }
    }
}
