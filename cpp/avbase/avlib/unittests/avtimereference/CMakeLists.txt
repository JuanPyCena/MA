###############################################################################
##
## Package:   
## Copyright: AviBit data processing GmbH, 2001-2014
##
## Module:    
##
###############################################################################

#   \file
#   \author  Ralf Gamillscheg, r.gamillscheg@avibit.com
#   \brief   CMake file for /home/users/ralf unittests


include(${AV_BUILD_SYSTEM_DIR}/cmake/CMakeLists_base.txt)

#------------------------------------------------------------------------------


#------------------------------------------------------------------------------

av_add_qtestlib_unittests(
    avreplaystatenotifier
    avtimereferenceadapterlocal
    avtimereferenceadaptersharedmemory
    avtimereferenceadapterserver
    avtimereferenceholder
    TEST_GROUP_NAME avlib
    DEPENDING_LIBRARIES avlib avunittesthelperlib avlibunittesthelperlib
)
#------------------------------------------------------------------------------
#! eof
