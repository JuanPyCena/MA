#avconfig_version 2

// -------------------------------------------------------------
namespace processconfig
{
    // -------------------------------------------------------------
    namespace thread_monitor
    {
        //! The hard time limit when a deadlock is assumed. Set 0 to disable.
        //!
        //! \suggested 30000
        //! \optional 1
        int deadlock_limit_msec = 5000;

        //! The threshold time limit when a process is assumed to be in danger
        //! of mistakenly being considered in a deadlock state.
        //!
        //! \suggested 15000
        //! \optional 1
        int info_limit_msec = 3000;

    } // namespace thread_monitor

} // namespace processconfig

// EOF
