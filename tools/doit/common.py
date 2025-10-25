import os

def home_folder_arg(default=os.getcwd()):
	return {
		"name": "home_folder",
		"short": "h",
		"long": "home-folder",
		"type": str,
		"default": default,
		"help": "Home folder to work in"
	}

def output_folder_arg(default="build"):
	return {
		"name": "output_folder",
		"short": "o",
		"long": "output-folder",
		"type": str,
		"default": default,
		"help": "Output folder for conan"
	}

def build_type_arg(default="Release"):
	return {
		"name": "build_type",
		"short": "t",
		"long": "build-type",
		"type": str,
		"default": default,
		"help": "Build type"
	}