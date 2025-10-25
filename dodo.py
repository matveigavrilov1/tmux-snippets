from tools.doit.conan import conan_install
from tools.doit.cmake import cmake_configure, cmake_build, cmake_install
from tools.doit.rm import rm

def task_conan_install():
	return conan_install()

def task_cmake_configure():
	return cmake_configure()

def task_cmake_build():
	return cmake_build()

def task_cmake_install():
	return cmake_install()

def task_rm():
	return rm()