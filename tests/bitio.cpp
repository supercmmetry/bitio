#include <gtest/gtest.h>
#include <bitio.h>

class BitioTest : testing::Test {};

TEST(BitioTest, read_test_1) {
    // Create sample file
    FILE *file = fopen("bitio_test.dat", "wb");
    uint8_t *raw = new uint8_t[1048576];

    for (int i = 0; i < 1048576; i++) {
        raw[i] = i % 256;
    }

    fwrite(raw, 1, 1048576, file);
    fclose(file);


    file = fopen("bitio_test.dat", "rb");

    auto stream = new bitio::stream(file);

    int count = 10;
    for (int i = 0; i < count; i++) {
        stream->read(0x4);
        stream->read(0x1);
        stream->read(0xB);
    }

    ASSERT_EQ(stream->read(0x8), 20);
}

TEST(BitioTest, read_test_2) {
    // Create sample file
    FILE *file = fopen("bitio_test.dat", "wb");
    uint8_t *raw = new uint8_t[100];

    for (int i = 0; i < 100; i++) {
        raw[i] = 0xff;
    }

    fwrite(raw, 1, 100, file);
    fclose(file);

    file = fopen("bitio_test.dat", "rb");

    auto stream = new bitio::stream(file);

    ASSERT_EQ(stream->read(0x8), 0xff);
    ASSERT_EQ(stream->read(0x1), 1);
    ASSERT_EQ(stream->read(0x2), 3);
    ASSERT_EQ(stream->read(0x3), 7);
    ASSERT_EQ(stream->read(0x4), 15);
    ASSERT_EQ(stream->read(0x5), 31);
    ASSERT_EQ(stream->read(0x6), 63);
    ASSERT_EQ(stream->read(0x7), 127);
    ASSERT_EQ(stream->read(10), 1023);
    ASSERT_EQ(stream->read(20), 1048575);
}

TEST(BitioTest, read_test_3) {
    // Create sample file
    FILE *file = fopen("bitio_test.dat", "wb");
    uint8_t *raw = new uint8_t[5];

    for (int i = 0; i < 5; i++) {
        raw[i] = i;
    }

    fwrite(raw, 1, 5, file);
    fclose(file);


    file = fopen("bitio_test.dat", "rb");

    auto stream = new bitio::stream(file, false, 2);

    int count = 2;
    for (int i = 0; i < count; i++) {
        stream->read(0x10);
    }

    ASSERT_EQ(stream->read(0x8), (count << 1) % 256);
}

TEST(BitioTest, read_test_4) {
    // Create sample file
    FILE *file = fopen("bitio_test.dat", "wb");
    uint8_t *raw = new uint8_t[1024];

    for (int i = 0; i < 1024; i++) {
        raw[i] = i % 256;
    }

    fwrite(raw, 1, 1024, file);
    fclose(file);


    file = fopen("bitio_test.dat", "rb");

    auto stream = new bitio::stream(file, false, 1);

    for (int i = 0; i < 32; i++) {
        stream->read(64);
    }

    ASSERT_EQ(stream->read(8), 0);
}

TEST(BitioTest, read_exception_neg_test) {
    // Create sample file
    FILE *file = fopen("bitio_test.dat", "wb");
    uint8_t *raw = new uint8_t[4];

    for (int i = 0; i < 4; i++) {
        raw[i] = i;
    }

    fwrite(raw, 1, 4, file);
    fclose(file);


    file = fopen("bitio_test.dat", "rb");

    auto stream = new bitio::stream(file, 2);

    int count = 2;
    for (int i = 0; i < count; i++) {
        stream->read(0x10);
    }
}

TEST(BitioTest, read_exception_pos_test) {
    // Create sample file
    FILE *file = fopen("bitio_test.dat", "wb");
    uint8_t *raw = new uint8_t[4];

    for (int i = 0; i < 4; i++) {
        raw[i] = i;
    }

    fwrite(raw, 1, 4, file);
    fclose(file);


    file = fopen("bitio_test.dat", "rb");

    auto stream = new bitio::stream(file, false,2);

    int count = 2;
    for (int i = 0; i < count; i++) {
        stream->read(0x10);
    }

    bool flag = false;

    try {
        stream->read(1);
    } catch (...) {
        flag = true;
    }

    ASSERT_TRUE(flag);
}

