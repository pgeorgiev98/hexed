#include <QtTest>
#include <QByteArray>
#include <QFile>
#include <QTemporaryFile>

#include <algorithm>

#include "bufferededitor.h"

class TestObject : public QObject
{
	Q_OBJECT

private slots:
	void testReading();
	void testDeleting();
	void testReadingAndDeleting();

private:
	void testReadingHelper(const QByteArray &data, const QVector<int> &indicesToRead);
	void testDeletingHelper(const QByteArray &data, const QVector<int> &indicesToDelete);
	void testReadingAndDeletingHelper(const QByteArray &data, const QVector<QPair<int, int>> &indicesToDeleteAndRead);

	template <typename R, typename C>
	C createContainer(int size, std::function<R(int)> func)
	{
		C container;
		container.resize(size);
		for (int i = 0; i < size; ++i)
		container[i] = func(i);
		return container;
	}

	template <typename R>
	QVector<R> createVector(int size, std::function<R(int)> func)
	{
		return createContainer<R, QVector<R>>(size, func);
	}

	QByteArray createByteArray(int size, std::function<char(int)> func)
	{
		return createContainer<char, QByteArray>(size, func);
	}
};

void TestObject::testReading()
{
	// Basic read test
	testReadingHelper("1234567890", {9, 2, 5, 0});

	// Test with bigger file and 'random' reads
	testReadingHelper(createByteArray(10'000, [](int i) { return i; }),
					createVector<int>(100, [](int i) { return (i * 11 + 71) % 10'000; }));

	// Test with a very large file and 'random' reads
	testReadingHelper(createByteArray(1'000'000, [](int i) { return i * 7 + 5; }),
					createVector<int>(10'000, [](int i) { return (i * 13 + 1) % 1'000'000; }));

	// Read the first half of a file
	testReadingHelper(createByteArray(100'000, [](int i) { return i * 11 + 5; }),
					QVector<int>(50'000, 0));

	// Read the second half of a file
	testReadingHelper(createByteArray(100'000, [](int i) { return i * 11 + 5; }),
					QVector<int>(50'000, 100'000 - 1));

	// Read 'random' bytes near the begining and near the end of the file
	testReadingHelper(createByteArray(1'000'000, [](int i) { return i * 9 + 3; }),
					createVector<int>(10'000, [](int i) { return (i * 13 + 1) % 10'000; }) +
					createVector<int>(10'000, [](int i) { return 960'000 + (i * 13 + 1) % 10'000; }));

	// Read 'random' bytes in the middle of the file
	testReadingHelper(createByteArray(1'000'000, [](int i) { return i * 17 + 3; }),
					createVector<int>(10'000, [](int i) { return 500'000 + (i * 23 + 13) % 10'000; }));
}

void TestObject::testDeleting() {
	// Basic delete test
	testDeletingHelper("abcdef", {1, 3});

	// Test with a bigger file and 'random' deletions
	testDeletingHelper(createByteArray(10'000, [](int i) { return i; }),
					createVector<int>(100, [](int i) { return (i * 11 + 71) % (10'000 - i); }));

	// Test with a very large file and 'random' deletions
	testDeletingHelper(createByteArray(1'000'000, [](int i) { return i * 7 + 5; }),
					createVector<int>(10'000, [](int i) { return (i * 13 + 1) % (1'000'000 - i); }));

	// Delete the first half of a file
	testDeletingHelper(createByteArray(100'000, [](int i) { return i * 11 + 5; }),
					QVector<int>(50'000, 0));

	// Delete the second half of a file
	testDeletingHelper(createByteArray(100'000, [](int i) { return i * 11 + 5; }),
					createVector<int>(50'000, [](int i) { return 100'000 - 1 - i; }));

	// Delete 'random' bytes near the begining and near the end of the file
	testDeletingHelper(createByteArray(1'000'000, [](int i) { return i * 9 + 3; }),
					createVector<int>(10'000, [](int i) { return (i * 13 + 1) % (10'000); }) +
					createVector<int>(10'000, [](int i) { return 960'000 + (i * 13 + 1) % (10'000); }));

	// Delete 'random' bytes in the middle of the file
	testDeletingHelper(createByteArray(1'000'000, [](int i) { return i * 17 + 3; }),
					createVector<int>(10'000, [](int i) { return 500'000 + (i * 23 + 13) % (10'000); }));
}

void TestObject::testReadingAndDeleting()
{
	// Basic delete test
	testReadingAndDeletingHelper("abcdefghi", {{0, 7}, {3, 5}, {5, 0}});

	// Test with a lot of 'random' deletions/reads
	testReadingAndDeletingHelper(createByteArray(1'000'000, [](int i) { return i * 7 + 5; }),
								 createVector<QPair<int, int>>(10'000, [](int i) {
									return QPair<int, int>((i * 13 + 1) % (1'000'000 - i),
														   (i * 23 + 5) % (1'000'000 - i)); }));

	// Delete from the begining and read from the end
	testReadingAndDeletingHelper(createByteArray(1'000'000, [](int i) { return i * 7 + 5; }),
								 createVector<QPair<int, int>>(10'000, [](int i) {
									return QPair<int, int>((i * 17 + 3) % 100'000,
														   900'000 + (i * 13 + 7) % (100'000 - i)); }));
}

void TestObject::testReadingHelper(const QByteArray &data, const QVector<int> &indicesToRead)
{
	QTemporaryFile file;
	QVERIFY(file.open());
	file.write(data);
	{
		BufferedEditor e(&file);
		for (int index : indicesToRead) {
			e.seek(index);
			auto byte = e.getByte();
			Q_ASSERT(byte.current);
			Q_ASSERT(*byte.current == data[index]);
		}
	}
}

void TestObject::testDeletingHelper(const QByteArray &data, const QVector<int> &indicesToDelete)
{
	QByteArray expectedData = data;
	for (int index : indicesToDelete)
		expectedData.remove(index, 1);

	QTemporaryFile file;
	QVERIFY(file.open());
	file.write(data);
	{
		BufferedEditor e(&file);
		for (int index : indicesToDelete) {
			e.seek(index);
			e.deleteByte();
		}
		QVERIFY(e.writeChanges());
	}
	QVERIFY(file.seek(0));
	QByteArray actualData = file.readAll();
	QCOMPARE(actualData, expectedData);
}

void TestObject::testReadingAndDeletingHelper(const QByteArray &data, const QVector<QPair<int, int>> &indicesToDeleteAndRead)
{
	QByteArray expectedData = data;

	QTemporaryFile file;
	QVERIFY(file.open());
	file.write(data);
	{
		BufferedEditor e(&file);
		for (auto index : indicesToDeleteAndRead) {
			e.seek(index.first);
			e.deleteByte();
			expectedData.remove(index.first, 1);
			e.seek(index.second);
			auto b = e.getByte();
			Q_ASSERT(b.current);
			Q_ASSERT(*b.current == expectedData[index.second]);
		}
		QVERIFY(e.writeChanges());
	}
	QVERIFY(file.seek(0));
	QByteArray actualData = file.readAll();
	QCOMPARE(actualData, expectedData);
}

QTEST_MAIN(TestObject)
#include "tests.moc"
