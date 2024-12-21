from conan import ConanFile
from conan.errors import ConanInvalidConfiguration


class ImageUploaderRecipe(ConanFile):
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    default_options = {
        "libcurl/*:with_libssh2": True
    }

#    def validate(self):
#        if self.info.settings.os == "Macos" and self.info.settings.arch == "armv8":
#            raise ConanInvalidConfiguration("ARM v8 not supported")

    def requirements(self):
        self.requires("tinyxml2/10.0.0")
        self.requires("boost/1.78.0")
        self.requires("libcurl/8.10.1")
        self.requires("pcre/8.45")
        self.requires("uriparser/0.9.8")
        self.requires("zlib/1.3.1")
        self.requires("minizip/1.3.1")
        self.requires("jsoncpp/1.9.6")
        self.requires("sqlite3/3.36.0")
        self.requires("base64/0.4.0")
        self.requires("glog/0.6.0@zenden2k/stable")
        self.requires("libwebp/1.4.0")
        self.requires("gtest/1.10.0")
        self.requires("gumbo-parser/0.10.1")
        self.requires("squirrel/3.0.0")
        self.requires("libmediainfo/22.03")

        # Add base64 dependency only for Windows
        #if self.settings.os == "Windows":
        #    self.requires("base64/0.4.0")
        #if self.settings.os != "Windows":  # we need cmake 3.19 in other platforms
        #    self.tool_requires("cmake/3.19.8")

    def layout(self):
        # We make the assumption that if the compiler is msvc the
        # CMake generator is multi-config
        if self.settings.get_safe("compiler") == "msvc":
            multi = True
        else:
            multi = False          

        self.folders.build = "build" if multi else f"build/{str(self.settings.build_type)}"
        self.folders.generators = "build"