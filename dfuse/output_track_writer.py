import sqlite3 as sql
from datetime import datetime

class OutputTrackWriter(object):

    def __init__(self, file: str):
        self._data = dict()

        self._connection = self._create_connection(file)

    @property
    def data(self) -> dict:
        return self._data

    @data.setter
    def data(self, data) -> None:
        self._data = data

    @property
    def connection(self) -> sql.Connection:
        return self._connection

    def writeIMMDataToDatabase(self) -> None:
        self.connection.execute("DROP TABLE IF EXISTS sd_track_dfuse3")
        self.connection.commit()
        print("Creating copy of sd_track")
        self.connection.execute("CREATE TABLE sd_track_dfuse3 AS SELECT * FROM sd_track")
        self.connection.commit()
        print("Start writing to database")
        rec_nums = self.connection.execute("SELECT rec_num FROM sd_track").fetchall()
        counter = {target: 0 for target in self.data.keys()}

        for rec_num, in rec_nums:
            target_id = self.connection.execute(
                "SELECT target_identification FROM sd_track WHERE rec_num={}".format(rec_num)).fetchall()[0][0]
            if target_id in self.data.keys():
                idx = counter[target_id]
                # try:
                pos_x  = int(self.data[target_id][idx][0][0])
                pos_y  = -int(self.data[target_id][idx][0][3])
                var_x  = self.data[target_id][idx][1][0][0]
                var_y  = self.data[target_id][idx][1][3][3]
                cov_xy = self.data[target_id][idx][1][3][0]
                time   = datetime.timestamp(self.data[target_id][idx][2])
                probs  = str([prob for prob in self.data[target_id][idx][3]]).strip("[]").replace(", ", "|")
                plot_ids = str(self.data[target_id][idx][4]).strip("[]").replace(",", "|")

                query = "UPDATE sd_track " \
                        "SET " \
                        "position_x={pos_x},"\
                        "position_y={pos_y},"\
                        "position_x_variance={var_x},"\
                        "position_y_variance={var_y},"\
                        "position_xy_covariance={cov_xy},"\
                        "plot_id_chain=\"{plot_ids}\","\
                        "track_comments=\"{probs}\" " \
                        "WHERE rec_num={rec_num}"\
                        .format(pos_x=pos_x,
                                pos_y=pos_y,
                                var_x=var_x,
                                var_y=var_y,
                                cov_xy=cov_xy,
                                rec_num=rec_num,
                         probs=probs,
                         plot_ids=plot_ids)
                self.connection.execute(query)
                counter[target_id] += 1
                # except:
                #     #self.connection.execute("DELETE FROM sd_track WHERE rec_num={rec_num}".format(rec_num=rec_num))
                #     pass
            else:
                self.connection.execute("DELETE FROM sd_track WHERE rec_num={rec_num}".format(rec_num=rec_num))
                pass
            self.connection.commit()

        print("Finished writing to database")

    def _create_connection(self, file) -> sql.Connection:
        try:
            return sql.connect(file)
        except:
            raise ConnectionRefusedError("Cannot connect to \"{}\"".format(file))