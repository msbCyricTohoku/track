# track

A simple directory tracking tool and lightweight alternative to Git, designed for near-instantaneous state snapshots.

**by msb - Dec 2025**

---

`track` is built for developers who need a super-fast local versioning system without the overhead of a full VCS. It uses optimized internal copy logic to capture directory states in milliseconds.
The tracked snapshots will be present in `.track` directory inside the directory you executed track!

* **Lightning Fast:** Snapshot logic designed for speed.
* **Simple Logic:** Minimalist approach to directory tracking.
* **Low Overhead:** No hidden databases or complex staging areas.
* **Portable:** Compiles into a single, standalone binary.
---
Build the tool using the provided Makefile:

```bash
make
```
---
```
trk
trk log
trk diff 2025-12-30_212338
```
---
Enjoy! -- msb
