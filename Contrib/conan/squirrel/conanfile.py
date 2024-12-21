import os
import shutil

from conan import ConanFile
from conan.tools.files import save, load, patch, copy
from conan.tools.cmake import cmake_layout, CMake
from conan.tools.scm import Git

class SquirrelConan(ConanFile):
    name = "squirrel"
    version = "3.0.0"
    license = "Apache-2.0"
    url = "http://squirrel-lang.org/"
    description = "Squirrel is a high level imperative, object-oriented programming language."
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "CMakeLists.txt", "squirrel/*", "sqstdlib/*", "include/*"

    def source(self):
        git = Git(self)
        git.clone(url="https://github.com/zenden2k/squirrel.git", target="./squirrel")
        #self.run("git clone -b v0.10.1 https://github.com/google/gumbo-parser.git")
        shutil.copyfile("CMakeLists.txt", "squirrel/CMakeLists.txt")

    def build(self):
        cmake = CMake(self)
        cmake.configure(build_script_folder="squirrel")
        cmake.build()

        # Explicit way:
        # self.run('cmake "%s/src" %s' % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        copy(self, pattern="*.h", src=os.path.join(self.source_folder, "include"), dst=os.path.join(self.package_folder, "include"))
        copy(self, pattern="*.a", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, pattern="*.so", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, pattern="*.lib", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, pattern="*.dll", src=self.build_folder, dst=os.path.join(self.package_folder, "bin"), keep_path=False)
        copy(self, pattern="*.dylib", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)



    def package_info(self):
        self.cpp_info
        self.cpp_info.components["sqstdlib"].libs = ["sqstdlib"]      
        self.cpp_info.components["sqstdlib"].requires = ["Squirrel"]
        self.cpp_info.components["sqstdlib"].defines= ["_SQ64"]
        self.cpp_info.components["Squirrel"].libs = ["squirrel"] 
        self.cpp_info.components["Squirrel"].defines= ["_SQ64"]