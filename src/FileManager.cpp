/*
 * FileManager.cpp
 *
 * Created: 06/11/2015 10:52:13
 *  Author: David
 */ 

#include "FileManager.hpp"
#include "PanelDue.hpp"
#include "Hardware/SerialIo.hpp"
#include <cctype>

#undef min
#undef max
#undef array
#undef result
#include <algorithm>
#define array _ecv_array
#define result _ecv_result

namespace FileManager
{
	typedef Vector<char, 2048> FileList;						// we use a Vector instead of a String because we store multiple null-terminated strings in it
	typedef Vector<const char* array, 100> FileListIndex;

	const char * array filesRoot = "0:/gcodes";
	const char * array macrosRoot = "0:/macros";
	const uint32_t FileListRequestTimeout = 8000;				// file info request timeout in milliseconds

	static FileList fileLists[3];								// one for gcode file list, one for macro list, one for receiving new lists into
	static FileListIndex fileIndices[3];						// pointers into the individual filenames in the list

	static int newFileList = -1;								// which file list we received a new listing into
	static int errorCode;
	static Path fileDirectoryName;
	static FileSet gcodeFilesList(evFile, evFilesUp, filesRoot, true);
	static FileSet macroFilesList(evMacro, evMacrosUp, macrosRoot, false);
	static FileSet * null displayedFileSet = nullptr;
	static uint8_t numVolumes = 1;								// how many SD card sockets we have (normally 1 or 2)

	// Return true if the second string is alphabetically greater then the first, case insensitive
	static bool StringGreaterThan(const char* a, const char* b)
	{
		return strcasecmp(a, b) > 0;
	}

	FileSet::FileSet(Event fe, Event fu, const char * array rootDir, bool pIsFilesList)
		: requestedPath(rootDir), currentPath(), timer(FileListRequestTimeout, "M20 S2 P", requestedPath.c_str()), which(-1), fileEvent(fe), upEvent(fu), scrollOffset(0),
		  isFilesList(pIsFilesList), cardNumber(0)
	{
	}

	void FileSet::Display()
	{
		RefreshPopup();
		filePopupTitleField->SetValue(cardNumber);
		filePopupTitleField->Show(isFilesList);
		macroPopupTitleField->Show(!isFilesList);
		changeCardButton->Show(isFilesList && numVolumes > 1);
		filesUpButton->SetEvent(upEvent, nullptr);
		mgr.SetPopup(fileListPopup, fileListPopupX, fileListPopupY);
		timer.SetPending();										// refresh the list of files
	}

	void FileSet::Reload(int whichList, const Path& dir, int errCode)
	{
		if (errCode == 0)
		{
			SetIndex(whichList);
			SetPath(dir.c_str());
			mgr.Show(fileListErrorField, false);
			RefreshPopup();
		}
		else
		{
			SetIndex(-1);
			RefreshPopup();
			fileListErrorField->SetValue(errCode);
			mgr.Show(fileListErrorField, true);
		}
		StopTimer();
	}

	// Refresh the list of files or macros in the Files popup window
	void FileSet::RefreshPopup()
	{
		if (which >= 0)
		{
			FileListIndex& fileIndex = fileIndices[which];
		
			// 1. Sort the file list
			fileIndex.sort(StringGreaterThan);

			// 2. Make sure the scroll position is still sensible
			if (scrollOffset < 0)
			{
				scrollOffset = 0;
			}
			else if ((unsigned int)scrollOffset >= fileIndex.size())
			{
				scrollOffset = ((fileIndex.size() - 1)/numFileRows) * numFileRows;
			}
		
			// 3. Display the scroll buttons if needed
			mgr.Show(scrollFilesLeftButton, scrollOffset != 0);
			mgr.Show(scrollFilesRightButton, scrollOffset + (numFileRows * numFileColumns) < fileIndex.size());
			mgr.Show(filesUpButton, IsInSubdir());
		
			// 4. Display the file list
			for (size_t i = 0; i < numDisplayedFiles; ++i)
			{
				TextButton *f = filenameButtons[i];
				if (i + scrollOffset < fileIndex.size())
				{
					const char *text = fileIndex[i + scrollOffset];
					f->SetText(text);
					f->SetEvent(fileEvent, text);
					mgr.Show(f, true);
				}
				else
				{
					f->SetText("");
					mgr.Show(f, false);
				}
			}
			displayedFileSet = this;
		}
		else
		{
			mgr.Show(scrollFilesLeftButton, false);
			mgr.Show(scrollFilesRightButton, false);
			for (size_t i = 0; i < numDisplayedFiles; ++i)
			{
				mgr.Show(filenameButtons[i], false);
			}
		}
	}
	
