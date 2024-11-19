#include <cstddef>

namespace Utils {
    /**
     * Work with enumerations:
     */
    template<class Enum>
    constexpr size_t EnumCast(Enum value) noexcept {
        return static_cast<size_t>(value);
    }

    template<class Enum>
    constexpr size_t EnumSize() noexcept {
        return EnumCast(Enum::COUNT);
    }

    template<class Enum>
    constexpr Enum EnumCast(size_t value) noexcept {
        return static_cast<Enum>(value);
    }

    template<class ...Args>
    constexpr size_t CreateMask(Args ... bits) noexcept {
        return (static_cast<size_t>(bits) | ...);
    }

    template<class ...Args>
    constexpr bool HasAny(size_t mask, Args ... bits) noexcept {
        return (mask & Utils::CreateMask(bits...)) > 0;
    }
}