#include <array>
#include <cstdint>
#include <vector>
#include "BoyerMoore.hpp"

#ifdef _MSC_VER
#include <stddef.h> // For ptrdiff_t
typedef ptrdiff_t ssize_t;
#endif

using namespace LR;

typedef std::array<size_t, 256> CharShiftTable;
typedef std::vector<size_t>     SuffixShiftTable;

struct BoyerMoore::Data
{
    std::vector<uint8_t> pattern;         /* Query pattern. */
    CharShiftTable       badCharShift;    /* Shift of bad char. */
    SuffixShiftTable     goodSuffixShift; /* Shift of good suffix. */
    size_t               m;               /* Query pattern length. */
};

static void BuildBadCharTable(CharShiftTable& table, const uint8_t* pat, size_t m)
{
    table.fill(m); // 默认最大偏移 m
    for (size_t i = 0; i + 1 < m; ++i)
    {
        table[pat[i]] = m - 1 - i;
    }
}

static void BuildGoodSuffixTable(SuffixShiftTable& table, const uint8_t* pat, size_t m)
{
    table.resize(m + 1);
    std::vector<size_t> border(m + 1);
    size_t              i = m;
    size_t              j = m + 1;
    border[i] = j;

    while (i > 0)
    {
        while (j <= m && pat[i - 1] != pat[j - 1])
        {
            if (table[j] == 0)
                table[j] = j - i;
            j = border[j];
        }
        --i;
        --j;
        border[i] = j;
    }

    size_t k = border[0];
    for (i = 0; i <= m; ++i)
    {
        if (table[i] == 0)
            table[i] = k;
        if (i == k)
            k = border[k];
    }
}

BoyerMoore::BoyerMoore(const void* pattern, size_t length)
{
    m_data = new Data;
    const uint8_t* pat = static_cast<const uint8_t*>(pattern);
    m_data->pattern.assign(pat, pat + length);
    m_data->m = length;

    BuildBadCharTable(m_data->badCharShift, pat, length);
    BuildGoodSuffixTable(m_data->goodSuffixShift, pat, length);
}

BoyerMoore::BoyerMoore(const BoyerMoore& orig)
{
    m_data = new Data;
    *m_data = *orig.m_data;
}

BoyerMoore& BoyerMoore::operator=(const BoyerMoore& orig)
{
    if (m_data != nullptr)
    {
        delete m_data;
    }
    m_data = new Data;
    *m_data = *orig.m_data;
    return *this;
}

BoyerMoore::~BoyerMoore()
{
    delete m_data;
}

std::optional<size_t> BoyerMoore::Search(const void* data, size_t size)
{
    if (!m_data || m_data->m == 0 || size < m_data->m)
    {
        return std::nullopt;
    }

    const uint8_t* text = static_cast<const uint8_t*>(data);
    const uint8_t* pat = m_data->pattern.data();
    size_t         m = m_data->m;

    size_t s = 0; /* Current offset. */
    while (s <= size - m)
    {
        ssize_t j = m - 1;

        while (j >= 0 && pat[j] == text[s + j])
        {
            --j;
        }

        if (j < 0)
        {
            return s; /* Match success. */
        }

        size_t badCharShift = m_data->badCharShift[text[s + j]];
        size_t goodSufShift = m_data->goodSuffixShift[j + 1];
        s += std::max(badCharShift, goodSufShift);
    }
    return std::nullopt;
}
