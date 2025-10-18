# REMOTE_CLI, for AHU_2040  

**REMOTE CLI** this is Linux-based `Command Line Interface` that mimics the typical AHU_2040 USB-based CLI behavior using Modbus RTU or TCP.
In addition to standard device management, it provides extra features such as settings management — allowing you to save the current configuration to a local file and restore it later.

Overall, it is a lightweight and efficient command-line application designed to offer an intuitive and user-friendly experience, similar to MikroTik or Cisco CLI environments. It enables developers to build powerful command-driven applications with essential features such as:

- **Command Autocompletion**: Use the `Tab` key to auto-complete commands and parameters.  
- **Command History Navigation**: Easily navigate through previous commands using the `Up` and `Down` arrow keys.  
- **Hierarchical Command Structure**: Supports structured commands with subcommands for better organization.  
- **Custom Command Registration**: Easily define and register custom commands.  
- **Built-in Help System**: Provides detailed help messages for commands.  
- **Interactive and Scripted Modes**: Can be used interactively or execute command scripts.  

## Cross-Platform Compatibility  
Currently, this application can be compiled on x86 Linux.
In the near future, it will also be available for Windows.


## Build and Installation  

### Prerequisites  

Ensure you have the following installed:  

- **CMake (≥3.10 recommended)** – Build system generator  
- **pkg-config**
- **libmodbus-dev**

### Building the application  

To download and build the project, follow these steps:  

```sh
# Clone the repository including submodules
git clone --recurse-submodule https://github.com/sp1mdo/remote_cli.git
cd remote_cli

# Create and enter the build directory
mkdir build && cd build

# Generate build files with CMake
cmake .. 

# Compile the source code
make 
```

### Running the CLI
## How to run it using Modbus RTU
```sh
  ./remote_cli -d /dev/ttyUSB<N> 
```

## How to run it using Modbus TCP
```sh
  ./remote_cli -i <IP_ADDR>
```
