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

#region KNOWN ISSUES	
	//_____________________________________________________________________________
	//
	//KNOWN ISSUES:
	//From my testing I have encountered at least one issue (though there may be more)
	//
	//1.	I receive an error code from the Unzip32.dll when I try to set a password, which requires
	//		setting the fEncrypt parameter of the ZPOPT structure to 1.  In this implementation this
	//		is setting the Encrypt property to EncryptionSpecifiedEnum.True
	//
	//
	//_____________________________________________________________________________
#endregion

using System;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Text;
using System.Windows.Forms;

namespace CSharpInfoZip_ZipSample
{

	#region Public Enums

	public enum TempDirectorySpecifiedEnum {False, True};		// 1 If Temp dir Wanted, Else 0
	public enum EncryptionSpecifiedEnum {False, True};				// 1 If Encryption Wanted, Else 0
	public enum SystemFilesIgnoredEnum {True, False};			// 1 To Include System/Hidden Files, Else 0
	public enum VolumeLabelStoredEnum {False, True};				// 1 If Storing Volume Label, Else 0
	public enum ExtraAttributesExcludedEnum {False, True};		// 1 If Excluding Extra Attributes, Else 0
	public enum DirectoryEntriesIgnoredEnum {False, True};		// 1 If Ignoring Directory Entries, Else 0
	public enum ExcludeDateEnum {False, True};						// 1 If Excluding Files Earlier Than Specified Date, Else 0
	public enum IncludeDateEnum {False, True};						// 1 If Including Files Earlier Than Specified Date, Else 0
	public enum VerboseMessagesEnum {False, True};				// 1 If Full Messages Wanted, Else 0
	public enum QuietMessageseEnum {False, True};					// 1 If Minimum Messages Wanted, Else 0
	public enum CRLF_LFTranslateEnum {False, True};				// 1 If Translate CR/LF To LF, Else 0
	public enum LF_CRLFTranslateEnum {False, True};				// 1 If Translate LF To CR/LF, Else 0
	public enum JunkDirEnum {False, True};								// 1 If Junking Directory Names, Else 0
	public enum GrowZipFileEnum {False, True};						// 1 If Allow Appending To Zip File, Else 0
	public enum ForceDOSNamesEnum {False, True};					// 1 If Making Entries Using DOS File Names, Else 0
	public enum MoveEnum {False, True};									// 1 If Deleting Files Added Or Updated, Else 0
	public enum DeleteEntriesEnum {False, True};						// 1 If Files Passed Have To Be Deleted, Else 0
	public enum UpdateEnum {False, True};								// 1 If Updating Zip File-Overwrite Only If Newer, Else 0
	public enum FreshenEnum {False, True};								// 1 If Freshing Zip File-Overwrite Only, Else 0
	public enum JunkSFXPrefixEnum {False, True};					// 1 If Junking SFX Prefix, Else 0
	public enum LatestTimeEnum {False, True};							// 1 If Setting Zip File Time To Time Of Latest File In Archive, Else 0
	public enum CommentEnum {False, True};							// 1 If Putting Comment In Zip File, Else 0
	public enum OffsetsEnum {False, True};								// 1 If Updating Archive Offsets For SFX Files, Else 0
	public enum PrivilegeSaveEnum {False, True};						// 1 If Not Saving Privileges, Else 0
	public enum RecurseEnum {None, Level1, Level2};				// 1 (-r), 2 (-R) If Recursing Into Sub-Directories, Else 0
	public enum RepairEnum {None, Fix, DeepFix};					// 1 = Fix Archive, 2 = Try Harder To Fix, Else 0
	public enum CompressionLevelEnum {Level0 = 48, Level1,	// Compression Level - 0 = Stored 6 = Default 9 = Max
															Level2, Level3,			//Start at 48 because 0 - 9 in ASCII codes is 48 - 57
															Level4, Level5, 
															Level6, Level7, Level8, 
															Level9};					

	#endregion

	#region Event Delegates

	public delegate void ZipDLLPrintMessageEventHandler(object sender, ZipDLLPrintMessageEventArgs  e);
	public delegate void ZipDLLServiceMessageEventHandler(object sender, ZipDLLServiceMessageEventArgs  e);

	#endregion

	/// <summary>
	/// Summary description for Zip.
	/// </summary>
	public class Zip
	{

