#ifndef MYECS_BITS_IMPL_TPP
#define MYECS_BITS_IMPL_TPP

#include <Inc/Bits.h>

namespace MyECS
{
    template<typename T, std::size_t count>
    requires std::is_unsigned_v<T>
    void Bits<T, count>::TrySet(size_t bitIndex)
    {
        if(bitIndex < count)
            _bits[bitIndex >> _divideShift] |= _setMask << (bitIndex & _moduloMask);
    }

    template<typename T, std::size_t count>
    requires std::is_unsigned_v<T>
    void Bits<T, count>::TryReset(size_t bitIndex)
    {
        if(bitIndex < count)
            _bits[bitIndex >> _divideShift] &= ~(_setMask << (bitIndex & _moduloMask));
    }

    template<typename T, std::size_t count>
    requires std::is_unsigned_v<T>
    bool Bits<T, count>::GetBitState(size_t bitIndex) const
    {
        return _bits[bitIndex >> _divideShift] & (_setMask << (bitIndex & _moduloMask));
    }

    template<typename T, std::size_t count>
    requires std::is_unsigned_v<T>
    bool Bits<T, count>::TryGetBitState(size_t bitIndex) const
    {
        if(bitIndex < count)
            return (_bits[bitIndex >> _divideShift] & (_setMask << (bitIndex & _moduloMask)));

        return false;
    }

    template<typename T, std::size_t count>
    requires std::is_unsigned_v<T>
    template<typename size_type, size_type... Indices>
    void Bits<T, count>::GetSetBitsOnType(std::integer_sequence<size_type, Indices...>,
            T varFromBitset, std::vector<uint32_t> &result, uint32_t offset) const
    {
        static auto checkIfWrapper_fn = [&result](size_type index, T varFromBitset){
            if(varFromBitset & (Bits<T,count>::_setMask << index))
                result.push_back(index);
        };

        (checkIfWrapper_fn(Indices + offset, varFromBitset), ...);
    }

    template<typename T, std::size_t count>
    requires std::is_unsigned_v<T>
    std::vector<uint32_t> Bits<T, count>::GetOnes() const
    {
        std::vector<uint32_t> result;
        result.reserve(_bits.size()/2);

        for(uint32_t i{0}; i<_bits.size(); ++i)
            GetSetBitsOnType(_compiledSequence, _bits[i], result, i);

        return result;
    }

    template<typename T, std::size_t count>
    requires std::is_unsigned_v<T>
    Bits<T, count>& Bits<T, count>::operator|=(const Bits<T, count>& other)
    {
        for(uint32_t i{0}; i<_bits.size(); ++i)
            _bits[i] |= other._bits[i];

        return *this;
    }


}

#endif