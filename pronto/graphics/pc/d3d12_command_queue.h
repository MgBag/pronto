#pragma once

#include <queue>  // For std::queue
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "platform/utils.h"

struct ID3D12Device2;
struct ID3D12CommandQueue;
struct ID3D12GraphicsCommandList;
struct ID3D12Fence;

enum D3D12_COMMAND_LIST_TYPE;

typedef void *HANDLE; //winnt fwd
typedef unsigned long long uint64_t; //stdint fwd

namespace pronto
{
  namespace graphics
  {
    class CommandList;

    class CommandQueue
    {
    public:
      CommandQueue(ID3D12Device2* device, D3D12_COMMAND_LIST_TYPE type);
      virtual ~CommandQueue();

      // Get an available command list from the command queue.
      CommandList* GetCommandList();

      // Execute a command list.
      // Returns the fence value to wait for for this command list.
      uint64_t ExecuteCommandList(CommandList* command_list);

      uint64_t Signal();
      bool IsFenceComplete(uint64_t fence_value);
      void WaitForFenceValue(uint64_t fence_value);
      void Flush();

      ID3D12CommandQueue* command_q() const;

      void Close();
    protected:
      CommandList* CreateCommandList();

    private:
      using CommandListQueue = std::queue<CommandList*>;

      D3D12_COMMAND_LIST_TYPE command_list_type_;
      ID3D12Device2* device_;
      ID3D12CommandQueue* command_q_;
      ID3D12Fence* fence_;
      HANDLE fence_event_;
      uint64_t fence_value_;
      CommandListQueue command_list_q_;

      // A thread to process in-flight command lists.
      using CommandListEntry = std::tuple<uint64_t, CommandList*>;

      ThreadSafeQueue<CommandListEntry> inflight_command_lists_;
      ThreadSafeQueue<CommandList*> available_command_lists_;

      std::thread process_in_flight_command_lists_thread_;
      std::atomic_bool process_in_flight_command_lists_;
      std::mutex process_in_flight_command_lists_thread_mutex_;
      std::condition_variable process_in_flight_command_lists_thread_cv_;

      void ProccessInFlightCommandLists();
    };
  }
}