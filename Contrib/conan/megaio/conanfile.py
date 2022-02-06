from conans import ConanFile, CMake, tools
import shutil

class MegaioConan(ConanFile):
    name = "megaio"
    version = "3.5.2"
    license = "(c) Mega Limited"
    author = "Sergey Svistunov zenden2k@gmail.com"
    url = "https://github.com/meganz/sdk"
    description = "Mega SDK"
    topics = ("mega.io", "cloud")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True, "libcurl:with_libssh2": True}
    generators = "cmake_find_package", "cmake_paths"
    exports_sources = "src/*"
    requires = ["openssl/1.1.1l", 
						"zlib/1.2.11",
						"libcurl/7.79.1",
						"cryptopp/8.5.0",
						"c-ares/1.17.2",
						"libuv/1.42.0",
						"libmediainfo/21.09",
						"gtest/1.10.0",
						"sqlite3/3.36.0"
                ]

    def source(self):
        self.run("git clone -b v3.5.2 https://github.com/meganz/sdk.git")
        shutil.copyfile("src/CMakeLists.txt", "sdk/src/CMakeLists.txt")

        
    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def build(self):
        tools.patch(base_path="sdk", patch_file="src/mega.diff")
        cmake = CMake(self)
        cmake.configure(source_folder="sdk/src")
        cmake.build()

        # Explicit way:
        # self.run('cmake %s/hello %s'
        #          % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        self.copy("*.h", dst="include", src="sdk/include")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["megaio"]

