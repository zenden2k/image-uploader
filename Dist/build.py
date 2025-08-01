import sys
import re
import datetime
import subprocess
import shutil
import os
import stat
import json
import hashlib
import time
import xml.etree.ElementTree
import git

from contextlib import contextmanager
from dotenv import load_dotenv

def get_bool_env(env_var, default=False):
    value = os.getenv(env_var, str(default))
    return value.lower() in ('true', '1', 'yes', 'on')

load_dotenv()

IS_RELEASE = get_bool_env("UPTOODA_BUILD_RELEASE")
TEST_MODE = get_bool_env("UPTOODA_BUILD_TEST_MODE")
BUILD_DOCS = True
PDB_DIR = "PDB"
OUTDIR = "Releases" if IS_RELEASE else "Packages" 
APP_NAME = "Uptooda"
IU_GIT_REPOSITORY = "https://github.com/zenden2k/image-uploader.git"
GIT_BRANCH = os.getenv("UPTOODA_BUILD_BRANCH", "master")
PARALLEL_JOBS = os.getenv("UPTOODA_BUILD_PARALLEL_JOBS", "6")
DOWNLOAD_CA_BUNDLE = True
DRDUMP_APP_GUID = "7b4202e6-8294-4be5-a18d-69c097167b46"
UPLOAD_TO_DRDUMP = get_bool_env("UPTOODA_BUILD_UPLOAD_TO_DRDUMP", True)
# --- Script requirements ---
# git
# cmake
# conan
# msbuild (you should run this script from Visual Studio Developer Command Prompt)
# nuget
# 7zip (7z.exe)
# WSL 2 & Ubuntu 20.04+
# -- On WSL2 --
# conan 
# cmake
# git
# doxygen
# msgfmt
# ---------------------------
# All programs must be added to the PATH environment variable

CMAKE_GENERATOR_VS2019 = "Visual Studio 16 2019";
CMAKE_GENERATOR_VS2022 = "Visual Studio 17 2022";

DEFAULT_BUILD_PROFILE = "default"
#DEFAULT_BUILD_PROFILE = "windows_vs2022_x64"

NUGET_ARCH_MAPPING = {
    'armv8': 'arm64',
    'x86_64': "x64",
    'x86': 'x86'
}

RESULT_ARCH_MAPPING = {
    'armv8': 'arm64',
    'x86_64': "x64",
    'x86': 'x86'
}

ALTERNATIVE_ARCH_DISPLAY_NAME = {
    'armv8': 'ARM64',
    'x86_64': "x86 64-bit",
    'x86': 'x86 32-bit',
    'aarch64': 'ARM64'
}

BUILD_TARGETS = [
    {
        'os': "Windows",
        'compiler': "VS2019",
        'build_type': "Release",
        'arch': 'x86',
        'host_profile': 'windows_vs2019_x86_release',
        'build_profile': DEFAULT_BUILD_PROFILE,
        'cmake_generator': CMAKE_GENERATOR_VS2019,
        'cmake_platform': "Win32", 
        'cmake_args': ["-DIU_ENABLE_FFMPEG=On"],
        'enable_webview2': True,
        'shell_ext_arch': 'Win32',
        'shell_ext_64bit_arch': 'x64',
        'run_tests': True,
        'ffmpeg_standalone' : False,
        'supported_os': 'Windows 7/8/8.1/10/11',
        'upload_pdb': True
    },  
    {
        'os': "Windows",
        'compiler': "VS2019",
        'build_type': "Release",
        'arch': 'x86_64',
        'host_profile': 'windows_vs2019_x64_release',
        'build_profile': DEFAULT_BUILD_PROFILE,
        'cmake_generator': CMAKE_GENERATOR_VS2019,
        'cmake_platform': "x64",
       # "-DIU_FFMPEG_STANDALONE=On", 
        'cmake_args': ["-DIU_ENABLE_FFMPEG=On"],
        'enable_webview2': True,
        'shell_ext_arch': 'Win32',
        'shell_ext_64bit_arch': 'x64',
        'ffmpeg_standalone' : False,
        'installer_arch': 'x64',
        'run_tests': True,
        'supported_os': 'Windows 7/8/8.1/10/11 (64 bit)',
        'upload_pdb': True
    }, 
    {
        'os': "Windows",
        'compiler': "VS2019",
        'build_type': "Release",
        'arch': 'armv8',
        'host_profile': 'windows_vs2019_arm64_release',
        'build_profile': DEFAULT_BUILD_PROFILE,
        'cmake_generator': CMAKE_GENERATOR_VS2019,
        'cmake_platform': "ARM64", 
        'cmake_args': ["-DIU_ENABLE_FFMPEG=On", "-DIU_ENABLE_MEDIAINFO=Off", "-DIU_ENABLE_MEGANZ=Off"], # "-DIU_LIBHEIF_WITH_DAV1D=Off"
        'enable_webview2': True,
        'shell_ext_64bit_arch': 'ARM64',
        'ffmpeg_standalone' : False,
        'installer_arch': 'arm64',
        'supported_os': 'Windows 10/11 (ARM64)'
    },
    {
        'os': "Linux",
        'compiler': "gcc",
        'build_type': "Release",
        'arch': 'x86_64',
        'host_profile': 'default',
        'build_profile': 'default',
        'cmake_generator': 'Ninja Multi-Config', 
        'cmake_args': ["-DIU_ENABLE_FFMPEG=On", "-DIU_BUILD_QIMAGEUPLOADER=On"],
        'deb_package_arch': 'amd64',
        'build_qt_gui': True,
        'run_tests': True,
        'supported_os': 'Linux (amd64)',
        'objcopy': 'objcopy'
    },
    {
        'os': "Linux",
        'compiler': "gcc",
        'build_type': "Release",
        'arch': 'aarch64',
        'host_profile': '',
        'build_profile': 'default',
        'cmake_generator': 'Ninja Multi-Config', 
        'cmake_args': ["-DCMAKE_TOOLCHAIN_FILE=../Conan/Toolchains/aarch64-linux-gnu.toolchain.cmake"], 
        'deb_package_arch': 'arm64',
        'build_qt_gui': False,
        'run_tests': False,
        'supported_os': 'Linux (arm64)',
        'objcopy': 'aarch64-linux-gnu-objcopy'
    },
]

