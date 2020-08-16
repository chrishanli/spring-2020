using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace shipper_gui
{
    public partial class FormEncoderPref : Form
    {
        // 供主调窗体访问的设置变量
        public int videoLen;
        public int videoWid;
        public int videoHei;

        public FormEncoderPref()
        {
            InitializeComponent();
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            // 设置值
            videoLen = (int)txtVideoLen.Value;
            if (chkComputed.Checked == false)
            {
                videoWid = (int)txtWidth.Value;
                videoHei = (int)txtHeight.Value;
            } else
            {
                videoWid = videoHei = 0;
            }

            this.Close();
        }

        private void FormEncoderPref_Load(object sender, EventArgs e)
        {
            // 设置值
            txtVideoLen.Value = videoLen;
            txtWidth.Value = videoWid;
            txtHeight.Value = videoHei;

            // 设置控件可见
            if (videoWid == 0 || videoHei == 0)
            {
                txtWidth.Enabled = false;
                txtHeight.Enabled = false;
                chkComputed.Checked = true;
            }
            else
            {
                txtWidth.Enabled = true;
                txtHeight.Enabled = true;
                chkComputed.Checked = false;
            }
        }

        private void chkComputed_CheckedChanged(object sender, EventArgs e)
        {
            if (chkComputed.Checked == false)
            {
                // 如果不计算，就让用户输入
                txtWidth.Enabled = true;
                txtHeight.Enabled = true;
            }
            else
            {
                // 如果要计算
                txtWidth.Enabled = false;
                txtHeight.Enabled = false;
                videoWid = 0;
                videoHei = 0;
            }
        }

    }
}
