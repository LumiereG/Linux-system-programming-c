# Package Manager Simulator

## Description
This program simulates a package manager within a virtual environment. It allows users to create virtual environments, install and remove packages, and manage dependencies using a simple command-line interface.

## Features
- Create a virtual environment with a specified name.
- Maintain a `requirements` file that stores package names and versions.
- Install packages by adding their names and versions to `requirements` and creating read-only package files with random content.
- Uninstall packages by removing them from `requirements` and deleting the corresponding package file.
- Execute operations on multiple environments simultaneously.
- Handle errors gracefully, ensuring no duplicate package installations and preventing operations on non-existent environments.

## Usage

### Create a Virtual Environment
To create a new virtual environment, use the `-c` flag along with the `-v` option specifying the environment name:
```sh
./sop-venv -c -v <ENVIRONMENT_NAME>
```
Example:
```sh
./sop-venv -c -v my_env
ls ./
# Output: my_env/
ls my_env/
# Output: requirements
```

### Install a Package
To install a package, use the `-i` option along with `-v` specifying the environment:
```sh
./sop-venv -v <ENVIRONMENT_NAME> -i <PACKAGE_NAME>==<VERSION>
```
Example:
```sh
./sop-venv -v my_env -i numpy==1.0.0
./sop-venv -v my_env -i pandas==1.2.0
cat my_env/requirements
# Output:
# numpy 1.0.0
# pandas 1.2.0
ls -l my_env/
# Output:
# -r--r--r-- 1 user user 20 Oct 13 19:51 numpy
# -r--r--r-- 1 user user 30 Oct 13 19:51 pandas
# -rw-r--r-- 1 user user 17 Oct 13 19:50 requirements
```

### Uninstall a Package
To remove a package, use the `-r` option along with `-v` specifying the environment:
```sh
./sop-venv -v <ENVIRONMENT_NAME> -r <PACKAGE_NAME>
```
Example:
```sh
./sop-venv -v my_env -r numpy
cat my_env/requirements
# Output:
# pandas 1.2.0
ls -l my_env/
# Output:
# -r--r--r-- 1 user user 30 Oct 13 19:51 pandas
# -rw-r--r-- 1 user user 17 Oct 13 19:50 requirements
```

### Handling Errors
If an operation is attempted on a non-existing environment, an error message is displayed:
```sh
./sop-venv -v non_existing_env -i pandas
# Output:
# sop-venv: the environment does not exist
# usage: ...
```

## Notes
- The program searches for environments in the current directory.
- The `-v` option can be specified multiple times (except during environment creation) to execute operations on multiple environments simultaneously.
- Any error encountered halts further execution.
- Duplicate package installations are not allowed.
- Attempting to uninstall a non-existent package results in an error.

## License
This project is open-source and available under the MIT License.

## Author
Svetlana Gridina