"""
    {
        'os': "Windows",
        'compiler': "VS2019",
        'build_type': "Release",
        'arch': 'x86',
        'host_profile': 'windows_vs2019_x86_release',
        'build_profile': DEFAULT_BUILD_PROFILE,
        'cmake_generator': CMAKE_GENERATOR_VS2019,
        'cmake_platform': "Win32", 
        'cmake_args': ["-DIU_ENABLE_FFMPEG=Off", "-DIU_ENABLE_MEDIAINFO=Off", "-DIU_ENABLE_MEGANZ=Off", "-DIU_ENABLE_SERVERS_CHECKER=Off", "-DIU_ENABLE_NETWORK_DEBUGGER=On"],
        'enable_webview2': True,
        'shell_ext_arch': 'Win32',
        'shell_ext_64bit_arch': 'x64',
        'run_tests': True,
        'ffmpeg_standalone' : False,
        'supported_os': 'Windows 7/8/8.1/10/11',
        'lite': True
    },  
    {
        'os': "Windows",
        'compiler': "VS2019",
        'build_type': "Release",
        'arch': 'x86_64',
        'host_profile': 'windows_vs2019_x64_release',
        'build_profile': DEFAULT_BUILD_PROFILE,
        'cmake_generator': CMAKE_GENERATOR_VS2019,
        'cmake_platform': "x64",
       # "-DIU_FFMPEG_STANDALONE=On", 
        'cmake_args': ["-DIU_ENABLE_FFMPEG=Off", "-DIU_ENABLE_MEDIAINFO=Off", "-DIU_ENABLE_MEGANZ=Off", "-DIU_ENABLE_SERVERS_CHECKER=Off", "-DIU_ENABLE_NETWORK_DEBUGGER=On"],
        'enable_webview2': True,
        'shell_ext_arch': 'Win32',
        'shell_ext_64bit_arch': 'x64',
        'ffmpeg_standalone' : False,
        'installer_arch': 'x64',
        'run_tests': True,
        'supported_os': 'Windows 7/8/8.1/10/11 (64 bit)',
        'lite': True
    }, 
    {
        'os': "Windows",
        'compiler': "VS2019",
        'build_type': "Release",
        'arch': 'armv8',
        'host_profile': 'windows_vs2019_arm64_release',
        'build_profile': DEFAULT_BUILD_PROFILE,
        'cmake_generator': CMAKE_GENERATOR_VS2019,
        'cmake_platform': "ARM64", 
        'cmake_args': ["-DIU_ENABLE_FFMPEG=Off", "-DIU_ENABLE_MEDIAINFO=Off", "-DIU_ENABLE_MEGANZ=Off", "-DIU_ENABLE_SERVERS_CHECKER=Off", "-DIU_ENABLE_NETWORK_DEBUGGER=On"], # "-DIU_LIBHEIF_WITH_DAV1D=Off"
        'enable_webview2': True,
        'shell_ext_64bit_arch': 'ARM64',
        'ffmpeg_standalone' : False,
        'installer_arch': 'arm64',
        'supported_os': 'Windows 10/11 (ARM64)',
        'lite': True
    },
"""

#BUILD_TARGETS = [BUILD_TARGETS[2]]

COMMON_BUILD_FOLDER = "Build_Release_Temp"
CONAN_PROFILES_REL_PATH = "../Conan/Profiles/"
VERSION_HEADER_FILE = "versioninfo-release.h" if IS_RELEASE else "versioninfo-nightly.h"
ENV_FILE = ".env"

def check_program(args, message=''):
    try:
        proc = subprocess.run(args)
        if proc.returncode == 0:
            return
    except Exception:
        pass
    print("Checking " + " ".join(args) + " failed")
    if message:
        print(message)
    sys.exit(1)              

