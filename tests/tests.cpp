#include <QtTest>
#include <QByteArray>
#include <QFile>
#include <QTemporaryFile>

#include <algorithm>

#include "bufferededitor.h"
#include "finder.h"

class TestObject : public QObject
{
	Q_OBJECT

private slots:
	void testReading();
	void testDeleting();
	void testReadingAndDeleting();
	void testReadingInsertingAndDeleting();
	void testUndoRedo();
	void testFindNext();

private:
	struct Indices4
	{
		int r, i, d;
		quint8 v;
	};

	struct Indices6
	{
		int r, i, d;
		quint8 v;
		int undo, redo;
	};

	void testReadingHelper(const QByteArray &data, const QVector<int> &indicesToRead);
	void testDeletingHelper(const QByteArray &data, const QVector<int> &indicesToDelete);
	void testReadingAndDeletingHelper(const QByteArray &data, const QVector<QPair<int, int>> &indicesToDeleteAndRead);
	void testReadingInsertingAndDeletingHelper(const QByteArray &data, const QVector<Indices4> &indices);
	void testUndoRedoHelper(const QByteArray &data, const QVector<Indices6> &indices);
	void testFindNextHelper(const QByteArray &data, const QVector<QPair<int, QByteArray>> &queries);

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

void TestObject::testReadingInsertingAndDeleting()
{
	// Test with a lot of 'random' insertions/deletions/reads
	testReadingInsertingAndDeletingHelper(createByteArray(1'000'000, [](int i) { return i * 7 + 5; }),
										  createVector<Indices4>(10'000, [](int i) {
											  return Indices4 {
												  (i * 13 + 9) % 950'000,
												  (i * 3 + 53) % 950'000,
												  (i * 17 + 7) % 950'000,
												  quint8(i * 23 + 2) }; }));

	// Delete from the begining, insert in the middle and read from everywhere
	testReadingInsertingAndDeletingHelper(createByteArray(1'000'000, [](int i) { return i * 7 + 5; }),
										  createVector<Indices4>(10'000, [](int i) {
											  return Indices4 {
												  (i * 12 + 7) % 200'000,
												  (i * 5 + 11) % 200'000 + 200'000,
												  (i * 52 + 1) % 950'000,
												  quint8(i * 7 + 11) }; }));

	// Insert in the begining, delete from the middle and read from everywhere
	testReadingInsertingAndDeletingHelper(createByteArray(1'000'000, [](int i) { return i * 7 + 5; }),
										  createVector<Indices4>(10'000, [](int i) {
											  return Indices4 {
												  (i * 77 + 83) % 200'000 + 200'000,
												  (i * 41 + 63) % 200'000,
												  (i * 12 + 5) % 950'000,
												  quint8(i * 23 + 2) }; }));
}

void TestObject::testUndoRedo()
{
	// Same as testReadingInsertingAndDeleting, but with a few undo/redos

	// Test with a lot of 'random' insertions/deletions/reads
	testUndoRedoHelper(createByteArray(1'000'000, [](int i) { return i * 7 + 5; }),
					   createVector<Indices6>(10'000, [](int i) {
						   return Indices6 {
							   (i * 13 + 9) % 950'000,
							   (i * 3 + 53) % 950'000,
							   (i * 17 + 7) % 950'000,
							   quint8(i * 23 + 2),
							   (i * 2 + 3) % 5,
							   (i * 3) % 8 }; }));

	// Delete from the begining, insert in the middle and read from everywhere
	testUndoRedoHelper(createByteArray(1'000'000, [](int i) { return i * 7 + 5; }),
					   createVector<Indices6>(10'000, [](int i) {
						   return Indices6 {
							   (i * 12 + 7) % 200'000,
							   (i * 5 + 11) % 200'000 + 200'000,
							   (i * 52 + 1) % 950'000,
							   quint8(i * 7 + 11),
							   (i * 2 + 3) % 5,
							   (i * 3) % 8 }; }));

	// Insert in the begining, delete from the middle and read from everywhere
	testUndoRedoHelper(createByteArray(1'000'000, [](int i) { return i * 7 + 5; }),
					   createVector<Indices6>(10'000, [](int i) {
						   return Indices6 {
							   (i * 77 + 83) % 200'000 + 200'000,
							   (i * 41 + 63) % 200'000,
							   (i * 12 + 5) % 950'000,
							   quint8(i * 23 + 2),
							   (i * 2 + 3) % 5,
							   (i * 3) % 8 }; }));
}

