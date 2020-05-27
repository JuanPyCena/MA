#!
#! Package:   AVCOMMON - AviBit Common Libraries
#! Copyright: AviBit data processing GmbH, 2001-2010
#! Author:    Dr. Thomas Leitner, t.leitner@avibit.com
#!
#! Module:    AVLIB - AviBit Library
#!
#! --------------------------------------------------------------------------

set(module avlib)

#-----------------------------------------------------------------------------

include_directories(${PROJECT_SOURCE_DIR}/avlibdeprecated)
include_directories(${PROJECT_SOURCE_DIR}/avuilib)

#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------

set(headers
    avprocesswatchdog.h
    avversionreader.h
    avpackageinfo.h
    avsharedmemory.h
    avconsolepropertyobject.h
    avtimereference/avreplaystatenotifier.h
    avtimereference/avtimereferenceadapter.h
    avtimereference/avtimereferenceadapterconfig.h
    avtimereference/avtimereferenceadapterlocal.h
    avtimereference/avtimereferenceadaptersharedmemory.h
    avtimereference/avtimereferenceadapterserver.h
    avtimereference/avtimereferenceadapterserverconfig.h
    avtimereference/avtimereferenceconfigdeprecated.h
    avtimereference/avtimereferenceholder.h
    avtimereference/av_unittest_timereferenceholder.h
    avenumhelpers.h
    avexecstandbycheckerunittestdriver.h
    avexecstandbycheckerconfig.h
    avexecstandbychecker.h
    avconfig2projectfactsconfig.h
    av4dtrajectory.h
    avclienthandlercallbackiface.h
    avconfig.h
    avconfig2.h
    avconfig2adapters.h
    avconfig2builtinparams.h
    avconfig2classic.h
    avconfig2classic_macros.h
    avconfig2cstyle.h
    avconfig2exporterfactory.h
    avconfig2interfaces.h
    avconfig2loadeddata.h
    avconfig2metadata.h
    avconfig2restrictions.h
    avconfig2saver.h
    avconfig2storagelocation.h
    avconfig2types.h
    avconfig2xml.h
    avconfig2sharedconfigholder.h
    avconfiglist.h
    avconsole.h
    avconsole_int.h
    avconsole_command.h
    avconsole_command_print_qobject.h
    avcrc.h
    avcstring.h
    avcvariant.h
    avdaemon.h
    avdataencoderbase.h
    avdataencoderrawpacket.h
    avdataframingdelimiters.h
    avdataframinglength.h
    avdatastream.h
    avdatetime.h
    avdebughelpers.h
    avdeprecate.h
    avdescriptionbuilder.h
    avdomhelper.h
    avdommanager.h
    avdomvariables.h
    avecfinalformatframing.h
    avenumtable.h
    avenvironment.h
    avenvstring.h
    avexithandler.h
    avexplicitsingleton.h
    avexpressionmetadata.h
    avexpressionparser.h
    avexpressions.h
    avfactory.h
    avfilewatcher.h
    avflightplanprovider.h
    avfromtostring.h
    avimagemaskbase.h
    avipaddress.h
    avlicense.h
    avlog.h
    avlog/avlogmetadata.h
    avlog/avlogmonitor.h
    avlog/avlogmatcher.h
    avlog/avlogindentmanipulator.h
    avlog/avlogindent.h
    avlogmuter.h
    avmacros.h
    avmatrix.h
    avmd5.h
    avmisc.h
    avpath.h
    avpriorityqueue.h
    avprocessstate.h
    avprocessstate_int.h
    avprocessstatedesc.h
    avprocessstatepoller.h
    avprofiling.h
    avprofilingreporters.h
    avprototype.h
    avprototypefactory.h
    avreadwritemutex.h
    avrule.h
    avruleconfig.h
    avrwydesignatorprovider.h
    avscreenshot.h
    avserverlist.h
    avshare.h
    avshm_varnames.h
    avshmcheckblock.h
    avsignalhandler.h
    avsingleton.h
    avsnprintf.h
    avstatistic.h
    avstdinreaderthread.h
    avstringmap.h
    avstringpairlist.h
    avsubimagebase.h
    avsysprof.h
    avsysprofconfig.h
    avsystemid.h
    avthread.h
    avthreadsingleton.h
    avtimer.h
    avtimereference.h
    avtostringrepresentation.h
    avtwowayhash.h
    avuniqueidgenerator.h
    avuniquenumbergenerator.h
    avvehicle.h
    avthreadobjectmanager.h
    avtypesafesignalspy.h
    avbytearrayhandler.h
    avdescriptionbuildersimple.h
    avmutexlocker.h
    avscreenshot.h
    avsignalpublisher.h
    avsimplecommandrunner.h
    avsystemstatus.h
    timeout_cursor.xbm
)

#-----------------------------------------------------------------------------

