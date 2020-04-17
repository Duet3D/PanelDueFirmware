/*
 * FileManager.cpp
 *
 * Created: 06/11/2015 10:52:13
 *  Author: David
 */

#include "FileManager.hpp"
#include "PanelDue.hpp"
#include "UserInterfaceConstants.hpp"
#include "UserInterface.hpp"
#include "Hardware/SerialIo.hpp"
#include "Library/Misc.hpp"
#include <cctype>

#undef min
#undef max
#undef array
#undef result
#include <algorithm>
#define array _ecv_array
#define result _ecv_result

#if SAM4S
// We have 64Kb SRAM on the SAM4S4B so we can support larger file lists
constexpr size_t FileListSize = 4096;
constexpr size_t MaxFiles = 200;
#else
// We have only 32Kb or 48Kb SRAM on the SAM3S2B or SAM3S4B
constexpr size_t FileListSize = 2048;
constexpr size_t MaxFiles = 100;
#endif

namespace FileManager
{
	typedef Vector<char, FileListSize> FileList;				// we use a Vector instead of a String because we store multiple null-terminated strings in it
	typedef Vector<const char* array, MaxFiles> FileListIndex;

	const char * array filesRoot = "0:/gcodes";
	const char * array macrosRoot = "0:/macros";
	const uint32_t FileListRequestTimeout = 8000;				// file info request timeout in milliseconds

	static FileList fileLists[3];								// one for gcode file list, one for macro list, one for receiving new lists into
	static FileListIndex fileIndices[3];						// pointers into the individual filenames in the list

	static int newFileList = -1;								// which file list we received a new listing into
	static int errorCode;
	static Path fileDirectoryName;
	static FileSet gcodeFilesList(filesRoot, NumDisplayedFiles, true);
	static FileSet macroFilesList(macrosRoot, NumDisplayedMacros, false);
	static FileSet * null displayedFileSet = nullptr;
	static uint8_t numVolumes = 1;								// how many SD card sockets we have (normally 1 or 2)

	// Return true if the second string is alphabetically greater then the first, case insensitive
	static inline bool StringGreaterThan(const char* a, const char* b)
	{
		return strcasecmp(a, b) > 0;
	}

	FileSet::FileSet(const char * array rootDir, unsigned int numDisp, bool pIsFilesList)
		: numDisplayed(numDisp), requestedPath(rootDir), currentPath(), timer(FileListRequestTimeout, "", requestedPath.c_str()), whichList(-1), scrollOffset(0),
		  isFilesList(pIsFilesList), cardNumber(0)
	{
	}

	void FileSet::Display()
	{
		FileListUpdated();
		UI::DisplayFilesOrMacrosList(isFilesList, cardNumber, numVolumes);
		SetPending();							// refresh the list of files
	}

	void FileSet::Reload(int whichList, const Path& dir, int errCode)
	{
		UI::FileListLoaded(isFilesList, errCode);			// do this first to show/hide the error message
		if (errCode == 0)
		{
			SetIndex(whichList);
			SetPath(dir.c_str());
		}
		else
		{
			SetIndex(-1);
		}
		FileListUpdated();
		StopTimer();
	}

	// The macros list has just been updated. If this is for the root, update the short macro list, leaving out any subfolders.
	void FileSet::ReloadMacroShortList(int errorCode)
	{
		unsigned int buttonNum = 0;
		const FileListIndex& index = fileIndices[GetIndex()];
		unsigned int fileNum = (errorCode == 0) ? 0 : index.size();
		bool again;
		do
		{
			// Find the next non-directory
			while (fileNum < index.size() && index[fileNum][0] == '*')
			{
				++fileNum;
			}
			if (fileNum < index.size())
			{
				again = UI::UpdateMacroShortList(buttonNum, index[fileNum]);
				++fileNum;
			}
			else
			{
				again = UI::UpdateMacroShortList(buttonNum, nullptr);
			}
			++buttonNum;
		} while (again);
	}

	// Refresh the list of files or macros in the Files popup window
	void FileSet::FileListUpdated()
	{
		if (whichList >= 0)
		{
			FileListIndex& fileIndex = fileIndices[whichList];

			// 2. Make sure the scroll position is still sensible
			if (scrollOffset < 0 || fileIndex.size() == 0)
			{
				scrollOffset = 0;
			}
			else if ((unsigned int)scrollOffset >= fileIndex.size())
			{
				const unsigned int scrollAmount = UI::GetNumScrolledFiles(isFilesList);
				scrollOffset = ((fileIndex.size() - 1)/scrollAmount) * scrollAmount;
			}

			// 3. Display the scroll buttons if needed
			UI::EnableFileNavButtons(isFilesList, scrollOffset != 0, scrollOffset + numDisplayed < fileIndex.size(), IsInSubdir());

			// 4. Display the file list
			for (size_t i = 0; i < numDisplayed; ++i)
			{
				if (i + scrollOffset < fileIndex.size())
				{
					const char *text = fileIndex[i + scrollOffset];
					UI::UpdateFileButton(isFilesList, i, (isFilesList) ? text : SkipDigitsAndUnderscore(text), text);
				}
				else
				{
					UI::UpdateFileButton(isFilesList, i, nullptr, nullptr);
				}
			}
			displayedFileSet = this;
		}
		else
		{
			UI::EnableFileNavButtons(isFilesList, false, false, false);
			for (size_t i = 0; i < numDisplayed; ++i)
			{
				UI::UpdateFileButton(isFilesList, i, nullptr, nullptr);
			}
		}
	}