		#region Private Vars

		private string m_Password;
		private string m_Comment;
		private string [] m_FilesToZip;
		private string m_ZipFileName;
		private int m_Stop;

		private ZIPUSERFUNCTIONS m_zuf;
		private ZPOPT m_zopts = new ZPOPT();
		private ASCIIEncoding m_Ascii = new ASCIIEncoding();

		private ZDLLPrintCallback m_CallbackPrint;
		private ZDLLServiceCallback m_CallbackService;
		private ZDLLPasswordCallback m_CallbackPassword;
		private ZDLLCommentCallback m_CallbackComment;

		private string m_Date = string.Empty;											// US Date (8 Bytes Long) "12/31/98"?
		private string m_RootDir = string.Empty;										// Root Directory Pathname (Up To 256 Bytes Long)
		private string m_TempDir = string.Empty;									// Temp Directory Pathname (Up To 256 Bytes Long)
		private TempDirectorySpecifiedEnum m_Temp;							// 1 If Temp dir Wanted, Else 0
//		private int m_Suffix;																	// Include Suffixes (Not Yet Implemented!)
		private EncryptionSpecifiedEnum m_EncryptionSpecified;				// 1 If Encryption Wanted, Else 0
		private SystemFilesIgnoredEnum m_SystemFilesIgnored;				// 1 To Include System/Hidden Files, Else 0
		private VolumeLabelStoredEnum m_VolumeLabelStored;				// 1 If Storing Volume Label, Else 0
		private ExtraAttributesExcludedEnum m_ExtraAttributesExcluded;	// 1 If Excluding Extra Attributes, Else 0
		private DirectoryEntriesIgnoredEnum m_DirEntriesIgnored;			// 1 If Ignoring Directory Entries, Else 0
		private ExcludeDateEnum m_ExcludeDate;									// 1 If Excluding Files Earlier Than Specified Date, Else 0
		private IncludeDateEnum m_IncludeDate;									// 1 If Including Files Earlier Than Specified Date, Else 0
		private VerboseMessagesEnum m_VerboseMessages;					// 1 If Full Messages Wanted, Else 0
		private QuietMessageseEnum m_QuietMessages;							// 1 If Minimum Messages Wanted, Else 0
		private CRLF_LFTranslateEnum m_CRLF_LFTranslate;					// 1 If Translate CR/LF To LF, Else 0
		private LF_CRLFTranslateEnum m_LF_CRLFTranslate;					// 1 If Translate LF To CR/LF, Else 0
		private JunkDirEnum m_JunkDir;													// 1 If Junking Directory Names, Else 0
		private GrowZipFileEnum m_GrowZipFile;										// 1 If Allow Appending To Zip File, Else 0
		private ForceDOSNamesEnum m_ForceDOSNames;						// 1 If Making Entries Using DOS File Names, Else 0
		private MoveEnum m_Move;														// 1 If Deleting Files Added Or Updated, Else 0
		private DeleteEntriesEnum m_DeleteEntries;									// 1 If Files Passed Have To Be Deleted, Else 0
		private UpdateEnum m_Update;													// 1 If Updating Zip File-Overwrite Only If Newer, Else 0
		private FreshenEnum m_Freshen;												// 1 If Freshing Zip File-Overwrite Only, Else 0
		private JunkSFXPrefixEnum m_JunkSFXPrefix;								// 1 If Junking SFX Prefix, Else 0
		private LatestTimeEnum m_LatestTime;										// 1 If Setting Zip File Time To Time Of Latest File In Archive, Else 0
		private CommentEnum m_CommentOption;									// 1 If Putting Comment In Zip File, Else 0
		private OffsetsEnum m_Offsets;													// 1 If Updating Archive Offsets For SFX Files, Else 0
		private PrivilegeSaveEnum m_PrivilegeSave;								// 1 If Not Saving Privileges, Else 0
//		private int m_Encryption;															// Read Only Property!!!
		private RecurseEnum m_Recurse;												// 1 (-r), 2 (-R) If Recursing Into Sub-Directories, Else 0
		private RepairEnum m_Repair;													// 1 = Fix Archive, 2 = Try Harder To Fix, Else 0
		private CompressionLevelEnum m_CompressionLevel;					// Compression Level - 0 = Stored 6 = Default 9 = Max

