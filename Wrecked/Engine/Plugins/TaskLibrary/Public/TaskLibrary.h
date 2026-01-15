#pragma once
#include "pch.h"

#include <vector>
#include <functional>

namespace UTaskLibrary
{
    struct Task
    {
        float ExecutionTime;
        std::function<void()> Callback;
        bool bExecuted = false;
    };

    struct Queue
    {
        std::vector<Task> Tasks;
        float StartTime = 0.0f;
        bool bInitialized = false;
    };

    void Initialize(const std::string& QueueName, float CurrentTime);
    void QueueTask(const std::string& QueueName, float CurrentTime, float DelaySeconds, std::function<void()> Callback);
    void ProcessTasks(const std::string& QueueName, float CurrentTime);
    void Clear(const std::string& QueueName);
    bool IsInitialized(const std::string& QueueName);
}