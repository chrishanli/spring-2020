using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace shipper_gui
{
    public partial class FormDecoderPref : Form
    {
        public int sample_freq = 28;
        public bool isVerify = false;

        public FormDecoderPref()
        {
            InitializeComponent();
        }

        private void FormDecoderPref_Load(object sender, EventArgs e)
        {
            // 设置值
            txtFPS.Value = sample_freq;
            chkVerify.Enabled = isVerify;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            sample_freq = (int)txtFPS.Value;
            this.Close();
        }

        private void groupBox1_Enter(object sender, EventArgs e)
        {

        }

        
    }
}
