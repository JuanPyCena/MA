from datetime import datetime

import numpy as np
import sqlite3 as sql

from src.database.ADBInterface import ADBInterface


class ADBWriter(ADBInterface):

    def __init__(self, file: str):
        super().__init__(file)

    def _generateIMMInput(self) -> None:
        pass

    def writeIMMDataToDatabase(self) -> None:
        print("Creating copy of sd_track")
        self.connection.execute("CREATE TABLE sd_track_dfuse3 AS SELECT * FROM sd_track")
        self.connection.commit()
        print("Start writing to database")
        rec_nums = self.connection.execute("SELECT rec_num FROM sd_track").fetchall()
        counter = {target: 0 for target in self.targets}

        for rec_num, in rec_nums:
            target_id = self.connection.execute(
                "SELECT target_identification FROM sd_track WHERE rec_num={}".format(rec_num)).fetchall()[0][0]
            if target_id in self.data.keys():
                idx = counter[target_id]
                try:
                    pos_x = int(self.data[target_id][0][idx][0])
                    pos_y = -int(self.data[target_id][0][idx][3])
                    var_x = self.data[target_id][1][idx][0][0]
                    var_y = self.data[target_id][1][idx][3][3]
                    cov_xy = self.data[target_id][1][idx][3][0]
                    # time = datetime.timestamp(self.data[target_id][2][idx]) - self.data[target_id][2][idx].date().timestamp

                    self.connection.execute("UPDATE sd_track "
                                            "SET "
                                            "position_x={pos_x},"
                                            "position_y={pos_y},"
                                            "position_x_variance={var_x},"
                                            "position_y_variance={var_y},"
                                            "position_xy_covariance={cov_xy} "
                                            # "time_recording={time} "
                                            "WHERE rec_num={rec_num}"
                                            .format(pos_x=pos_x,
                                                    pos_y=pos_y,
                                                    var_x=var_x,
                                                    var_y=var_y,
                                                    cov_xy=cov_xy,
                                                    # time=time,
                                                    rec_num=rec_num))
                    counter[target_id] += 1
                except:
                    # self.connection.execute("DELETE FROM sd_track WHERE rec_num={rec_num}".format(rec_num=rec_num))
                    pass
            else:
                # self.connection.execute("DELETE FROM sd_track WHERE rec_num={rec_num}".format(rec_num=rec_num))
                pass
            self.connection.commit()

        print("Finished writing to database")