		#endregion

		public Zip()
		{
		}

		#region Properties
		
		public string Password
		{
			get {return m_Password;}
			set {m_Password = value;}
		}

		public string Comment
		{
			get {return m_Comment;}
			set {m_Comment = value;}
		}

		public string [] FilesToZip
		{
			get {return m_FilesToZip;}
			set {m_FilesToZip = value;	}
		}

		public string ZipFileName
		{
			get {return m_ZipFileName;}
			set {m_ZipFileName = value;}
		}

		public string Date
		{
			get {return m_Date;}
			set {m_Date = value;}
		}

		public string RootDir
		{
			get {return m_RootDir;}
			set {m_RootDir = value;}
		}
		
		public string TempDir
		{
			get {return m_TempDir;}
			set {m_TempDir = value;}
		}

		public TempDirectorySpecifiedEnum Temp
		{
			get {return m_Temp;}
			set {m_Temp = value;}
		}

//		public int Suffix
//		{
//			get {return m_Suffix;}
//			set {m_Suffix = value;}
//		}

		public EncryptionSpecifiedEnum Encrypt
		{
			get {return m_EncryptionSpecified;}
			set {m_EncryptionSpecified = value;}
		}

		public SystemFilesIgnoredEnum System
		{
			get {return m_SystemFilesIgnored;}
			set {m_SystemFilesIgnored = value;}
		}

		public VolumeLabelStoredEnum Volume
		{
			get {return m_VolumeLabelStored;}
			set {m_VolumeLabelStored = value;}
		}

		public ExtraAttributesExcludedEnum Extra
		{
			get {return m_ExtraAttributesExcluded;}
			set {m_ExtraAttributesExcluded = value;}
		}

		public DirectoryEntriesIgnoredEnum NoDirEntries
		{
			get {return m_DirEntriesIgnored;}
			set {m_DirEntriesIgnored = value;}
		}

		public ExcludeDateEnum ExcludeDate
		{
			get {return m_ExcludeDate;}
			set {m_ExcludeDate = value;}
		}

		public IncludeDateEnum IncludeDate
		{
			get {return m_IncludeDate;}
			set {m_IncludeDate = value;}
		}

		public VerboseMessagesEnum Verbose
		{
			get {return m_VerboseMessages;}
			set {m_VerboseMessages = value;}
		}

		public QuietMessageseEnum Quiet
		{
			get {return m_QuietMessages;}
			set {m_QuietMessages = value;}
		}

		public CRLF_LFTranslateEnum CRLF_LF
		{
			get {return m_CRLF_LFTranslate;}
			set {m_CRLF_LFTranslate = value;}
		}

		public LF_CRLFTranslateEnum LF_CRLF
		{
			get {return m_LF_CRLFTranslate;}
			set {m_LF_CRLFTranslate = value;}
		}

		public JunkDirEnum JunkDir
		{
			get {return m_JunkDir;}
			set {m_JunkDir = value;}
		}

		public GrowZipFileEnum Grow
		{
			get {return m_GrowZipFile;}
			set {m_GrowZipFile = value;}
		}

		public ForceDOSNamesEnum Force
		{
			get {return m_ForceDOSNames;}
			set {m_ForceDOSNames = value;}
		}

		public MoveEnum Move
		{
			get {return m_Move;}
			set {m_Move = value;}
		}

		public DeleteEntriesEnum DeleteEntries
		{
			get {return m_DeleteEntries;}
			set {m_DeleteEntries = value;}
		}

		public UpdateEnum Update
		{
			get {return m_Update;}
			set {m_Update = value;}
		}

		public FreshenEnum Freshen
		{
			get {return m_Freshen;}
			set {m_Freshen = value;}
		}

		public JunkSFXPrefixEnum JunkSFX
		{
			get {return m_JunkSFXPrefix;}
			set {m_JunkSFXPrefix = value;}
		}

		public LatestTimeEnum LatestTime
		{
			get {return m_LatestTime;}
			set {m_LatestTime = value;}
		}

		public CommentEnum CommentOption
		{
			get {return m_CommentOption;}
			set {m_CommentOption = value;}
		}

		public OffsetsEnum Offsets
		{
			get {return m_Offsets;}
			set {m_Offsets = value;}
		}

