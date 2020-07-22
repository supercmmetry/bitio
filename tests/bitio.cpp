#include <gtest/gtest.h>
#include <bitio.h>

class BitioTest: testing::Test {};

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

    auto stream = new bitio::stream(file, 2);

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

    auto stream = new bitio::stream(file, 32);

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

    auto stream = new bitio::stream(file, 2);

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


