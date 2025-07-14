# Nibble

**Nibble** is an instruction set for a virtual machine that runs programs written in hexadecimal-encoded bytecode.

## Instruction Set

Registers (and branch-conditions) are encoded as unsigned 4-bit binary (i.e., nibbles), represented by one hexadecimal digit; whereas, addresses and values are encoded as signed 8-bit binary (i.e., bytes), represented by two hexadecimal digits.

> [!IMPORTANT]
>
> 1. **'Opc.'** stands for 'opcode'; the operation.
> 2. **'Opr.'** stands for 'operands'; the operand(s) of an operation.

| **Opc.** | **Opr.** | **Description**                                                                            |
|----------|----------|------------------------------------------------------------------------------------------|
| `0`      | `___`    | **Halt** the program.                                                                    |
| `1`      | `RXY`    | **Read** value from address `XY` and store into register `R`.                            |
| `2`      | `RXY`    | **Write** value from register `R` into address `XY`.                                     |
| `3`      | `RXY`    | **Copy** value `XY` into register `R`.                                                   |
| `4`      | `RS_`    | **Duplicate** register `S` into register `R`.                                            |
| `5`      | `RST`    | **Add** register `T` to register `S` and store into register `R`.                        |
| `6`      | `RST`    | **Subtract** register `T` from register `S` and store into register `R`.                 |
| `7`      | `RXY`    | **Shift** register `R` by `XY`; left if positive, otherwise right.                       |
| `8`      | `RS_`    | **NOT** of `S`, stored into register `R`.                                                |
| `9`      | `RST`    | **AND** of `S` and `T`, stored into register `R`.                                        |
| `A`      | `RST`    | **OR** of `S` and `T`, stored into register `R`.                                         |
| `B`      | `RST`    | **XOR** of `S` and `T`, stored into register `R`.                                        |
| `C`      | `RL_`    | **Compare** register `L` with register `R`.                                              |
| `D`      | `CXY`    | **Branch** to address `XY`, only if `L` and `R` satisfy condition `C`:                   |
|          | `0`      | None                                                                                     |
|          | `1`      | Equal ($=$)                                                                              |
|          | `2`      | Not Equal ($\neq$)                                                                       |
|          | `3`      | Less Than ($<$)                                                                          |
|          | `4`      | Greater Than ($>$)                                                                       |
|          | `5`      | Less Than or Equal ($\leq$)                                                              |
|          | `6`      | Greater Than or Equal ($\geq$)                                                           |
| `E`      | `R__`    | **Input** value into register `R`.                                                       |
| `F`      | `R__`    | **Output** value from register `R`.                                                      |

> [!NOTE]
> - **NOT**, **AND**, **OR**, and **XOR** are bitwise operations.
> - `_` should be replaced by `0`.

---

## Examples

All of the examples are found in the `examples` folder.

### 1. Addition

Read two values, add them together, then write the result.

| **Ln.**  | **01**   | **02**   | **Comment**                                                                                                                                                                                            |
|----------|----------|----------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **00.**  |  `E1`    |  `00`    | **Input** value into register `R1`.                                                                                                                                                                    |
| **02.**  |  `E2`    |  `00`    | **Input** value into register `R2`.                                                                                                                                                                    |
| **04.**  |  `53`    |  `12`    | **Add** register `R2` to register `R1`, stored into register `R3`.                                                                                                                                     |
| **06.**  |  `F3`    |  `00`    | **Output** value from register `R3`.                                                                                                                                                                   |
| **08.**  |  `00`    |  `00`    | **Halt** the program.                                                                                                                                                                                  |

> **`addition.nib`**
>
> ```
> E100
> E200
> 5312
> F300
> 0000
> ```

### 2. Multiplication

Input two values, multiply them together, then output the result.

> [!WARNING]
> Values are assumed to be positive or zero, not negative.

| **Ln.**  | **01**   | **02**   | **Comment**                                                                                                                                                                                            |
|----------|----------|----------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **00.**  |  `30`    |  `00`    | **Copy** value `0` into register `R0`.                                                                                                                                                                 |
| **02.**  |  `E1`    |  `00`    | **Input** value into register `R1`.                                                                                                                                                                    |
| **04.**  |  `E2`    |  `00`    | **Input** value into register `R2`.                                                                                                                                                                    |
| **06.**  |  `33`    |  `01`    | **Copy** value `0x01` into register `R3`.                                                                                                                                                              |
| **08.**  |  `C3`    |  `20`    | **Compare** register `R2` with register `R3`.                                                                                                                                                          |
| **0A.**  |  `D3`    |  `12`    | **Branch** to address `0x12`, if `R2` is less than `R3`.                                                                                                                                               |
| **0C.**  |  `50`    |  `01`    | **Add** register `R1` to register `R0`, stored into register `R0`.                                                                                                                                     |
| **0E.**  |  `62`    |  `23`    | **Subtract** register `R3` from register `R2`, stored into register `R2`.                                                                                                                              |
| **10.**  |  `D0`    |  `08`    | **Branch** to address `0x08`.                                                                                                                                                                          |
| **12.**  |  `F0`    |  `00`    | **Output** value from register `R0`.                                                                                                                                                                   |
| **14.**  |  `00`    |  `00`    | **Halt** the program.                                                                                                                                                                                  |

> **`multiplication.nib`**
>
> ```
> 3000
> E100
> E200
> 3301
> C320
> D312
> 5001
> 6223
> D008
> F000
> 0000
> ```

---

## Build

Use [`make`](https://www.gnu.org/software/make/) to build, delete, and test the program.

### Usage

```
make [targets]
```

### Targets

| Target     | Description                                    |
|------------|------------------------------------------------|
| `all`      | Build the program.                             |
| `delete`   | Remove the program.                            |
| `help`     | Display the usage message.                     |
| `test`     | Run the test cases.                            |

---

## Edit

It is recommended to install the [Hex Editor](https://marketplace.visualstudio.com/items?itemName=ms-vscode.hexeditor) extension for [Visual Studio Code](https://code.visualstudio.com) in a [Codespace](https://github.com/features/codespaces).

> [!NOTE]
> - Open a `.nib` file by right-clicking on it in the "Explorer", selecting "Open With...", and then "Hex Editor".
> - "Open Settings" via the cog icon and then set "Show Decoded Text" to `false` and "Bytes per row" to `2`.

---

## Run

### Usage

```
usage: nib [options] file
```

### Options

| Short Flag | Long Flag       | Description                  |
|------------|-----------------|------------------------------|
| `-d`       | `--debug`       | Enable debugging output.     |
| `-h`       | `--help`        | Display the usage message.   |
| `-v`       | `--version`     | Display the program version. |

---

Copyright &copy; 2025 Jack C. Lloyd.  
All rights reserved.
