# 🐚 MicroShell: A Minimalist Unix Shell

MicroShell is a lightweight Unix-like shell implemented in C. It allows the execution of user commands in a custom terminal environment and supports common shell operations such as command chaining, piping, background processes, file redirection, and conditional execution.

---

## ✨ Features

- ✅ **Basic Command Execution**  
  Supports simple commands with 1 to 4 arguments.

- 🛑 **Session Termination**  
  `dter` — safely exits the shell session.

- 🔢 **Text File Analysis**  
  `# filename.txt` — counts the number of words in a `.txt` file.

- 📚 **File Concatenation**  
  `file1.txt ~ file2.txt ~ ...` — concatenates up to 4 `.txt` files.

- ⚙️ **Background Processing**  
  Add `+` to run commands in the background. Use `fore` to bring them back to the foreground.

- 🔗 **Piping**  
  Supports up to 4 commands chained using `|`.

- 📤 **Input & Output Redirection**  
  - `<` — input redirection  
  - `>` — output redirection  
  - `>>` — append output to a file

- 📋 **Sequential Execution**  
  Use `;` to run multiple commands one after another.

- 🔀 **Conditional Execution**  
  Supports logical operations with `&&` (AND) and `||` (OR), up to 4 conditions.

---

## 🛠 Compilation

To compile MicroShell:

```bash
gcc -o microshell microshell.c
```

---

## 🚀 Running MicroShell

Start the shell using:

```bash
./microshell
```

MicroShell will begin a command loop waiting for user input.

---

## 🧪 Example Commands

### 🔹 Exit MicroShell
```bash
microshell$ dter
```

### 🔹 Count words in a file
```bash
microshell$ # sample.txt
```

### 🔹 Concatenate `.txt` files
```bash
microshell$ file1.txt ~ file2.txt ~ file3.txt ~ file4.txt
```

### 🔹 Run a command in the background
```bash
microshell$ command arg1 arg2 +
```

### 🔹 Bring background process to foreground
```bash
microshell$ fore
```

### 🔹 Pipe multiple commands
```bash
microshell$ ls -l | grep txt | wc | wc -w
```

### 🔹 Redirect input/output
```bash
# Input from file
microshell$ grep "text" < input.txt

# Output to file
microshell$ ls -l > output.txt

# Append output to file
microshell$ date >> log.txt
```

### 🔹 Execute commands sequentially
```bash
microshell$ date ; pwd ; ls -l
```

### 🔹 Conditional command execution
```bash
microshell$ command1 && command2 || command3 && command4
```

---

## 🙋‍♀️ Author

**Aditi Patel**  
👩‍💻 [GitHub](https://github.com/pateladiti0401)  
📧 pateladiti542@gmail.com
