#ifndef VKLITE_CORE_HPP
#define VKLITE_CORE_HPP

#include <bit>
#include <span>
#include <string_view>
#include <type_traits>
#include <system_error>

namespace vklite {
    struct ApiVersion {
        ApiVersion() = default;

        constexpr explicit ApiVersion(uint32_t value) : value(value) {}

        constexpr ApiVersion(uint32_t variant, uint32_t major, uint32_t minor,
                             uint32_t patch)
            : value(VK_MAKE_API_VERSION(variant, major, minor, patch)) {}

        constexpr uint32_t getVariant() const noexcept {
            return VK_API_VERSION_VARIANT(value);
        }

        constexpr uint32_t getMajor() const noexcept {
            return VK_API_VERSION_MAJOR(value);
        }

        constexpr uint32_t getMinor() const noexcept {
            return VK_API_VERSION_MINOR(value);
        }

        constexpr uint32_t getPatch() const noexcept {
            return VK_API_VERSION_PATCH(value);
        }

        uint32_t value = 0;
    };

    enum class ObjectType : int32_t;

    template<class T, ObjectType K>
    struct Handle {
        T handle = {};

        explicit operator bool() const noexcept { return !!handle; }
    };

    struct Object {
        ObjectType type;
        uint64_t handle;

        constexpr Object(ObjectType type, uint64_t handle)
            : type(type), handle(handle) {}

        template<class T, ObjectType K>
        constexpr Object(Handle<T, K> h)
            : type(K), handle(uint64_t(h.handle)) {}
    };

    template<class Enum, class Flags>
    struct FlagSet {
        FlagSet() = default;

        constexpr FlagSet(Enum val) : flags(static_cast<Flags>(val)) {}

        constexpr explicit FlagSet(Flags val) : flags(val) {}

        constexpr explicit operator bool() const { return flags != 0; }

        constexpr bool contains(FlagSet other) const {
            return (flags & other.flags) == other.flags;
        }

        constexpr Flags toUnderlying() const { return flags; }

        friend bool operator==(FlagSet a, FlagSet b) = default;

        friend constexpr FlagSet operator|(FlagSet a, FlagSet b) noexcept {
            return FlagSet(a.flags | b.flags);
        }

        friend constexpr FlagSet operator&(FlagSet a, FlagSet b) noexcept {
            return FlagSet(a.flags & b.flags);
        }

        friend constexpr FlagSet operator^(FlagSet a, FlagSet b) noexcept {
            return FlagSet(a.flags ^ b.flags);
        }

        Flags flags = 0;
    };

    enum class [[nodiscard]] Result : int32_t;

    const char* getResultText(Result r) noexcept;

    struct ErrorCategory : std::error_category {
        const char* name() const noexcept override { return "vklite"; }

        std::string message(int condition) const override {
            return getResultText(Result(condition));
        }
    };

    inline const std::error_category& errorCategory() noexcept {
        static const ErrorCategory ret;
        return ret;
    }

    inline std::error_condition make_error_condition(Result e) {
        return std::error_condition(int(e), errorCategory());
    }

    inline void check(Result r) {
        if (const int ec = int32_t(r))
            throw std::system_error(ec, errorCategory());
    }

    template<class T>
    struct [[nodiscard]] Ret {
        Result result;
        T value;

        T get() const& {
            check(result);
            return value;
        }

        T get() && {
            check(result);
            return std::move(value);
        }

        bool extract(T& out) {
            if (int32_t(result))
                return false;
            out = std::move(value);
            return true;
        }
    };
} // namespace vklite

#endif // VKLITE_CORE_HPP