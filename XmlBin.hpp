#pragma once

#include <bit>
#include <cassert>

namespace xmlbin {
    using ByteOffset = std::uint32_t;
    using Index = std::uint32_t;
    using Size = std::uint32_t;

    struct Segment {
        ByteOffset offset;
        Size count;
    };

    template<class T>
    struct Idx {
        Index value = 0;
        auto operator<=>(const Idx&) const = default;
    };

    struct StrId {
        Index value = 0;
        auto operator<=>(const StrId&) const = default;
        explicit operator bool() const noexcept { return value != 0; }
    };

    template<class T>
    struct Sequence {
        Idx<T> start;
        Size count = 0;
    };

    struct UniqueStrEntry {
        Index idx;
        Index offset;
    };

    constexpr std::uint32_t shiftL(std::uint32_t value, unsigned n) {
        assert(value < (1u << (32u - n)));
        return value << n;
    }

    template<class Kind>
    struct TagId {
        static constexpr unsigned kBits =
            std::bit_width(unsigned(Kind::NUM) - 1u);
        static constexpr std::uint32_t kMask = (1u << kBits) - 1u;

        constexpr TagId() = default;

        constexpr TagId(Kind kind, Index index)
            : data(shiftL(index, kBits) | unsigned(kind)) {}

        constexpr Kind getKind() const { return Kind(data & kMask); }

        constexpr Index getIndex() const { return data >> kBits; }

    private:
        std::uint32_t data = 0u;
    };

    enum class NodeKind { Text, Element, NUM };

    using NodeId = TagId<NodeKind>;

    struct Attribute {
        StrId name;
        StrId value;
    };

    struct Element {
        StrId tag;
        Sequence<Attribute> attrs;
        Sequence<NodeId> children;
    };

    struct Context {
        Segment strings;
        Segment uniqueStrings;
        Segment nodes;
        Segment attrs;
        Segment elems;
    };
} // namespace xmlbin