		public PrivilegeSaveEnum Privilege
		{
			get {return m_PrivilegeSave;}
			set {m_PrivilegeSave = value;}
		}

//		public int Encryption
//		{
//			get {return m_Encryption;}
//			set {m_Encryption = value;}
//		}

		public RecurseEnum Recurse
		{
			get {return m_Recurse;}
			set {m_Recurse = value;}
		}

		public RepairEnum Repair
		{
			get {return m_Repair;}
			set {m_Repair = value;}
		}

		public CompressionLevelEnum Level
		{
			get {return m_CompressionLevel;}
			set {m_CompressionLevel = value;}
		}


		#endregion

		#region Structures

		//ZPOPT Is Used To Set The Options In The ZIP32.DLL
		[ StructLayout( LayoutKind.Sequential )]
		protected struct ZPOPT
		{
			[MarshalAs(UnmanagedType.LPTStr)]
			public string Date;					// US Date (8 Bytes Long) "12/31/98"?
			[MarshalAs(UnmanagedType.LPTStr)]
			public string szRootDir;			// Root Directory Pathname (Up To 256 Bytes Long)
			[MarshalAs(UnmanagedType.LPTStr)]
			public string szTempDir;			// Temp Directory Pathname (Up To 256 Bytes Long)
			public int fTemp;					// 1 If Temp dir Wanted, Else 0
			public int fSuffix;					// Include Suffixes (Not Yet Implemented!)
			public int fEncrypt;					// 1 If Encryption Wanted, Else 0
			public int fSystem;					// 1 To Include System/Hidden Files, Else 0
			public int fVolume;					// 1 If Storing Volume Label, Else 0
			public int fExtra;						// 1 If Excluding Extra Attributes, Else 0
			public int fNoDirEntries;			// 1 If Ignoring Directory Entries, Else 0
			public int fExcludeDate;			// 1 If Excluding Files Earlier Than Specified Date, Else 0
			public int fIncludeDate;			// 1 If Including Files Earlier Than Specified Date, Else 0
			public int fVerbose;					// 1 If Full Messages Wanted, Else 0
			public int fQuiet;						// 1 If Minimum Messages Wanted, Else 0
			public int fCRLF_LF;				// 1 If Translate CR/LF To LF, Else 0
			public int fLF_CRLF;				// 1 If Translate LF To CR/LF, Else 0
			public int fJunkDir;					// 1 If Junking Directory Names, Else 0
			public int fGrow;						// 1 If Allow Appending To Zip File, Else 0
			public int fForce;						// 1 If Making Entries Using DOS File Names, Else 0
			public int fMove;						// 1 If Deleting Files Added Or Updated, Else 0
			public int fDeleteEntries;			// 1 If Files Passed Have To Be Deleted, Else 0
			public int fUpdate;					// 1 If Updating Zip File-Overwrite Only If Newer, Else 0
			public int fFreshen;					// 1 If Freshing Zip File-Overwrite Only, Else 0
			public int fJunkSFX;				// 1 If Junking SFX Prefix, Else 0
			public int fLatestTime;				// 1 If Setting Zip File Time To Time Of Latest File In Archive, Else 0
			public int fComment;				// 1 If Putting Comment In Zip File, Else 0
			public int fOffsets;					// 1 If Updating Archive Offsets For SFX Files, Else 0
			public int fPrivilege;				// 1 If Not Saving Privileges, Else 0
			public int fEncryption;				// Read Only Property!!!
			public int fRecurse;					// 1 (-r), 2 (-R) If Recursing Into Sub-Directories, Else 0
			public int fRepair;					// 1 = Fix Archive, 2 = Try Harder To Fix, Else 0
			public byte flevel;					// Compression Level - 0 = Stored 6 = Default 9 = Max
		}

		//CallBack string.  This is a byte array 4K long
		protected struct ZipCBChar
		{
			[MarshalAs(UnmanagedType.ByValArray, SizeConst= 4096, ArraySubType = UnmanagedType.U1)]
			public byte [] ch;
		}


		//This Structure Is Used For The ZIP32.DLL Function Callbacks
		[ StructLayout( LayoutKind.Sequential )]
		protected struct ZIPUSERFUNCTIONS
		{
			public ZDLLPrintCallback ZDLLPRNT;
			public ZDLLCommentCallback ZDLLCOMMENT;
			public ZDLLPasswordCallback ZDLLPASSWORD;
			public ZDLLServiceCallback ZDLLSERVICE;
		}

