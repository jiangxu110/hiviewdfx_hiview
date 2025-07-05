/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "file_util.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <istream>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>
#include <vector>

#include "common_utils.h"
#include "directory_ex.h"
#include "file_ex.h"

namespace OHOS {
namespace HiviewDFX {
namespace FileUtil {
using namespace std;
bool LoadStringFromFile(const std::string& filePath, std::string& content)
{
    return OHOS::LoadStringFromFile(filePath, content);
}

bool LoadLinesFromFile(const std::string& filePath, std::vector<std::string>& lines)
{
    std::ifstream file(filePath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            lines.emplace_back(line);
        }
        file.close();
        return true;
    }
    return false;
}

bool LoadStringFromFd(int fd, std::string& content)
{
    return OHOS::LoadStringFromFd(fd, content);
}

bool SaveStringToFile(const std::string& filePath, const std::string& content, bool truncated)
{
    return OHOS::SaveStringToFile(filePath, content, truncated);
}

bool SaveStringToFd(int fd, const std::string& content)
{
    return OHOS::SaveStringToFd(fd, content);
}

bool LoadBufferFromFile(const std::string& filePath, std::vector<char>& content)
{
    return OHOS::LoadBufferFromFile(filePath, content);
}

bool SaveBufferToFile(const std::string& filePath, const std::vector<char>& content, bool truncated)
{
    return OHOS::SaveBufferToFile(filePath, content, truncated);
}

bool FileExists(const std::string& fileName)
{
    return OHOS::FileExists(fileName);
}

std::string ExtractFilePath(const std::string& fileFullName)
{
    return OHOS::ExtractFilePath(fileFullName);
}

std::string ExtractFileName(const std::string& fileFullName)
{
    return OHOS::ExtractFileName(fileFullName);
}

std::string ExtractFileExt(const std::string& fileName)
{
    return OHOS::ExtractFileExt(fileName);
}

std::string IncludeTrailingPathDelimiter(const std::string& path)
{
    return OHOS::IncludeTrailingPathDelimiter(path);
}

std::string ExcludeTrailingPathDelimiter(const std::string& path)
{
    return OHOS::ExcludeTrailingPathDelimiter(path);
}

void GetDirFiles(const std::string& path, std::vector<std::string>& files, bool isRecursive)
{
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }

    while (true) {
        struct dirent* ptr = readdir(dir);
        if (ptr == nullptr) {
            break;
        }

        // current dir or parent dir
        if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) {
            continue;
        } else if (ptr->d_type == DT_DIR && isRecursive) {
            std::string pathStringWithDelimiter = IncludeTrailingPathDelimiter(path) + string(ptr->d_name);
            GetDirFiles(pathStringWithDelimiter, files);
        } else {
            files.push_back(IncludeTrailingPathDelimiter(path) + string(ptr->d_name));
        }
    }
    closedir(dir);
}

void GetDirDirs(const std::string& path, std::vector<std::string>& dirs)
{
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        return;
    }

    while (true) {
        struct dirent *ptr = readdir(dir);
        if (ptr == nullptr) {
            break;
        }
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        } else if (ptr->d_type == DT_DIR) {
            dirs.push_back(IncludeTrailingPathDelimiter(path) + string(ptr->d_name));
        }
    }
    closedir(dir);
}

bool ForceCreateDirectory(const std::string& path)
{
    return OHOS::ForceCreateDirectory(path);
}

bool ForceCreateDirectory(const string& path, mode_t mode)
{
    string::size_type index = 0;
    do {
        index = path.find('/', index + 1);
        string subPath = (index == string::npos) ? path : path.substr(0, index);
        if (access(subPath.c_str(), F_OK) != 0) {
            if (mkdir(subPath.c_str(), mode) != 0) {
                return false;
            }
        }
    } while (index != string::npos);
    return access(path.c_str(), F_OK) == 0;
}

bool ForceRemoveDirectory(const std::string& path, bool isNeedDeleteGivenDirSelf)
{
    return OHOS::ForceRemoveDirectory(path);
}

uint64_t GetFileSize(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) ? 0 : static_cast<uint64_t>(st.st_size);
}

bool RemoveFile(const std::string& fileName)
{
    return OHOS::RemoveFile(fileName);
}

uint64_t GetFolderSize(const std::string& path)
{
    return OHOS::GetFolderSize(path);
}

// inner function, and param is legitimate
bool ChangeMode(const string& fileName, const mode_t& mode)
{
    return (chmod(fileName.c_str(), mode) == 0);
}

bool ChangeModeFile(const string& fileName, const mode_t& mode)
{
    if (access(fileName.c_str(), F_OK) != 0) {
        return false;
    }

    return ChangeMode(fileName, mode);
}

bool ChangeModeDirectory(const std::string& path, const mode_t& mode)
{
    return OHOS::ChangeModeDirectory(path, mode);
}

bool PathToRealPath(const std::string& path, std::string& realPath)
{
    return OHOS::PathToRealPath(path, realPath);
}

mode_t Umask(const mode_t& mode)
{
    return umask(mode);
}

int Open(const std::string& path, const int flags, const mode_t mode)
{
    return open(path.c_str(), flags, mode);
}

void CreateDirWithDefaultPerm(const std::string& path, uid_t aidRoot, uid_t aidSystem)
{
    FileUtil::ForceCreateDirectory(path);
    chown(path.c_str(), aidRoot, aidSystem);
}

void FormatPath2UnixStyle(std::string &path)
{
    // unimplemented
}

void RemoveFolderBeginWith(const std::string &path, const std::string &folderName)
{
    // unimplemented
}

