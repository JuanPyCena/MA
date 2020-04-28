from src.testbench.TestbenchPlotter import TestbenchPlotter
from src.testbench.TestbenchIMM import TestbenchIMM
from src.utils.DataFileInterface import DataFilteInterface as DataReadWrite

def main():
    test_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_linear_uniform_motion.csv"
    imm_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\src\\testbench\\imm_data\\imm_data_linear_uniform_motion.csv"
    test_data = DataReadWrite.read_test_data(test_data_file)

    imm = TestbenchIMM(test_data, imm_data_file)
    imm.run()
    plotter = TestbenchPlotter(test_data_file, imm_data_file)

    plotter.plot_test_data()
    plotter.plot_imm_data()
    plotter.plot_combined()

if __name__ == "__main__":
    main()


