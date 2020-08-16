using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Media;
using System.Net;
using System.Net.NetworkInformation;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CONC_GUI
{
    public partial class FormServerConfig : Form
    {
        bool isWorkingDirSet = false;
        bool isIPSet = false;
        bool isPortSet = true;


        public FormServerConfig()
        {
            InitializeComponent();
            // 获取所有IP，及设置comboBox
            string[] allIP = loadAllIPs();
            if (allIP.Count() == 0)
            {
                // 本机木有IP地址
                this.cboxIPAddr.Items.Add("No IP detected!");
                this.cboxIPAddr.Enabled = false;
            } else
            {
                this.cboxIPAddr.Items.Add("Please Select or Enter an Address");
                foreach (string ipAddr in allIP)
                {
                    this.cboxIPAddr.Items.Add(ipAddr);
                }
            }
            this.cboxIPAddr.SelectedIndex = 0;

            // 禁用button
            btnStartServer.Enabled = false;
        }

        // 获取本机所有IP
        public static string[] loadAllIPs()
        {
            List<string> str = new List<string>();
            string hostName = Dns.GetHostName();
            // 本机名   
            System.Net.IPAddress[] addressList = Dns.GetHostAddresses(hostName);
            // 会返回所有地址，包括IPv4和IPv6   
            foreach (IPAddress ip in addressList)
            {
                string _temp_ip = ip.MapToIPv4().ToString();
                string[] ipstr = _temp_ip.Split('.');
                string temp = str.Where(s => s == _temp_ip).FirstOrDefault();
                if (temp == null)
                {
                    str.Add(_temp_ip);
                }
            }
            return str.ToArray();
        }

        // 检查该TCP端口是否在用
        private bool isTCPPortInUse(int port)
        {
            bool flag = false;
            IPGlobalProperties properties = IPGlobalProperties.GetIPGlobalProperties();
            IPEndPoint[] ipendpoints = null;
            ipendpoints = properties.GetActiveTcpListeners();

            foreach (IPEndPoint ipendpoint in ipendpoints)
            {
                if (ipendpoint.Port == port)
                {
                    flag = true;
                    break;
                }
            }
            ipendpoints = null;
            properties = null;
            return flag;
        }

        // 检查是否能开始
        private void enableSendButtonIfSendable()
        {
            if (isPortSet && isWorkingDirSet && isIPSet)
            {
                this.btnStartServer.Enabled = true;
            } else
            {
                this.btnStartServer.Enabled = false;
            }
        }


        private void btnBrowse_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            if (fbd.ShowDialog() == DialogResult.OK)
            {
                txtWorkingDir.Text = fbd.SelectedPath;
                txtWorkingDir.ForeColor = Color.Black;
            } 
            if (this.txtWorkingDir.Text != "")
            {
                isWorkingDirSet = true;
            } else
            {
                isWorkingDirSet = false;
            }

            enableSendButtonIfSendable();
        }

        private void cboxIPAddr_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (cboxIPAddr.SelectedIndex == 0)
            {
                isIPSet = false;
            } else
            {
                isIPSet = true;
            }

            enableSendButtonIfSendable();
        }


        private void txtPort_TextChanged(object sender, EventArgs e)
        {
            // 是数字，才表示指定了正确的port
            if (Regex.IsMatch(txtPort.Text, "^[0-9]+$"))
            {
                isPortSet = true;
                btnCheck.Enabled = true;
            }
            else
            {
                isPortSet = false;
                btnCheck.Enabled = false;
            }

            enableSendButtonIfSendable();
        }

        // 检查PORT是否被占用，提示用户
        private void btnCheck_Click(object sender, EventArgs e)
        {
            if (!isPortSet)
            {
                SystemSounds.Exclamation.Play();
                MessageBox.Show(this, "Monitor port was not specified.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            // 显示加载中窗体
            ProgressSpecifier ps = new ProgressSpecifier();
            ps.Show();

            bool rep = isTCPPortInUse(Int32.Parse(this.txtPort.Text));
            ps.Close();
            if (rep == false)
            {
                MessageBox.Show(this, "This port is not in use at this time.", "Info", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            else
            {
                MessageBox.Show(this, "This port is occupied at this time. Please change one.", "Info", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
        }

        private void btnStartServer_Click(object sender, EventArgs e)
        {
            DialogResult res = MessageBox.Show(this, "Are you sure to leave the CONC GUI and go ahead to the CLI mode?", "Confirm", MessageBoxButtons.OKCancel, MessageBoxIcon.Question);
            if (res == DialogResult.Cancel)
            {
                return;
            }

            MessageBox.Show(this, "The CLI window will not close unless all transfer session is done, or error occurred.", "Info", MessageBoxButtons.OK, MessageBoxIcon.Information);

            string command = "sv -w=" + 
                "\"" + txtWorkingDir.Text + "\" -a=" + 
                cboxIPAddr.Text + " -p=" + 
                txtPort.Text;
            // 新建进程
            using (Process pc = new Process())
            {
                string exeName = Application.StartupPath + @"\res\conc";

                pc.StartInfo.FileName = exeName;
                pc.StartInfo.Arguments = command;

                pc.StartInfo.CreateNoWindow = false;  // 这回不隐藏窗口运行

                pc.StartInfo.UseShellExecute = true;
                try
                {
                    pc.Start();
                }
                catch (Exception) { };
            }

            this.Close();
        }

        private void FormServerConfig_Load(object sender, EventArgs e)
        {

        }

        private void cboxIPAddr_TextChanged(object sender, EventArgs e)
        {
            if (cboxIPAddr.Text == "")
            {
                cboxIPAddr.SelectedIndex = 0;
                isIPSet = false;
            } else
            {
                isIPSet = true;
            }
            enableSendButtonIfSendable();
        }
    }
}
