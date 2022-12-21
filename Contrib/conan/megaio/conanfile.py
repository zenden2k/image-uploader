from conans import ConanFile, CMake, tools
import shutil

class MegaioConan(ConanFile):
    name = "megaio"
    version = "3.5.2"
    license = "(c) Mega Limited"
    author = "mega.nz"
    url = "https://github.com/meganz/sdk"
    description = "Mega SDK"
    topics = ("mega.nz", "cloud")
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False], 
        "fPIC": [True, False], 
        "UseCryptopp": [True, False],
        "UseOpenSsl": [True, False], 
        "UseCurl": [True, False], 
        "UseSqlite":  [True, False], 
        "UseMediainfo": [True, False],
        "UseFreeImage": [True, False],
        "UseSodium": [True, False],
        "EnableSync": [True, False],
        "EnableChat": [True, False],
        "HaveFfmpeg": [True, False],
        "UseWebrtc": [True, False],
        "UseLibuv": [True, False],
        "UseQt": [True, False],
        "UsePdfium": [True, False],
    }
    default_options = {
        "shared": False, 
        "fPIC": True, 
        "UseCryptopp": True,
        "UseOpenSsl": True,
        "UseCurl": True,
        "UseSqlite": True,
        "UseMediainfo": True,
        "UseFreeImage": False,
        "UseSodium": False,
        "EnableSync": True,
        "EnableChat": False,
        "HaveFfmpeg": False,
        "UseWebrtc": False,
        "UseLibuv": False,
        "UseQt": False,
        "UsePdfium": False,
        "libcurl:with_libssh2": True
    }
    generators = "cmake", "cmake_find_package", "cmake_paths"
    exports_sources = "src/*"
    requires = [
                    "zlib/1.2.11",
                    "c-ares/1.17.2",
                    "gtest/1.10.0",
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
        cmake.definitions["USE_CRYPTOPP"] = 1 if self.options.UseCryptopp else 0
        cmake.definitions["USE_OPENSSL"] = 1 if self.options.UseOpenSsl else 0
        cmake.definitions["USE_CURL"] = 1 if self.options.UseCurl else 0
        cmake.definitions["USE_SQLITE"] = 1 if self.options.UseSqlite else 0
        cmake.definitions["USE_MEDIAINFO"] = 1 if self.options.UseMediainfo else 0
        cmake.definitions["USE_FREEIMAGE"] = 1 if self.options.UseFreeImage else 0
        cmake.definitions["USE_SODIUM"] = 1 if self.options.UseSodium else 0
        cmake.definitions["ENABLE_SYNC"] = 1 if self.options.EnableSync else 0
        cmake.definitions["ENABLE_CHAT"] = 1 if self.options.EnableChat else 0
        cmake.definitions["HAVE_FFMPEG"] = 1 if self.options.HaveFfmpeg else 0
        cmake.definitions["USE_WEBRTC"] = 1 if self.options.UseWebrtc else 0
        cmake.definitions["USE_LIBUV"] = 1 if self.options.UseLibuv else 0
        cmake.definitions["USE_QT"] = 1 if self.options.UseQt else 0
        cmake.definitions["USE_PDFIUM"] = 1 if self.options.UsePdfium else 0
        
        cmake.configure(source_folder="sdk/src")
        cmake.build()

        # Explicit way:
        # self.run('cmake %s/hello %s'
        #          % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)
        
    def requirements(self):
        if self.options.UseCryptopp:
            self.requires("cryptopp/8.5.0")
        if self.options.UseOpenSsl:
            self.requires("openssl/1.1.1l")
        if self.options.UseCurl:
            self.requires("libcurl/7.79.1")
        if self.options.UseSqlite:
            self.requires("sqlite3/3.36.0")
        if self.options.UseMediainfo:
            self.requires("libmediainfo/21.09")
        #if self.options.UseFreeImage:
        #if self.options.UseSodium:
        #if self.options.UseWebrtc:
        if self.options.UseLibuv:
            self.requires("libuv/1.42.0")
        #if self.options.UseQt
        #if self.options.UsePdfium
        

    def package(self):
        self.copy("*.h", dst="include", src="sdk/include")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.dylib*", dst="lib", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["megaio"]
        if self.options.UseMediainfo:
            self.cpp_info.defines.append("USE_MEDIAINFO")
        if self.options.UseSqlite:    
            self.cpp_info.defines.append("USE_SQLITE")
        if self.options.UseCryptopp:    
            self.cpp_info.defines.append("USE_CRYPTOPP")
        if self.options.UseOpenSsl:
            self.cpp_info.defines.append("USE_OPENSSL")
        if self.options.UseOpenSsl:
            self.cpp_info.defines.append("USE_CURL")
        if self.options.UseSodium:
            self.cpp_info.defines.append("USE_SODIUM")
        if self.options.EnableSync:   
            self.cpp_info.defines.append("ENABLE_SYNC")
        if self.options.EnableChat:   
            self.cpp_info.defines.append("ENABLE_CHAT")
            
        ## NO_READLINE - skipped
        
        if self.options.UseFreeImage:
           self.cpp_info.defines.append("USE_FREEIMAGE")
           
        if self.options.HaveFfmpeg:
           self.cpp_info.defines.append("HAVE_FFMPEG")   
        #if self.options.UseLibuv:
        #    self.cpp_info.defines.append("HAVE_LIBUV")   
            
        #if self.options.UsePdfium:
        #    self.cpp_info.defines.append("HAVE_PDFIUM")   
                
                
        
        

