#include "FileSystem.h"
#include "Container/AutoPtr.h"
#include "Container/String.h"
#include "Container/Vector.h"
#include "Container/WString.h"
#ifdef __ANDROID__
#	include <SDL_rwops.h>
#endif
#include <cstdio>
#include <cstring>
#include <sys/stat.h>

#ifdef _WIN32
#	ifndef _MSC_VER
#		define _WIN32_IE 0x501
#	endif
#	include <windows.h>
#	include <shellapi.h>
#	include <direct.h>
#	include <shlobj.h>
#	include <sys/types.h>
#	include <sys/utime.h>
#else
#	include <dirent.h>
#	include <cerrno>
#	include <unistd.h>
#	include <utime.h>
#	include <sys/wait.h>
#	define MAX_PATH 256
#endif

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif


namespace KhFileSystem
{

unsigned LastModifiedTime(const FString& fileName)
{
	if (fileName.IsEmpty())
		return 0;

#ifdef _WIN32
	struct _stat st;
	if (!_stat(fileName.CString(), &st))
		return (unsigned)st.st_mtime;
	else
		return 0;
#else
	struct stat st;
	if (!stat(fileName.CString(), &st))
		return (unsigned)st.st_mtime;
	else
		return 0;
#endif
}

bool SetLastModifiedTime(const FString& fileName, unsigned newTime)
{
	if (fileName.IsEmpty())
		return false;

#ifdef WIN32
	struct _stat oldTime;
	struct _utimbuf newTimes;
	if (_stat(fileName.CString(), &oldTime) != 0)
		return false;
	newTimes.actime = oldTime.st_atime;
	newTimes.modtime = newTime;
	return _utime(fileName.CString(), &newTimes) == 0;
#else
	struct stat oldTime;
	struct utimbuf newTimes;
	if (stat(fileName.CString(), &oldTime) != 0)
		return false;
	newTimes.actime = oldTime.st_atime;
	newTimes.modtime = newTime;
	return utime(fileName.CString(), &newTimes) == 0;
#endif
}

bool FileExists(const FString& fileName)
{
	FString fixedName = NativePath(RemoveTrailingSlash(fileName));

#ifdef _WIN32
	DWORD attributes = GetFileAttributesW(FWString(fixedName).CString());
	if (attributes == INVALID_FILE_ATTRIBUTES || attributes & FILE_ATTRIBUTE_DIRECTORY)
		return false;
#else
	struct stat st;
	if (stat(fixedName.CString(), &st) || st.st_mode & S_IFDIR)
		return false;
#endif

	return true;
}

bool DirExists(const FString& pathName)
{
#ifndef WIN32
	// Always return true for the root directory
	if (pathName == "/")
		return true;
#endif

	FString fixedName = NativePath(RemoveTrailingSlash(pathName));

#ifdef _WIN32
	DWORD attributes = GetFileAttributesW(FWString(fixedName).CString());
	if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY))
		return false;
#else
	struct stat st;
	if (stat(fixedName.CString(), &st) || !(st.st_mode & S_IFDIR))
		return false;
#endif

	return true;
}

