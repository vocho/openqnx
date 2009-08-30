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
	/// Summary description for ZipDLLPrintMessageEventArgs.
	/// </summary>
	public class ZipDLLPrintMessageEventArgs
	{
		
		#region Private Vars

		private string m_PrintMessage = string.Empty;

		#endregion

		public ZipDLLPrintMessageEventArgs(string msg)
		{
			m_PrintMessage = msg;
		}

		#region Properties

		public string PrintMessage 
		{
			get {return m_PrintMessage;}
		}

		#endregion

	}
}