	void FileSet::Scroll(int amount)
	{
		scrollOffset += amount;
		FileListUpdated();
	}

	void FileSet::SetPath(const char * array pPath)
	{
		currentPath.copy(pPath);
	}

	// Return true if the path has more than one directory component on card 0, or at least one directory component on other cards
	bool FileSet::IsInSubdir() const
	{
		// Find the start of the first component of the path
		size_t start = (currentPath.size() >= 2 && isdigit(currentPath[0]) && currentPath[1] == ':') ? 2 : 0;
		if (currentPath[start] == '/')
		{
			++start;
		}
		size_t end = currentPath.size();
		if (end > start && currentPath[end - 1] == '/')
		{
			--end;			// remove trailing '/' if there is one
		}
		// If we are on card 0, then /gcodes or /macros is the effective root, so skip one path component
		if (cardNumber == 0)
		{
			while (end != 0 && currentPath[end] != '/')
			{
				--end;
			}
		}
		return (end > start);
	}

	// Request the parent path
	void FileSet::RequestParentDir()
	{
		whichList = -1;
		FileListUpdated();									// this hides the file list until we receive a new one

		size_t end = currentPath.size();
		// Skip any trailing '/'
		if (end != 0 && currentPath[end - 1] == '/')
		{
			--end;
		}
		// Find the last '/'
		while (end != 0)
		{
			--end;
			if (currentPath[end] == '/')
			{
				break;
			}
		}
		requestedPath.clear();
		for (size_t i = 0; i < end; ++i)
		{
			requestedPath.add(currentPath[i]);
		}
		SetPending();
	}

	// Build a subdirectory of the current path
	void FileSet::RequestSubdir(const char * array dir)
	{
		whichList = -1;
		FileListUpdated();									// this hides the file list until we receive a new one

		requestedPath.copy(currentPath);
		if (requestedPath.size() == 0 || (requestedPath[requestedPath.size() - 1] != '/'))
		{
			requestedPath.add('/');
		}
		requestedPath.cat(dir);
		SetPending();
	}

	// Use the timer to send a command and repeat it if no response is received
	void FileSet::SetPending()
	{
		timer.SetCommand(((GetFirmwareFeatures() & noM20M36) != 0) ? "M408 S20 P" : "M20 S2 P");
		timer.SetArgument(CondStripDrive(requestedPath.c_str()));
		timer.SetQuoteArgument(GetFirmwareFeatures() & quoteFilenames);
		timer.SetPending();
	}

	// Select the next SD card
	bool FileSet::NextCard()
	{
		if (isFilesList && numVolumes > 1)
		{
			unsigned int cn = cardNumber + 1;
			if (cn >= numVolumes)
			{
				cn = 0;
			}
			return SelectCard(cn);
		}
		return false;
	}

	// Select a particular SD card
	bool FileSet::SelectCard(unsigned int cardNum)
	{
		if (isFilesList && cardNum < numVolumes)
		{
			cardNumber = cardNum;
			UI::DisplayFilesOrMacrosList(isFilesList, cardNumber, numVolumes);
			whichList = -1;
			FileListUpdated();								// this hides the file list until we receive a new one

			if (cardNumber == 0)
			{
				SetupRootPath();
			}
			else
			{
				// Send a command to mount the removable card. RepRapFirmware will ignore it if the card is already mounted and there are any files open on it.
				SerialIo::SendString("M21 P");
				SerialIo::SendInt(cardNumber);
				SerialIo::SendChar('\n');
				requestedPath.printf("%u:", (unsigned int)cardNumber);
			}
			SetPending();
			return true;
		}
		return false;
	}

	void FileSet::SetupRootPath()
	{
		requestedPath.copy((cardNumber == 0 && (GetFirmwareFeatures() & noGcodesFolder) == 0) ? filesRoot : "0:/");
	}

	// This is called on the gcode files list when the firmware features are changed from the previous values
	void FileSet::FirmwareFeaturesChanged()
	{
		SetupRootPath();
	}

	// This is called when a new JSON response is received
	void BeginNewMessage()
	{
		fileDirectoryName.clear();
		errorCode = 0;
		newFileList = -1;
	}

