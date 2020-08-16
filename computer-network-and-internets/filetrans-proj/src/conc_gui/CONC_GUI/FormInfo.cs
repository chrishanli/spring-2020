using System.Windows.Forms;

namespace CONC_GUI
{
    public partial class FormInfo : Form
    {
        public FormInfo()
        {
            InitializeComponent();
        }

        private void linkLabel1_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            System.Diagnostics.Process.Start(this.linkLabel1.Text);
        }

        private void FormInfo_Load(object sender, System.EventArgs e)
        {
            this.txtDialogue.Text = "The CONC GUI (" + System.Reflection.Assembly.GetExecutingAssembly().GetName().Version.ToString() + ") - A Graphical User Interface runs over the CONC core, client and server.\r\n";

            this.txtDialogue.AppendText("\r\nServer usage: Fill in informations about the host server, and simply press start to open a server instance.\r\nClient usage: Simply browse file or folder and specify a target server through its address and port that the pending data are to be delevered.\r\n\r\nThe CONC® Core - Conc file transfer solutions. Copyright © 2020 Han Li Studios. All Rights Reserved. \r\n\r\nThis application (CONC-GUI) is under the MIT License:\r\nCopyright(C) 2020 Miao Xinyu, Li Han.\r\nPermission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\r\n\r\nThe above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\r\n\r\nTHE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.");
        }
    }
}
