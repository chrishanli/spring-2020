using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CONC_GUI
{
    public partial class FormInitialize : Form
    {

        public bool selectedClient = false;
        public bool selectedServer = false;

        public FormInitialize()
        {
            InitializeComponent();
        }

        private void btnOpenCllient_Click(object sender, EventArgs e)
        {
            selectedClient = true;
            this.Close();
        }

        private void btnOpenServer_Click(object sender, EventArgs e)
        {
            selectedServer = true;
            this.Close();
        }

        private void btnAbout_Click(object sender, EventArgs e)
        {
            FormInfo fi = new FormInfo();
            fi.ShowDialog();
        }

        private void FormInitialize_Load(object sender, EventArgs e)
        {
            this.lblVersion.Text = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.ToString();
        }
    }
}
