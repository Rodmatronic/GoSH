# GoSH
A simple portable cross-platform Unix shell written in C. So far, it has been tested on Linux, FreeBSD, Haiku, and Darwin.

Here is an example of GoSH running as the default shell on Linux Mint
![image](https://github.com/Rodmatronic/GoSH/assets/105672808/64648576-eb55-4d9a-b17a-be47b552c736)

Another example of it running as the main shell in a custom Linux distribution

![image](https://github.com/Rodmatronic/GoSH/assets/105672808/0aea8e97-2237-4392-80b4-abe4743d2795)


# How can I use it?
To build the binary, you must have the clang compiler installed.

To compile it, run `sh compile.sh`, or `bash compile.sh` in the GoSH master directory. `gosh` should appear in the same directory.

To install it on most 'Nix systems (aside from macOS/Darwin) run `sh install.sh`, it should compile (from `compile.sh`) and put itself into `/bin`.
