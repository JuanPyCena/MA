from conans import ConanFile, CMake
import os

class AVBaseTest(ConanFile):
    generators = "cmake", "virtualenv"

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder = os.path.join(self.source_folder, ".."))
        #cmake.build()

    def test(self):
        cmake = CMake(self)
        cmake.test()
