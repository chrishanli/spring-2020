using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace shipper_gui
{
    public partial class FormSel : Form
    {

        public enum Selected
        {
            ENCODER,
            DECODER,
            NONE
        }

        public Selected UserSelected = Selected.NONE;

        public FormSel()
        {
            InitializeComponent();
        }

        private void btnEncoder_Click(object sender, EventArgs e)
        {
            this.UserSelected = Selected.ENCODER;
            this.Close();
        }

        private void FormSel_Load(object sender, EventArgs e)
        {

        }

        private void btnDecoder_Click(object sender, EventArgs e)
        {
            this.UserSelected = Selected.DECODER;
            this.Close();
        }
    }
}
