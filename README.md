<img src="https://raw.githubusercontent.com/pgeorgiev98/hexed/master/app/res/icon.png" width="96" height="96" />

# Hexed

An open-source, cross-platform binary file hex editor written in C++ with Qt5.

## Features
- [x] Displays both the hex and ascii values of the bytes
- [x] Loads only the parts of the file that are needed in memory
- [x] Can have many files open at once in tabs
- [x] Can replace bytes and append bytes to the file
- [ ] Can insert bytes at any position in the file
- [x] Can delete bytes at any position in the file
- [x] Supports both hex and ascii keyboard input
- [ ] Can search the file
- [x] Has undo and redo functionality
- [ ] Can compare binary files side by side

Be aware that this is in a pretty early stage of development and is just a hobby project of mine,
so **I cannot give any guarantees that you will not lose your data**.
And, of course, any feedback for such issues is greatly appreciated.

You've been warned.

## Installing from source

### Compiling

	git clone https://github.com/pgeorgiev98/hexed
	mkdir hexed/build
	cd hexed/build

	# Make sure you're using the qt5 version of qmake. On some distributions
	# you may have to call `qmake -qt=5 ..` instead for example
	qmake ..

	# You may use make -jN where N is the number of threads your CPU has.
	# This should greatly speed up the compilation
	make

	# You can now run the application without installing it
	./hexed

### Installing

After compiling you can install it by running

	sudo make install

