#include "List.h"



namespace KhFileSystem
{

template<> void Swap<FListBase>(FListBase& first, FListBase& second)
{
    first.Swap(second);
}

void FListBase::AllocatePtrs()
{
    _ptrs = new FListNodeBase*[3];

    SetSize(0);
    _ptrs[1] = nullptr;
    _ptrs[2] = nullptr;
}

}
