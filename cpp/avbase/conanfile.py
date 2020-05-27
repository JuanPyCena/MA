from conans.model.conan_file import ConanFile
from conans import tools
import os

class AVBase(ConanFile):
    name = "avbase"
    settings = "os", "build_type", "arch"
    generators = "cmake", "virtualenv"

    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto"
    }

    build_requires = "avconantools/1.0.0@official/release"
    requires = "build-system/9.1.0@official/release", "qt5/5.9.1@official/release"

    def build(self):
        import avconantools
        avconantools.build(self)

    def package(self):
        import avconantools
        avconantools.package(self)

    def package_id(self):
        self.info.requires.full_package_mode() #Depend on exact package/options (e.g. compiler)

    def package_info(self):
        self.env_info.path.append(os.path.join(self.package_folder, "bin"))
        self.env_info.LD_LIBRARY_PATH.append(os.path.join(self.package_folder, "lib"))
        self.env_info.PYTHONPATH.append(os.path.join(self.package_folder, "avlib/bbtestlib"))
        self.env_info.AVBASE_DIR = self.package_folder
        self.env_info.AV_PLUGIN_PATH.append(os.path.join(self.package_folder, "lib/plugins"))
