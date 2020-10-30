/*
 * FileManager.h
 *
 * Created: 06/11/2015 10:52:38
 *  Author: David
 */ 

#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_

#include "Configuration.hpp"
#include "General/String.h"
#include "DisplaySize.hpp"
#include "RequestTimer.hpp"
#include "Events.hpp"
#include "FirmwareFeatures.hpp"

namespace FileManager
{
	const size_t maxPathLength = 100;
	typedef String<maxPathLength> Path;

	class FileSet
	{
	private:
		const unsigned numDisplayed;
		Path requestedPath;
		Path currentPath;
		RequestTimer timer;
		int whichList;
		int scrollOffset;
		bool IsInSubdir() const;
		const bool isFilesList;			// true for a file list, false for a macro list
		uint8_t cardNumber;

	public:
		FileSet(const char * _ecv_array rootDir, unsigned numDisp, bool pIsFilesList);
		void Display();
		void Reload(int whichList, const Path& dir, int errCode);
		void ReloadMacroShortList(int errorCode);
		void FileListUpdated();
		void Scroll(int amount);
		void SetIndex(int index) { whichList = index; }
		int GetIndex() const { return whichList; }
		void SetPath(const char * _ecv_array pPath);
		const char * _ecv_array GetPath() { return currentPath.c_str(); }
		void RequestParentDir()
			pre(IsInSubdir());
		void RequestSubdir(const char * _ecv_array dir);
		void SetPending();
		void StopTimer() { timer.Stop(); }
		bool ProcessTimer() { return timer.Process(); }
		bool NextCard();
		bool SelectCard(unsigned int cardNum);
		void FirmwareFeaturesChanged();

	private:
		void SetupRootPath();
	};

	void BeginNewMessage();
	void EndReceivedMessage();
	void BeginReceivingFiles();
	void ReceiveFile(const char * _ecv_array data);
	void ReceiveDirectoryName(const char * _ecv_array data);
	void ReceiveErrorCode(int err);

	void DisplayFilesList();
	void DisplayMacrosList();
	void ScrollFiles(int amount);
	void ScrollMacros(int amount);

	void RequestFilesSubdir(const char * _ecv_array dir);
	void RequestMacrosSubdir(const char * _ecv_array dir);
	void RequestFilesParentDir();
	void RequestMacrosParentDir();
	const char * _ecv_array GetFilesDir();
	const char * _ecv_array GetMacrosDir();
	const char * _ecv_array GetMacrosRootDir();

	void RefreshFilesList();
	void RefreshMacrosList();
	bool ProcessTimers();
	bool NextCard();
	bool SelectCard(unsigned int cardNum);
	void SetNumVolumes(size_t n);
	void FirmwareFeaturesChanged();
}

#endif /* FILEMANAGER_H_ */
