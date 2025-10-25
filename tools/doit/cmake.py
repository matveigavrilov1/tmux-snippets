from doit.tools import Interactive
from tools.doit.common import home_folder_arg, output_folder_arg, build_type_arg

def cmake_configure():
	def configure(home_folder: str, output_folder: str, build_type: str):
		generator_opt="-G Ninja"
		toolchain_opt=f"-DCMAKE_TOOLCHAIN_FILE=\"{home_folder}/{output_folder}/conan_toolchain.cmake\""
		build_type_opt=f"-DCMAKE_BUILD_TYPE={build_type}"

		dirs_opt=f"-S {home_folder} -B {home_folder}/{output_folder}"

		return f"cmake .. {generator_opt} {toolchain_opt} {build_type_opt} {dirs_opt}"

	return {
		'basename': "cmake-configure",
		'doc': "Configure CMake for build",
		'actions': [
			Interactive(configure)
		],
		'verbosity': 2,
		'params': [
			build_type_arg(),
			home_folder_arg(),
			output_folder_arg()
		]
	}


def cmake_build():
	def build(home_folder: str, output_folder: str, build_type: str):
		return f"cmake --build {home_folder}/{output_folder} --config {build_type}"

	return {
		'basename': "cmake-build",
		'doc': "Build project with configured cmake",
		'actions': [
			Interactive(build)
		],
		'verbosity': 2,
		'params': [
			build_type_arg(),
			home_folder_arg(),
			output_folder_arg()
		]
	}

def cmake_install():
	def install(home_folder: str, output_folder: str, build_type: str):
		return f"cmake --install {home_folder}/{output_folder} --config {build_type}"

	return {
		'basename': "cmake-install",
		'doc': "Install cmake build artifacts",
		'actions': [
			Interactive(install)
		],
		'verbosity': 2,
		'params': [
			build_type_arg(),
			home_folder_arg(),
			output_folder_arg()
		]
	}

