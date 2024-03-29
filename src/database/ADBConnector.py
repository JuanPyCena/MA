from datetime import datetime

import numpy as np
import sqlite3 as sql

from src.database.ADBInterface import ADBInterface

class ADBConnector(ADBInterface):

    def __init__(self, file: str):
        super().__init__(file)
        self._generateIMMInput()

    def writeIMMDataToDatabase(self) -> None:
        pass

    def _generateIMMInput(self) -> None:
        for target in self.targets:
            positions   = self._getTargetPositions(table="sd_mlat", target=target)
            covariances = self._getTargetCovariance(table="sd_mlat", target=target)
            timestamps  = self._getDateTime(table="sd_mlat", target=target)
            self.data = {target: (positions, covariances, timestamps)}