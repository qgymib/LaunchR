#ifndef LAUNCHR_UTILS_DSL_HPP
#define LAUNCHR_UTILS_DSL_HPP

#include <string>

namespace LR
{

namespace DSL
{

struct Tokenizer
{
    enum class Type
    {
        IDENT,
        STRING,
        ATTR,
        COLON,
        GT,
        LT,
        GE,
        LE,
        EQ,
        AND,
        OR,
        NOT,
        LPAREN,
        RPAREN,
        PIPE,
        END
    };

    struct Token
    {
        Type        type;  /* Token type. */
        std::string value; /* Token value. */
    };

    /**
     * @brief Construct tokenizer.
     * @param[in] str UTF-8 string.
     */
    explicit Tokenizer(const char* str);
    ~Tokenizer();



    struct Data;
    struct Data* m_data;
};

struct Parser
{
};

} // namespace DSL

} // namespace LR

#endif