bool WriteBufferToFd(int fd, const char* buffer, size_t size)
{
    if (fd < 0) {
        return false;
    }

    if (buffer == nullptr) {
        return false;
    }

    ssize_t writeSize = size;
    if (writeSize != TEMP_FAILURE_RETRY(write(fd, buffer, size))) {
        return false;
    }

    return true;
}

int CreateFile(const std::string &path, mode_t mode)
{
    if (FileExists(path)) {
        return 0;
    } else {
        std::ofstream fout(path);
        if (!fout.is_open()) {
            return -1;
        }
        fout.flush();
        fout.close();
        if (!ChangeMode(path, mode)) {
            return -1;
        }
    }
    return 0;
}

int CopyFile(const std::string &src, const std::string &des)
{
    std::ifstream fin(src, ios::binary);
    if (!fin.is_open()) {
        return -1;
    }

    std::ofstream fout(des, ios::binary);
    if (!fout.is_open()) {
        fin.close(); // 确保输入文件被关闭
        return -1;
    }

    try {
        fout << fin.rdbuf();
        if (fout.fail()) {
            fout.clear();
            fin.close();
            fout.close();
            return -1;
        }
        fout.flush();
        if (fout.fail()) {
            fin.close();
            fout.close();
            return -1;
        }
    } catch (const std::exception& e) {
        fin.close();
        fout.close();
        return -1;
    }

    fin.close();
    fout.close();
    return 0;
}

int CopyFileFast(const std::string &src, const std::string &des)
{
    int fdIn = open(src.c_str(), O_RDONLY);
    if (fdIn < 0) {
        return -1;
    }

    int fdOut = open(des.c_str(), O_CREAT | O_RDWR, 0664);
    if (fdOut < 0) {
        if (close(fdIn) != 0) {
            // 记录关闭失败，但不影响返回值
        }
        return -1;
    }

    struct stat st;
    if (stat(src.c_str(), &st) != 0) {
        if (close(fdIn) != 0) {
            // 记录关闭失败
        }
        if (close(fdOut) != 0) {
            // 记录关闭失败
        }
        return -1;
    }

    uint64_t totalLen = static_cast<uint64_t>(st.st_size);
    uint64_t copyTotalLen = 0;

    while (copyTotalLen < totalLen) {
        ssize_t copyLen = sendfile(fdOut, fdIn, nullptr, totalLen - copyTotalLen);
        if (copyLen <= 0) {
            if (errno == EINTR) {
                continue; // 被信号中断，重试
            }
            break; // 其他错误，退出循环
        }
        copyTotalLen += static_cast<uint64_t>(copyLen);
    }

    // 确保数据写入磁盘
    if (fsync(fdOut) != 0) {
        // 同步失败，但继续关闭文件描述符
    }

    int closeInResult = close(fdIn);
    int closeOutResult = close(fdOut);

    // 检查是否所有数据都被复制以及文件描述符是否成功关闭
    bool copySuccess = (copyTotalLen == totalLen);
    bool closeSuccess = (closeInResult == 0 && closeOutResult == 0);

    return (copySuccess && closeSuccess) ? 0 : -1;
}

bool IsDirectory(const std::string &path)
{
    struct stat statBuffer;
    if (stat(path.c_str(), &statBuffer) == 0 && S_ISDIR(statBuffer.st_mode)) {
        return true;
    }
    return false;
}

bool GetLastLine(std::istream &fin, std::string &line, uint32_t maxLen)
{
    if (fin.tellg() <= 0) {
        return false;
    } else {
        fin.seekg(-1, fin.cur);
    }
    uint32_t count = 0;
    while (fin.good() && fin.peek() == fin.widen('\n') && fin.tellg() > 0 && count < maxLen) {
        fin.seekg(-1, fin.cur);
        count++;
    }
    if (!fin.good() || count >= maxLen) {
        return false;
    }
    if (fin.tellg() == 0) {
        return true;
    }
    count = 0;
    while (fin.good() && fin.peek() != fin.widen('\n') && fin.tellg() > 0 && count < maxLen) {
        fin.seekg(-1, fin.cur);
        count++;
    }
    if (!fin.good() || count >= maxLen) {
        return false;
    }
    if (fin.tellg() != 0) {
        fin.seekg(1, fin.cur);
    }
    auto oldPos = fin.tellg();
    getline(fin, line);
    fin.seekg(oldPos);
    return true;
}

std::string GetFirstLine(const std::string& path)
{
    std::ifstream inFile(path.c_str());
    if (!inFile) {
        return "";
    }
    std::string firstLine;
    getline(inFile, firstLine);
    inFile.close();
    return firstLine;
}

std::string GetParentDir(const std::string &path)
{
    string str = ExtractFilePath(path);
    if (str.empty()) {
        return "";
    }
    return str.substr(0, str.size() - 1);
}

bool IsLegalPath(const std::string& path)
{
    if (path.find("./") != std::string::npos ||
        path.find("../") != std::string::npos) {
        return false;
    }
    return true;
}

bool RenameFile(const std::string& src, const std::string& dest)
{
    if (std::rename(src.c_str(), dest.c_str()) == 0) {
        return true;
    }
    return false;
}

bool GetDirXattr(const std::string& dir, const std::string& name, std::string& value)
{
    char buf[BUF_SIZE_256] = {0};
    if (getxattr(dir.c_str(), name.c_str(), buf, BUF_SIZE_256) == -1) {
        return false;
    }
    value = buf;
    return true;
}
} // namespace FileUtil
} // namespace HiviewDFX
} // namespace OHOS
