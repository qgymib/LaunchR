#ifndef LAUNCHR_UTILS_BOYER_MOORE_HPP
#define LAUNCHR_UTILS_BOYER_MOORE_HPP

#include <cstddef>
#include <optional>

namespace LR
{

/**
 * @brief Boyer-Moore that support binary search.
 */
struct BoyerMoore
{
    /**
     * @brief Constructor for Boyer-Moore
     * @param[in] pattern Pattern data.
     * @param[in] length Pattern length.
     */
    BoyerMoore(const void* pattern, size_t length);
    BoyerMoore(const BoyerMoore& orig);
    BoyerMoore& operator=(const BoyerMoore&);
    ~BoyerMoore();

    /**
     * @brief Search for the pattern in given binary data.
     * @param[in] data Data address.
     * @param[in] size Data length.
     * @return If found, return the matching start position. If not found, return null.
     */
    std::optional<size_t> Search(const void* data, size_t size);

    struct Data;
    struct Data* m_data;
};

} // namespace LR

#endif
