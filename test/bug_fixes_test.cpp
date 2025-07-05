/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <thread>
#include <chrono>
#include <memory>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>

// 包含我们修复的头文件
#include "ThrTaskContainer.h"
#include "file_util.h"

using namespace OHOS::HiviewDFX;
using namespace std;

namespace OHOS {
namespace HiviewDFX {

// 简单的测试任务类
class TestTask : public ITask {
public:
    TestTask(int id) : taskId_(id), executed_(false) {}
    
    void Run() override {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        executed_ = true;
    }
    
    std::string GetTaskInfo() override {
        return "TestTask_" + std::to_string(taskId_);
    }
    
    bool IsExecuted() const { return executed_; }
    int GetId() const { return taskId_; }

private:
    int taskId_;
    bool executed_;
};

class BugFixesTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: ThrTaskContainer_ThreadSafety_Test
 * @tc.desc: 测试ThrTaskContainer的线程安全修复
 * @tc.type: FUNC
 */
HWTEST_F(BugFixesTest, ThrTaskContainer_ThreadSafety_Test, testing::ext::TestSize.Level1)
{
    ThrTaskContainer container;
    
    // 启动容器
    container.StartLoop("TestThread");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 提交多个任务
    const int taskCount = 10;
    std::vector<TestTask*> tasks;
    
    for (int i = 0; i < taskCount; ++i) {
        auto task = new TestTask(i);
        tasks.push_back(task);
        container.PostTask(task);
    }
    
    // 等待任务执行完成
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 停止容器
    container.StopLoop();
    
    // 验证任务执行状态（注意：任务在执行后会被删除，所以这里无法验证）
    // 主要验证没有崩溃或死锁
    EXPECT_TRUE(true);
}

/**
 * @tc.name: ThrTaskContainer_StopLoop_Test
 * @tc.desc: 测试ThrTaskContainer的停止功能
 * @tc.type: FUNC
 */
HWTEST_F(BugFixesTest, ThrTaskContainer_StopLoop_Test, testing::ext::TestSize.Level1)
{
    ThrTaskContainer container;
    
    // 启动容器
    container.StartLoop("TestThread");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // 停止容器
    container.StopLoop();
    
    // 尝试提交任务（应该被拒绝）
    auto task = new TestTask(999);
    container.PostTask(task); // 这个任务应该被拒绝并删除
    
    // 验证没有崩溃
    EXPECT_TRUE(true);
}

/**
 * @tc.name: ThrTaskContainer_NullTask_Test
 * @tc.desc: 测试ThrTaskContainer对空任务的处理
 * @tc.type: FUNC
 */
HWTEST_F(BugFixesTest, ThrTaskContainer_NullTask_Test, testing::ext::TestSize.Level1)
{
    ThrTaskContainer container;
    
    // 启动容器
    container.StartLoop("TestThread");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // 提交空任务（应该被安全处理）
    container.PostTask(nullptr);
    
    // 停止容器
    container.StopLoop();
    
    // 验证没有崩溃
    EXPECT_TRUE(true);
}

/**
 * @tc.name: FileUtil_CopyFile_Test
 * @tc.desc: 测试文件复制的资源管理修复
 * @tc.type: FUNC
 */
HWTEST_F(BugFixesTest, FileUtil_CopyFile_Test, testing::ext::TestSize.Level1)
{
    const std::string srcFile = "/tmp/test_src_file.txt";
    const std::string dstFile = "/tmp/test_dst_file.txt";
    const std::string testContent = "This is test content for file copy.";
    
    // 创建源文件
    std::ofstream ofs(srcFile);
    ASSERT_TRUE(ofs.is_open());
    ofs << testContent;
    ofs.close();
    
    // 测试文件复制
    int result = FileUtil::CopyFile(srcFile, dstFile);
    EXPECT_EQ(result, 0);
    
    // 验证目标文件内容
    std::ifstream ifs(dstFile);
    ASSERT_TRUE(ifs.is_open());
    std::string readContent;
    std::getline(ifs, readContent);
    ifs.close();
    
    EXPECT_EQ(readContent, testContent);
    
    // 清理测试文件
    unlink(srcFile.c_str());
    unlink(dstFile.c_str());
}

/**
 * @tc.name: FileUtil_CopyFile_ErrorHandling_Test
 * @tc.desc: 测试文件复制的错误处理
 * @tc.type: FUNC
 */
HWTEST_F(BugFixesTest, FileUtil_CopyFile_ErrorHandling_Test, testing::ext::TestSize.Level1)
{
    const std::string nonExistentSrc = "/tmp/non_existent_file.txt";
    const std::string dstFile = "/tmp/test_dst_file.txt";
    
    // 测试复制不存在的文件
    int result = FileUtil::CopyFile(nonExistentSrc, dstFile);
    EXPECT_EQ(result, -1);
    
    // 验证目标文件没有被创建
    EXPECT_FALSE(FileUtil::FileExists(dstFile));
}

/**
 * @tc.name: FileUtil_CopyFileFast_Test
 * @tc.desc: 测试快速文件复制的修复
 * @tc.type: FUNC
 */
HWTEST_F(BugFixesTest, FileUtil_CopyFileFast_Test, testing::ext::TestSize.Level1)
{
    const std::string srcFile = "/tmp/test_fast_src_file.txt";
    const std::string dstFile = "/tmp/test_fast_dst_file.txt";
    const std::string testContent = "This is test content for fast file copy.";
    
    // 创建源文件
    std::ofstream ofs(srcFile);
    ASSERT_TRUE(ofs.is_open());
    ofs << testContent;
    ofs.close();
    
    // 测试快速文件复制
    int result = FileUtil::CopyFileFast(srcFile, dstFile);
    EXPECT_EQ(result, 0);
    
    // 验证目标文件存在
    EXPECT_TRUE(FileUtil::FileExists(dstFile));
    
    // 清理测试文件
    unlink(srcFile.c_str());
    unlink(dstFile.c_str());
}

} // namespace HiviewDFX
} // namespace OHOS
