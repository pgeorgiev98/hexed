<img src="https://raw.githubusercontent.com/pgeorgiev98/hexed/master/res/icon.png" width="96" height="96" />

# Hexed

An open-source, cross-platform binary file hex editor written in C++ with Qt5.

## Features
- Displays both the hex and ascii values of the bytes
- Loads only the parts of the file that are needed in memory
- Can have many files open at once in tabs
- Undo and redo functionality

And that's pretty much it :)

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