	// This is called at the end of a JSON response
	void EndReceivedMessage()
	{
		if (newFileList >= 0)
		{
			// We received a new file list, which may be for the files or the macro list. Find out which.
			size_t i;
			bool card0;
			if (fileDirectoryName.size() >= 2 && isdigit(fileDirectoryName[0]) && fileDirectoryName[1] == ':')
			{
				i = 2;
				card0 = (fileDirectoryName[0] == '0');
			}
			else
			{
				i = 0;
				card0 = true;
			}
			if (fileDirectoryName[i] == '/')
			{
				++i;
			}
			String<10> temp;
			while (i < fileDirectoryName.size() && fileDirectoryName[i] != '/')
			{
				temp.add(fileDirectoryName[i++]);
			}

			fileIndices[newFileList].sort(StringGreaterThan);		// put the index in alphabetical order

			if (card0 && temp.equalsIgnoreCase("macros"))
			{
				macroFilesList.Reload(newFileList, fileDirectoryName, errorCode);
				if (i + 1 >= fileDirectoryName.size())				// if in root of /macros
				{
					macroFilesList.ReloadMacroShortList(errorCode);
				}
			}
			else if (!UI::IsDisplayingFileInfo())
			{
				gcodeFilesList.Reload(newFileList, fileDirectoryName, errorCode);
			}
			newFileList = -1;
		}
	}

	// This is called when we start receiving a list of files
	void BeginReceivingFiles()
	{
		// Find a free file list and index to receive the filenames into
		newFileList = 0;
		while (newFileList == gcodeFilesList.GetIndex() || newFileList == macroFilesList.GetIndex())
		{
			++newFileList;
		}

		_ecv_assert(0 <= newFileList && newFileList < 3);
		fileLists[newFileList].clear();
		fileIndices[newFileList].clear();
	}

	// This is called for each filename received
	void ReceiveFile(const char * array data)
	{
		if (newFileList >= 0)
		{
			FileList& fileList = fileLists[newFileList];
			FileListIndex& fileIndex = fileIndices[newFileList];
			size_t len = strlen(data) + 1;		// we are going to copy the null terminator as well
			if (len + fileList.size() < fileList.capacity() && fileIndex.size() < fileIndex.capacity())
			{
				fileIndex.add(fileList.c_ptr() + fileList.size());
				fileList.add(data, len);
			}
		}
	}

	// This is called when we receive the directory name
	void ReceiveDirectoryName(const char * array data)
	{
		fileDirectoryName.copy(data);
	}

	// This is called when we receive an error code
	void ReceiveErrorCode(int err)
	{
		if (newFileList >= 0)
		{
			// We have received a file list, so this error code relates to it
			errorCode = err;
		}
	}

	void DisplayFilesList()
	{
		gcodeFilesList.Display();
	}

	void DisplayMacrosList()
	{
		macroFilesList.Display();
	}

	void ScrollFiles(int amount)
	{
		gcodeFilesList.Scroll(amount);
	}

	void ScrollMacros(int amount)
	{
		macroFilesList.Scroll(amount);
	}

	void RequestFilesSubdir(const char * array dir)
	{
		gcodeFilesList.RequestSubdir(dir);
	}

	void RequestMacrosSubdir(const char * array dir)
	{
		macroFilesList.RequestSubdir(dir);
	}

	void RequestFilesParentDir()
	{
		gcodeFilesList.RequestParentDir();
	}

	void RequestMacrosParentDir()
	{
		macroFilesList.RequestParentDir();
	}

	const char * array GetFilesDir()
	{
		return gcodeFilesList.GetPath();
	}

	const char * array GetMacrosDir()
	{
		return macroFilesList.GetPath();
	}

	const char * array GetMacrosRootDir()
	{
		return macrosRoot;
	}

	void RefreshFilesList()
	{
		gcodeFilesList.SetPending();
	}

	void RefreshMacrosList()
	{
		macroFilesList.SetPending();
	}

	// This is called from the main loop to check for timer events
	bool ProcessTimers()
	{
		bool done = macroFilesList.ProcessTimer();
		if (!done)
		{
			done = gcodeFilesList.ProcessTimer();
		}
		return done;
	}

	bool NextCard()
	{
		return gcodeFilesList.NextCard();
	}

	bool SelectCard(unsigned int cardNum)
	{
		return gcodeFilesList.SelectCard(cardNum);
	}

	void SetNumVolumes(unsigned int n)
	{
		if (n > 0 && n <= 10)
		{
			numVolumes = n;
		}
	}

	// This is called when the host tells us its firmware type, and it is not the type we were previously assuming
	void FirmwareFeaturesChanged()
	{
		gcodeFilesList.FirmwareFeaturesChanged();
	}
}		// end namespace

// End
