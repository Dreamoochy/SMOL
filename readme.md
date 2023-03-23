# Switch-off Monitor On screen Lock (SMOL)

A little utility that switches off monitor when Windows session is locked

---

### How To Use

* Execute app
* Lock session (e.g. via Win+L) - now monitor should be switched off[^1]
* Terminate app by executing it once again

---

### Build notes

**Masm32**
* Add Masm32 **Bin** path to PATH environment variable
* Add Masm32 **Lib** path to LIB environment variable
* Add Masm32 **Include** path to INCLUDE environment variable

**GCC**
* Add GCC **Bin**, **Lib**, **Include** paths to PATH environment variable

Run **build.cmd** for the appropriate version (masm32 or gcc-cpp)

**Rust**
* Use `cargo build`

---

### Supported Operating Systems
MS Windows is the only supported OS. Tested on 32-bit and 64-bit edition.

[^1]: On Windows 10+ session lock != screen lock. The session considered locked when password box becomes visible.