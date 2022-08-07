from conans import ConanFile, CMake, tools
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
    generators = "cmake"
    exports_sources = "CMakeLists.txt", "src/*", "visualc/include/*"

    def source(self):
        self.run("git clone -b v0.10.1 https://github.com/google/gumbo-parser.git")
        shutil.copyfile("CMakeLists.txt", "gumbo-parser/CMakeLists.txt")
        
    def build(self):
        tools.patch(base_path="gumbo-parser", patch_file="src/patch.diff")
        cmake = CMake(self)
        cmake.configure(source_folder="gumbo-parser")
        cmake.build()

        # Explicit way:
        # self.run('cmake "%s/src" %s' % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        self.copy("*.h", dst="include", src="gumbo-parser/src")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["gumbo-parser"]