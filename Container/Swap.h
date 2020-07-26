#pragma once

#include "../AutoConfig.h"

namespace KhFileSystem
{

class FHashBase;
class FListBase;
class FVectorBase;
class FString;

/// Swap two values.
template<class _Ty> inline void Swap(_Ty& first, _Ty& second)
{
    _Ty temp = first;
    first = second;
    second = temp;
}

/// Swap two hash sets/maps.
template<> void Swap<FHashBase>(FHashBase& first, FHashBase& second);

/// Swap two lists.
template<> void Swap<FListBase>(FListBase& first, FListBase& second);

/// Swap two vectors.
template<> void Swap<FVectorBase>(FVectorBase& first, FVectorBase& second);

/// Swap two strings.
template<> void Swap<FString>(FString& first, FString& second);

}