def get_recent_commits(repo_path, max_count=10, from_commit=None, to_commit=None):
    """
    Gets list of commits from git repository in the same format as bash script
    
    Args:
        repo_path (str): Path to git repository
        max_count (int): Maximum number of commits to retrieve (ignored if range is specified)
        from_commit (str): Starting commit for range retrieval (not included)
        to_commit (str): Ending commit for range retrieval (included)
    
    Returns:
        list: List of dictionaries with commit information
    """
    try:
        # Open repository
        repo = git.Repo(repo_path)
        
        # If commit range is specified, get commits between them
        if from_commit and to_commit:
            # Get commits in range (equivalent to git rev-list --ancestry-path $1..$2)
            commits = list(repo.iter_commits(f"{from_commit}..{to_commit}"))
        else:
            # Get recent commits
            commits = list(repo.iter_commits(max_count=max_count))
        
        commit_list = []
        
        for commit in commits:
            commit_title = commit.message.strip().split('\n')[0]
            commit_info = {
                'commit_hash': commit.hexsha,
                'author': commit.author.name,
                'date': commit.committed_datetime.strftime('%a %b %d %H:%M:%S %Y %z'),
                'commit_message': commit_title
            }
            commit_list.append(commit_info)
            
        return commit_list
        
    except git.exc.InvalidGitRepositoryError:
        print(f"Error: {repo_path} is not a git repository")
        return []
    except Exception as e:
        print(f"Error retrieving commits: {e}")
        return []

def write_json_header(jsonfile, json_builds_file_name, source_dir, version_header_defines, git_commit_message):

    if os.path.exists(json_builds_file_name):
        with open(json_builds_file_name) as json_builds_file:
            json_builds_data = json.load(json_builds_file)
    else:
        json_builds_data = {

        }
    last_commit_hash = json_builds_data.get("last_commit_hash")

    now = datetime.datetime.now()

    dictionary = {
        "product": APP_NAME,
        "build_number":  version_header_defines['IU_BUILD_NUMBER'],
        "version":  version_header_defines['IU_APP_VER'],
        "version_clean": version_header_defines['IU_APP_VER_CLEAN'],
        "date": now.strftime('%Y-%m-%d %H:%M:%S'),
        "branch_name": version_header_defines['IU_BRANCH_NAME'],
        "commit_hash": version_header_defines['IU_COMMIT_HASH'],
        "commits": [
        ],
        "files": []
    }

    arg =  last_commit_hash
    if not last_commit_hash: # or last_commit_hash== version_header_defines['IU_COMMIT_HASH']:
        arg = 'HEAD~1'

    commits = get_recent_commits(source_dir, 10, arg, "HEAD")

    dictionary['commits'] = commits
 
    with open(jsonfile, "w") as outfile:
        json.dump(dictionary, outfile, indent=4)

    with open(json_builds_file_name, "w") as outfile:
        json.dump(json_builds_data, outfile, indent=4)    
    
    return dictionary

def write_builds_file(json_builds_file_name, version_header_defines):
    if os.path.exists(json_builds_file_name):
        with open(json_builds_file_name) as json_builds_file:
            json_builds_data = json.load(json_builds_file)
    else:
        json_builds_data = {

        }

    json_builds_data["last_commit_hash"] = version_header_defines['IU_COMMIT_HASH']
    
    with open(json_builds_file_name, "w") as outfile:
        json.dump(json_builds_data, outfile, indent=4)

def add_output_file(dictionary, target, jsonfile, name, path, relativePath, subproduct = ''):
    if relativePath[0] == '/':
        relativePath = relativePath[1:]
    filename = os.path.basename(path)
    hash = calc_sha256_from_file(path)
    with open(path + ".sha256", "w") as hash_file:
        hash_file.write(hash + " *" + filename)
    arch_alt = str(ALTERNATIVE_ARCH_DISPLAY_NAME.get(target.get("arch")))
    file = {
        "name": name,
        "target_name": get_target_full_name(target),
        "arch": get_out_arch_name(target),
        "arch_alt": arch_alt,
        "compiler": target.get("compiler"),
        "os": target.get("os"),
        "filename": filename,
        "path": relativePath,
        "subproduct": subproduct,
        "sha256": hash,
        "size": os.path.getsize(path),
        "supported_os": target.get("supported_os"),
    }
    dictionary["files"] += [file]
    with open(jsonfile, "w") as outfile:
        json.dump(dictionary, outfile, indent=4)
    return dictionary

def calc_sha256_from_file(filepath):
    BUF_SIZE = 65536  # lets read stuff in 64kb chunks!

    sha256 = hashlib.sha256()

    with open(filepath, 'rb') as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break
            sha256.update(data)

    return sha256.hexdigest()

def mkdir_if_not_exists(dir):
    if not os.path.exists(dir):
        try:
            os.makedirs(dir)
            #os.mkdir(dir)
        except OSError as error: 
            print(error)
            exit(1)
    
def get_target_full_name(target):
    return "_".join([target["os"], target["compiler"], target["arch"], target["build_type"]])
    