		#endregion

		#region DLL Function Declares

		//NOTE:
		//All the following Declares assumes ZIP32.DLL Is In Your \Windows\System Directory!

		//set zip callbacks
		[DllImport("zip32.dll", SetLastError=true)]
		protected static extern int ZpInit(ref ZIPUSERFUNCTIONS zuf);

		//set zip options
		[DllImport("zip32.dll", SetLastError=true)]
		protected static extern int ZpSetOptions(ref ZPOPT zopts);

		//get zip options
		[DllImport("zip32.dll", SetLastError=true)]
		protected static extern  ZPOPT ZpGetOptions();

		//zip archive
		[DllImport("zip32.dll", SetLastError=true)]
		protected static extern int ZpArchive(int argc, string funame, string [] zipnames);
		

		#endregion

		#region Delegates

		protected delegate int ZDLLPrintCallback (ref ZipCBChar m, uint x);
		protected delegate int ZDLLServiceCallback (ref ZipCBChar m, uint x);
		protected delegate short ZDLLPasswordCallback (ref ZipCBChar p, int n, ref ZipCBChar m, ref ZipCBChar name);
		protected delegate short ZDLLCommentCallback (ref ZipCBChar m);

		#endregion

		#region Events
			
		public event ZipDLLPrintMessageEventHandler ReceivePrintMessage;
		public event ZipDLLServiceMessageEventHandler ReceiveServiceMessage;

		#endregion

		#region Protected Functions

		protected virtual void OnReceivePrintMessage (ZipDLLPrintMessageEventArgs e)
		{
			if (ReceivePrintMessage != null) 
			{
				ReceivePrintMessage(this, e); 
			}
		}

		protected virtual void OnReceiveServiceMessage (ZipDLLServiceMessageEventArgs e)
		{
			if (ReceiveServiceMessage != null) 
			{
				ReceiveServiceMessage(this, e); 
			}
		}

		#endregion

		#region Callback Functions

		//The zip32.dll passes activity messages to the declared byte array.  Decode
		//the byte array to see the message
		protected int ZDLLPrint (ref ZipCBChar msg, uint x)
		{
			string s = string.Empty;

			if (msg.ch[0] == 0) return 0;
			s = m_Ascii.GetString(msg.ch,0, (int)x);

			//Raise this event
			ZipDLLPrintMessageEventArgs e = new ZipDLLPrintMessageEventArgs(s);
			OnReceivePrintMessage(e);

			return 0;
		}

		/*	 
		DLLSERVICE *ServCallBk  = Callback function designed to be used for
                          allowing the application to process Windows messages,
                          or canceling the operation, as well as giving the
                          option of a progress indicator. If this function
                          returns a non-zero value, then it will terminate
                          what it is doing. It provides the application with
                          the name of the name of the archive member it has
                          just processed, as well as it's original size.
						  
		msg.ch = the name of the file being zipped
		x = The size of the file being zipped
		
		 * */
		protected int ZDLLService (ref ZipCBChar msg, uint x)
		{

			string s = string.Empty;
			int i = 0;

			for (i = 0; i <= msg.ch.Length; i ++)
				if (msg.ch[i] == 0) break;
			 s = m_Ascii.GetString(msg.ch,0,i);

			//Raise this event
			ZipDLLServiceMessageEventArgs e = new ZipDLLServiceMessageEventArgs(s, (int)x);
			OnReceiveServiceMessage (e);

			return m_Stop;
		}

		protected short ZDLLPassword (ref ZipCBChar p, int n, ref ZipCBChar m, ref ZipCBChar name)
		{
			if (m_Password == null | m_Password == string.Empty) return 1;

			//clear the byte array
			for (int i = 0; i <= n-1; i ++)
				p.ch[i] = 0;

			m_Ascii.GetBytes(m_Password, 0, m_Password.Length, p.ch, 0);
			
			return 0;
		}
		
