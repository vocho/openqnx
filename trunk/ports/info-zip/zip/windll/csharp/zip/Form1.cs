#region README

	//_____________________________________________________________________________
	//
	//Sample C# code, .NET Framework 1.1, contributed to the Info-Zip project by 
	//Adrian Maull, April 2005.
	//
	//If you have questions or comments, contact me at adrian.maull@sprintpcs.com.  Though
	//I will try to respond to coments/questions, I do not guarantee such response.
	//
	//THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	//KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	//IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	//PARTICULAR PURPOSE.
	//
	//_____________________________________________________________________________


#endregion

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;

namespace CSharpInfoZip_ZipSample
{
	/// <summary>
	/// Summary description for Form1.
	/// </summary>
	public class Form1 : System.Windows.Forms.Form
	{
		private System.Windows.Forms.Button btnSimpleZip;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.TextBox textBox1;
		private System.Windows.Forms.TextBox textBox2;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Button btnZipRecurse;
		private System.Windows.Forms.Button btnStop;

		//Define the Zip object
		private Zip m_ZipObj;

		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public Form1()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if (components != null) 
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.btnSimpleZip = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.textBox1 = new System.Windows.Forms.TextBox();
			this.textBox2 = new System.Windows.Forms.TextBox();
			this.label2 = new System.Windows.Forms.Label();
			this.btnZipRecurse = new System.Windows.Forms.Button();
			this.btnStop = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// btnSimpleZip
			// 
			this.btnSimpleZip.Location = new System.Drawing.Point(8, 16);
			this.btnSimpleZip.Name = "btnSimpleZip";
			this.btnSimpleZip.TabIndex = 0;
			this.btnSimpleZip.Text = "Simple Zip";
			this.btnSimpleZip.Click += new System.EventHandler(this.btnSimpleZip_Click);
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(8, 64);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(168, 16);
			this.label1.TabIndex = 1;
			this.label1.Text = "Zip DLL print callback message:";
			// 
			// textBox1
			// 
			this.textBox1.Location = new System.Drawing.Point(8, 80);
			this.textBox1.Multiline = true;
			this.textBox1.Name = "textBox1";
			this.textBox1.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.textBox1.Size = new System.Drawing.Size(456, 88);
			this.textBox1.TabIndex = 2;
			this.textBox1.Text = "";
			// 
			// textBox2
			// 
			this.textBox2.Location = new System.Drawing.Point(8, 200);
			this.textBox2.Multiline = true;
			this.textBox2.Name = "textBox2";
			this.textBox2.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
			this.textBox2.Size = new System.Drawing.Size(456, 88);
			this.textBox2.TabIndex = 4;
			this.textBox2.Text = "";
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(8, 184);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(184, 16);
			this.label2.TabIndex = 3;
			this.label2.Text = "Zip DLL service callback message:";
			// 
			// btnZipRecurse
			// 
			this.btnZipRecurse.Location = new System.Drawing.Point(96, 16);
			this.btnZipRecurse.Name = "btnZipRecurse";
			this.btnZipRecurse.Size = new System.Drawing.Size(128, 23);
			this.btnZipRecurse.TabIndex = 5;
			this.btnZipRecurse.Text = "Zip a folder recursively";
			this.btnZipRecurse.Click += new System.EventHandler(this.btnZipRecurse_Click);
			// 
			// btnStop
			// 
			this.btnStop.Location = new System.Drawing.Point(392, 16);
			this.btnStop.Name = "btnStop";
			this.btnStop.TabIndex = 6;
			this.btnStop.Text = "Stop";
			this.btnStop.Click += new System.EventHandler(this.btnStop_Click);
			// 
			// Form1
			// 
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.ClientSize = new System.Drawing.Size(474, 304);
			this.Controls.Add(this.btnStop);
			this.Controls.Add(this.btnZipRecurse);
			this.Controls.Add(this.textBox2);
			this.Controls.Add(this.textBox1);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.btnSimpleZip);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.MaximizeBox = false;
			this.MinimizeBox = false;
			this.Name = "Form1";
			this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
			this.Text = "Form1";
			this.Load += new System.EventHandler(this.Form1_Load);
			this.ResumeLayout(false);

		}
		#endregion

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main() 
		{
			Application.Run(new Form1());
		}


		#region Event Handlers

		private void Form1_Load(object sender, System.EventArgs e)
		{
			btnStop.Enabled = false;
		}

		private void btnSimpleZip_Click(object sender, System.EventArgs e)
		{

			textBox1.Text = string.Empty;
			textBox2.Text = string.Empty;

			//Set up the files array
			string [] files = new string[2];
			
			//NOTE:
			//Change these file names to whatever is appropriate
			//on your system
			files[0] = @"c:\tmp\testzipfile1.rtf";
			files[1] = @"c:\tmp\testzipfile2.rtf";

			//Instantiate the Zip object
			m_ZipObj = new Zip();

			//Set the Zip object properties
			m_ZipObj.FilesToZip = files;
			m_ZipObj.Verbose = VerboseMessagesEnum.True;
			m_ZipObj.Level = CompressionLevelEnum.Level6;

			//This is optional.  This is how to add a comment
			m_ZipObj.Comment = "Test Comment";
			m_ZipObj.CommentOption = CommentEnum.True;

			//NOTE:
			//File name of the resulting zip file.  Change this as appropriate
			m_ZipObj.ZipFileName = @"c:\tmp\zip\csharpzip.zip";

			//Wire the event handlers to receive the events from the Zip class
			m_ZipObj.ReceivePrintMessage +=new ZipDLLPrintMessageEventHandler(zipObj_ReceivePrintMessage);
			m_ZipObj.ReceiveServiceMessage +=new ZipDLLServiceMessageEventHandler(zipObj_ReceiveServiceMessage);

			btnStop.Enabled = true;

			//Zip the files
			int ret = m_ZipObj.ZipFiles();

			//Examine the return code
			MessageBox.Show("Done.  Return Code: " + ret.ToString());
		}



		private void btnZipRecurse_Click(object sender, System.EventArgs e)
		{
			//NOTE:		
			//Consult the zip32 limits provided in the documentation

			textBox1.Text = string.Empty;
			textBox2.Text = string.Empty;

			//Instantiate the Zip object
			m_ZipObj = new Zip();

			//_____________________________________________________________________________
			//WORK AROUND:
			//The InfoZip documentation states that the zip32.dll can recurse directories if the -r or -R flag is specified. 
			//In code this is specified by setting the fRecurse flag of the ZPOPT structure to 1 (-r) or 2 (-R).
			//However, in my test, when I specified either of the recurse flags I frequently received errors coming
			//back from the ZpArchive function.  My work around is to recurse the specified directories and prepare
			//an array of file names and pass that array to zip32.  Everything seems to work OK if I do this.

			//If you want to try the recurse option, below is an example of how to do it.

//			string [] files = new string[1];
//
//			//NOTE:
//			//Change this to whatever is appropriate
//			System.IO.Directory.SetCurrentDirectory(@"C:\TmpTest");
//
//			//Specify the file mask you want to use.  Consult the zip32.dll documentation for the 
//			//-r and -R options
//			files[0] = @"*.*";
//
//			m_ZipObj.FilesToZip = files;
//			m_ZipObj.Recurse = RecurseEnum.Level2;
//			m_ZipObj.Verbose = VerboseMessagesEnum.True;
//			m_ZipObj.Level = CompressionLevelEnum.Level6;
//
//			//NOTE:
//			//File name of the resulting zip file.  Change this as appropriate
//			m_ZipObj.ZipFileName = @"c:\tmp\zip\csharprecurse.zip";
//
//			//Wire the event handlers to receive the events from the Zip class
//			m_ZipObj.ReceivePrintMessage +=new ZipDLLPrintMessageEventHandler(zipObj_ReceivePrintMessage);
//			m_ZipObj.ReceiveServiceMessage +=new ZipDLLServiceMessageEventHandler(zipObj_ReceiveServiceMessage);
//
//			btnStop.Enabled = true;
//
//			//Zip the files
//			int ret = m_ZipObj.ZipFiles();
//
//			//Examine the return code
//			MessageBox.Show("Done. Ret: " + ret.ToString());

			//_____________________________________________________________________________
			//Prepare an array of all the files in the directory

			//NOTE:
			//Change this to whatever is appropriate
			string root = @"C:\Tmp\TestZip";

			ArrayList fileList = new ArrayList();
			RecurseGetDirsAndFiles(root, fileList);
			fileList.TrimToSize();

			//Build the files array.  Practically, the .NET limit here is your RAM
			string [] files = new string[fileList.Count];
			int idx = 0;

			IEnumerator en = fileList.GetEnumerator();
			while ( en.MoveNext() )
			{
				files[idx] = en.Current.ToString();
				idx++;
			}

			//Set the Zip object properties
			m_ZipObj.FilesToZip = files;
			m_ZipObj.Verbose = VerboseMessagesEnum.True;
			m_ZipObj.Level = CompressionLevelEnum.Level6;

			//This is optional.  This is how to add a comment
			m_ZipObj.Comment = "Test Comment";
			m_ZipObj.CommentOption = CommentEnum.True;
			
			//NOTE:
			//File name of the resulting zip file.  Change this as appropriate
			m_ZipObj.ZipFileName = @"c:\tmp\zip\csharpzip2.zip";

			//Wire the event handlers to receive the events from the Zip class
			m_ZipObj.ReceivePrintMessage +=new ZipDLLPrintMessageEventHandler(zipObj_ReceivePrintMessage);
			m_ZipObj.ReceiveServiceMessage +=new ZipDLLServiceMessageEventHandler(zipObj_ReceiveServiceMessage);

			btnStop.Enabled = true;

			//Zip the files
			int ret = m_ZipObj.ZipFiles();

			//Examine the return code
			MessageBox.Show("Done. Ret: " + ret.ToString());
		}

		private void zipObj_ReceivePrintMessage(object sender, ZipDLLPrintMessageEventArgs e)
		{
			textBox1.Text = e.PrintMessage + "\r\n";
			Application.DoEvents();
		}

		private void zipObj_ReceiveServiceMessage(object sender, ZipDLLServiceMessageEventArgs e)
		{
			textBox2.Text = "Zipping: " + e.ArchiveFileName + " - " + e.SizeOfArchiveFile.ToString() + " bytes." + "\r\n";
			Application.DoEvents();
		}

		#endregion

		#region Private Functions

		private void RecurseGetDirsAndFiles(string path, ArrayList al)
		{
			System.IO.DirectoryInfo d = new System.IO.DirectoryInfo(path);
	
			// List files.
			foreach(System.IO.FileInfo f in d.GetFiles())
				al.Add(f.FullName);

			// Recurse into directories.
			foreach(System.IO.DirectoryInfo dx in d.GetDirectories())
				RecurseGetDirsAndFiles(dx.FullName,al);
		}

		
		private void btnStop_Click(object sender, System.EventArgs e)
		{
			m_ZipObj.Stop();
		}


		#endregion

	}
}
