Ecryptfs-Tools-for-Android
==========================

What's the purpose of this project?
---
The aim of this project is to provide increased security and privacy for devices running Android OS.
There are 2 main privacy features that end users and developers may benefit from:

      a) User data encryption
      
      b) API for building and managing local secure storage

How do you plan to achieve that?
---
By providing the core tools and libraries for creating and managing secure crypto containers based on ecryptfs.

Why did you choose encrypted file system instead of using already available block device encryption (dm-crypt)?
---
First granularity and second the ability to mount a file system without specifying the amount of space needed to extend into.

What are these tools and libraries you are talking about?
---
The core library for providing secure storage is libefs (encrypted file storage library). Everything else revolves around the core:

    libkeyutils : a LGPL shared library used to manage ecryptfs keys. The library is linked dynamically with libefs
    efs-tools: tool to test libefs api
    libefs_init - small static library to be built with init.

Are these libraries and tools sufficient to build the Android features?
---
No. The android_integration folder will provide vold, init, framework and Settings integration.

--Catalin

