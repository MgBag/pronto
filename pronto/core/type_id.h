#pragma once
#include <memory>

namespace pronto
{
  using TID = uint16_t;
  using EID = uint32_t;

  class Entity;
  class Component;

  class CompTIDBase
  {
  protected:
    static TID last_;
  public:
    static const size_t size() { return last_; }
  };

  class EntTIDBase
  {
  protected:
    static TID last_;
  public:
    static const size_t size() { return last_; }
  };

  template<class T>
  class EntTID : EntTIDBase
  {
  public:
    static const TID tid() { return tid_; }

  private:
    static TID tid_;
  };

  template<class T>
  class CompTID : CompTIDBase
  {
  public:
    static const TID tid() { return tid_; }

  private:
    static TID tid_;
  };

  class EntID
  {
  protected:
    static EID last_;
    EID eid_;
  public:
    EntID()
    {
      eid_ = last_++;
    }

    const EID eid() const { return eid_; }
    static const EID size() { return last_; }

    bool operator==(const EntID& rhs) { return this->eid() == rhs.eid(); }
  };

  template<typename T>
  TID EntTID<T>::tid_ = last_++;

  template<typename T>
  TID CompTID<T>::tid_ = last_++;
}
