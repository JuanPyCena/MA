#avconfig_version 3

// SWE-3465 - fix overriding an inherited parameter
ports.someport = 1;

// SWE-3496 - bug with references and inheritance
instance1.port_4 = 5;

// -------------------------------------------------------------
namespace ports_base
{
    //! no help
    uint someport = 10;

    //! no help
    uint someotherport = 2;

} // namespace ports_base

// -------------------------------------------------------------
namespace ports : ports_base
{
} // namespace ports

// -------------------------------------------------------------
namespace template
{
    //! no help
    uint port_1 = 3;

    //! no help
    uint & port_2 = ports.someport;

    //! no help
    uint & port_3 = ports.someotherport;

    //! no help
    uint & port_4 = ports.someotherport;

    //! no help
    //!
    //! \cmdline p5
    uint & port_5 = ports.someotherport;

} // namespace template

// -------------------------------------------------------------
namespace instance1 : template
{
    //! no help
    uint & port_1 = ports.someport;

    //! no help
    uint port_2 = 4;

} // namespace instance1

// EOF
