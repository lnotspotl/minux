# minux

## Install
```bash
cd compile && make depend && make
```

## Run in qemu
```bash
cd compile && make run
```

## Index codebase
```bash
sudo apt install bear
bear make
mkdir build && mv ./compile_commands.json ./build/
```

To exit qemu, press `Ctrl+A` followed by `Ctrl+C`.
