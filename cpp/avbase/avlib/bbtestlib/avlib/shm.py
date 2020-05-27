# -*- coding: utf-8 -*-
# ----------------------------------------------------------------------------
# Copyright: AviBit GmbH, 2001-2015
# ----------------------------------------------------------------------------

"""
.. moduleauthor:: Christian Muschick <c.muschick@avibit.com>
.. moduleauthor:: Andrease Eibler <a.eibler@avibit.com>
"""

# Python 3 compatibility
from __future__ import print_function, unicode_literals, absolute_import, division
from future.builtins import *

from avtestlib.staging.context import ProcessContext
from avtestlib import test

####################################################################################################

class ShmAccess(object):
    """
    Provides functionality to access the shared memory "parameter" segment. Note that the "time reference"
    segment is distinct and not covered by this class.

    Access is handled via the lproc executable. No explicit error checking is done, it is assumed that the lproc
    correctly reports errors via connection.printError().

    The lproc is lazily started, via the python property mechanism.
    """

    LPROC_EXECUTABLE_NAME = "lproc"

    ######################################################################################

    def __init__(self, context):
        """
        Parameters
        ----------
        context : ProcessContext
            Usually will at least determine the application name which is used for SHM access.
        """
        self.context = context
        self.__lproc = self.context.create("lproc_shmaccess", self.LPROC_EXECUTABLE_NAME)
        self.__lproc.arguments += ["-keep_running", "1"]

    ######################################################################################

    def create_segment(self, process_name):
        self._lproc.console.execute("createShmSegment " + process_name)

    ######################################################################################

    def set_variable(self, process_name, variable, value):
        self._lproc.console.execute("setShmVariable " + process_name + " " + variable + " " + value)

    ######################################################################################

    def get_variable(self, process_name, variable):
        return self._lproc.console.execute("getShmVariable " + process_name + " " + variable)

    ######################################################################################

    def segment_exists(self, process_name):
        return self._lproc.console.execute("isShmSegmentPresent " + process_name) == "1"

    ######################################################################################

    def list_segments(self):
        return self._lproc.console.execute("listShmSegments")

    ######################################################################################

    @property
    def _lproc(self):
        """
        lproc is started lazily when the first SHM operation is requested.
        """
        if not self.__lproc.running: self.__lproc.start()
        return self.__lproc

# end of file