	void FileSet::Scroll(int amount)
	{
		scrollOffset += amount;
		RefreshPopup();
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
		which = -1;
		RefreshPopup();									// this hides the file list until we receive a new one

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
		for (size_t i = 0; i < end && !requestedPath.full(); ++i)
		{
			requestedPath.add(currentPath[i]);
		}
		timer.SetPending();
	}

	// Build a subdirectory of the current path
	void FileSet::RequestSubdir(const char * array dir)
	{
		which = -1;
		RefreshPopup();									// this hides the file list until we receive a new one

		requestedPath.copy(currentPath);
		if (requestedPath.size() == 0 || (requestedPath[requestedPath.size() - 1] != '/' && !requestedPath.full()))
		{
			requestedPath.add('/');
		}
		requestedPath.catFrom(dir);
		timer.SetPending();
	}
	
	void FileSet::ChangeCard()
	{
		if (isFilesList && numVolumes > 1)
		{
			++cardNumber;
			if (cardNumber >= numVolumes)
			{
				cardNumber = 0;
			}

			filePopupTitleField->SetValue(cardNumber);		// update the card number on the display
			which = -1;
			RefreshPopup();									// this hides the file list until we receive a new one

			if (cardNumber == 0)
			{
				requestedPath.copy("0:/gcodes");
			}
			else
			{
				// Send a command to mount the removable card. RepRapFirmware will ignore it if the card is already mounted and there are any files open on it.
				SerialIo::SendString("M21 P");
				SerialIo::SendInt(cardNumber);
				SerialIo::SendChar('\n');
				requestedPath.printf("%u:", (unsigned int)cardNumber);
			}
			timer.SetPending();

		}
	}
	
	void BeginNewMessage()
	{
		fileDirectoryName.clear();
		errorCode = 0;
		newFileList = -1;
	}

	void EndReceivedMessage(bool displayingFileInfo)
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
			while (i < fileDirectoryName.size() && fileDirectoryName[i] != '/' && !temp.full())
			{
				temp.add(fileDirectoryName[i++]);
			}
			
			if (card0 && temp.equalsIgnoreCase("macros"))
			{
				macroFilesList.Reload(newFileList, fileDirectoryName, errorCode);
			}
			else if (!displayingFileInfo)
			{
				gcodeFilesList.Reload(newFileList, fileDirectoryName, errorCode);
			}
			newFileList = -1;
		}
	}

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

	void ReceiveDirectoryName(const char * array data)
	{
		fileDirectoryName.copy(data);
	}
	
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

	void Scroll(int amount)
	{
		if (displayedFileSet != nullptr)
		{
			displayedFileSet->Scroll(amount);
		}
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

	void RequestFilesRootDir()
	{
		gcodeFilesList.RequestRootDir();
	}

	void RequestMacrosRootDir()
	{
		macroFilesList.RequestRootDir();
	}

	const char * array GetFilesDir()
	{
		return gcodeFilesList.GetPath();
	}

	const char * array GetMacrosDir()
	{
		return macroFilesList.GetPath();
	}

	void RefreshFilesList()
	{
		gcodeFilesList.SetPending();
	}

	bool ProcessTimers()
	{
		bool done = macroFilesList.ProcessTimer();
		if (!done)
		{
			done = gcodeFilesList.ProcessTimer();
		}
		return done;
	}

	void ChangeCard()
	{
		gcodeFilesList.ChangeCard();
	}
	
	void SetNumVolumes(unsigned int n)
	{
		if (n > 0 && n <= 10)
		{
			numVolumes = n;
		}
	}
}		// end namespace

// End
