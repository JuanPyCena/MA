from src.testbench.TestbenchPlotter import TestbenchPlotter
from src.testbench.TestbenchIMM import TestbenchIMM

def main():
    test_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_linear_uniform_motion.csv"
    imm_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\src\\testbench\\imm_data"
    plotter = TestbenchPlotter(test_data_file, imm_data_file)

    plotter.plot_test_data()
    plotter.plot_imm_data()

if __name__ == "__main__":
    main()


