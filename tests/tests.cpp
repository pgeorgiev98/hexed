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
	void testDeletion();

private:
	void testDeleteBytes(const QByteArray &data, const QVector<int> &indicesToDelete);

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

void TestObject::testDeletion() {
	// Basic delete test
	testDeleteBytes("abcdef", {1, 3});

	// Test with a bigger file and 'random' deletions
	testDeleteBytes(createByteArray(10'000, [](int i) { return i; }),
					createVector<int>(100, [](int i) { return (i * 11 + 71) % (10'000 - i); }));

	// Test with a very large file and 'random' deletions
	testDeleteBytes(createByteArray(1'000'000, [](int i) { return i * 7 + 5; }),
					createVector<int>(10'000, [](int i) { return (i * 13 + 1) % (1'000'000 - i); }));

	// Delete the first half of a file
	testDeleteBytes(createByteArray(100'000, [](int i) { return i * 11 + 5; }),
					QVector<int>(50'000, 0));

	// Delete the second half of a file
	testDeleteBytes(createByteArray(100'000, [](int i) { return i * 11 + 5; }),
					createVector<int>(50'000, [](int i) { return 100'000 - 1 - i; }));

	// Delete 'random' bytes near the begining and near the end of the file
	testDeleteBytes(createByteArray(1'000'000, [](int i) { return i * 9 + 3; }),
					createVector<int>(10'000, [](int i) { return (i * 13 + 1) % (10'000); }) +
					createVector<int>(10'000, [](int i) { return 960'000 + (i * 13 + 1) % (10'000); }));

	// Delete 'random' bytes in the middle of the file
	testDeleteBytes(createByteArray(1'000'000, [](int i) { return i * 17 + 3; }),
					createVector<int>(10'000, [](int i) { return 500'000 + (i * 23 + 13) % (10'000); }));
}

void TestObject::testDeleteBytes(const QByteArray &data, const QVector<int> &indicesToDelete)
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

QTEST_MAIN(TestObject)
#include "tests.moc"
