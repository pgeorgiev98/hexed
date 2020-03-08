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
	testDeleteBytes("abcdef", {1, 3});

	testDeleteBytes(createByteArray(10000, [](int i) { return i; }),
					createVector<int>(100, [](int i) { return (i * 11 + 71) % (10000 - i); }));

	testDeleteBytes(createByteArray(1000000, [](int i) { return i * 7 + 5; }),
					createVector<int>(10000, [](int i) { return (i * 13 + 1) % (1000000 - i); }));
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
