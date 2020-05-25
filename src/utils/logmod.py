# Felix Sonnleitner, 26.12.2017
# Python 3.6
# Loggind Module used to create and read logfiles.

import os

from datetime import datetime
from datetime import date

WHITEHEADER = "                     "

class Logger:
    '''This Class is used to create logfiles and to to log statements into them'''
    def __init__(self):
        self.hour       = None   # The current hour, used to decide if a new logfile is to be created
        self.fileName   = None   # The name of the current logfile
        self.logContent = None   # A list containing the lines of a logfile which was read, also includes the linebreak of each line.
        self.file       = None   # Current logfile

        self.create_log_file()   # create the initial logfile when starting the program

    #################################################################################

    def write_to_log(self, logInput, consoleOutput=False, timestamp=True):
        """
        :param logInput: string
                    This input is statet into the latest logfile. It is prefixed with the current date and time.
               consoleOutput: bool
                    If it is desired to print the the logstatements to the console aswell this can be defined
                    via this Prameter. Default = False
                timestamp: bool
                    If desired the ouput can be without a timestamp but with a series of whitespaces
                    Default = True
        """
        now = str(datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
        if timestamp:
            logStatement = now + ": " + logInput
        else:
            logStatement = WHITEHEADER + logInput

        # Console output
        if consoleOutput:
            print(logStatement)

        # If new hour create a new logfile
        if self.hour is not datetime.now().hour:
            self.file.close()
            self.create_log_file()

        try:
            with open(self.fileName, 'a') as file:
                file.write(logStatement + "\n")
        except:
            print("ERROR: Could not write to log")

    #################################################################################

    def read_from_log(self, logFile):
        """
        :param logFile: string
                    Defines the File that should be read.
        :return A list of strings. Each Element is a line of the logfile. Containing the linebreak
        """
        try:
            file = open(logFile, "r")
            self.logContent = file.readlines()
        except:
            print("ERROR: Could not read from log " + logFile)

        return self.logContent

    #################################################################################

    def create_log_file(self, logName=None):
        """
        :param logName: string
                    If a custom logfileName is desired this parameter defines the name for the file
                    Default is None, therfor the files is named after the current date + the current hour
                    Example: 2017-12-26_21h.log
         """
        self.hour = str(datetime.now().hour)

        if logName is not None:
            self.fileName = logName
        else:
            fileName = str(date.today().isoformat()) + "_" + self.hour + "h.log"
            here    = os.path.dirname(os.path.realpath(__file__))
            subdir  = "../../log"
            logpath = os.path.join(here, subdir, fileName)
            if not os.path.exists(os.path.join(here,subdir)):
                os.mkdir(os.path.join(here,subdir))
            self.fileName = logpath

        try:
            self.file = open(self.fileName, 'a')
        except:
            print("ERROR: Could not create logfile " + self.fileName)

    #################################################################################

    def delete_old_logfiles(self,logName=None):
        """
        :param logName: string
                    If a custom logfileName is desired this parameter defines the name for the file
                    Default is None, therfor the files deleted are named after the current date + hour.inRange(0,25)
                    Example: 2017-12-26_21h.log
        """
        limit = date.today().day - 1
        here = os.path.dirname(os.path.realpath(__file__))
        subdir = "log"
        fileNames = []
        ret = False

        if logName is not None:
            fileName = logName
        else:
            for h in range(0,25):
                fileNames.append(str(date.today().year) + "-" + str(date.today().month) + "-" + str(limit) + "_" + str(h) + "h.log")

        if len(fileNames) != 0:
            for fileName in fileNames:
                try:
                    if os.path.exists(os.path.join(here, subdir, fileName)):
                        os.remove(os.path.join(here, subdir, fileName))
                        ret = True
                except:
                    ret = False
        else:
            if os.path.exists(os.path.join(here, subdir, fileName)):
                try:
                    os.remove(os.path.join(here, subdir, fileName))
                    ret = True
                except:
                    ret = False

        return ret
#EOF