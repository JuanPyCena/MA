from src.testbench.TestbenchPlotter import TestbenchPlotter
from src.testbench.TestbenchIMM import TestbenchIMM
# from src.testbench.TestbenchIMM_filterpy import TestbenchIMM
from src.utils.DataFileInterface import DataFilteInterface as DataReadWrite
from src.testbench.TestbenchIMM_database import TestbenchIMMDataBase
from src.database.ADBConnector import ADBConnector as DataBase
from src.database.ADBWriter import ADBWriter as DBWriter

def main():

    test_data_file = "D:\\programming\\masterarbeit\\data\\test_eval_test_run_info.adb"
    writer = DBWriter(test_data_file)

    # imm_data_file = "D:\\programming\\masterarbeit\\\src\\testbench\\imm_data\\imm_data_RYR8MJ.csv"
    # imm = TestbenchIMMDataBase(test_data_file, imm_data_file, "RYR8MJ", writer)
    # imm.run()
    # plotter = TestbenchPlotter(
    #     "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_acc_then_const.csv",
    #     imm_data_file)
    # plotter.plot_imm_data()

    for target in DataBase("D:\\programming\\masterarbeit\\data\\test_eval_test_run_info.adb").targets:
        imm_data_file = "D:\\programming\\masterarbeit\\\src\\testbench\\imm_data\\imm_data_{}.csv".format(target)
        imm = TestbenchIMMDataBase(test_data_file, imm_data_file, target, writer)
        imm.run()
        plotter = TestbenchPlotter("D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_acc_then_const.csv", imm_data_file)
        plotter.plot_imm_data()

    #writer.writeIMMDataToDatabase()
if __name__ == "__main__":
    main()