# Kush

Kush is a general purpose programming language designed to build simple, safe
and efficient programs. It is heavily inspired by C.

### Example 1: Linear Search

The following example demonstrates the linear search algorithm in Kush.

```
void main() {
    var names = [
        'Samuel Rowe',
        'Joel E. Rego',
        'Bill Gates',
        'Linus Trovalds',
        'Abraham Lincoln',
        'Isaac Newton',
        'Albert Einstein',
        'Richard Feynman',
        'Christopher Nolan',
        'Benedict Cumberbatch'
    ];
    var key = 'Marshall Mathers';
    var result = -1;
    var i = 0;
    while i < names.size {
        if String:equals(key, names[i]) {
            result = i;
            break;
        }
        i += 1;
    }

    if result != -1 {
        print('Found the key at index ' + result + '.');
    }
    else {
        print('Could not find the key.');
    }
}
```

### Example 2: Factorial

The following example demonstrates calculation of factorial in Kush.
```
void main() {
    print('Enter an integer: ');
    var n = parseInteger(scan());

    var result = 1;
    var i = 1;
    while i <= n {
        result *= i;
        i += 1;
    }

    print_i(result);
}
```

### Example 3: Counter

The following example demonstrates closures in Kush.

```
i32 Counter();

Counter makeCounter(i32 i) {
	 return @ -> i += 1;
}

void main() {
    var next = makeCounter(-1);
    while next() < 10 {
        print('Hello, world!');
    }
}
```

## Installation

Before you install Kush, you need to install JTK and pkg-config on your system.

JTK is a library that provides core utilities such as collections, unit testing,
I/O streams, threads and much more. You can find the latest version of JTK
[here](https://github.com/itssamuelrowe/jtk).

### Compiling Kush on Windows

For compiling Kush on Windows, you first need to install MSYS2 or WSL.
The steps required to install MSYS2 or WSL are beyond the scope of this
documentation. Although you can find tutorials on installing them on the
Internet.

Please follow the steps given below if you are compiling on MSYS2. If you
are using WSL to compile Kush, please follow the steps described under the
Linux section.

1. Extract the source code to any directory of your choice, for the sake of this
   documentation we will refer this location as 'C:\kush'.
2. Change the directory to 'C:\kush'.
    ```
    cd 'C:/kush'
    ```
3. Create a temporary directory for compiling the source code. For the sake of this
   documentation we will refer to this directory as 'build'.
   ```
   mkdir build
   ```
4. Change directory to the newly created directory.
   ```
   cd build
   ```
5. Invoke CMake to generate make files. In this documentation, we show the
   commands to generate GNU Makefiles. You can easily generate other makefiles
   similarly.
   ```
   cmake .. -G 'MSYS Makefiles'
   ```
6. Invoke the GNU Make program. The command may be different on your system,
   if you have not setup an alias.
   ```
   make
   ```
   This should compile the compiler, virtual machine, and all the other executables.
   Archives and executable files should be produced in your current working directory.

   As of this version, a plethora of warnings are generated. We are working to
   eradicate these warnings.

### Compiling Kush on Linux and WSL

1. For compiling Kush on Linux or WSL, you need CMake and GNU Make (or any other make
   utility that CMake is compatible with).
2. Extract the source code to any directory of your choice, for the sake of this
   documentation we will refer this location as '/mnt/g/kush'.
3. Change the directory to '/mnt/g/kush'.
    ```
    cd '/mnt/g/kush'
    ```
4. Create a temporary directory for compiling the source code. For the sake of this
   documentation we will refer to this directory as 'build'.
   ```
   mkdir build
   ```
5. Change directory to the newly created directory.
   ```
   cd build
   ```
6. Invoke CMake to generate make files. In this documentation, we show the
   commands to generate the default Makefiles, on our system GNU Makefiles is
   the default target. You can easily generate other makefiles by specifying
   the target make files using the `-G` flag.
   ```
   cmake ..
   ```
7. Invoke the GNU Make program.
   ```
   make
   ```
   This should compile the compiler, virtual machine, and all the other executables.
   Archives and executable files should be produced in your current working directory.

   As of this version, a plethora of warnings are generated. We are working to
   eradicate these warnings.

## Contributing

We welcome all contributors.

Kush was created with a vision to help programmers, like you and I, write code
better. With your contributions, we can get there sooner. We appreciate your help!

## License

Copyright (C) 2017-2020 Samuel Rowe, Joel E. Rego

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
