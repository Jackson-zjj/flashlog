# coding=utf-8
import shutil
import os
import sys
import argparse

SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]
BUILD_DIR_PATH = SCRIPT_PATH + "/../build"


def clear():
    if os.path.exists(BUILD_DIR_PATH):
        shutil.rmtree(BUILD_DIR_PATH)


def get_project_path():
    project_path = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    return project_path


def gene_proto_buf_source_file(out_dir):
    protoc_path = get_project_path() + "/script/bin/protoc_linux"
    if sys.platform.startswith("win"):
        protoc_path = get_project_path() + "/script/bin/protoc_win.exe"
    elif sys.platform.startswith("darwin"):
        protoc_path = get_project_path() + "/script/bin/protoc_mac"

    build_cmd = "{0} -I={1}/logger/proto --cpp_out={2} {1}/logger/proto/*.proto".format(
        protoc_path, get_project_path(), out_dir
    )
    ret = os.system(build_cmd)
    if ret != 0:
        print("!!!generate pb source file fail")
        return False
    return True


def build_mac(args=None):
    # todo 编译到debug文件夹中
    # platform_dir = "%s/%s" % (BUILD_DIR_PATH, config)
    platform_dir = "%s" % (BUILD_DIR_PATH)
    os.makedirs(platform_dir, exist_ok=True)

    if not gene_proto_buf_source_file(platform_dir):
        print("protobuf生成失败，终止构建")
        return False

    os.chdir(platform_dir)

    src_dir = get_project_path()
    build_dir = src_dir + "/build"
    print(build_dir)
    print(platform_dir)

    build_cmd = (
        f"cmake "
        f"-S {src_dir} "
        f"-B {build_dir} "
        f'-G "Unix Makefiles" '
        f"-DCMAKE_BUILD_TYPE={args.config} "
    )

    # 指定vcpkg工具链
    vcpkg_root = os.environ.get("VCPKG_ROOT")
    vcpkg_toolchain = os.path.join(vcpkg_root, "scripts", "buildsystems", "vcpkg.cmake")
    vcpkg_triplet = "arm64-osx" if (sys.platform == "darwin" and os.uname().machine == "arm64") else "x64-osx"
    if os.path.exists(vcpkg_toolchain):
        build_cmd += f" -DCMAKE_TOOLCHAIN_FILE={vcpkg_toolchain}"
        build_cmd += f" -DVCPKG_TARGET_TRIPLET={vcpkg_triplet}"

    if args.test:
        build_cmd += " -DBUILD_STL_TEST=ON"

    if args.clang:
        build_cmd += " -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"

    print("build cmd:" + build_cmd)
    ret = os.system(build_cmd)
    if ret != 0:
        print("!!!!!!!!!!!!!!!!!!build fail")
        return False

    os.chdir(build_dir)
    build_cmd = "cmake --build . --config %s --parallel 8" % (args.config)
    ret = os.system(build_cmd)
    if ret != 0:
        print("build fail!!!!!!!!!!!!!!!!!!!!")
        return False
    return True


def main():
    # clear()
    os.makedirs(BUILD_DIR_PATH, exist_ok=True)
    parser = argparse.ArgumentParser(description="build mac")
    parser.add_argument(
        "--config", 
        type=str, 
        choices=["Debug", "Release"],  
        default="Debug", 
        help="compile mode: Debug/Release (default: Debug)"
    )
    parser.add_argument(
        "--test", action="store_true", default=False, help="run unittest"
    )
    parser.add_argument(
        "--clang", action="store_true", default=False, help="enable clang-tidy check"
    )
    args = parser.parse_args()

    if not build_mac(args=args):
        exit(1)


if __name__ == "__main__":
    main()
