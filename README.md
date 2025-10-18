# REMOTE_CLI, for AHU_2040  

**REMOTE CLI** this is Linux based `Command Line Interface` that can mimic typical AHU_2040 USB-based CLI, using Modbus RTU or TCP.
Apart from typical device management it provides additional features such as management of settings, in terms it can save current settings to the local file and vice-versa.
Also in general it is a lightweight and efficient command-line application interface designed to provide an intuitive and user-friendly experience, similar to MikroTik or Cisco CLI. It enables developers to build powerful command-driven applications with essential features like:  

- **Command Autocompletion**: Use the `Tab` key to auto-complete commands and parameters.  
- **Command History Navigation**: Easily navigate through previous commands using the `Up` and `Down` arrow keys.  
- **Hierarchical Command Structure**: Supports structured commands with subcommands for better organization.  
- **Custom Command Registration**: Easily define and register custom commands.  
- **Built-in Help System**: Provides detailed help messages for commands.  
- **Interactive and Scripted Modes**: Can be used interactively or execute command scripts.  

## Cross-Platform Compatibility  
As of now, this application can be compiled under x86 Linux, however in neareset future, It should be also available under Windows


## Build and Installation  

### Prerequisites  

Ensure you have the following installed:  

- **CMake (≥3.10 recommended)** – Build system generator  
- **pkg-config**
- **libmodbus-dev**

### Building the CLI Library  

To download and build the project, follow these steps:  

```sh
# Clone the repository including submodules
git clone --recurse-submodule https://github.com/sp1mdo/remote_cli.git
cd remote_cli

# Create and enter the build directory
mkdir build
cd build

# Generate build files with CMake
cmake .. 

# Compile the source code
make 
```
## How to run it using Modbus RTU
```sh
  ./remote_cli -d /dev/ttyUSB<N> 
```

## How to run it using Modbus TCP
```sh
  ./remote_cli -i <IP_ADDR>
```
