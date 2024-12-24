from conan import ConanFile
from conan.errors import ConanInvalidConfiguration


class ImageUploaderRecipe(ConanFile):
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    default_options = {
        "libcurl/*:with_libssh2": True,
        "ffmpeg/*:shared": False,
		"ffmpeg/*:disable_all_encoders": True,
		"ffmpeg/*:disable_all_muxers": True,
		"ffmpeg/*:with_programs": False,
		"ffmpeg/*:disable_protocols": True,
		"ffmpeg/*:with_libmp3lame": False,
		"ffmpeg/*:with_libfdk_aac": False,
		"ffmpeg/*:avdevice": False,
		"ffmpeg/*:avfilter": False,
		"ffmpeg/*:with_freetype": False,
		"ffmpeg/*:with_openjpeg": False,
		"ffmpeg/*:with_vorbis": False,
		"ffmpeg/*:with_opus": False,
		"ffmpeg/*:with_libx264": False,
		"ffmpeg/*:with_libvpx": False,
		"ffmpeg/*:with_libiconv": False,
		"ffmpeg/*:with_libx265": False,
		"ffmpeg/*:with_openh264": False,
		"libheif/*:with_dav1d": True
    }

    def configure(self):
        if self.settings.os != "Windows":
            self.options["ffmpeg/*"].with_vaapi = False
            self.options["ffmpeg/*"].with_vdpau = False
            self.options["ffmpeg/*"].with_vulkan = False
            self.options["ffmpeg/*"].with_libalsa = False
            self.options["ffmpeg/*"].with_xcb = False
            self.options["ffmpeg/*"].with_pulse = False
            self.options["ffmpeg/*"].with_libaom = False
            self.options["ffmpeg/*"].with_xlib = False
            
        if self.settings.os == "Windows" and self.settings.arch == "armv8":
            self.options["libheif/*"].with_dav1d = False
            self.options["ffmpeg/*"].with_libdav1d = False
            self.options["ffmpeg/*"].with_libsvtav1 = False
            self.options["megaio/*"].UseMediainfo = False
#    def validate(self):
#        if self.info.settings.os == "Macos" and self.info.settings.arch == "armv8":
#            raise ConanInvalidConfiguration("ARM v8 not supported")

    def requirements(self):
        self.requires("boost/1.78.0")
        self.requires("libcurl/8.10.1")
        self.requires("pcre/8.45")
        self.requires("uriparser/0.9.8")
        self.requires("zlib/1.3.1", force=True)
        self.requires("minizip/1.3.1")
        self.requires("jsoncpp/1.9.6")
        self.requires("sqlite3/3.36.0")
        self.requires("base64/0.4.0")
        self.requires("glog/0.6.0@zenden2k/stable")
        self.requires("libwebp/1.4.0", force=True)
        self.requires("gtest/1.10.0")
        self.requires("gumbo-parser/0.10.1")
        self.requires("squirrel/3.0.0")
        self.requires("tinyxml2/10.0.0", force=True)
        self.requires("ffmpeg/5.1.3")
        self.requires("megaio/3.5.2")
        

        if self.settings.os == "Windows" and self.settings.arch == "armv8":
            self.requires("openssl/1.1.1w", force=True)
        else:
            self.requires("openssl/3.3.2")
            self.requires("libmediainfo/22.03")
            self.requires("dav1d/1.4.3", force=True)
        # self.requires("xz_utils/5.4.2")

        # Add base64 dependency only for Windows
        if self.settings.os == "Windows":
            self.requires("base-classes/1.0.0")
            self.requires("libheif/1.13.0")
 
        #    
        #if self.settings.os != "Windows":  # we need cmake 3.19 in other platforms
        #    self.tool_requires("cmake/3.19.8")

    def layout(self):
        # We make the assumption that if the compiler is msvc the
        # CMake generator is multi-config
        #if self.settings.get_safe("compiler") == "msvc":
        #    multi = True
        #else:
        multi = False          

        self.folders.build = "build" if multi else f"build/{str(self.settings.build_type)}"
        self.folders.generators = "build"