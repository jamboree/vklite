#pragma once

#include <memory>
#include <iterator>

template<class I, class Out>
void eytzingerImpl(I n, Out& out, I k) {
    if (k <= n) {
        const auto k2 = k << 1u;
        eytzingerImpl<I>(n, out, k2);
        out(k - 1);
        eytzingerImpl<I>(n, out, k2 | 1u);
    }
}

template<class I, class Out>
void eytzinger(I n, Out&& out) {
    eytzingerImpl<I>(n, out, 1);
}

template<class Pred, class Swap>
std::size_t indirectPartition(std::size_t i, std::size_t e, Pred pred,
                              Swap swap) {
    for (; i != e; ++i) {
        if (!pred(i)) {
            for (std::size_t j = i; ++j != e;) {
                if (pred(j)) {
                    swap(j, i);
                    ++i;
                }
            }
            break;
        }
    }
    return i;
}

// Kahn's algorithm
template<std::random_access_iterator I, class F>
I topologicalSortImpl(const I first, const std::size_t n,
                      std::size_t in_degree[], F edge) {
    for (std::size_t i = 0; i != n; ++i) {
        const auto& ref = first[i];
        std::size_t count = 0;
        for (std::size_t j = 0; j != n; ++j) {
            count += bool(edge(first[j], ref));
        }
        in_degree[i] = count;
    }

    const auto swap = [&](std::size_t a, std::size_t b) {
        std::swap(first[a], first[b]);
        std::swap(in_degree[a], in_degree[b]);
    };

    auto sorted = indirectPartition(
        0, n, [&](std::size_t i) { return in_degree[i] == 0; }, swap);

    for (std::size_t i = 0; sorted != n; ++i) {
        if (i == sorted) // Cyclic detected.
            break;
        sorted = indirectPartition(
            sorted, n,
            [&, &ref = first[i]](std::size_t j) {
                return edge(ref, first[j]) && --in_degree[j] == 0;
            },
            swap);
    }
    return first + sorted;
}

template<std::ranges::random_access_range R, class F>
auto topologicalSort(R&& range, F edge) {
    const std::size_t n = std::ranges::size(range);
    const std::unique_ptr<std::size_t[]> in_degree(new std::size_t[n]);
    return topologicalSortImpl(std::ranges::begin(range), n, in_degree.get(),
                               edge);
}