def try_conan_host_profile(target, conan_profile_dir, profile_name):
    if profile_name: 
        try_profile = conan_profile_dir + "/" + profile_name
    else:
        try_profile = conan_profile_dir + "/" + get_target_full_name(target).lower()

    if os.path.isfile(try_profile):
        if target["os"] == "Windows":
            return os.path.abspath(try_profile)
        else:
            return try_profile
    elif profile_name: 
        return profile_name
    else:
        return "default"
    
def try_conan_build_profile(target, conan_profile_dir, profile_name, host_profile_name):
    if profile_name: 
        try_profile = conan_profile_dir + "/" + profile_name
        if os.path.isfile(try_profile):
            return os.path.abspath(try_profile)
        return profile_name
    elif host_profile_name:
        return host_profile_name
    return "default"

def del_rw(action, name, exc):
    os.chmod(name, stat.S_IWRITE)
    os.remove(name)

@contextmanager
def cwd(path):
    oldpwd = os.getcwd()
    os.chdir(path)
    try:
        yield
    finally:
        os.chdir(oldpwd)
        
def check_conan_version(args):
    conan_output = subprocess.check_output(args).decode("utf-8").strip() 
    reg = re.compile(r"Conan version ([\d]+)\.[\d\.]+", flags=re.IGNORECASE) 
    res = reg.match(conan_output) 

    if res:
        conan_major_version = int(res.group(1))
        if conan_major_version < 2:
            print("Only Conan 2.x is supported.")
            sys.exit(1)
    else:
        print("Warning: Unknown Conan version")

def generate_version_header(filename, inc_version):
    result = {}
#    if not os.path.exists(filename):
#       shutil.copyfile(filename + ".dist", filename)
#
    with open(filename) as f:
        content = f.readlines()
        
    content = [x.strip() for x in content] 
    reg = re.compile("#define ([a-zA-Z0-9_]+) \"(.*?)\"")
    out_text = ""
    for line in content:
        res = reg.match(line)
        if res:
            define_name = res.group(1) 
            result[define_name] = str(res.group(2))

            if define_name == "IU_BUILD_NUMBER":
                if inc_version:
                    build_number = int(res.group(2))+1
                    print("New IU build: {}".format(build_number))
                else:
                    build_number = int(res.group(2))
                result[define_name] = str(build_number)
                out_text += "#define {} \"{}\"\n".format(define_name, str(build_number))
            elif define_name == "IU_APP_VER":
                if IS_RELEASE:
                    result[define_name] = result[define_name].replace("-nightly", "")
                else:
                    result[define_name] = now.strftime("%Y%m%d-nightly")
                out_text += "#define {} \"{}\"\n".format(define_name, str( result[define_name] ))
            elif define_name == "IU_BUILD_DATE":
                now = datetime.datetime.now()
                out_text += "#define {} \"{}\"\n".format(define_name, now.strftime("%d.%m.%Y"))   
            elif define_name == "IU_COMMIT_HASH":
                git_hash = subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode("utf-8").strip()
                out_text += "#define {} \"{}\"\n".format(define_name, git_hash)
            elif define_name == "IU_COMMIT_HASH_SHORT":
                git_hash = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).decode("utf-8").strip()
                out_text += "#define {} \"{}\"\n".format(define_name, git_hash)              
            elif define_name == "IU_BRANCH_NAME":        
                git_branch_name = subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).decode("utf-8").strip()
                out_text += "#define {} \"{}\"\n".format(define_name, git_branch_name)  
            else:
                out_text += line + "\n"
        else:
            out_text += line + "\n"       
    text_file = open(filename, "w")
    text_file.write(out_text)
    text_file.close()
    return result

def get_out_arch_name(target):
    src_arch = target.get("arch")
    if RESULT_ARCH_MAPPING.get(src_arch):
        return RESULT_ARCH_MAPPING.get(src_arch)
    return src_arch

def modify_update_file(component_name, filepath, version_header_defines, json_data):
    #print("filepath", filepath)
    # Open original file
    et = xml.etree.ElementTree.parse(filepath)

    root = et.getroot()
    now = datetime.datetime.now()

    root.attrib['TimeStamp'] = str(int(now.timestamp()))
    root.attrib['Date'] = now.strftime('%Y-%m-%d %H:%M:%S')
    root.attrib['DisplayName'] = component_name + " " + version_header_defines['IU_APP_VER'] +  " build "  + version_header_defines['IU_BUILD_NUMBER']
    root.attrib['DownloadPage'] = "https://svistunov.dev/uptooda_downloads" if IS_RELEASE else "https://svistunov.dev/uptooda_nightly"
    text = '';
    for commit in json_data['commits']:
        text += "- " + commit['commit_message'] + "\n"
    root.find('Info').text = text
    et.write(filepath, encoding='utf-8', xml_declaration=True) 

#
# -------- PROGRAM START ---------------------------------------------------
#
if len(sys.argv) > 1:
    git_branch = sys.argv[1]
else:
    git_branch = GIT_BRANCH

check_program(["git", "--version"])

check_program(["cmake", "--version"])
check_program(["msbuild", "--version"], "You should run this script from Visual Studio Developer Command Prompt")
innosetup_output = ""
try:
    innosetup_output = subprocess.check_output(["iscc", "/?"]).decode("utf-8").strip() 
