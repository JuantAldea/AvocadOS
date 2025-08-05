# AvocadOS

![AvocadOS Splash](https://github.com/JuantAldea/AvocadOS/blob/master/.github/docs/splash.png)

## About

AvocadOS is a minimalist hobbyist kernel for the i386 architecture, crafted to explore the fundamentals of operating system development. It offers a simple, educational platform for enthusiasts to experiment with process management, memory handling, and basic hardware interactions.

## Features

- **Basic Kernel Functionality**: Implements core OS components such as process scheduling and memory management.
- **i386 Architecture Support**: Designed specifically for 32-bit x86 systems.
- **Lightweight Design**: Minimal codebase for ease of understanding and modification.
- **Educational Focus**: Ideal for learning low-level programming and OS concepts.

## Getting Started

### Prerequisites

- A C compiler (e.g., GCC)
- NASM (Netwide Assembler)
- QEMU, Bochs or another emulator for testing
- GNU Make

### Building

1. Clone the repository:
   ```bash
   git clone https://github.com/JuantAldea/AvocadOS.git
   cd AvocadOS
   ```
2. Build the kernel:
   ```bash
   make
   ```
3. Run in QEMU:
   ```bash
   make run
   ```



## License

This project is licensed under the GPLv2 License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Inspired by various open-source hobbyist OS projects.
- Thanks to the OS development community for resources and tutorials.
  - https://wiki.osdev.org/
  - https://forum.osdev.org/
  - http://www.osdever.net/
