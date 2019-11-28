#pragma once
#include <vector>
#include <string>

namespace pronto
{
  template<typename T>
  class Ntree
  {
  public:
    Ntree<T>(T data)
      : data_(data)
    {
    }

    Ntree<T>(T data, const std::string& name)
      : data_(data)
    {
      node_name_ = name;
    }

    Ntree<T>(const Ntree& item)
      : data_(item.data_),
      node_name_(item.node_name_)
    {
      children_ = item.children_;
    }

    Ntree<T>& AddChild(Ntree<T>& item)
    {
      children_.push_back(item);
      return *children_.rbegin();
    }

    Ntree<T>& AddChild(T data)
    {
      children_.push_back(Ntree<T>(data));
      return *children_.rbegin();
    }

    Ntree<T>& AddChild()
    {
      children_.push_back(Ntree<T>(T()));
      return *children_.rbegin();
    }
    
    Ntree<T>& AddChild(const std::string& name)
    {
      children_.push_back(Ntree<T>(T()));
      auto& node = *children_.rbegin();
      node.node_name_ = name; // for some reason using the constructor would yield a different result
      return node;
    }

    std::vector<Ntree<T>>& children()
    {
      return children_;
    }

    T& data()
    {
      return data_;
    }

  private:
    std::string node_name_;
    T data_;
    std::vector<Ntree<T>> children_;
  };
}