TEST(BitioTest, write_test_1) {
    FILE *file = fopen("bitio_test.dat", "wb");

    auto stream = new bitio::stream(file, true, 1024);
    stream->write(4, 8);
    stream->write(10, 5);
    stream->write(0xffff, 16);
    stream->write(0xffff, 64);
    stream->flush();
    stream->close();

    file = fopen("bitio_test.dat", "rb");
    stream = new bitio::stream(file, false,2);

    ASSERT_EQ(stream->read(8), 4);
    ASSERT_EQ(stream->read(5), 10);
    ASSERT_EQ(stream->read(16), 0xffff);
    ASSERT_EQ(stream->read(64), 0xffff);
}

TEST(BitioTest, write_test_2) {
    FILE *file = fopen("bitio_test.dat", "wb");

    auto stream = new bitio::stream(file, true, 1);
    for (int i = 0; i < 1024; i++) {
        stream->write(i % 256, 12 + (i % 32));
    }

    stream->flush();
    stream->close();

    file = fopen("bitio_test.dat", "rb");
    stream = new bitio::stream(file, false,1);

    for (int i = 0; i < 1024; i++) {
        ASSERT_EQ(stream->read(12 + (i % 32)), i % 256);
    }
}

TEST(BitioTest, write_test_3) {
    FILE *file = fopen("bitio_test.dat", "wb");

    auto stream = new bitio::stream(file, true,13);
    for (int i = 0; i < 128; i++) {
        stream->write(i % 256, 64);
    }

    stream->flush();
    stream->close();

    file = fopen("bitio_test.dat", "rb");
    stream = new bitio::stream(file, false,19);

    for (int i = 0; i < 128; i++) {
        ASSERT_EQ(stream->read(64), i % 256);
    }
}

TEST(BitioTest, seek_test_1) {
    remove("bitio_test.dat");
    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, true, 1);
    stream->write(0xfeff, 16);
    stream->seek(-0x8);
    stream->flush();
    ASSERT_EQ(stream->read(0x8), 0xff);
}

TEST(BitioTest, seek_test_2) {
    remove("bitio_test.dat");
    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, true, 1);
    stream->write(0xffff, 16);
    stream->seek(-0x1);
    stream->seek(-0x7);
    stream->seek(0x8);
    stream->seek(-0x8);

    ASSERT_EQ(stream->read(0x8), 0xff);
}

TEST(BitioTest, seek_rw_test_1) {
    remove("bitio_test.dat");

    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, true, 100);
    stream->write(0xfff3, 16);
    stream->seek(-0x8);
    stream->write(0xfe, 0x8);
    stream->seek(-0x8);

    ASSERT_EQ(stream->read(0x8), 0xfe);
}

TEST(BitioTest, seek_rw_test_2) {
    remove("bitio_test.dat");
    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, true, 100);

    stream->write(0xff, 12);
    stream->seek(-1);
    stream->seek(-11);
    ASSERT_EQ(stream->read(12), 0xff);

    stream->write(0xff, 10);
    stream->seek(-10);
    stream->write(0xfe, 10);
    stream->seek(-10);
    ASSERT_EQ(stream->read(10), 0xfe);
}

TEST(BitioTest, seek_rw_test_3) {
    remove("bitio_test.dat");
    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, true, 1);

    stream->write(0xff, 12);
    stream->seek(-1);
    stream->seek(-11);
    ASSERT_EQ(stream->read(12), 0xff);

    stream->write(0xff, 10);
    stream->seek(-10);
    stream->write(0xfe, 10);
    stream->seek(-10);
    ASSERT_EQ(stream->read(10), 0xfe);
}

TEST(BitioTest, seek_rw_test_4) {
    remove("bitio_test.dat");
    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, true, 2);

    stream->write(0xff, 12);
    stream->seek(-1);
    stream->seek(-11);
    ASSERT_EQ(stream->read(12), 0xff);

    stream->write(0xff, 10);
    stream->seek(-10);
    stream->write(0xfe, 10);
    stream->seek(-10);
    ASSERT_EQ(stream->read(10), 0xfe);
}

TEST(BitioTest, seek_rw_test_5) {
    remove("bitio_test.dat");

    FILE *file = fopen("bitio_test.dat", "w+");
    auto stream = new bitio::stream(file, true, 2);

    stream->write(1234, 11);
    stream->seek(-3);
    stream->seek(-5);
    stream->read(1);
    stream->seek(-2);

    ASSERT_EQ(stream->read(11), 1234);

    stream->flush();
    stream->close();

    file = fopen("bitio_test.dat", "rb+");
    stream = new bitio::stream(file, false, 1);

    ASSERT_EQ(stream->read(11), 1234);
}





