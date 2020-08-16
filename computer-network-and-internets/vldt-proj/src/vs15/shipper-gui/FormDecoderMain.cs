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
    public partial class FormDecoderMain : Form
    {
        // 用户设置
        private string inputVideoPath;
        private string outputFilePath;
        private string outputFileName = "out.bin";
        private string outputVeriPath;
        private string outputVeriName = "verify.bin";
        private int sample_freq = 28;
        private bool isVerify = false;

        private bool isInputSet = false;
        private bool isOutpuSet = false;
        private bool isVerifSet = false;

        public FormDecoderMain()
        {
            InitializeComponent();
        }

        private void FormDecoderMain_Load(object sender, EventArgs e)
        {
            // 设置按钮
            btnStart.Enabled = false;
        }


        private void setInputFile()
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Multiselect = false;
            dialog.Title = "Choose Input Video";
            dialog.Filter = "mp4 File(*.mp4)|*.mp4";
            dialog.RestoreDirectory = true;
            if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                inputVideoPath = dialog.FileName;
                txtFileP.Text = inputVideoPath;
                isInputSet = true;
            }
        }

        private void setOutputFile()
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.FileName = outputFileName;
            dialog.Title = "Choose Place to Store Output File";
            dialog.Filter = "All File(*.*)|*.*";
            dialog.RestoreDirectory = true;
            if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                outputFilePath = dialog.FileName;
                txtOutputP.Text = outputFilePath;
                isOutpuSet = true;
            }
        }

        private void setVeriFile()
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.FileName = outputVeriName;
            dialog.Title = "Choose Place to Store Output File";
            dialog.Filter = "All File(*.*)|*.*";
            dialog.RestoreDirectory = true;
            if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
            {
                outputVeriPath = dialog.FileName;
                txtVeriFile.Text = outputVeriPath;
                isVerifSet = true;
            }
        }

        private void btnBrowVideo_Click(object sender, EventArgs e)
        {
            setInputFile();
            if (isInputSet && isOutpuSet && isVerifSet)
            {
                btnStart.Enabled = true;
            }
        }

        private void btnBrowOutp_Click(object sender, EventArgs e)
        {
            setOutputFile();
            if (isInputSet && isOutpuSet && isVerifSet)
            {
                btnStart.Enabled = true;
            }
        }

        private void btnBrowVeri_Click(object sender, EventArgs e)
        {
            setVeriFile();
            if (isInputSet && isOutpuSet && isVerifSet)
            {
                btnStart.Enabled = true;
            }
        }

        private void btnDecSett_Click(object sender, EventArgs e)
        {
            // 新建窗体
            FormDecoderPref frmPref = new FormDecoderPref();
            frmPref.sample_freq = sample_freq;
            frmPref.isVerify = isVerify;

            // 显示窗体
            frmPref.ShowDialog();

            // 接受返回值
            this.sample_freq = frmPref.sample_freq;
            this.isVerify = frmPref.isVerify;
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            setInputFile();
        }

        private void assignToolStripMenuItem_Click(object sender, EventArgs e)
        {
            setVeriFile();
        }

        private void toolOutFile_Click(object sender, EventArgs e)
        {
            setOutputFile();
        }

        private void preferencesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // 新建窗体
            FormDecoderPref frmPref = new FormDecoderPref();
            frmPref.sample_freq = sample_freq;
            frmPref.isVerify = isVerify;

            // 显示窗体
            frmPref.ShowDialog();

            // 接受返回值
            this.sample_freq = frmPref.sample_freq;
            this.isVerify = frmPref.isVerify;
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
            txtBoxLog.AppendText("> Decode is started, please wait...\n");
            progressBar1.Value = 0;
            PerformThread = new Thread(new ThreadStart(this.performDecode));
            PerformThread.Start();

            // 设置状态栏
            tbState.Text = "Decoding";
        }

        // 实现控制进度条的委托
        delegate void ShowProgressDelegate(int totalStep, int currentStep);
        delegate void SetMainDelegate();
        Thread progressThread;
        Thread PerformThread;


        // 命令执行结束后的委托方法
        private void CMDProgressOver()
        {
            if (this.btnStart.InvokeRequired)
            {
                while (!this.btnStart.IsHandleCreated)
                {
                    if (this.btnStart.Disposing ||
                        this.btnStart.IsDisposed)
                        return;
                }
                SetMainDelegate d = new SetMainDelegate(CMDProgressOver);
                this.btnStart.Invoke(d, new object[] {});
            }
            else
            {
                // 设置btn
                btnStart.Enabled = true;
                progressBar1.Value = 100;

                // 设置状态栏
                tbState.Text = "Ready";
            }
        }
        
        // 在另一Process进行Encode
        private void performDecode()
        {
            string cmd;
            if (!isVerify)
            {
                cmd = String.Format("\"{0}\" \"{1}\" \"{2}\" --sample-freq {3} ", inputVideoPath, outputFilePath, outputVeriPath, sample_freq);
            } else
            {
                cmd = String.Format("\"{0}\" \"{1}\" \"{2}\" --sample-freq {3} --verify", inputVideoPath, outputFilePath, outputVeriPath, sample_freq);
            }

            performCMD(cmd);
            this.CMDProgressOver();
        }

        // 多线程增加progress条数方法
        public void fakeAddProgress(object state)
        {
            for (int i = 0; i < 100; i++)
            {
                Thread.Sleep(100);
                this.Invoke(new ShowProgressDelegate(ShowProgress), new object[] { 100, i });
            }
        }

        // 显示Progress代理方法，以便实时更新Progress Bar
        void ShowProgress(int totalStep, int currentStep)
        {
            this.progressBar1.Maximum = totalStep;
            this.progressBar1.Value = currentStep;
        }

        private delegate void AddMsgHandler(string msg);

        private void OutputDataRecved(object sender, DataReceivedEventArgs e)
        {
            AddMsgHandler handler = delegate (string msg)
            {
                this.txtBoxLog.AppendText(msg + Environment.NewLine);
            };
            if (this.txtBoxLog.InvokeRequired)
            {
                this.txtBoxLog.Invoke(handler, e.Data);
            }
        }


        private void performCMD(string command)
        {
            // 新建进程
            using (Process pc = new Process())
            {
                string exeName = Application.StartupPath + @"\decode.exe";

                pc.StartInfo.FileName = exeName;
                pc.StartInfo.Arguments = command;

                pc.StartInfo.CreateNoWindow = true;  // 隐藏窗口运行
                pc.StartInfo.RedirectStandardError = true;//重定向错误流
                pc.StartInfo.RedirectStandardInput = true;//重定向输入流
                pc.StartInfo.RedirectStandardOutput = true;//重定向输出流
                pc.StartInfo.UseShellExecute = false;
                pc.OutputDataReceived += new DataReceivedEventHandler(OutputDataRecved);

                pc.Start();
                pc.BeginOutputReadLine();

                //output = pc.StandardOutput.ReadToEnd();//读取结果        
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