void TestObject::testFindNext()
{
	testFindNextHelper("foo bar baz abb aabbabb aaabbaa", {{0, "aabbaa"}});
	testFindNextHelper("foo bar foooo abb asdbafwoofofooo aaabbfooaa", {{0, "foo"}});

	testFindNextHelper(createByteArray(100'000, [](int i) { return i; }),
					   {{0, createByteArray(10, [](int i) { return 100 + i; })}});
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

void TestObject::testReadingInsertingAndDeletingHelper(const QByteArray &data, const QVector<Indices4> &indices)
{
	QByteArray expectedData = data;

	QTemporaryFile file;
	QVERIFY(file.open());
	file.write(data);
	{
		BufferedEditor e(&file);
		for (auto index : indices) {
			e.seek(index.r);
			auto b = e.getByte();
			Q_ASSERT(b.current);
			Q_ASSERT(*b.current == expectedData[index.r]);

			e.seek(index.d);
			e.insertByte(index.v);
			expectedData.insert(index.d, index.v);

			e.seek(index.d);
			e.deleteByte();
			expectedData.remove(index.d, 1);
		}
		QVERIFY(e.writeChanges());
	}
	QVERIFY(file.seek(0));
	QByteArray actualData = file.readAll();
	QCOMPARE(actualData, expectedData);
}

void TestObject::testUndoRedoHelper(const QByteArray &data, const QVector<Indices6> &indices)
{
	QByteArray expectedData = data;

	struct Modification
	{
		enum class Type
		{
			Insert, Delete
		};

		Type type;
		char byte;
		qint64 pos;
	};
	QVector<Modification> modifications;
	int modificationIndex = 0;

	QTemporaryFile file;
	QVERIFY(file.open());
	file.write(data);
	{
		BufferedEditor e(&file);
		for (auto index : indices) {
			e.seek(index.r);
			auto b = e.getByte();
			Q_ASSERT(b.current);
			QCOMPARE(*b.current, expectedData[index.r]);

			if (modificationIndex < modifications.size()) {
				modifications.remove(modificationIndex, modifications.size() - modificationIndex);
				QCOMPARE(modifications.size(), modificationIndex);
			}

			e.seek(index.i);
			e.insertByte(index.v);
			expectedData.insert(index.i, index.v);
			modifications.append(Modification{Modification::Type::Insert, char(index.v), index.i});
			++modificationIndex;

			e.seek(index.d);
			e.deleteByte();
			modifications.append(Modification{Modification::Type::Delete, char(expectedData[index.d]), index.d});
			expectedData.remove(index.d, 1);
			++modificationIndex;

			while (index.undo) {
				QCOMPARE((modificationIndex > 0), e.canUndo());
				if (!e.canUndo())
					break;
				--index.undo;
				e.undo();
				Modification &m = modifications[--modificationIndex];
				switch (m.type) {
				case Modification::Type::Insert: {
					expectedData.remove(m.pos, 1);
					break;
				}
				case Modification::Type::Delete: {
					expectedData.insert(m.pos, m.byte);
					break;
				}
				}
			}

			while (index.redo) {
				QCOMPARE((modificationIndex < modifications.size()), e.canRedo());
				if (!e.canRedo())
					break;
				--index.redo;
				e.redo();
				Modification &m = modifications[modificationIndex++];
				switch (m.type) {
				case Modification::Type::Insert: {
					expectedData.insert(m.pos, m.byte);
					break;
				}
				case Modification::Type::Delete: {
					expectedData.remove(m.pos, 1);
					break;
				}
				}
			}
		}
		QVERIFY(e.writeChanges());
	}
	QVERIFY(file.seek(0));
	QByteArray actualData = file.readAll();
	QCOMPARE(actualData, expectedData);
}

void TestObject::testFindNextHelper(const QByteArray &data, const QVector<QPair<int, QByteArray> > &queries)
{
	QTemporaryFile file;
	QVERIFY(file.open());
	file.write(data);
	BufferedEditor e(&file);
	for (auto q : queries) {
		Finder *finder = new Finder(&e);
		finder->search(q.first, q.second);

		qint64 position = q.first;
		qint64 match;
		do {
			finder->findNext();

			match = -1;
			for (; position < data.size() - q.second.size() + 1; ++position) {
				int j;
				for (j = 0; j < q.second.size(); ++j)
					if (data[int(position) + j] != q.second[j])
						break;
				if (j == q.second.size()) {
					match = position;
					position += j;
					break;
				}
			}

			qint64 result = finder->searchResultPosition();
			QCOMPARE(result, match);
		} while (match != -1);
	}
}

QTEST_MAIN(TestObject)
#include "tests.moc"
