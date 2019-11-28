#include "d3d12_command_queue.h"
#include "d3d12_command_list.h" 
#include <d3d12.h>
#include <cstdint>
#include <assert.h>

namespace pronto
{
  namespace graphics
  {
    //---------------------------------------------------------------------------------------------
    CommandQueue::CommandQueue(ID3D12Device2* device, D3D12_COMMAND_LIST_TYPE type) :
      fence_value_(0),
      command_list_type_(type),
      device_(device)
    {
      D3D12_COMMAND_QUEUE_DESC desc = {};
      desc.Type = type;
      desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
      desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
      desc.NodeMask = 0;

      assert(SUCCEEDED(
        device_->CreateCommandQueue(&desc, IID_PPV_ARGS(&command_q_))));

      command_q_->SetName(L"command q lyf");

      assert(SUCCEEDED(
        device_->CreateFence(
          fence_value_,
          D3D12_FENCE_FLAG_NONE,
          IID_PPV_ARGS(&fence_))));

      fence_event_ = ::CreateEvent(NULL, FALSE, FALSE, NULL);
      assert(fence_event_ && "Failed to create fence event handle.");

      process_in_flight_command_lists_thread_ = 
        std::thread(&CommandQueue::ProccessInFlightCommandLists, this);

      process_in_flight_command_lists_ = true;
    }

    //---------------------------------------------------------------------------------------------
    CommandQueue::~CommandQueue()
    {
      process_in_flight_command_lists_ = false;
      process_in_flight_command_lists_thread_.join();
    }

    //---------------------------------------------------------------------------------------------
    CommandList* CommandQueue::GetCommandList()
    {
      CommandList* command_list;

      if (available_command_lists_.Empty() == false)
      {
        available_command_lists_.TryPop(command_list);
      }
      else
      {
        command_list = new CommandList(device_, command_list_type_);
      }

      return command_list;
    }

    //---------------------------------------------------------------------------------------------
    uint64_t CommandQueue::ExecuteCommandList(CommandList* command_list)
    {
      ID3D12GraphicsCommandList* d3d_c_list = command_list->command_list();

      d3d_c_list->Close();

      ID3D12CommandList* const ppCommandLists[] = {
        d3d_c_list
      };

      command_q_->ExecuteCommandLists(1, ppCommandLists);

      //command_allocator_q_.emplace(CommandAllocatorEntry{ fence_value, command_allocator });
      //command_list_q_.push(command_list);

      uint64_t fence_value = Signal();

      inflight_command_lists_.Push({ fence_value, command_list });

      return fence_value;
    }

    //---------------------------------------------------------------------------------------------
    uint64_t CommandQueue::Signal()
    {
      uint64_t fence_value = ++fence_value_;
      command_q_->Signal(fence_, fence_value);
      return fence_value;
    }

    //---------------------------------------------------------------------------------------------
    bool CommandQueue::IsFenceComplete(uint64_t fence_value)
    {
      return fence_->GetCompletedValue() >= fence_value;
    }

    //---------------------------------------------------------------------------------------------
    void CommandQueue::WaitForFenceValue(uint64_t fence_value)
    {
      if (IsFenceComplete(fence_value) == false)
      {
        fence_->SetEventOnCompletion(fence_value, fence_event_);
        ::WaitForSingleObject(fence_event_, MAXDWORD);
      }
    }

    //---------------------------------------------------------------------------------------------
    void CommandQueue::Flush()
    {
      std::unique_lock<std::mutex> lock(process_in_flight_command_lists_thread_mutex_);
      process_in_flight_command_lists_thread_cv_.wait(
        lock, [this] { return inflight_command_lists_.Empty(); });

      // In case the command queue was signaled directly 
      // using the CommandQueue::Signal method then the 
      // fence value of the command queue might be higher than the fence
      // value of any of the executed command lists.
      WaitForFenceValue(fence_value_);
    }

    //---------------------------------------------------------------------------------------------
    ID3D12CommandQueue* CommandQueue::command_q() const
    {
      return command_q_;
    }

    //---------------------------------------------------------------------------------------------
    void CommandQueue::Close()
    {
      Flush();
      ::CloseHandle(fence_event_);
    }

    //---------------------------------------------------------------------------------------------
    CommandList* CommandQueue::CreateCommandList()
    {
      return new CommandList(device_, command_list_type_);
    }

    void CommandQueue::ProccessInFlightCommandLists()
    {
      std::unique_lock<std::mutex> lock(process_in_flight_command_lists_thread_mutex_, std::defer_lock);

      while (process_in_flight_command_lists_)
      {
        CommandListEntry command_list_entry;

        lock.lock();
        while (inflight_command_lists_.TryPop(command_list_entry))
        {
          auto fence_value = std::get<0>(command_list_entry);
          auto command_list = std::get<1>(command_list_entry);

          WaitForFenceValue(fence_value);

          command_list->Reset();

          available_command_lists_.Push(command_list);
        }

        lock.unlock();
        process_in_flight_command_lists_thread_cv_.notify_one();

        std::this_thread::yield();
      }
    }
  }
}