except subprocess.CalledProcessError as exception:
    innosetup_output = str(exception.output)
    pass
except Exception:
    print("Please install Inno Setup and add it to PATH env variable")
    sys.exit(1)

if not "Inno Setup" in innosetup_output:
    print("Please install Inno Setup and add it to PATH env variable")
    sys.exit(1)

check_conan_version(['conan', '--version'])

#check_conan_version(['wsl', '-e', 'conan', '--version'])
check_program(["wsl", "-e", "cmake","--version"])
check_program(["wsl", "-e", "git","--version"])

if DOWNLOAD_CA_BUNDLE:
    proc = subprocess.run(["wsl", "-e", "./mk-ca-bundle.pl", "curl-ca-bundle.crt"])
    if proc.returncode != 0:
        print("Failed to download curl-ca-bundle")
        exit(1)

curl_ca_bundle = os.path.abspath("curl-ca-bundle.crt")

if not os.path.exists:
    print("curl-ca-bundle.crt not found!")
    sys.exit(1)

mkdir_if_not_exists(OUTDIR)
mkdir_if_not_exists(PDB_DIR)
outdir_abs = os.path.abspath(OUTDIR)
pdbdir_abs = os.path.abspath(PDB_DIR)
repo_dir = COMMON_BUILD_FOLDER + "/Repo"

if not TEST_MODE:
    if os.path.exists(COMMON_BUILD_FOLDER): 
        print("Directory exists, cleaning directory...")
        shutil.rmtree(COMMON_BUILD_FOLDER, onerror=del_rw)

if not os.path.exists(COMMON_BUILD_FOLDER):
    try:
        os.mkdir(COMMON_BUILD_FOLDER)
    except OSError as error: 
        print(error)
        exit(1)


if os.path.exists(repo_dir) and not TEST_MODE:
    print("Directory exists, clearing directory...")
    shutil.rmtree(repo_dir, onerror=del_rw)

if not os.path.exists(repo_dir):
    proc = subprocess.run(["git", "clone", "-b", git_branch, IU_GIT_REPOSITORY, repo_dir])

    if proc.returncode != 0:
        print("Git clone failed to directory " + repo_dir)

git_commit_message = subprocess.check_output("git log -1 --pretty=%B").decode("utf-8").strip() 
if not os.path.exists(VERSION_HEADER_FILE):
    shutil.copyfile("../Source/versioninfo.h.dist", VERSION_HEADER_FILE)

version_file_abs_path = os.path.abspath(VERSION_HEADER_FILE)
env_file_abs_path = os.path.abspath(ENV_FILE)

generate_version_header(VERSION_HEADER_FILE, True)
repo_dir_abs = os.path.abspath(repo_dir)
shutil.copyfile(VERSION_HEADER_FILE, repo_dir + "/Source/versioninfo.h")
shutil.copyfile(curl_ca_bundle, repo_dir + "/Dist/curl-ca-bundle.crt")
shutil.copyfile("../Data/" + ENV_FILE, repo_dir + "/Data/" + ENV_FILE)  

dist_directory = os.path.dirname(os.path.realpath(__file__))
#with cwd(repo_dir):
version_header_defines = generate_version_header(repo_dir_abs + "/Source/versioninfo.h", False)
app_ver = version_header_defines["IU_APP_VER"]
build_number = version_header_defines["IU_BUILD_NUMBER"]

proc = subprocess.run("wsl -e /bin/bash generate_mo.sh", cwd=repo_dir_abs + "/Lang/")
if proc.returncode !=0:
    print("Cannot generate language files")

if BUILD_DOCS:   
    proc = subprocess.run("wsl -e /bin/bash generate.sh", cwd=repo_dir_abs + "/Dist/DocGen/")
    if proc.returncode !=0:
        print("Cannot generate documentation")

new_build_dir = outdir_abs + "/" + app_ver+ "-build-" + build_number; 
mkdir_if_not_exists(new_build_dir)
json_file_path = new_build_dir + "/build_info.json"
json_builds_info_file = outdir_abs + "/builds.json"
json_data = write_json_header(json_file_path, json_builds_info_file, repo_dir_abs, version_header_defines, git_commit_message)
used_dist_dir = "/Dist/";

src_xml_file = repo_dir_abs + "/Data/Update/iu_core.xml"
modify_update_file(APP_NAME, src_xml_file, version_header_defines, json_data)
shutil.copyfile(src_xml_file, new_build_dir + '/' + ("iu_core.xml" if IS_RELEASE else "iu_core_nightly.xml" ))
src_xml_file = repo_dir_abs + "/Data/Update/iu_serversinfo.xml"
modify_update_file("Servers update", src_xml_file, version_header_defines, json_data)
shutil.copyfile(src_xml_file, new_build_dir + '/' + ("iu_serversinfo.xml" if IS_RELEASE else "iu_serversinfo_nightly.xml") )

#if TEST_MODE:
#    if not os.path.islink(repo_dir_abs + "/Dist_Test"):
#        os.symlink(dist_directory, repo_dir_abs + "/Dist_Test")
#    used_dist_dir = "/Dist_Test/"

