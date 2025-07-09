## Memstore - In-Memory Key Value Store
Fast access/retrieval key-value pair based database that stores data entirely in the RAM.
Similar in concept to Redis and Memcached. Only works on linux-based systems or WSL.

---

## 1.0 About
Memstore utilizes the unordered_map from C++ STL for storing data in-memory. Persistence is achieved by maintaining a data.log file in disk which is updated on every data change.

- 1.1 Data Type support:
    1. Integer
    2. Long Integer
    3. Doubles
    4. Boolean values
    5. Strings

*keys are always strings*

- 1.2 Program provides two modes:
    1. CLI: Default mode. Allows running memstore commands from the command-line.
    2. Server: Runs the program API, allowing remote command execution over HTTP (In development).

- 1.3 Authorization: Program can be configured for request authorization.

- 1.4 Encryption: Data can be encrypted using a tiny encryption algorithm (TEA) when transmitted over HTTP. This feature is not implemented yet.
*encryption and authorization need to be activated using a server.conf file. See section 4*

---

## 2.0 Usage
*Only works on linux-based systems or WSL.*

### 2.1 Prerequisites 
- Makefile
- GNU Compiler
- For auth & encryption, .env and server.conf files.

### 2.2 Build
- Run make to build
```bash
make
```
- An executable named 'bin' will be created

### 2.3 Run
- CLI Mode
```bash
./bin // default
OR
./bin -m cli
```

- Server mode
```bash
./bin -m server
```

- Program will create a data.log file (if it doesn't exist) for persistent storage.
- On every subsequent startup, the content from the data.log file will be loaded and a key-value map will be constructed.

---

## 3.0 CLI
Memstore has 3 basic commands:

- 3.1 **GET**: Read a value by key and display on the console.
    - SYNTAX: GET <key_name> 
    *case-insensitive*

    ```bash
    GET session_id
    get username 
    ```

- 3.2 **SET**: Create a new key-value pair or update an existing key.
    - SYNTAX: SET <key_name> <value_type> <value>
    - Value types:
        1. Primitives: 
            - Integer = INT
            - Long integer = LONG
            - Double = DOUBLE
            - Boolean = BOOL *==value can be true, 1, false or 0==*
            *case-insensitive*

        ```bash
        SET active INT 23
        SET phone_number LONG 123456789
        set pi double 3.141
        Set is_online bool 1
        Set is_online bool true
        SET is_active BOOL false
        SET is_active BOOL 0
        ```

        2. Strings = STRING:
            - For single-word values, simply write the value in place.
            - For multi-word values, enclose them in "double quotes".

        ```bash
        SET username STRING hassansajid
        SET full_name STRING "Hassan Sajid"
        ```

- 3.3 **DEL**: Delete an existing key-value pair.
    - SYNTAX: DEL <key_name>
    *case-insensitive*

    ```bash
    DEL username
    del active
    ```

- 3.4 **HELP**: Displays and describes available commands.

- 3.5 **EXIT**: Terminates and exits the program.

*all commands are case-insensitive*


## 4.0 Server 
There are 3 GET routes and 1 POST route accessible over HTTP for interacting with the database.

- **4.1 Server configuration:**
    - Server configuration is done using a server.conf file present in the root of the project.
    - Syntax for writing config options: <option_name> <option_value>
    *option_name has to be in uppercase*
    - Configurable options:
        1. PORT: default is 8080
        2. AUTHORIZE: default is 0
            - Value can be either 1 (enabled) or 0 (disabled).
            - If 1, server will require an Authorization header in incoming requests with Basic credentials.
            - If 1, the program requires an AUTH_KEY in a .env file. Syntax for env variables in .env file is same as in server.conf file. 
            - The credential is a non-whitespace token that is matched with the key specified in the .env file.
        3. ENCRYPTION: default is 0
            - Value can be either 1 (enabled) or 0 (disabled).
            - **Not implemented yet.**
            - If 1, program requires a KEY in .env file. Key is strictly a 16 byte no-whitespace token (16 characters).
            - Encryption is done using a handwritten Tiny Encryption Algo (TEA). It is not much secure.
        4. server.config example:
            ```bash
                PORT 5177
                AUTHORIZE 1
                ENCRYPTION 0
            ```

- **4.2 Server routes:**
    There are 3 GET routes and 1 POST route accessible over HTTP for interacting with the database.
    - GET routes:
        1. /: Home route. Returns a json response describing the API. 
        2. /get (?key=some_key): Fetches the value for an existing key. Params are required as specified.
        3. /del (?key=some_key): Deletes an existing key-value pair. Params are required as specified.
    - POST routes:
        1. /set: Creates a new entry, or updates an existing one.
            - As of now, this route only accepts data of x-www-form-urlencoded type.
            - Required data:
                - "key": key name
                - "value": value of the item
                - "type": type of the value. Can be "int", "double", "float", "bool" or "string".


## 5.0 Contribute
- You can submit issues, bugs, ideas and suggestions through the Issues tab.
- Contact me if you wish to contribute.
- hassansajid.dev@gmail.com