		protected short ZDLLComment (ref ZipCBChar msg)
		{
			if (m_Comment == null | m_Comment == string.Empty) return 1;

			//clear the byte array
			for (int i = 0; i <= msg.ch.Length-1; i ++)
				msg.ch[i] = 0;

			//set the comment in the zip file
			m_Ascii.GetBytes(m_Comment, 0, m_Comment.Length, msg.ch, 0);

			return 0;
		}

		#endregion

		#region Public Functions

		public int ZipFiles()
		{
			//check to see if there is enough information to proceed.
			//Exceptions can be thrown if required data is not passed in
			if (m_FilesToZip.Length == 0) return -1;
			if (m_ZipFileName == string.Empty) return -1;

			m_zuf = new ZIPUSERFUNCTIONS();

			//set up the callback delegates
			m_CallbackPrint = new ZDLLPrintCallback(ZDLLPrint);
			m_CallbackService = new ZDLLServiceCallback(ZDLLService);
			m_CallbackPassword = new ZDLLPasswordCallback(ZDLLPassword);
			m_CallbackComment = new ZDLLCommentCallback(ZDLLComment);

			//set up the callback structure
			m_zuf.ZDLLPRNT = m_CallbackPrint;
			m_zuf.ZDLLSERVICE = m_CallbackService;
			m_zuf.ZDLLPASSWORD = m_CallbackPassword;
			m_zuf.ZDLLCOMMENT = m_CallbackComment;
			
			//init the zip process
			int ret = ZpInit(ref m_zuf);

			//set the zip options
			ZPOPT zopt = CreateZPOPTOptions();
			ret = ZpSetOptions(ref zopt);

			//zip the files
			try
			{
				ret = ZpArchive(m_FilesToZip.Length, m_ZipFileName, m_FilesToZip);
			}
			catch(Exception e)
			{
				MessageBox.Show (e.ToString() + "\r\n" + "Last Win32ErrorCode: " + Marshal.GetLastWin32Error());
				//You can check the meaning of return codes here:
				//http://msdn.microsoft.com/library/default.asp?url=/library/en-us/debug/base/system_error_codes__0-499_.asp
			}
			return ret;
		}

		public void Stop ()
		{
			//m_Stop gets returned from the DLLService callback.
			//A value of 1 means abort processing.
			m_Stop = 1;
			Application.DoEvents();
		}

		#endregion

		#region Private Functions

		private ZPOPT CreateZPOPTOptions()
		{
			ZPOPT zopt = new ZPOPT();
			
			zopt.Date = m_Date;
			zopt.szRootDir = m_RootDir;
			zopt.szTempDir = m_TempDir;
			zopt.fTemp = (int)m_Temp;
			zopt.fSuffix = 0;
			zopt.fEncrypt = (int)m_EncryptionSpecified;
			zopt.fSystem = (int)m_SystemFilesIgnored;
			zopt.fVolume = (int)m_VolumeLabelStored;
			zopt.fExtra = (int)m_ExtraAttributesExcluded;
			zopt.fNoDirEntries = (int)m_DirEntriesIgnored;
			zopt.fExcludeDate = (int)m_ExcludeDate;
			zopt.fIncludeDate = (int)m_IncludeDate;
			zopt.fVerbose = (int)m_VerboseMessages;
			zopt.fQuiet = (int)m_QuietMessages;
			zopt.fCRLF_LF = (int)m_CRLF_LFTranslate;
			zopt.fLF_CRLF = (int)m_LF_CRLFTranslate;
			zopt.fJunkDir = (int)m_JunkDir;
			zopt.fGrow = (int)m_GrowZipFile;
			zopt.fForce = (int)m_ForceDOSNames;
			zopt.fMove = (int)m_Move;
			zopt.fDeleteEntries = (int)m_DeleteEntries;
			zopt.fUpdate = (int)m_Update;
			zopt.fFreshen = (int)m_Freshen;
			zopt.fJunkSFX = (int)m_JunkSFXPrefix;
			zopt.fLatestTime = (int)m_LatestTime;
			zopt.fComment = (int)m_CommentOption;
			zopt.fOffsets = (int)m_Offsets;
			zopt.fPrivilege = (int)m_PrivilegeSave;
			zopt.fEncryption = 0;
			zopt.fRecurse = (int)m_Recurse;
			zopt.fRepair = (int)m_Repair;
			zopt.flevel = (byte)m_CompressionLevel;

			return zopt;
		}

		#endregion
	}
}
