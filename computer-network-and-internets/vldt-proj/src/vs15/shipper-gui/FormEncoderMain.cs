using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace shipper_gui
{
    public partial class FormEncoderMain : Form
    {

        // 用户设置
        private string inputFilePath;
        private string outputVideoPath;
        private string outputVideoFileName = "output.avi";
        private int videoLen;
        private int colourNum = 4;
        private int videoWid;
        private int videoHei;

        private bool isInputSet = false;
        private bool isOutpuSet = false;

        public FormEncoderMain()
        {
            InitializeComponent();
        }

        private void FormEncoderMain_Load(object sender, EventArgs e)
        {
            // 设置相关变量
            videoLen = 5000;
            videoWid = 0;
            videoHei = 0;
            // 设置按钮
            btnStart.Enabled = false;
        }



        private void setInputFile()
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Multiselect = false;
            dialog.Title = "Choose Input File";
            dialog.Filter = "All Files(*.*)|*.*";
            dialog.RestoreDirectory = true;
            if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                inputFilePath = dialog.FileName;
                txtFileP.Text = inputFilePath;
                isInputSet = true;
            }
        }

        private void setOutputFile()
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.FileName = outputVideoFileName;
            dialog.Title = "Choose Place to Store Output Video";
            dialog.Filter = "AVI File(*.avi)|*.avi";
            dialog.RestoreDirectory = true;
            if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                outputVideoPath = dialog.FileName;
                txtOutputP.Text = outputVideoPath;
                isOutpuSet = true;
            }
        }

        private void btnBrowFile_Click(object sender, EventArgs e)
        {
            setInputFile();
            if (isInputSet && isOutpuSet)
            {
                btnStart.Enabled = true;
            }
        }

        private void btnBrowVideo_Click(object sender, EventArgs e)
        {
            setOutputFile();
            if (isInputSet && isOutpuSet)
            {
                btnStart.Enabled = true;
            }
        }

        private void btnEncSett_Click(object sender, EventArgs e)
        {
            // 新建窗体
            FormEncoderPref frmPref = new FormEncoderPref();
            frmPref.videoLen = videoLen;
            frmPref.videoHei = videoHei;
            frmPref.videoWid = videoWid;

            // 显示窗体
            frmPref.ShowDialog();

            // 接受返回值
            this.videoLen = frmPref.videoLen;
            this.videoHei = frmPref.videoHei;
            this.videoWid = frmPref.videoWid;
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            setInputFile();
        }

        private void assignToolStripMenuItem_Click(object sender, EventArgs e)
        {
            setOutputFile();
        }

        private void preferencesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // 新建窗体
            FormEncoderPref frmPref = new FormEncoderPref();
            frmPref.videoLen = videoLen;
            frmPref.videoHei = videoHei;
            frmPref.videoWid = videoWid;

            // 显示窗体
            frmPref.ShowDialog();

            // 接受返回值
            this.videoLen = frmPref.videoLen;
            this.videoHei = frmPref.videoHei;
            this.videoWid = frmPref.videoWid;
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            System.Environment.Exit(0);
        }

        private void progressBar1_Click(object sender, EventArgs e)
        {

        }

        private void btnSaveLog_Click(object sender, EventArgs e)
        {
            string saveLogPath;
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.FileName = "log.log";
            dialog.Title = "Choose Place to Store Logs";
            dialog.Filter = "Log File(*.log)|*.log";
            dialog.RestoreDirectory = false;
            if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                saveLogPath = dialog.FileName;
                // 保存文件
                txtBoxLog.SaveFile(saveLogPath, RichTextBoxStreamType.PlainText);
            }
        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            btnStart.Enabled = false;
            txtBoxLog.AppendText("> Encode is started, please wait...\n");
            PerformThread = new Thread(new ThreadStart(this.performEncode));
            PerformThread.Start();

            // 设置状态栏
            tbState.Text = "Generating";
        }

        // 实现控制进度条的委托
        delegate void ShowProgressDelegate(int totalStep, int currentStep);
        delegate void SetTextDelegate(string text);
        Thread progressThread;
        Thread PerformThread;


        // 命令执行结束后的委托方法
        private void CMDProgressOver(string text)
        {
            if (this.txtBoxLog.InvokeRequired)
            {
                while (!this.txtBoxLog.IsHandleCreated)
                {
                    if (this.txtBoxLog.Disposing ||
                        this.txtBoxLog.IsDisposed)
                        return;
                }
                SetTextDelegate d = new SetTextDelegate(CMDProgressOver);
                this.txtBoxLog.Invoke(d, new object[] { text });
            }
            else
            {
                this.txtBoxLog.AppendText(text);
                // 顺便设置btn
                btnStart.Enabled = true;
                // 顺便阻塞线程，并且设置进度条为满
                if (progressThread.IsAlive)
                {
                    progressThread.Abort();
                }
                progressBar1.Value = 100;

                // 设置状态栏
                tbState.Text = "Ready";
            }
        }
        
        // 在另一线程进行Encode
        private void performEncode()
        {
            string output = "";
            string cmd = "encode " + String.Format("\"{0}\" \"{1}\" {2}", inputFilePath, outputVideoPath, videoLen);
            // 进度条进程
            ParameterizedThreadStart threadProgressBar = new ParameterizedThreadStart(fakeAddProgress);
            
            progressThread = new Thread(threadProgressBar);
            progressThread.IsBackground = true;
            progressThread.Start();
            // 调用控制台
            performCMD(cmd, out output);
            // 多线程结束方法
            this.CMDProgressOver(output);
        }

        // 多线程增加progress条数方法
        public void fakeAddProgress(object state)
        {
            for (int i = 0; i < 100; i++)
            {
                Thread.Sleep(1000);
                this.Invoke(new ShowProgressDelegate(ShowProgress), new object[] { 100, i });
            }
        }

        // 显示Progress代理方法，以便实时更新Progress Bar
        void ShowProgress(int totalStep, int currentStep)
        {
            this.progressBar1.Maximum = totalStep;
            this.progressBar1.Value = currentStep;
        }

        


        private void performCMD(string command, out string output)
        {
            // 新建进程
            using (Process pc = new Process())
            {
                pc.StartInfo.FileName = "cmd.exe";
                pc.StartInfo.WorkingDirectory = Application.StartupPath;
                pc.StartInfo.CreateNoWindow = true;  // 隐藏窗口运行
                pc.StartInfo.RedirectStandardError = true;//重定向错误流
                pc.StartInfo.RedirectStandardInput = true;//重定向输入流
                pc.StartInfo.RedirectStandardOutput = true;//重定向输出流
                pc.StartInfo.UseShellExecute = false;
                pc.Start();

                pc.StandardInput.WriteLine(command + "&exit");//输入CMD命令
                pc.StandardInput.AutoFlush = true;

                output = pc.StandardOutput.ReadToEnd();//读取结果        
                pc.WaitForExit();
                pc.Close();
            }
        }

        private void txtBoxLog_TextChanged(object sender, EventArgs e)
        {
            //Set the current caret position at the end
            txtBoxLog.SelectionStart = txtBoxLog.Text.Length;
            //Now scroll it automatically
            txtBoxLog.ScrollToCaret();
        }

        private void aboutDecoderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FormAbout ab = new FormAbout();
            ab.ShowDialog();
        }
    }

}
