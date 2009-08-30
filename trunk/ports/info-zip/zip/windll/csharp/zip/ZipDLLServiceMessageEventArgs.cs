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

namespace CSharpInfoZip_ZipSample
{
	/// <summary>
	/// Summary description for ZipDLLServiceMessageEventArgs.
	/// </summary>
	public class ZipDLLServiceMessageEventArgs
	{

		#region Private Vars

		private int m_SizeOfArchiveFile = 0;
		private string m_ArchiveFileName = string.Empty;

		#endregion

		//fileName - name of an individual file in the zip
		//archiveFileBytes - size of an individual file in the zip
		public ZipDLLServiceMessageEventArgs(string fileName, int archiveFileBytes)
		{
			m_SizeOfArchiveFile = archiveFileBytes;
			m_ArchiveFileName = fileName;
		}
		
		#region Properties

		public int SizeOfArchiveFile
		{
			get {return m_SizeOfArchiveFile;}
		}

		public string ArchiveFileName
		{
			get {return m_ArchiveFileName;}
		}

		#endregion

	}
}
