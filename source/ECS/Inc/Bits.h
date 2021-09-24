#ifndef MYECS_BITS_H
#define MYECS_BITS_H

#include <cinttypes>
#include <array>

namespace MyECS
{
    template<typename T, std::size_t count> requires std::is_unsigned_v<T>
    struct Bits
    {
        Bits() { _bits.fill(0); }

        void TrySet(size_t bitIndex);
        void Set(size_t bitIndex);
        void TryReset(size_t bitIndex);
        void Reset(size_t bitIndex);
        bool IsAndNonZero(const Bits<T,count>& other) const;

        void ResetAll();

        bool GetBitState(size_t bitIndex) const;
        bool TryGetBitState(size_t bitIndex) const;

        std::vector<uint32_t> GetOnes() const;

        Bits& operator|=(const Bits<T, count>& other);
        Bits& operator&=(const Bits<T, count>& other);

        private:
            static constexpr uint8_t _typeSize = sizeof(T) * 8;
            static constexpr std::size_t _trueCount = ((count / _typeSize) + ((count % _typeSize == 0) ? 0 : 1));

            std::array<T, _trueCount> _bits;

            static constexpr T _moduloMask = _typeSize - static_cast<T>(1);
            static constexpr T _divideShift = std::log2p1(_typeSize) - static_cast<T>(1);
            static constexpr T _setMask = static_cast<T>(1);
            static constexpr auto _compiledSequence = std::make_integer_sequence<uint8_t, _typeSize>();

            ///function retrieves and adds indices of set bits to result vector it is helper function for
            ///function "std::vector<uint32_t> GetOnes() const;"
            template<typename size_type = std::size_t, size_type ...Indices>
            void GetSetBitsOnType(std::integer_sequence<size_type, Indices...>, T varFromBitset,
                                 std::vector<uint32_t>& result, uint32_t offset) const;

            friend bool operator==(const Bits<T, count>& lhs, const Bits<T, count>& rhs)
            {
                for(std::size_t i{0}; i<lhs._bits.size(); ++i)
                    if(lhs._bits[i] != rhs._bits[i]) return false;

                return true;
            }
    };

}

#include "Impl/Bits_impl.tpp"

#endif
