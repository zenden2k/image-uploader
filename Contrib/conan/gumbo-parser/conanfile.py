import os

from conan import ConanFile
from conan.tools.files import save, load, patch, copy
from conan.tools.cmake import cmake_layout, CMake
from conan.tools.scm import Git

import shutil

class GumboParserConan(ConanFile):
    name = "gumbo-parser"
    version = "0.10.1"
    license = "Apache-2.0"
    url = "https://github.com/google/gumbo-parser"
    description = "An HTML5 parsing library in pure C99"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = {"shared": False}
    generators = "CMakeToolchain", "CMakeDeps", 
    exports_sources = "CMakeLists.txt", "src/*", "visualc/include/*"

    def source(self):
        git = Git(self)
        git.clone(url="https://github.com/google/gumbo-parser.git", target="./gumbo-parser", args=["-b", "v0.10.1"])
        #self.run("git clone -b v0.10.1 https://github.com/google/gumbo-parser.git")
        shutil.copyfile("src/CMakeLists.txt", "gumbo-parser/CMakeLists.txt")
        
    def build(self):
        patch(self, base_path="gumbo-parser", patch_file="src/patch.diff")
        cmake = CMake(self)
        cmake.configure(build_script_folder="gumbo-parser")
        cmake.build()

        # Explicit way:
        # self.run('cmake "%s/src" %s' % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)
    def layout(self):
        cmake_layout(self)

    def package(self):
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))
        copy(self, pattern="*.h", src=os.path.join(self.source_folder, "gumbo-parser/src"), dst=os.path.join(self.package_folder, "include"))
        copy(self, pattern="*.a", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, pattern="*.so", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, pattern="*.lib", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)
        copy(self, pattern="*.dll", src=self.build_folder, dst=os.path.join(self.package_folder, "bin"), keep_path=False)
        copy(self, pattern="*.dylib", src=self.build_folder, dst=os.path.join(self.package_folder, "lib"), keep_path=False)

        #self.copy("*.h", dst="include", src="gumbo-parser/src")
        #self.copy("*.lib", dst="lib", keep_path=False)
        #self.copy("*.dll", dst="bin", keep_path=False)
        #self.copy("*.dylib*", dst="lib", keep_path=False)
        #self.copy("*.so", dst="lib", keep_path=False)
        #self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["gumbo-parser"]