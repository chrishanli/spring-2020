namespace shipper_gui
{
    partial class FormSel
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
            this.btnDecoder = new System.Windows.Forms.Button();
            this.btnEncoder = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // btnDecoder
            // 
            this.btnDecoder.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btnDecoder.Location = new System.Drawing.Point(52, 61);
            this.btnDecoder.Name = "btnDecoder";
            this.btnDecoder.Size = new System.Drawing.Size(190, 55);
            this.btnDecoder.TabIndex = 0;
            this.btnDecoder.Text = "Open Decoder";
            this.btnDecoder.UseVisualStyleBackColor = true;
            this.btnDecoder.Click += new System.EventHandler(this.btnDecoder_Click);
            // 
            // btnEncoder
            // 
            this.btnEncoder.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.btnEncoder.Location = new System.Drawing.Point(52, 145);
            this.btnEncoder.Name = "btnEncoder";
            this.btnEncoder.Size = new System.Drawing.Size(190, 55);
            this.btnEncoder.TabIndex = 1;
            this.btnEncoder.Text = "Open Encoder";
            this.btnEncoder.UseVisualStyleBackColor = true;
            this.btnEncoder.Click += new System.EventHandler(this.btnEncoder_Click);
            // 
            // FormSel
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 15F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
            this.ClientSize = new System.Drawing.Size(292, 268);
            this.Controls.Add(this.btnEncoder);
            this.Controls.Add(this.btnDecoder);
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "FormSel";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Shipper > Main";
            this.Load += new System.EventHandler(this.FormSel_Load);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnDecoder;
        private System.Windows.Forms.Button btnEncoder;
    }
}