static void ScanDirInternal(TVector<FString>& result, FString path, const FString& startPath,
	const FString& filter, unsigned flags, bool recursive)
{
	path = AddTrailingSlash(path);
	FString deltaPath;
	if (path.Length() > startPath.Length())
		deltaPath = path.Substring(startPath.Length());

	FString filterExtension = filter.Substring(filter.Find('.'));
	if (filterExtension.Contains('*'))
		filterExtension.Clear();

#ifdef _WIN32
	WIN32_FIND_DATAW info;
	HANDLE _handle = FindFirstFileW(FWString(path + "*").CString(), &info);
	if (_handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			FString fileName(info.cFileName);
			if (!fileName.IsEmpty())
			{
				if (info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN && !(flags & SCAN_HIDDEN))
					continue;
				if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (flags & SCAN_DIRS)
						result.Push(deltaPath + fileName);
					if (recursive && fileName != "." && fileName != "..")
						ScanDirInternal(result, path + fileName, startPath, filter, flags, recursive);
}
				else if (flags & SCAN_FILES)
				{
					if (filterExtension.IsEmpty() || fileName.EndsWith(filterExtension))
						result.Push(deltaPath + fileName);
				}
}
	} while (FindNextFileW(_handle, &info));

	FindClose(_handle);
}
#else
	DIR *dir;
	struct dirent *de;
	struct stat st;
	dir = opendir(NativePath(path).CString());
	if (dir)
	{
		while ((de = readdir(dir)))
		{
			/// \todo Filename may be unnormalized Unicode on Mac OS X. Re-normalize as necessary
			FString fileName(de->d_name);
			bool normalEntry = fileName != "." && fileName != "..";
			if (normalEntry && !(_flags & SCAN_HIDDEN) && fileName.StartsWith("."))
				continue;
			FString pathAndName = path + fileName;
			if (!stat(pathAndName.CString(), &st))
			{
				if (st.st_mode & S_IFDIR)
				{
					if (_flags & SCAN_DIRS)
						result.Push(deltaPath + fileName);
					if (recursive && normalEntry)
						ScanDirInternal(result, path + fileName, startPath, _filter, _flags, recursive);
				}
				else if (_flags & SCAN_FILES)
				{
					if (filterExtension.IsEmpty() || fileName.EndsWith(filterExtension))
						result.Push(deltaPath + fileName);
				}
			}
		}
		closedir(dir);
	}
#endif
}

void ScanDir(TVector<FString>& result, const FString& pathName, const FString& filter, unsigned flags, bool recursive)
{
	FString initialPath = AddTrailingSlash(pathName);
	ScanDirInternal(result, initialPath, initialPath, filter, flags, recursive);
}

FString ExecutableDir()
{
	FString ret;

#if defined(_WIN32)
	wchar_t exeName[MAX_PATH];
	exeName[0] = 0;
	GetModuleFileNameW(0, exeName, MAX_PATH);
	ret = Path(FString(exeName));
#elif defined(__APPLE__)
	char exeName[MAX_PATH];
	memset(exeName, 0, MAX_PATH);
	unsigned _size = MAX_PATH;
	_NSGetExecutablePath(exeName, &_size);
	ret = Path(FString(exeName));
#elif defined(__linux__)
	char exeName[MAX_PATH];
	memset(exeName, 0, MAX_PATH);
	pid_t pid = getpid();
	FString link = "/proc/" + FString(pid) + "/exe";
	readlink(link.CString(), exeName, MAX_PATH);
	ret = Path(FString(exeName));
#endif

	// Sanitate /./ construct away
	ret.Replace("/./", "/");
	return ret;
}

void SplitPath(const FString& fullPath, FString& pathName, FString& fileName, FString& extension, bool lowercaseExtension)
{
	FString fullPathCopy = NormalizePath(fullPath);

	size_t extPos = fullPathCopy.FindLast('.');
	size_t pathPos = fullPathCopy.FindLast('/');

	if (extPos != FString::NPOS && (pathPos == FString::NPOS || extPos > pathPos))
	{
		extension = fullPathCopy.Substring(extPos);
		if (lowercaseExtension)
			extension = extension.ToLower();
		fullPathCopy = fullPathCopy.Substring(0, extPos);
	}
	else
		extension.Clear();

	pathPos = fullPathCopy.FindLast('/');
	if (pathPos != FString::NPOS)
	{
		fileName = fullPathCopy.Substring(pathPos + 1);
		pathName = fullPathCopy.Substring(0, pathPos + 1);
	}
	else
	{
		fileName = fullPathCopy;
		pathName.Clear();
	}
}

FString Path(const FString& fullPath)
{
	FString path, file, extension;
	SplitPath(fullPath, path, file, extension);
	return path;
}

FString FileName(const FString& fullPath)
{
	FString path, file, extension;
	SplitPath(fullPath, path, file, extension);
	return file;
}

