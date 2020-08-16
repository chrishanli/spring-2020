using System;
using System.Windows.Forms;

namespace CONC_GUI
{
    public partial class FormAdvanced : Form
    {
        // 端口设置
        public string portMon;
        public string portErr;
        public bool portHasChanged = false;

        // 构造函数
        public FormAdvanced(string defaultMonPort, string defaultErrPort)
        {
            InitializeComponent();
            portMon = defaultMonPort;
            portErr = defaultErrPort;
            txtMonPort.Text = portMon;
            txtErrPort.Text = portErr;
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            // 保存
            portMon = txtMonPort.Text;
            portErr = txtErrPort.Text;
            portHasChanged = true;
            this.Close();
        }
    }
}
