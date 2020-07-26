#pragma once
#include "AutoConfig.h"
#include "Container/String.h"

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <io.h>
#include <iostream>

namespace KhFileSystem
{
/// Return files.
static const unsigned SCAN_FILES = 0x1;
/// Return directories.
static const unsigned SCAN_DIRS = 0x2;
/// Return also hidden files.
static const unsigned SCAN_HIDDEN = 0x4;

/// Return the file's last modified time as seconds since epoch, or 0 if can not be accessed.
unsigned LastModifiedTime(const FString& fileName);
/// Set the file's last modified time as seconds since epoch. Return true on success.
bool SetLastModifiedTime(const FString& fileName, unsigned newTime);
/// Check if a file exists.
bool FileExists(const FString& fileName);
/// Check if a directory exists.
bool DirExists(const FString& pathName);
/// Scan a directory for specified files.
void ScanDir(TVector<FString>& result, const FString& pathName, const FString& filter, unsigned flags = SCAN_FILES, bool recursive = false);
/// Return the executable's directory.
FString ExecutableDir();
/// Split a full path to path, filename and extension. The extension will be converted to lowercase by default.
void SplitPath(const FString& fullPath, FString& pathName, FString& fileName, FString& extension, bool lowercaseExtension = true);
/// Return the path from a full path.
FString Path(const FString& fullPath);
/// Return the filename from a full path.
FString FileName(const FString& fullPath);
/// Return the extension from a full path, converted to lowercase by default.
FString Extension(const FString& fullPath, bool lowercaseExtension = true);
/// Return the filename and extension from a full path. The case of the extension is preserved by default, so that the file can be opened in case-sensitive operating systems.
FString FileNameAndExtension(const FString& fullPath, bool lowercaseExtension = false);
/// Replace the extension of a file name with another.
FString ReplaceExtension(const FString& fullPath, const FString& newExtension);
/// Add a slash at the end of the path if missing and convert to normalized format (use slashes.)
FString AddTrailingSlash(const FString& pathName);
/// Remove the slash from the end of a path if exists and convert to normalized format (use slashes.)
FString RemoveTrailingSlash(const FString& pathName);
/// Return the parent path, or the path itself if not available.
FString ParentPath(const FString& pathName);
/// Convert a path to normalized (internal) format which uses slashes.
FString NormalizePath(const FString& pathName);
/// Convert a path to the format required by the operating system.
FString NativePath(const FString& pathName);
/// Convert a path to the format required by the operating system in wide characters.
FWString WideNativePath(const FString& pathName);
/// Return whether a path is absolute.
bool IsAbsolutePath(const FString& pathName);


/// Get path files.
void GetFiles(FString path, TVector<FString>& files, bool recursion = true);
/// Copy file.
bool Copyfile(const FString& srcFileName, const FString& destFileName);
/// Create a directory.
bool CreateDir(const FString& pathName);
/// Delete a file. Return true on success.
bool Delete(const FString& fileName);
/// Rename a file. Return true on success.
bool Rename(const FString& srcFileName, const FString& destFileName);

}