for idx, target in enumerate(BUILD_TARGETS):
    target_full_name = get_target_full_name(target)
    #directory_name = target_full_name
    build_dir_path = COMMON_BUILD_FOLDER + "/Build_" + target_full_name + "_" + str(idx)
    if not os.path.exists(build_dir_path):
        try:
            os.mkdir(build_dir_path)
        except OSError as error: 
            print(error)
            exit(1)
    build_dir_path_abs = os.path.abspath(build_dir_path)
    host_profile = try_conan_host_profile(target, CONAN_PROFILES_REL_PATH, target.get("host_profile"))
    build_profile = try_conan_build_profile(target, CONAN_PROFILES_REL_PATH, target.get("build_profile"), host_profile)
    arch = target.get("arch")
    print("Building target:", target_full_name)
    print("Conan host profile:", host_profile)   
    print("Conan build profile:", build_profile) 

    with cwd(build_dir_path):
        include_path = "include"
        lib_path = "lib"
        mkdir_if_not_exists(include_path)
        mkdir_if_not_exists(lib_path)
        link_path = repo_dir_abs + "/Build"
        if os.path.islink(link_path):
            os.unlink(link_path)
        os.symlink(build_dir_path_abs, link_path)
        #if target.get("enable_webview2"):
        if False:
            proc = subprocess.run(["nuget", "install", "Microsoft.Web.WebView2", "-Version", "1.0.1823.32", "-ExcludeVersion", "-NonInteractive","-OutputDirectory", "packages"])
            if proc.returncode !=0:
                print("Nuget WebView2 install failed")
                exit(1)
            nuget_package_path = "packages/Microsoft.Web.WebView2/build/native/";
            nuget_arch =  NUGET_ARCH_MAPPING[arch]
            shutil.copyfile(nuget_package_path + nuget_arch + "/WebView2LoaderStatic.lib", "lib/WebView2LoaderStatic.lib")    
            shutil.copyfile(nuget_package_path + "/include/WebView2.h", include_path +"/WebView2.h")    
            shutil.copyfile(nuget_package_path + "/include/WebView2EnvironmentOptions.h", include_path + "/WebView2EnvironmentOptions.h")    

            proc = subprocess.run(["nuget", "install", "Microsoft.Windows.ImplementationLibrary", "-Version", "1.0.230411.1", "-ExcludeVersion", "-NonInteractive", "-OutputDirectory", "packages"])
            if proc.returncode !=0:
                print("Nuget WIL install failed")
                exit(1)
            nuget_package_path = "packages/Microsoft.Windows.ImplementationLibrary/include/";
            #shutil.copy2(nuget_package_path + "wil/", "include/wil/")
            files = os.listdir(nuget_package_path + "wil")
            mkdir_if_not_exists(include_path + "/wil")
            shutil.copytree(nuget_package_path + "wil", include_path + "/wil", dirs_exist_ok=True)
        #Microsoft.Windows.ImplementationLibrary

        if target.get('ffmpeg_standalone'):
            src_dir = dist_directory + "/Libs/FFmpeg/" + target.get("arch") + "/lib/"
            if not os.path.exists(src_dir):
                print("You must put ffmpeg libraries into", src_dir)
            files=os.listdir(src_dir)
            for fname in files:
                shutil.copy2(os.path.join(src_dir,fname), lib_path)
            src_dir = dist_directory + "/Libs/FFmpeg/" + target.get("arch") + "/include/"
            shutil.copytree(src_dir, include_path, dirs_exist_ok=True) 
            src_dir =  dist_directory + "/Libs/FFmpeg/" + target.get("arch") + "/bin/"
            dest_dir = "GUI/Release/"
            shutil.copytree(src_dir, dest_dir, dirs_exist_ok=True, ignore=shutil.ignore_patterns('*.lib')) 

        #cmake ..\Source -G "Visual Studio 16 2019" -A Win32 -DCMAKE_BUILD_TYPE=Debug -DIU_HOST_PROFILE=vs2019_x86_debug -DIU_BUILD_PROFILE=vs2022_x64 -DIU_ENABLE_FFMPEG=On -DIU_ENABLE_WEBVIEW2=On 
        build_type = target.get("build_type")
        command = ["cmake", "../Repo/Source", "-G", target.get("cmake_generator"), "-DCMAKE_BUILD_TYPE=" + build_type, 
                   "-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake",
                   # "-DCMAKE_CONFIGURATION_TYPES:STRING="+build_type,
                    "-DCONAN_BUILD_PROFILE=" + build_profile
                ]
        if target.get("host_profile") != "":
            command += ["-DCONAN_HOST_PROFILE=" + host_profile]
        if target.get("cmake_platform"):
            command += ["-A", target.get("cmake_platform")]
        if target.get("os") == "Linux":
            command =  ['wsl', '-e'] + command
        if target.get('ffmpeg_standalone'):
            command += ["-DIU_FFMPEG_STANDALONE=On"]
        
        if target.get("enable_webview2"):
            command += ["-DIU_ENABLE_WEBVIEW2=On"]

        if target.get("cmake_args"):
            command += target.get("cmake_args")
        
        print("Running command:", " ".join(command))
        proc = subprocess.run(command)
        if proc.returncode !=0:
            print("Generate failed")
            exit(1)

        command = ["cmake", "--build", ".", "-j", PARALLEL_JOBS, "--config", target.get("build_type")]
        if target.get("os") == "Linux":
            command =  ['wsl', '-e'] + command
                
        print("Running command:", " ".join(command))
        proc = subprocess.run(command)
        if proc.returncode !=0:
            print("Build failed")
            exit(1)
        
        if target.get("run_tests"):
            command = ["Tests/Release/Tests"]
            if target.get("os") == "Linux":
                command =  ['wsl', '-e'] + command
                
            print("Running command:", " ".join(command))
            proc = subprocess.run(command)
            if proc.returncode !=0:
                print("Tests run failed")
                exit(1)

        if target["os"] == "Windows":
            #ext_native_arch = 
            if target.get("shell_ext_arch"):
                # /p:Configuration="Release optimized";Platform=Win32
                command = ["msbuild", "..\\Repo\\Source\\ShellExt\\ExplorerIntegration.sln", "/p:Configuration=ReleaseOptimized;Platform=" + target["shell_ext_arch"]]
                print("Running command:", " ".join(command))
                proc = subprocess.run(command)
                if proc.returncode !=0:
                    print("Shell extension build failed")
                    exit(1)
            
            if target.get("shell_ext_64bit_arch"):
                command = ["msbuild", "..\\Repo\\Source\\ShellExt\\ExplorerIntegration.sln", "/p:Configuration=ReleaseOptimized;Platform="+target["shell_ext_64bit_arch"]]
                print("Running command:", " ".join(command))
                proc = subprocess.run(command)
                if proc.returncode !=0:
                    print("Shell extension 64 bit Build failed")
                    exit(1)
            relative_path = r"/Windows/" + get_out_arch_name(target) + '/'
            package_os_dir = new_build_dir + relative_path
            mkdir_if_not_exists(package_os_dir)
            pdb_os_dir = pdbdir_abs + "/" + app_ver+ "-build-" + build_number + relative_path; 
            mkdir_if_not_exists(pdb_os_dir)

            # Creating archive
            command =  repo_dir_abs + used_dist_dir + r"create_portable.bat"
            print("Running command:", command)
            proc = subprocess.run(command, cwd=repo_dir_abs + used_dist_dir)
            if proc.returncode !=0:
                print("Create archive failed")
                exit(1)
            appname_suffix = " Lite" if target.get("lite") else ""
            output_filename_suffix = "-lite" if target.get("lite") else ""
            file_from = r"output\uptooda-" + app_ver + "-build-" + build_number+ "-openssl-portable.7z"
          
            filename =  "uptooda-" + app_ver + "-build-" + build_number + output_filename_suffix + "-" + get_out_arch_name(target) + ".7z"
            file_to = package_os_dir + "\\" +filename
            print("Copy file from:", file_from)
            print("Copy file to:", file_to)
            shutil.copyfile(file_from, file_to)
            json_data = add_output_file(json_data, target, json_file_path, "7zip archive", file_to, relative_path + filename, APP_NAME + " (GUI" + appname_suffix +")")

            if not target.get("lite"):
                # Creating CLI archive (uptooda=cli)
                command =  repo_dir_abs + used_dist_dir + r"create_cli.bat"
                print("Running command:", command)
                proc = subprocess.run(command, cwd=repo_dir_abs + used_dist_dir)
                if proc.returncode !=0:
                    print("Create archive failed")
                    exit(1)

                file_from = r"output\\uptooda-cli-{version}-build-{build}.7z".format(version=app_ver,build=build_number ); 
                filename = "uptooda-cli-{version}-build-{build}{suffix}-{arch}.7z".format(version=app_ver,
                                                                                        build=build_number,
                                                                                        suffix=output_filename_suffix,
                                                                                        arch=get_out_arch_name(target))
                file_to = package_os_dir +"\\"+filename
                print("Copy file from:", file_from)
                print("Copy file to:", file_to)
                shutil.copyfile(file_from, file_to)
                json_data = add_output_file(json_data, target, json_file_path, "7zip archive", file_to, relative_path + filename, APP_NAME + " (CLI" + appname_suffix +")")

            # Creating installer for Windows
            print("Running command:", repo_dir_abs + used_dist_dir + r"create_portable.bat")
            args = ["iscc.exe", "/dWIN64FILES", "/O"+build_dir_path_abs + r"\Installer"]
            if target.get('ffmpeg_standalone'):
                args += ["/dIU_FFMPEG_STANDALONE"]

            #if target.get('installer_arch'):
            args += ["/dIU_ARCH=" + str(target.get('installer_arch') or '')]   

            args += [repo_dir_abs + used_dist_dir + "iu_setup_script.iss"]

            print("Running command:", " ".join(args))    
            proc = subprocess.run(args, cwd=repo_dir_abs + used_dist_dir)
            if proc.returncode !=0:
                print("Create installer failed")
                exit(1)

            # Copying installer
            file_from = r"Installer\uptooda-" + app_ver + "-build-" + build_number+ "-setup.exe"
            filename = "uptooda-" + app_ver + "-build-" + build_number + output_filename_suffix + "-" + get_out_arch_name(target)+"-setup.exe"
            file_to = package_os_dir + filename
            print("Copy file from:", file_from)
            print("Copy file to:", file_to)
            shutil.copyfile(file_from, file_to)
            json_data = add_output_file(json_data, target, json_file_path, "Installer", file_to, relative_path + filename, APP_NAME + " (GUI" + appname_suffix +")")


            shutil.copyfile("GUI\\Release\\Uptooda.pdb", pdb_os_dir  + "/Uptooda.pdb")
            
            if UPLOAD_TO_DRDUMP and target.get("upload_pdb"):
                command = ["SYMUPLOAD", DRDUMP_APP_GUID, version_header_defines["IU_APP_VER_CLEAN"] + "." + build_number, "0", ".\\GUI\\Release\\*.pdb"]
                proc = subprocess.run(command)
                if proc.returncode !=0:
                    print("Failed to upload PDB files to DrDump server")
                    exit(1)

                command = ["SYMUPLOAD", DRDUMP_APP_GUID, version_header_defines["IU_APP_VER_CLEAN"] + "." + build_number, "0", ".\\GUI\\Release\\*.exe"]
                proc = subprocess.run(command)
                if proc.returncode !=0:
                    print("Failed to upload EXE files to DrDump server")
                    exit(1)    

        elif target["os"] == "Linux":
            args = ["wsl", "-e", "/bin/bash", "create-package.sh", target.get("deb_package_arch"), target.get("objcopy")]
            working_dir = repo_dir_abs + used_dist_dir + "Linux/"
            print("Running command:", " ".join(args), "; working_dir="+working_dir)
            proc = subprocess.run(args, cwd=working_dir)

            if proc.returncode !=0:
                print("Failed to create debian package for CLI")
                exit(1)
            
            relative_path = r"/Linux/" + get_out_arch_name(target) + "/"
            package_os_dir = new_build_dir + relative_path
            mkdir_if_not_exists(package_os_dir)

            file_from = r"Linux\\uptooda-cli_{version_clean}.{build_number}_{arch}.deb".format(version_clean=version_header_defines["IU_APP_VER_CLEAN"],
                                                                                            build_number=build_number,
                                                                                            arch=target.get("deb_package_arch")
            )
            filename = "uptooda-cli_" + app_ver + "-build-" + build_number + "_" + target.get("deb_package_arch") +".deb"
            file_to = package_os_dir + filename
            print("Copy file from:", file_from)
            print("Copy file to:", file_to)
            shutil.copyfile(file_from, file_to)
            json_data = add_output_file(json_data, target, json_file_path, "Debian package", file_to, relative_path + filename, APP_NAME + " (CLI)")
       
            file_from = r"Linux\\uptooda-cli-{version_clean}.{build_number}-{arch}.tar.xz".format(version_clean=version_header_defines["IU_APP_VER_CLEAN"],
                                                                                            build_number=build_number,
                                                                                            arch=target.get("deb_package_arch")
            )
            filename = "uptooda-cli_" + app_ver + "-build-" + build_number + "_" + target.get("deb_package_arch") +".tar.xz"
            file_to = package_os_dir + filename
            print("Copy file from:", file_from)
            print("Copy file to:", file_to)
            shutil.copyfile(file_from, file_to)
            json_data = add_output_file(json_data, target, json_file_path, ".tar.xz archive", file_to, relative_path + filename, APP_NAME + " (CLI)")

            # Qt GUI
            if target.get("build_qt_gui"):
                args = ["wsl", "-e", "/bin/bash", "create-qimageuploader-package.sh", target.get("deb_package_arch"), target.get("objcopy")]
                working_dir = repo_dir_abs + used_dist_dir + "Linux/"
                print("Running command:", " ".join(args), "; working_dir="+working_dir)
                proc = subprocess.run(args, cwd=working_dir)

                if proc.returncode !=0:
                    print("Failed to create debian package for Qt GUI")
                    exit(1)

                file_from = r"Linux\\uptooda_{version_clean}.{build_number}_{arch}.deb".format(version_clean=version_header_defines["IU_APP_VER_CLEAN"],
                                                                                                build_number=build_number,
                                                                                                arch=target.get("deb_package_arch")
                )
                filename = "uptooda_" + app_ver + "-build-" + build_number + "_" + target.get("deb_package_arch") +".deb"
                file_to = package_os_dir + filename
                print("Copy file from:", file_from)
                print("Copy file to:", file_to)
                shutil.copyfile(file_from, file_to)
                json_data = add_output_file(json_data, target, json_file_path, "Debian package", file_to, relative_path + filename, APP_NAME + " (Qt GUI)")
        

    print("Target finished successfully:", target_full_name)

#if TEST_MODE:
#    os.unlink(repo_dir_abs + "/Dist_Test")
write_builds_file(json_builds_info_file, version_header_defines)
print("Finish.")

  