// Copyright 2020 Dow, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

#include <atomic>
#include <functional>

struct TaskControl
{
    // Signals emitted from notification functions must use Qt::QueuedConnection.
    using StartedFunctionType = std::function<void()>;
    using ProgressFunctionType = std::function<void(double)>;
    using MessageFunctionType = std::function<void(const std::string&)>;
    using FinishedFunctionType = std::function<void()>;

    TaskControl() : interrupt_{false} {}

    TaskControl(const TaskControl&) = delete;
    TaskControl& operator=(const TaskControl&) = delete;
    TaskControl(TaskControl&&) = default;
    TaskControl& operator=(TaskControl&&) = default;

    void setStartedFunction(const StartedFunctionType& fn)
    {
        startedFn_ = fn;
    }

    void setFinishedFunction(const FinishedFunctionType& fn)
    {
        finishedFn_ = fn;
    }

    void setProgressFunction(const ProgressFunctionType& fn)
    {
        progressFn_ = fn;
    }

    void setMessageFunction(const MessageFunctionType& fn)
    {
        messageFn_ = fn;
    }

    void requestInterrupt()
    {
        interrupt_ = true;
    }

    bool interruptRequested()
    {
        return interrupt_;
    }

    void progress(double progress)
    {
        if (progressFn_)
            progressFn_(progress);
    }

    void message(const std::string& message)
    {
        if (messageFn_)
            messageFn_(message);
    }

    void started()
    {
        if (startedFn_)
            startedFn_();
    }

    void finished()
    {
        if (finishedFn_)
            finishedFn_();
    }

private:
    std::atomic_bool interrupt_;
    StartedFunctionType startedFn_;
    FinishedFunctionType finishedFn_;
    ProgressFunctionType progressFn_;
    MessageFunctionType messageFn_;
};

