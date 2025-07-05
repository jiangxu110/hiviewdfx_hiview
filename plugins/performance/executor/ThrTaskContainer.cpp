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

#include "ThrTaskContainer.h"
#include <sys/prctl.h>
#include <thread>
#include "hiview_logger.h"

namespace OHOS {
namespace HiviewDFX {
DEFINE_LOG_LABEL(0xD002D66, "Hiview-XPerformance");

/* ThrTaskContainer */
ThrTaskContainer::~ThrTaskContainer()
{
    StopLoop();
}

void ThrTaskContainer::StartLoop(const std::string& threadName)
{
    std::unique_lock<std::mutex> lock(mut);
    if (isRunning_.load()) {
        HIVIEW_LOGW("Thread loop is already running");
        return;
    }

    shouldStop_.store(false);
    isRunning_.store(true);
    workerThread_ = std::make_unique<std::thread>(&ThrTaskContainer::Entry, this, threadName);
}

void ThrTaskContainer::StopLoop()
{
    {
        std::unique_lock<std::mutex> lock(mut);
        if (!isRunning_.load()) {
            return;
        }
        shouldStop_.store(true);
    }
    cv.notify_all();

    if (workerThread_ && workerThread_->joinable()) {
        workerThread_->join();
    }

    CleanupTasks();
    isRunning_.store(false);
}

void ThrTaskContainer::PostTask(ITask* task)
{
    if (task == nullptr) {
        HIVIEW_LOGE("PostTask: task is null");
        return; // 不抛异常，改为记录日志并返回
    }

    std::unique_lock<std::mutex> uniqueLock(mut);
    if (shouldStop_.load()) {
        HIVIEW_LOGW("PostTask: container is stopping, task rejected");
        delete task; // 清理任务避免内存泄漏
        return;
    }

    if (!IsTaskOverLimit()) {
        tasks.push_back(task);
        cv.notify_one();
    } else {
        delete task; // 如果超过限制，清理任务
        HIVIEW_LOGW("PostTask: task limit exceeded, task rejected");
    }
}

bool ThrTaskContainer::IsTaskOverLimit()
{
    if (tasks.size() < maxTaskSize) {
        return false;
    }
    HIVIEW_LOGW("Task queue over limit (%zu), clearing all tasks", tasks.size());
    CleanupTasks();
    return true;
}

void ThrTaskContainer::CleanupTasks()
{
    for (auto& task : tasks) {
        if (task != nullptr) {
            delete task;
        }
    }
    tasks.clear();
}

void ThrTaskContainer::Entry(const std::string& threadName)
{
    std::unique_lock<std::mutex> uniqueLock(mut);
    prctl(PR_SET_NAME, threadName.c_str(), nullptr, nullptr, nullptr);

    while (!shouldStop_.load()) {
        // 等待任务或停止信号
        cv.wait(uniqueLock, [this] {
            return !tasks.empty() || shouldStop_.load();
        });

        // 检查是否需要停止
        if (shouldStop_.load()) {
            break;
        }

        // 处理所有可用任务
        while (!tasks.empty() && !shouldStop_.load()) {
            ITask* task = tasks.front();
            tasks.erase(tasks.begin());

            if (task == nullptr) {
                HIVIEW_LOGE("Entry: task is null, skipping");
                continue;
            }

            // 在执行任务时释放锁，避免阻塞其他操作
            uniqueLock.unlock();

            try {
                task->Run();
                delete task; // 执行完成后清理任务
            } catch (const std::exception& e) {
                HIVIEW_LOGE("Entry: task execution failed: %{public}s", e.what());
                delete task; // 异常情况下也要清理任务
            } catch (...) {
                HIVIEW_LOGE("Entry: task execution failed with unknown exception");
                delete task;
            }

            uniqueLock.lock();
        }
    }

    HIVIEW_LOGI("Thread %{public}s exiting", threadName.c_str());
}
} // HiviewDFX
} // OHOS