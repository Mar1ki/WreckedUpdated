#include "pch.h"
#include "Engine/Plugins/TaskLibrary/Public/TaskLibrary.h"

static std::unordered_map<std::string, UTaskLibrary::Queue> Queues;

void UTaskLibrary::Initialize(const std::string& QueueName, float CurrentTime)
{
    auto& Queue = Queues[QueueName];
    if (Queue.bInitialized) return;
    Queue.bInitialized = true;
    Queue.StartTime = CurrentTime;
}

void UTaskLibrary::QueueTask(const std::string& QueueName, float CurrentTime, float DelaySeconds, std::function<void()> Callback)
{
    auto& Queue = Queues[QueueName];
    float ExecutionTime = CurrentTime + DelaySeconds; 
    Queue.Tasks.push_back({ExecutionTime, Callback, false});
}

void UTaskLibrary::ProcessTasks(const std::string& QueueName, float CurrentTime)
{
    auto it = Queues.find(QueueName);
    if (it == Queues.end() || !it->second.bInitialized) return;

    auto& Queue = it->second;

    for (auto& Task : Queue.Tasks)
    {
        if (!Task.bExecuted && CurrentTime >= Task.ExecutionTime)
        {
            Task.Callback();
            Task.bExecuted = true;
        }
    }
}

void UTaskLibrary::Clear(const std::string& QueueName)
{
    auto it = Queues.find(QueueName);
    if (it != Queues.end())
    {
        it->second.Tasks.clear();
        it->second.bInitialized = false;
        it->second.StartTime = 0.0f;
    }
}

bool UTaskLibrary::IsInitialized(const std::string& QueueName)
{
    auto it = Queues.find(QueueName);
    return it != Queues.end() && it->second.bInitialized;
}