FString Extension(const FString& fullPath, bool lowercaseExtension)
{
	FString path, file, extension;
	SplitPath(fullPath, path, file, extension, lowercaseExtension);
	return extension;
}

FString FileNameAndExtension(const FString& fileName, bool lowercaseExtension)
{
	FString path, file, extension;
	SplitPath(fileName, path, file, extension, lowercaseExtension);
	return file + extension;
}

FString ReplaceExtension(const FString& fullPath, const FString& newExtension)
{
	FString path, file, extension;
	SplitPath(fullPath, path, file, extension);
	return path + file + newExtension;
}

FString AddTrailingSlash(const FString& pathName)
{
	FString ret = pathName.Trimmed();
	ret.Replace('\\', '/');
	if (!ret.IsEmpty() && ret.Back() != '/')
		ret += '/';
	return ret;
}

FString RemoveTrailingSlash(const FString& pathName)
{
	FString ret = pathName.Trimmed();
	ret.Replace('\\', '/');
	if (!ret.IsEmpty() && ret.Back() == '/')
		ret.Resize(ret.Length() - 1);
	return ret;
}

FString ParentPath(const FString& path)
{
	size_t pos = RemoveTrailingSlash(path).FindLast('/');
	if (pos != FString::NPOS)
		return path.Substring(0, pos + 1);
	else
		return FString();
}

FString NormalizePath(const FString& pathName)
{
	return pathName.Replaced('\\', '/');
}

FString NativePath(const FString& pathName)
{
#ifdef _WIN32
	return pathName.Replaced('/', '\\');
#else
	return pathName;
#endif
}

FWString WideNativePath(const FString& pathName)
{
#ifdef _WIN32
	return FWString(pathName.Replaced('/', '\\'));
#else
	return FWString(pathName);
#endif
}

bool IsAbsolutePath(const FString& pathName)
{
	if (pathName.IsEmpty())
		return false;

	FString path = NormalizePath(pathName);

	if (path[0] == '/')
		return true;

#ifdef _WIN32
	if (path.Length() > 1 && IsAlpha(path[0]) && path[1] == ':')
		return true;
#endif

	return false;
}



void GetFiles(FString path, TVector<FString>& files, bool recursion)
{
	long   hFile = 0;
	struct _finddata_t fileinfo;
	if ((hFile = _findfirst(FString(path).Append("\\*").CString(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR) && recursion)
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					GetFiles(FString(path).Append("\\").Append(fileinfo.name), files);
			}
			else
			{
				files.Push(FString(path).Append("\\").Append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

bool Copyfile(const FString& srcFileName, const FString& destFileName)
{
	std::ifstream in(srcFileName.CString(), std::ios::binary);
	std::ofstream out(destFileName.CString(), std::ios::binary);
	if (!in) 
	{
		return false;
	}
	if (!out) 
	{
		return false;
	}
	out << in.rdbuf();
	in.close();
	out.close();

	return true;
}

bool CreateDir(const FString& pathName)
{
#ifdef _WIN32
	bool success = (CreateDirectoryW(WideNativePath(RemoveTrailingSlash(pathName)).CString(), 0) == TRUE) ||
		(GetLastError() == ERROR_ALREADY_EXISTS);
#else
	bool success = mkdir(NativePath(RemoveTrailingSlash(pathName)).CString(), S_IRWXU) == 0 || errno == EEXIST;
#endif

	return success;
}

bool Delete(const FString& fileName)
{
#ifdef _WIN32
	return DeleteFileW(WideNativePath(fileName).CString()) != 0;
#else
	return remove(NativePath(fileName).CString()) == 0;
#endif
}

bool Rename(const FString& srcFileName, const FString& destFileName)
{
#ifdef _WIN32
	return MoveFileW(WideNativePath(srcFileName).CString(), WideNativePath(destFileName).CString()) != 0;
#else
	return rename(NativePath(srcFileName).CString(), NativePath(destFileName).CString()) == 0;
#endif
}

}