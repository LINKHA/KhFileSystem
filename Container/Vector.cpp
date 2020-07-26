#include "Allocator.h"
#include "Vector.h"



namespace KhFileSystem
{

FVectorBase::FVectorBase() :
    _buffer(nullptr)
{
}

void FVectorBase::Swap(FVectorBase& vector)
{
    KhFileSystem::Swap(_buffer, vector._buffer);
}

unsigned char* FVectorBase::AllocateBuffer(size_t size)
{
    // Include space for _size and capacity
    return new unsigned char[size + 2 * sizeof(size_t)];
}

template<> void Swap<FVectorBase>(FVectorBase& first, FVectorBase& second)
{
    first.Swap(second);
}

}
