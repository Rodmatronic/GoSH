# GoSH
A simple portable cross-platform Unix shell written in C.

So far, it has been tested on Linux, FreeBSD, Haiku, and Darwin.

(here is the shell compiled on, and running on Darwin)
![329817658-9991852b-0c68-4a43-86d0-3f7e81f1d47a](https://github.com/Rodmatronic/GoSH/assets/105672808/d83e13c9-2832-47c7-91b3-8b931293576a)

![image](https://github.com/Rodmatronic/GoSH/assets/105672808/3cca05eb-a7f8-475e-ada2-9aa0f80a67cb)

![image](https://github.com/Rodmatronic/GoSH/assets/105672808/b07bbaca-ff92-4702-8f69-f38e4a2198ac)

![image](https://github.com/Rodmatronic/GoSH/assets/105672808/60c30230-66ac-41d7-a94b-07702f421a35)

# How can I use it?
To build the binary, you must have the clang compiler installed.

To compile it, run `sh compile.sh`, or `bash compile.sh` in the GoSH master directory. `gosh` should appear in the same directory.

To install it on most 'Nix systems (aside from macOS/Darwin) run `sh install.sh`, it should compile (from `compile.sh`) and put itself into `/bin`.