set(sources
    avprocesswatchdog.cpp
    avversionreader.cpp
    avpackageinfo.cpp
    avsharedmemory.cpp
    avconsolepropertyobject.cpp
    avtimereference/avreplaystatenotifier.cpp
    avtimereference/avtimereferenceadapter.cpp
    avtimereference/avtimereferenceadapterconfig.cpp
    avtimereference/avtimereferenceadapterlocal.cpp
    avtimereference/avtimereferenceadaptersharedmemory.cpp
    avtimereference/avtimereferenceadapterserver.cpp
    avtimereference/avtimereferenceadapterserverconfig.cpp
    avtimereference/avtimereferenceconfigdeprecated.cpp
    avtimereference/avtimereferenceholder.cpp
    avtimereference/av_unittest_timereferenceholder.cpp
    avexecstandbycheckerconfig.cpp
    avexecstandbychecker.cpp
    avexecstandbycheckerunittestdriver.cpp
    avconfig2projectfactsconfig.cpp
    av4dtrajectory.cpp
    avconfig.cpp
    avconfig2.cpp
    avconfig2adapters.cpp
    avconfig2builtinparams.cpp
    avconfig2classic.cpp
    avconfig2cstyle.cpp
    avconfig2exporterfactory.cpp
    avconfig2interfaces.cpp
    avconfig2loadeddata.cpp
    avconfig2metadata.cpp
    avconfig2restrictions.cpp
    avconfig2saver.cpp
    avconfig2storagelocation.cpp
    avconfig2xml.cpp
    avconsole.cpp
    avconsole_int.cpp
    avconsole_command.cpp
    avconsole_command_print_qobject.cpp
    avcstring.cpp
    avcvariant.cpp
    avdaemon.cpp
    avdataencoderrawpacket.cpp
    avdataframingdelimiters.cpp
    avdataframinglength.cpp
    avdatastream.cpp
    avdatetime.cpp
    avdebughelpers.cpp
    avdeprecate.cpp
    avdescriptionbuilder.cpp
    avdomhelper.cpp
    avdommanager.cpp
    avdomvariables.cpp
    avecfinalformatframing.cpp
    avenumtable.cpp
    avenvironment.cpp
    avenvstring.cpp
    avexpressionmetadata.cpp
    avexpressionparser.cpp
    avexpressions.cpp
    avfilewatcher.cpp
    avfromtostring.cpp
    avimagemaskbase.cpp
    avipaddress.cpp
    avlicense.cpp
    avlog.cpp
    avlog/avlogmetadata.cpp
    avlog/avlogmonitor.cpp
    avlog/avlogmatcher.cpp
    avlog/avlogindentmanipulator.cpp
    avlog/avlogindent.cpp
    avlogmuter.cpp
    avmacros.cpp
    avmd5.cpp
    avmisc.cpp
    avpath.cpp
    avprocessstate.cpp
    avprocessstate_int.cpp
    avprocessstatedesc.cpp
    avprocessstatepoller.cpp
    avprofiling.cpp
    avprofilingreporters.cpp
    avprototype.cpp
    avrule.cpp
    avruleconfig.cpp
    avscreenshot.cpp
    avserverlist.cpp
    avsignalhandler.cpp
    avshmcheckblock.cpp
    avstdinreaderthread.cpp
    avstringpairlist.cpp
    avsubimagebase.cpp
    avsysprof.cpp
    avsystemid.cpp
    avthread.cpp
    avtimer.cpp
    avtimereference.cpp
    avtostringrepresentation.cpp
    avuniqueidgenerator.cpp
    avvehicle.cpp
    avthreadobjectmanager.cpp
    avbytearrayhandler.cpp
    avdescriptionbuildersimple.cpp
    avmutexlocker.cpp
    avscreenshot.cpp
    avsimplecommandrunner.cpp
    avsystemstatus.cpp
)

#-----------------------------------------------------------------------------
# add module-wide QT_USE_QSTRINGBUILDER definition (reference: http://doc.qt.io/qt-5/qstring.html)
# so that '+' will automatically be performed as the QStringBuilder '%' everywhere.

add_definitions(-DQT_USE_QSTRINGBUILDER)

#-----------------------------------------------------------------------------
# add platform specific sources

if (UNIX)
    set(headers ${headers} avsharedmemory_private_unix.h)
    set(sources ${sources} avsharedmemory_private_unix.cpp)
endif (UNIX)

if (WIN32)
    set(headers ${headers} avsharedmemory_private_windows.h)
    set(sources ${sources} avsharedmemory_private_windows.cpp)
    # in order to make MSVC stop complaining about deprecated POSIX functions
    SET_SOURCE_FILES_PROPERTIES(avdaemon.cpp PROPERTIES COMPILE_DEFINITIONS _CRT_NONSTDC_NO_WARNINGS)
endif (WIN32)

add_executable(${module} ${headers} ${sources} ${ui_sources})
target_link_libraries(${module} ${QT5_LIBRARIES})

if(WIN32)
target_link_libraries(${module} ws2_32) #link winsock2
target_link_libraries(${module} netapi32) #link Netbios
target_link_libraries(${module} dbghelp) # for minidump creation
endif(WIN32)

#! eof
