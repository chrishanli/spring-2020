namespace shipper_gui
{
    partial class FormDecoderMain
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FormDecoderMain));
            this.menuStrip = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolOpen = new System.Windows.Forms.ToolStripMenuItem();
            this.toolOutFile = new System.Windows.Forms.ToolStripMenuItem();
            this.toolAssignVeri = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolDecodeSett = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
            this.toolExit = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutDecoderToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
            this.noHelpsAvailableToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.groupBoxPaths = new System.Windows.Forms.GroupBox();
            this.btnBrowVeri = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.txtVeriFile = new System.Windows.Forms.TextBox();
            this.btnBrowOutp = new System.Windows.Forms.Button();
            this.btnBrowInpu = new System.Windows.Forms.Button();
            this.txtOutputP = new System.Windows.Forms.TextBox();
            this.txtFileP = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.txtBoxLog = new System.Windows.Forms.RichTextBox();
            this.btnSaveLog = new System.Windows.Forms.Button();
            this.btnDecSett = new System.Windows.Forms.Button();
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.tbState = new System.Windows.Forms.ToolStripStatusLabel();
            this.btnStart = new System.Windows.Forms.Button();
            this.progressBar1 = new System.Windows.Forms.ProgressBar();
            this.menuStrip.SuspendLayout();
            this.groupBoxPaths.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.statusStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip
            // 
            this.menuStrip.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.menuStrip.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.menuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.menuStrip.Location = new System.Drawing.Point(0, 0);
            this.menuStrip.Name = "menuStrip";
            this.menuStrip.Size = new System.Drawing.Size(552, 28);
            this.menuStrip.TabIndex = 2;
            this.menuStrip.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolOpen,
            this.toolOutFile,
            this.toolAssignVeri,
            this.toolStripMenuItem1,
            this.toolDecodeSett,
            this.toolStripMenuItem2,
            this.toolExit});
            this.fileToolStripMenuItem.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(52, 24);
            this.fileToolStripMenuItem.Text = "&Start";
            // 
            // toolOpen
            // 
            this.toolOpen.Name = "toolOpen";
            this.toolOpen.ShortcutKeys = ((System.Windows.Forms.Keys)(((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Alt) 
            | System.Windows.Forms.Keys.I)));
            this.toolOpen.Size = new System.Drawing.Size(295, 26);
            this.toolOpen.Text = "&Import Video File...";
            this.toolOpen.Click += new System.EventHandler(this.openToolStripMenuItem_Click);
            // 
            // toolOutFile
            // 
            this.toolOutFile.Name = "toolOutFile";
            this.toolOutFile.ShortcutKeys = ((System.Windows.Forms.Keys)(((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Alt) 
            | System.Windows.Forms.Keys.O)));
            this.toolOutFile.Size = new System.Drawing.Size(295, 26);
            this.toolOutFile.Text = "&Assign Output File...";
            this.toolOutFile.Click += new System.EventHandler(this.toolOutFile_Click);
            // 
            // toolAssignVeri
            // 
            this.toolAssignVeri.Name = "toolAssignVeri";
            this.toolAssignVeri.ShortcutKeys = ((System.Windows.Forms.Keys)(((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Alt) 
            | System.Windows.Forms.Keys.V)));
            this.toolAssignVeri.Size = new System.Drawing.Size(295, 26);
            this.toolAssignVeri.Text = "&Assign Verify File...";
            this.toolAssignVeri.Click += new System.EventHandler(this.assignToolStripMenuItem_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(292, 6);
            // 
            // toolDecodeSett
            // 
            this.toolDecodeSett.Name = "toolDecodeSett";
            this.toolDecodeSett.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Alt | System.Windows.Forms.Keys.P)));
            this.toolDecodeSett.Size = new System.Drawing.Size(295, 26);
            this.toolDecodeSett.Text = "&Decode Settings";
            this.toolDecodeSett.Click += new System.EventHandler(this.preferencesToolStripMenuItem_Click);
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(292, 6);
            // 
            // toolExit
            // 
            this.toolExit.Name = "toolExit";
            this.toolExit.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.Q)));
            this.toolExit.Size = new System.Drawing.Size(295, 26);
            this.toolExit.Text = "&Exit Shipper";
            this.toolExit.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutDecoderToolStripMenuItem,
            this.toolStripMenuItem3,
            this.noHelpsAvailableToolStripMenuItem});
            this.helpToolStripMenuItem.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(53, 24);
            this.helpToolStripMenuItem.Text = "&Help";
            // 
            // aboutDecoderToolStripMenuItem
            // 
            this.aboutDecoderToolStripMenuItem.Name = "aboutDecoderToolStripMenuItem";
            this.aboutDecoderToolStripMenuItem.Size = new System.Drawing.Size(212, 26);
            this.aboutDecoderToolStripMenuItem.Text = "&About Decoder";
            this.aboutDecoderToolStripMenuItem.Click += new System.EventHandler(this.aboutDecoderToolStripMenuItem_Click);
            // 
            // toolStripMenuItem3
            // 
            this.toolStripMenuItem3.Name = "toolStripMenuItem3";
            this.toolStripMenuItem3.Size = new System.Drawing.Size(209, 6);
            // 
            // noHelpsAvailableToolStripMenuItem
            // 
            this.noHelpsAvailableToolStripMenuItem.Enabled = false;
            this.noHelpsAvailableToolStripMenuItem.Name = "noHelpsAvailableToolStripMenuItem";
            this.noHelpsAvailableToolStripMenuItem.Size = new System.Drawing.Size(212, 26);
            this.noHelpsAvailableToolStripMenuItem.Text = "No Helps Available";
            // 
            // groupBoxPaths
            // 
            this.groupBoxPaths.Controls.Add(this.btnBrowVeri);
            this.groupBoxPaths.Controls.Add(this.label3);
            this.groupBoxPaths.Controls.Add(this.txtVeriFile);
            this.groupBoxPaths.Controls.Add(this.btnBrowOutp);
            this.groupBoxPaths.Controls.Add(this.btnBrowInpu);
            this.groupBoxPaths.Controls.Add(this.txtOutputP);
            this.groupBoxPaths.Controls.Add(this.txtFileP);
            this.groupBoxPaths.Controls.Add(this.label2);
            this.groupBoxPaths.Controls.Add(this.label1);
            this.groupBoxPaths.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.groupBoxPaths.Location = new System.Drawing.Point(12, 31);
            this.groupBoxPaths.Name = "groupBoxPaths";
            this.groupBoxPaths.Size = new System.Drawing.Size(528, 130);
            this.groupBoxPaths.TabIndex = 3;
            this.groupBoxPaths.TabStop = false;
            this.groupBoxPaths.Text = "&Paths";
            // 
            // btnBrowVeri
            // 
            this.btnBrowVeri.Location = new System.Drawing.Point(447, 88);
            this.btnBrowVeri.Name = "btnBrowVeri";
            this.btnBrowVeri.Size = new System.Drawing.Size(75, 28);
            this.btnBrowVeri.TabIndex = 8;
            this.btnBrowVeri.Text = "Browse...";
            this.btnBrowVeri.UseVisualStyleBackColor = true;
            this.btnBrowVeri.Click += new System.EventHandler(this.btnBrowVeri_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(20, 92);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(77, 20);
            this.label3.TabIndex = 7;
            this.label3.Text = "&Verify File:";
            // 
            // txtVeriFile
            // 
            this.txtVeriFile.Location = new System.Drawing.Point(103, 89);
            this.txtVeriFile.Name = "txtVeriFile";
            this.txtVeriFile.ReadOnly = true;
            this.txtVeriFile.Size = new System.Drawing.Size(338, 27);
            this.txtVeriFile.TabIndex = 6;
            // 
            // btnBrowOutp
            // 
            this.btnBrowOutp.Location = new System.Drawing.Point(447, 55);
            this.btnBrowOutp.Name = "btnBrowOutp";
            this.btnBrowOutp.Size = new System.Drawing.Size(75, 28);
            this.btnBrowOutp.TabIndex = 5;
            this.btnBrowOutp.Text = "Browse...";
            this.btnBrowOutp.UseVisualStyleBackColor = true;
            this.btnBrowOutp.Click += new System.EventHandler(this.btnBrowOutp_Click);
            // 
            // btnBrowInpu
            // 
            this.btnBrowInpu.Location = new System.Drawing.Point(447, 22);
            this.btnBrowInpu.Name = "btnBrowInpu";
            this.btnBrowInpu.Size = new System.Drawing.Size(75, 28);
            this.btnBrowInpu.TabIndex = 4;
            this.btnBrowInpu.Text = "Browse...";
            this.btnBrowInpu.UseVisualStyleBackColor = true;
            this.btnBrowInpu.Click += new System.EventHandler(this.btnBrowVideo_Click);
            // 
            // txtOutputP
            // 
            this.txtOutputP.Location = new System.Drawing.Point(103, 56);
            this.txtOutputP.Name = "txtOutputP";
            this.txtOutputP.ReadOnly = true;
            this.txtOutputP.Size = new System.Drawing.Size(338, 27);
            this.txtOutputP.TabIndex = 3;
            // 
            // txtFileP
            // 
            this.txtFileP.Location = new System.Drawing.Point(103, 23);
            this.txtFileP.Name = "txtFileP";
            this.txtFileP.ReadOnly = true;
            this.txtFileP.Size = new System.Drawing.Size(338, 27);
            this.txtFileP.TabIndex = 2;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 59);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(91, 20);
            this.label2.TabIndex = 1;
            this.label2.Text = "&Output Path:";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(29, 26);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(68, 20);
            this.label1.TabIndex = 0;
            this.label1.Text = "&File Path:";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.txtBoxLog);
            this.groupBox1.Controls.Add(this.btnSaveLog);
            this.groupBox1.Controls.Add(this.btnDecSett);
            this.groupBox1.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.groupBox1.Location = new System.Drawing.Point(12, 167);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(528, 272);
            this.groupBox1.TabIndex = 4;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Preferences and Status";
            // 
            // txtBoxLog
            // 
            this.txtBoxLog.Location = new System.Drawing.Point(6, 60);
            this.txtBoxLog.Name = "txtBoxLog";
            this.txtBoxLog.ReadOnly = true;
            this.txtBoxLog.Size = new System.Drawing.Size(516, 206);
            this.txtBoxLog.TabIndex = 7;
            this.txtBoxLog.Text = "";
            this.txtBoxLog.TextChanged += new System.EventHandler(this.txtBoxLog_TextChanged);
            // 
            // btnSaveLog
            // 
            this.btnSaveLog.Location = new System.Drawing.Point(145, 26);
            this.btnSaveLog.Name = "btnSaveLog";
            this.btnSaveLog.Size = new System.Drawing.Size(133, 28);
            this.btnSaveLog.TabIndex = 6;
            this.btnSaveLog.Text = "Save Logs...";
            this.btnSaveLog.UseVisualStyleBackColor = true;
            this.btnSaveLog.Click += new System.EventHandler(this.btnSaveLog_Click);
            // 
            // btnDecSett
            // 
            this.btnDecSett.Location = new System.Drawing.Point(6, 26);
            this.btnDecSett.Name = "btnDecSett";
            this.btnDecSett.Size = new System.Drawing.Size(133, 28);
            this.btnDecSett.TabIndex = 5;
            this.btnDecSett.Text = "Decode Settings";
            this.btnDecSett.UseVisualStyleBackColor = true;
            this.btnDecSett.Click += new System.EventHandler(this.btnDecSett_Click);
            // 
            // statusStrip
            // 
            this.statusStrip.ImageScalingSize = new System.Drawing.Size(20, 20);
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tbState});
            this.statusStrip.Location = new System.Drawing.Point(0, 476);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.RenderMode = System.Windows.Forms.ToolStripRenderMode.Professional;
            this.statusStrip.Size = new System.Drawing.Size(552, 25);
            this.statusStrip.SizingGrip = false;
            this.statusStrip.TabIndex = 5;
            this.statusStrip.Text = "statusStrip1";
            // 
            // tbState
            // 
            this.tbState.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.tbState.Name = "tbState";
            this.tbState.Size = new System.Drawing.Size(50, 20);
            this.tbState.Text = "Ready";
            // 
            // btnStart
            // 
            this.btnStart.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btnStart.Location = new System.Drawing.Point(12, 445);
            this.btnStart.Name = "btnStart";
            this.btnStart.Size = new System.Drawing.Size(75, 28);
            this.btnStart.TabIndex = 7;
            this.btnStart.Text = "&Start";
            this.btnStart.UseVisualStyleBackColor = true;
            this.btnStart.Click += new System.EventHandler(this.btnStart_Click);
            // 
            // progressBar1
            // 
            this.progressBar1.Location = new System.Drawing.Point(93, 445);
            this.progressBar1.Name = "progressBar1";
            this.progressBar1.Size = new System.Drawing.Size(447, 28);
            this.progressBar1.TabIndex = 6;
            this.progressBar1.Click += new System.EventHandler(this.progressBar1_Click);
            // 
            // FormDecoderMain
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(120F, 120F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ClientSize = new System.Drawing.Size(552, 501);
            this.Controls.Add(this.btnStart);
            this.Controls.Add(this.progressBar1);
            this.Controls.Add(this.statusStrip);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.groupBoxPaths);
            this.Controls.Add(this.menuStrip);
            this.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip;
            this.MaximizeBox = false;
            this.Name = "FormDecoderMain";
            this.Text = "Shipper > Decoder";
            this.Load += new System.EventHandler(this.FormDecoderMain_Load);
            this.menuStrip.ResumeLayout(false);
            this.menuStrip.PerformLayout();
            this.groupBoxPaths.ResumeLayout(false);
            this.groupBoxPaths.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem toolOpen;
        private System.Windows.Forms.ToolStripMenuItem toolAssignVeri;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem toolDecodeSett;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem toolExit;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutDecoderToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
        private System.Windows.Forms.ToolStripMenuItem noHelpsAvailableToolStripMenuItem;
        private System.Windows.Forms.GroupBox groupBoxPaths;
        private System.Windows.Forms.Button btnBrowOutp;
        private System.Windows.Forms.Button btnBrowInpu;
        private System.Windows.Forms.TextBox txtOutputP;
        private System.Windows.Forms.TextBox txtFileP;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RichTextBox txtBoxLog;
        private System.Windows.Forms.Button btnSaveLog;
        private System.Windows.Forms.Button btnDecSett;
        private System.Windows.Forms.StatusStrip statusStrip;
        private System.Windows.Forms.ToolStripStatusLabel tbState;
        private System.Windows.Forms.Button btnStart;
        private System.Windows.Forms.ProgressBar progressBar1;
        private System.Windows.Forms.ToolStripMenuItem toolOutFile;
        private System.Windows.Forms.TextBox txtVeriFile;
        private System.Windows.Forms.Button btnBrowVeri;
        private System.Windows.Forms.Label label3;
    }
}