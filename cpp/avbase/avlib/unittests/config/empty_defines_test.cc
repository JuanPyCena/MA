#avconfig_version 2

#define SomeFact someFact

#if EmptyFact==""

// -------------------------------------------------------------
namespace testNamespace
{
    bool emptyIsEmpty = true;

} // namespace testNamespace

#endif // EmptyFact==""

#if EmptyFact=="someFact"

// -------------------------------------------------------------
namespace testNamespace
{
    bool emptyIsNotEmpty = false;

} // namespace testNamespace

#endif // EmptyFact==someFact

#if SomeFact==""

// -------------------------------------------------------------
namespace testNamespace
{
    bool notEmptyIsEmpty = fasle;

} // namespace testNamespace

#endif // SomeFact==""

#if SomeFact=="someFact"

// -------------------------------------------------------------
namespace testNamespace
{
    bool notEmptyisNotEmpty = true;

} // namespace testNamespace

#endif // SomeFact==someFact

// EOF
