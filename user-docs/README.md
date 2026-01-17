Man pages and user documentation must be rebuilt manually after modifying rst
files.  Requires sphinx-build installed on system.

Rebuilding is done from the project binary directory (usually `glabels-qt/build`).

Man Pages
---------
```
$ make manpages
```

QtHelp Pages
------------
```
$ make qthelp-manual
```

Simple HTML Pages
-----------------
These are suitable to peruse using a web browser.
```
$ make html-manual
```

Rebuilding All User Documentation
---------------------------------
```
$ make user-docs
```
